//	LHC = LIGHT HIVE CONTROL (by Boris & Wiedi)
//  August '13
//  based on "How to access GPIO registers from C-code on the Raspberry-Pi Example program" by Dom and Gert

#define _XOPEN_SOURCE 600

// Access from ARM Running Linux

#define BCM2708_PERI_BASE        0x20000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)

int  mem_fd;
void *gpio_map;

// I/O access
volatile unsigned *gpio;


// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0

#define ROWS 8
#define COLS 8

char header[] = {'B', 'P', 'S', 'W' };
char state[ROWS * COLS * 3];

char bus[] = {27, 18, 17, 15, 14, 4, 3, 2};
char strobes[] = {23, 24, 22, 10}; /* r, g, b, rows */

//
// Set up a memory regions to access GPIO
//
void setup_io() {
   /* open /dev/mem */
   if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
      printf("can't open /dev/mem \n");
      exit(-1);
   }

   /* mmap GPIO */
   gpio_map = mmap(
      NULL,             //Any adddress in our space will do
      BLOCK_SIZE,       //Map length
      PROT_READ|PROT_WRITE, // Enable reading & writting to mapped memory
      MAP_SHARED,       //Shared with other processes
      mem_fd,           //File to map
      GPIO_BASE         //Offset to GPIO peripheral
   );

   close(mem_fd); //No need to keep mem_fd open after mmap

   if (gpio_map == MAP_FAILED) {
      printf("mmap error %d\n", (int)gpio_map);//errno also set!
      exit(-1);
   }

   // Always use volatile pointer!
   gpio = (volatile unsigned *)gpio_map;


}


void init_io() {
	setup_io();

	/* init bus */
	for(int i = 0; i < sizeof(bus); i++) {
		INP_GPIO(bus[i]);
		OUT_GPIO(bus[i]);
	}
	
	/* init strobes */
	for(int i = 0; i < sizeof(strobes); i++) {
		INP_GPIO(strobes[i]);
		OUT_GPIO(strobes[i]);
	}
}


void strobe(char pin) {
		GPIO_SET = 1 << pin;
		GPIO_CLR = 1 << pin;
}


void refresh() {
	for(int row = 0; row < ROWS; row++) {
		for(int col = 0; col < COLS; col++) {
			for(int color = 0; color < 3; color++) {
				for(int bit_col = 0; bit_col < COLS; bit_col++) {
					if(bit_col == col && state[row * 3 * ROWS + bit_col * 3 + color] > 0) {
						GPIO_CLR = 1 << bus[bit_col];
					} else {
						GPIO_SET = 1 << bus[bit_col];
					}
				}
				strobe(strobes[color]);	
				//usleep(5000);		
			}
			for(int bit_row = 0; bit_row < ROWS; bit_row++) {
				if(bit_row == row) {
					GPIO_SET = 1 << bus[bit_row];
				} else {
					GPIO_CLR = 1 << bus[bit_row];
				}
			}
			strobe(strobes[3]);
			//usleep(50000);
		}
	}
	
}


int setNonblocking(int fd) {
	/* from http://www.kegel.com/dkftpbench/nonblocking.html */
	int flags;

	/* If they have O_NONBLOCK, use the Posix way to do it */
#if defined(O_NONBLOCK)
	/* Fixme: O_NONBLOCK is defined but broken on SunOS 4.1.x and AIX 3.2.5. */
	if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
		flags = 0;
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
	/* Otherwise, use the old way of doing it */
	flags = 1;
	return ioctl(fd, FIOBIO, &flags);
#endif
}


int create_socket(char *port) {
	int sockfd;
	int rv;
	struct addrinfo hints, *servinfo, *p;
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	
	if((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
		printf("getaddrinfo: %s\n", gai_strerror(rv));
		exit(-1);
	}
	
	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}

		if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("listener: bind");
			continue;
		}
		break;
	}

	if(p == NULL) {
		printf("listener: failed to bind socket\n");
		exit(-1);
	}

	freeaddrinfo(servinfo);	
	setNonblocking(sockfd);
	return sockfd;
}


int main(int argc, char **argv) {
	init_io();
		
	int sockfd = create_socket("2357");
	int len;
	char buf[sizeof(header) + sizeof(state)];

	/* red */
	for(int i = 0; i < ROWS * COLS * 3; i++) {
		state[i] = 1;
		state[++i] = 0;
		state[++i] = 0;
	}

	while(1) {
		refresh();
		
		if((len = recvfrom(sockfd, buf, sizeof(header) + sizeof(state), 0, NULL, 0)) != -1) {
			if(len < sizeof(header) + sizeof(state)) {
				printf("invalid size %i != %i\n", len, sizeof(header) + sizeof(state));
				continue;
			}
				
			if(strncmp(buf, header, sizeof(header) != 0)) {
				printf("invalid header\n");
				continue;
			}
				
			memcpy(state, buf + sizeof(header), sizeof(state));
		}
	}

  return 0;
}

