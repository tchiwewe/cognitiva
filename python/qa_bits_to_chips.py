#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Copyright 2014 <+YOU OR YOUR COMPANY+>.
# 
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
# 
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this software; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
# 

from gnuradio import gr, gr_unittest
from gnuradio import blocks
import cognitiva_swig as cognitiva

class qa_bits_to_chips (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_bits_to_chips (self):
        # set up fg
        # 31 = 00011111, nibbles: 0001 => 1 and 1111 => 15 
        #15: 11001001 01100000 01110111 10111000 => 201 96 119 184
        #1: 11101101 10011100 00110101 00100010 => 237 156 53 34
        print '****'
        src_data = (31, 31)
        expected_result = (184, 119, 96, 201, 34, 53, 156, 237, 184, 119, 96, 201, 34, 53, 156, 237)
        src = blocks.vector_source_b(src_data)
        sqr = cognitiva.bits_to_chips()
        dst = blocks.vector_sink_b()
        self.tb.connect(src, sqr)
        self.tb.connect(sqr, dst)
        
        self.tb.run ()
        
        # check data
        result_data = dst.data()        
        self.assertEqual(expected_result, result_data)
    
    def test_002_bits_to_chips(self):
	pn_sequence=(653456430, 3986437410, 786023250,	585997365, 1378802115, 891481500, 3276943065, 2620728045, 2358642555,	3100205175, 2072811015, 2008598880, 125537430, 1618458825, 2517072780,3378542520)

if __name__ == '__main__':
    gr_unittest.run(qa_bits_to_chips, "qa_bits_to_chips.xml")
