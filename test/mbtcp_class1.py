#!/usr/bin/env python

import unittest
from pymodbus.client.sync import ModbusTcpClient

SERVER_HOST = '10.0.0.25'

class TestMbTcpClass1(unittest.TestCase):
	read_values = range(0xAA50, 0xAA60)
	write_values = range(0xBB50, 0xBB60)
	
	def setUp(self):
		self.client = ModbusTcpClient(SERVER_HOST)
		self.client.connect()
		
	def tearDown(self):
		self.client.close()
		
	def test_write_single_holding_register(self):
		rq = self.client.write_register(8, 0xCC)
		self.assertEqual(rq.function_code, 0x06)
		rr = self.client.read_holding_registers(8, 1)
		self.assertEqual(rr.registers[0], 0xCC)
		rq = self.client.write_register(16, 0x00)
		self.assertEqual(rq.function_code, 0x86)
		rq = self.client.write_register(8, 0xAA58)
		
	def test_write_coil(self):
		rq = self.client.write_coil(0, True)
		self.assertEqual(rq.function_code, 0x05)
		
		rq = self.client.write_coil(0, False)
		self.assertEqual(rq.function_code, 0x05)
		
		rq = self.client.write_coil(256, False)
		self.assertEqual(rq.function_code, 0x85)
		
	def test_read_coil(self):
		coil_read_values = [True, False, True, False, False, True, False, False]
		

if __name__ == '__main__':
	unittest.main()
