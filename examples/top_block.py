#!/usr/bin/env python
##################################################
# Gnuradio Python Flow Graph
# Title: Top Block
# Generated: Mon Sep 22 22:46:07 2014
##################################################

execfile("/home/tchiwewe/.grc_gnuradio/cognitiva_phy.py")
from gnuradio import blocks
from gnuradio import eng_notation
from gnuradio import gr
from gnuradio import uhd
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from grc_gnuradio import wxgui as grc_wxgui
from optparse import OptionParser
import cognitiva
import pmt
import time
import wx

class top_block(grc_wxgui.top_block_gui):

    def __init__(self, address="type=b100", param_freq=800e6):
        grc_wxgui.top_block_gui.__init__(self, title="Top Block")

        ##################################################
        # Parameters
        ##################################################
        self.address = address
        self.param_freq = param_freq

        ##################################################
        # Variables
        ##################################################
        self.samp_rate = samp_rate = 4e6

        ##################################################
        # Blocks
        ##################################################
        self.uhd_usrp_source_0 = uhd.usrp_source(
        	",".join((address, "")),
        	uhd.stream_args(
        		cpu_format="fc32",
        		channels=range(1),
        	),
        )
        self.uhd_usrp_source_0.set_samp_rate(samp_rate)
        self.uhd_usrp_source_0.set_center_freq(param_freq, 0)
        self.uhd_usrp_source_0.set_gain(10, 0)
        self.uhd_usrp_source_0.set_antenna("TX/RX", 0)
        self.uhd_usrp_sink_0 = uhd.usrp_sink(
        	",".join((address, "")),
        	uhd.stream_args(
        		cpu_format="fc32",
        		channels=range(1),
        	),
        )
        self.uhd_usrp_sink_0.set_samp_rate(samp_rate)
        self.uhd_usrp_sink_0.set_center_freq(param_freq, 0)
        self.uhd_usrp_sink_0.set_gain(0, 0)
        self.uhd_usrp_sink_0.set_antenna("TX/RX", 0)
        self.cognitiva_phy_0 = cognitiva_phy(
            debug_mask=0,
            phy_ver=2,
        )
        self.cognitiva_cognitiva_mac_0 = cognitiva.cognitiva_mac(
          "::3040", 
          True, 
          0, 
          10, 
          0.1, 
          2, 
          True, 
          0.05, 
          10.0, 
          True, 
          True, 
          1)
          
        self.blocks_random_pdu_0 = blocks.random_pdu(250, 250, chr(0xFF), 2)
        self.blocks_message_strobe_0_0 = blocks.message_strobe(pmt.intern("TEST"), 1000)

        ##################################################
        # Connections
        ##################################################
        self.connect((self.uhd_usrp_source_0, 0), (self.cognitiva_phy_0, 0))
        self.connect((self.cognitiva_phy_0, 0), (self.uhd_usrp_sink_0, 0))

        ##################################################
        # Asynch Message Connections
        ##################################################
        self.msg_connect(self.blocks_message_strobe_0_0, "strobe", self.blocks_random_pdu_0, "generate")
        self.msg_connect(self.blocks_random_pdu_0, "pdus", self.cognitiva_cognitiva_mac_0, "payload in")
        self.msg_connect(self.cognitiva_phy_0, "to_mac", self.cognitiva_cognitiva_mac_0, "mpdu in")
        self.msg_connect(self.cognitiva_cognitiva_mac_0, "mpdu out", self.cognitiva_phy_0, "from_mac")


    def get_address(self):
        return self.address

    def set_address(self, address):
        self.address = address

    def get_param_freq(self):
        return self.param_freq

    def set_param_freq(self, param_freq):
        self.param_freq = param_freq
        self.uhd_usrp_source_0.set_center_freq(self.param_freq, 0)
        self.uhd_usrp_sink_0.set_center_freq(self.param_freq, 0)

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.uhd_usrp_source_0.set_samp_rate(self.samp_rate)
        self.uhd_usrp_sink_0.set_samp_rate(self.samp_rate)

if __name__ == '__main__':
    import ctypes
    import sys
    if sys.platform.startswith('linux'):
        try:
            x11 = ctypes.cdll.LoadLibrary('libX11.so')
            x11.XInitThreads()
        except:
            print "Warning: failed to XInitThreads()"
    parser = OptionParser(option_class=eng_option, usage="%prog: [options]")
    parser.add_option("-a", "--address", dest="address", type="string", default="type=b100",
        help="Set Device Address [default=%default]")
    parser.add_option("-f", "--param-freq", dest="param_freq", type="eng_float", default=eng_notation.num_to_str(800e6),
        help="Set Default Frequency [default=%default]")
    (options, args) = parser.parse_args()
    tb = top_block(address=options.address, param_freq=options.param_freq)
    tb.Start(True)
    tb.Wait()
