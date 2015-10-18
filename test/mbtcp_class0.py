#!/usr/bin/env python

import unittest
from pymodbus.client.sync import ModbusTcpClient

SERVER_HOST = '10.0.0.25'

class TestMbTcpClass0(unittest.TestCase):
	read_values = range(0xAA50, 0xAA60)
	write_values = range(0xBB50, 0xBB60)
	
	def setUp(self):
		self.client = ModbusTcpClient(SERVER_HOST)
		self.client.connect()
		
	def tearDown(self):
		self.client.close()
		
	def test_read_holding_registers(self):
		rv = self.client.read_holding_registers(0, 16)
		self.assertEqual(rv.function_code, 0x03)
		self.assertEqual(rv.registers, self.read_values)
		
	def test_read_holding_registers_exception(self):
		rv = self.client.read_holding_registers(16, 1)
		self.assertEqual(rv.function_code, 0x83)
		self.assertEqual(rv.exception_code, 0x02)
	
	def test_write_holding_registers(self):
		rq = self.client.write_registers(0, self.write_values)
		self.assertEqual(rq.function_code, 0x10)
		rr = self.client.read_holding_registers(0, 16)
		self.assertEqual(rr.registers, self.write_values)
		rq = self.client.write_registers(0, self.read_values)
		self.assertEqual(rq.function_code, 0x10)
		
	def test_write_holding_registers_exception(self):
		rq = self.client.write_registers(16, [0x00])
		self.assertEqual(rq.function_code, 0x90)
		self.assertEqual(rq.exception_code, 0x02)

if __name__ == '__main__':
	unittest.main()
