#!/usr/bin/env python

from socket import *
from struct import *
from time import sleep
import random

client_socket = socket(AF_INET, SOCK_DGRAM)

def send(dst, state):
	hdr = 'BPSW'
	body = ''
	for led in state:
		body += pack('BBB', *led)
	client_socket.sendto(hdr + body, dst)

dst = ('192.168.15.109', 2357)

colors = [
		(1, 0, 0),
		(0, 1, 0),
		(0, 0, 1),
		(1, 1, 0),
		(0, 1, 1),
		(1, 0, 1),
		(1, 1, 1),
		(0, 0, 0),
]
	
while True:
	for c in colors:
		send(dst, [c] * 64)
		sleep(0.2)
	for x in range(10):
		send(dst, [random.choice(colors[:3]) for x in range(64)])
		sleep(1)
	




