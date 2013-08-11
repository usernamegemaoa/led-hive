#!/usr/bin/env python
from gpio import GPIO
import random
from time import time, sleep

class Leds:
	rows = 8
	cols = 8


	def __init__(self):
		self.date = [
			GPIO(27, 'out'),
			GPIO(18, 'out'),
			GPIO(17, 'out'),
			GPIO(15, 'out'),
			GPIO(14, 'out'),
			GPIO(4, 'out'),
			GPIO(3, 'out'),
			GPIO(2, 'out'),
			]
		self.strobeColors = {
			'r': GPIO(23, 'out'),
			'g': GPIO(24, 'out'),
			'b': GPIO(22, 'out')
		}

		self.strobeRow = GPIO(10, 'out')
		
		self.state = [(0, 0, 0)] * self.rows * self.cols
	
	
	def set(self, x, y, r, g, b):
		self.state[x * self.rows + y] = (r, g, b)
		
	
	def _write(self):
		for row in range(self.rows):
			for col in range(self.cols):
				for bit_col in range(self.cols):
					self.date[bit_col].set("0" if bit_col == col and self.state[row * self.rows + col][0] else "1")
				self.strobeColors['r'].strobe(0)
				
				for bit_col in range(self.cols):
					self.date[bit_col].set("0" if bit_col == col and self.state[row * self.rows + col][1] else "1")
				self.strobeColors['g'].strobe(0)
										
				for bit_col in range(self.cols):
					self.date[bit_col].set("0" if bit_col == col and self.state[row * self.rows + col][2] else "1")
				self.strobeColors['b'].strobe(0)
										
				
				for bit_row in range(self.rows):
					self.date[bit_row].set("1" if bit_row == row else "0")
				self.strobeRow.strobe(0)
				#sleep(0.01)
				
	
	def random(self):
		for pin in self.date:
			pin.set(random.choice(['1', '0']))
		
		color = random.choice(['r', 'g', 'b'])
		self.strobeColors[color].strobe(0)
		
		for pin in self.date:
			pin.set(random.choice(['1', '0']))
		self.strobeRow.strobe(0)
		
		
		
if __name__ == '__main__':
	bla = Leds()
	while True:
		bla.state = [(1, 0, 0)] * 64
		for x in range(10):
			bla._write()
		
		bla.state = [(0, 1, 0)] * 64
		for x in range(10):
			bla._write()
		
		
	bla.state = [(0, 1, 1)] * 64
	for x in range(1000):
		bla._write()
