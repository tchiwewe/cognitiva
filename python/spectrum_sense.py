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

from gnuradio import analog
from gnuradio import blocks
from gnuradio import digital
from gnuradio import fft
from gnuradio import filter
from gnuradio import gr
from gnuradio.fft import window
from gnuradio.filter import firdes
#from optparse import OptionParser
import math
import numpy as np

class spectrum_sense(gr.hier_block2):
    """
    docstring for block spectrum_sense
    """
    def __init__(self, fft_size, samp_rate):
        gr.hier_block2.__init__(self,
            "spectrum_sense",
            gr.io_signature(1, 1, gr.sizeof_gr_complex),  # Input signature
            gr.io_signature(1, 1, gr.sizeof_float * fft_size)) # Output signature
            
	self.fft_size = fft_size
	self.samp_rate = samp_rate
	
	##################################################
	# Blocks
	##################################################  
    
	# stream to vector for fft
	s2v = blocks.stream_to_vector(gr.sizeof_gr_complex, self.fft_size)

	#self.one_in_n = gr.keep_one_in_n(gr.sizeof_float * self.fft_size,
        #                                 max(1, int(self.sample_rate/self.fft_size/self.fft_rate)))
	
	# filter window
	mywindow = filter.window.blackmanharris(self.fft_size)
	
	# fft
	ffter = fft.fft_vcc(self.fft_size, True, mywindow, True)
	
	# complex to magnitude block
	c2mag = blocks.complex_to_mag_squared(self.fft_size) # use sqrt of this for power

	# average
	avg = filter.single_pole_iir_filter_ff(0.4, self.fft_size)
	
	multiply_const = blocks.multiply_const_vff((np.ones(self.fft_size)*(1.0/self.samp_rate)))

	nlog10 = blocks.nlog10_ff(10, self.fft_size, 0)
	
	# FIXME  We need to add 3dB to all bins but the DC bin
        #self.log = gr.nlog10_ff(20, self.fft_size,
        #                       -20*math.log10(self.fft_size)                # Adjust for number of bins
        #                       -10*math.log10(power/self.fft_size)        # Adjust for windowing loss
        #                       -20*math.log10(ref_scale/2))                # Adjust for reference scale
	
	#v2s = blocks.vector_to_stream(gr.sizeof_float*1, self.fft_size)
	
	# connect blocks
	self.connect(self, s2v, ffter, c2mag, avg, multiply_const, nlog10, self)
