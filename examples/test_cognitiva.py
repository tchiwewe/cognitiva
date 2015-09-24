#!/usr/bin/env python2
##################################################
# GNU Radio Python Flow Graph
# Title: Test Cognitiva
# Generated: Thu Sep 24 12:21:33 2015
##################################################

if __name__ == '__main__':
    import ctypes
    import sys
    if sys.platform.startswith('linux'):
        try:
            x11 = ctypes.cdll.LoadLibrary('libX11.so')
            x11.XInitThreads()
        except:
            print "Warning: failed to XInitThreads()"

import os
import sys
sys.path.append(os.environ.get('GRC_HIER_PATH', os.path.expanduser('~/.grc_gnuradio')))

from cognitiva_phy import cognitiva_phy
from gnuradio import analog
from gnuradio import blocks
from gnuradio import eng_notation
from gnuradio import gr
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from gnuradio.wxgui import forms
from grc_gnuradio import wxgui as grc_wxgui
from optparse import OptionParser
import cognitiva
import foo
import pmt
import wx

class test_cognitiva(grc_wxgui.top_block_gui):

    def __init__(self):
        grc_wxgui.top_block_gui.__init__(self, title="Test Cognitiva")

        ##################################################
        # Variables
        ##################################################
        self.variable_slider_0 = variable_slider_0 = 0
        self.samp_rate = samp_rate = 4e6
        self.phy_ver = phy_ver = 0

        ##################################################
        # Blocks
        ##################################################
        _variable_slider_0_sizer = wx.BoxSizer(wx.VERTICAL)
        self._variable_slider_0_text_box = forms.text_box(
        	parent=self.GetWin(),
        	sizer=_variable_slider_0_sizer,
        	value=self.variable_slider_0,
        	callback=self.set_variable_slider_0,
        	label='variable_slider_0',
        	converter=forms.float_converter(),
        	proportion=0,
        )
        self._variable_slider_0_slider = forms.slider(
        	parent=self.GetWin(),
        	sizer=_variable_slider_0_sizer,
        	value=self.variable_slider_0,
        	callback=self.set_variable_slider_0,
        	minimum=0,
        	maximum=1000,
        	num_steps=100,
        	style=wx.SL_HORIZONTAL,
        	cast=float,
        	proportion=1,
        )
        self.Add(_variable_slider_0_sizer)
        self.foo_packet_pad_1 = foo.packet_pad(False, False, 0.001, 2000, 2000)
        self.foo_packet_pad_0 = foo.packet_pad(False, False, 0.001, 2000, 2000)
        self.cognitiva_phy_1 = cognitiva_phy(
            debug_mask=0,
            parameter_dwell_delay=0.001,
            parameter_fft_size=1024,
            parameter_tune_delay=0.1,
            phy_ver=phy_ver,
            samp_rate=samp_rate,
        )
        self.cognitiva_phy_0 = cognitiva_phy(
            debug_mask=0,
            parameter_dwell_delay=0.001,
            parameter_fft_size=1024,
            parameter_tune_delay=0.1,
            phy_ver=phy_ver,
            samp_rate=samp_rate,
        )
        self.cognitiva_cognitiva_mac_2 = cognitiva.cognitiva_mac(
          "::1020", 
          0,
          True, 
          0, 
          0, 
          0.1, 
          2.0, 
          True, 
          0.05, 
          10.0, 
          True, 
          False, 
          0)
          
        self.cognitiva_cognitiva_mac_1 = cognitiva.cognitiva_mac(
          "::3040", 
          0,
          True, 
          0, 
          10, 
          0.1, 
          2.0, 
          True, 
          0.05, 
          10.0, 
          True, 
          True, 
          0)
          
        self.blocks_throttle_1 = blocks.throttle(gr.sizeof_gr_complex*1, samp_rate,True)
        self.blocks_throttle_0 = blocks.throttle(gr.sizeof_gr_complex*1, samp_rate,True)
        self.blocks_socket_pdu_1 = blocks.socket_pdu("UDP_SERVER", "localhost", "52002", 10000, False)
        self.blocks_socket_pdu_0 = blocks.socket_pdu("UDP_SERVER", "localhost", "52001", 10000, False)
        self.blocks_random_pdu_0 = blocks.random_pdu(1024, 1024, chr(0xFF), 2)
        self.blocks_message_strobe_0_0 = blocks.message_strobe(pmt.intern("TEST"), 1000)
        self.blocks_add_xx_0 = blocks.add_vcc(1)
        self.analog_noise_source_x_0 = analog.noise_source_c(analog.GR_GAUSSIAN, variable_slider_0/1000, 0)

        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.blocks_message_strobe_0_0, 'strobe'), (self.blocks_random_pdu_0, 'generate'))    
        self.msg_connect((self.blocks_random_pdu_0, 'pdus'), (self.cognitiva_cognitiva_mac_2, 'payload_in'))    
        self.msg_connect((self.blocks_socket_pdu_0, 'pdus'), (self.cognitiva_cognitiva_mac_2, 'payload_in'))    
        self.msg_connect((self.blocks_socket_pdu_1, 'pdus'), (self.cognitiva_cognitiva_mac_1, 'payload_in'))    
        self.msg_connect((self.cognitiva_cognitiva_mac_1, 'payload_out'), (self.blocks_socket_pdu_1, 'pdus'))    
        self.msg_connect((self.cognitiva_cognitiva_mac_1, 'control_out'), (self.cognitiva_phy_1, 'control_in'))    
        self.msg_connect((self.cognitiva_cognitiva_mac_1, 'mpdu_out'), (self.cognitiva_phy_1, 'psdu_in'))    
        self.msg_connect((self.cognitiva_cognitiva_mac_2, 'payload_out'), (self.blocks_socket_pdu_0, 'pdus'))    
        self.msg_connect((self.cognitiva_cognitiva_mac_2, 'control_out'), (self.cognitiva_phy_0, 'control_in'))    
        self.msg_connect((self.cognitiva_cognitiva_mac_2, 'mpdu_out'), (self.cognitiva_phy_0, 'psdu_in'))    
        self.msg_connect((self.cognitiva_phy_0, 'control_out'), (self.cognitiva_cognitiva_mac_2, 'control_in'))    
        self.msg_connect((self.cognitiva_phy_0, 'psdu_out'), (self.cognitiva_cognitiva_mac_2, 'mpdu_in'))    
        self.msg_connect((self.cognitiva_phy_1, 'control_out'), (self.cognitiva_cognitiva_mac_1, 'control_in'))    
        self.msg_connect((self.cognitiva_phy_1, 'psdu_out'), (self.cognitiva_cognitiva_mac_1, 'mpdu_in'))    
        self.connect((self.analog_noise_source_x_0, 0), (self.blocks_add_xx_0, 1))    
        self.connect((self.blocks_throttle_0, 0), (self.blocks_add_xx_0, 0))    
        self.connect((self.cognitiva_phy_0, 0), (self.foo_packet_pad_0, 0))    
        self.connect((self.cognitiva_phy_1, 0), (self.foo_packet_pad_1, 0))    
        self.connect((self.foo_packet_pad_0, 0), (self.blocks_throttle_0, 0))    
        self.connect((self.foo_packet_pad_1, 0), (self.blocks_throttle_1, 0))    
        self.connect((self.blocks_throttle_1, 0), (self.cognitiva_phy_0, 0))    
        self.connect((self.blocks_add_xx_0, 0), (self.cognitiva_phy_1, 0))    


    def get_variable_slider_0(self):
        return self.variable_slider_0

    def set_variable_slider_0(self, variable_slider_0):
        self.variable_slider_0 = variable_slider_0
        self._variable_slider_0_slider.set_value(self.variable_slider_0)
        self._variable_slider_0_text_box.set_value(self.variable_slider_0)
        self.analog_noise_source_x_0.set_amplitude(self.variable_slider_0/1000)

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.blocks_throttle_0.set_sample_rate(self.samp_rate)
        self.blocks_throttle_1.set_sample_rate(self.samp_rate)
        self.cognitiva_phy_0.set_samp_rate(self.samp_rate)
        self.cognitiva_phy_1.set_samp_rate(self.samp_rate)

    def get_phy_ver(self):
        return self.phy_ver

    def set_phy_ver(self, phy_ver):
        self.phy_ver = phy_ver
        self.cognitiva_phy_0.set_phy_ver(self.phy_ver)
        self.cognitiva_phy_1.set_phy_ver(self.phy_ver)


if __name__ == '__main__':
    parser = OptionParser(option_class=eng_option, usage="%prog: [options]")
    (options, args) = parser.parse_args()
    tb = test_cognitiva()
    tb.Start(True)
    tb.Wait()
