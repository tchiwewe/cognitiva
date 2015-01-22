#!/usr/bin/env python
##################################################
# Gnuradio Python Flow Graph
# Title: Cognitiva Node A
# Generated: Wed Sep 24 21:17:51 2014
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
import time
import wx

class cognitiva_node_a(grc_wxgui.top_block_gui):

    def __init__(self, param_freq=800e6, address="name=b100a"):
        grc_wxgui.top_block_gui.__init__(self, title="Cognitiva Node A")

        ##################################################
        # Parameters
        ##################################################
        self.param_freq = param_freq
        self.address = address

        ##################################################
        # Variables
        ##################################################
        self.samp_rate = samp_rate = 4e6
        self.phy_ver = phy_ver = 0

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
            phy_ver=phy_ver,
        )
        self.cognitiva_cognitiva_mac_0 = cognitiva.cognitiva_mac(
          "::1020", 
          False, 
          0, 
          10, 
          0.1, 
          2, 
          True, 
          0.05, 
          10.0, 
          True, 
          False, 
          0)
          
        self.blocks_throttle_0 = blocks.throttle(gr.sizeof_char*1, 22050,True)
        self.blocks_tagged_stream_to_pdu_0 = blocks.tagged_stream_to_pdu(blocks.byte_t, "packet_len_stream")
        self.blocks_stream_to_tagged_stream_0 = blocks.stream_to_tagged_stream(gr.sizeof_char, 1, 1024, "packet_len_stream")
        self.blocks_file_source_0 = blocks.file_source(gr.sizeof_char*1, "../../../data/music_low_rate_.mp3", True)

        ##################################################
        # Connections
        ##################################################
        self.connect((self.uhd_usrp_source_0, 0), (self.cognitiva_phy_0, 0))
        self.connect((self.cognitiva_phy_0, 0), (self.uhd_usrp_sink_0, 0))
        self.connect((self.blocks_stream_to_tagged_stream_0, 0), (self.blocks_tagged_stream_to_pdu_0, 0))
        self.connect((self.blocks_file_source_0, 0), (self.blocks_throttle_0, 0))
        self.connect((self.blocks_throttle_0, 0), (self.blocks_stream_to_tagged_stream_0, 0))

        ##################################################
        # Asynch Message Connections
        ##################################################
        self.msg_connect(self.cognitiva_cognitiva_mac_0, "mpdu out", self.cognitiva_phy_0, "from_mac")
        self.msg_connect(self.cognitiva_phy_0, "to_mac", self.cognitiva_cognitiva_mac_0, "mpdu in")
        self.msg_connect(self.blocks_tagged_stream_to_pdu_0, "pdus", self.cognitiva_cognitiva_mac_0, "payload in")


    def get_param_freq(self):
        return self.param_freq

    def set_param_freq(self, param_freq):
        self.param_freq = param_freq
        self.uhd_usrp_source_0.set_center_freq(self.param_freq, 0)
        self.uhd_usrp_sink_0.set_center_freq(self.param_freq, 0)

    def get_address(self):
        return self.address

    def set_address(self, address):
        self.address = address

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.uhd_usrp_source_0.set_samp_rate(self.samp_rate)
        self.uhd_usrp_sink_0.set_samp_rate(self.samp_rate)

    def get_phy_ver(self):
        return self.phy_ver

    def set_phy_ver(self, phy_ver):
        self.phy_ver = phy_ver
        self.cognitiva_phy_0.set_phy_ver(self.phy_ver)

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
    parser.add_option("-f", "--param-freq", dest="param_freq", type="eng_float", default=eng_notation.num_to_str(800e6),
        help="Set Default Frequency [default=%default]")
    parser.add_option("-a", "--address", dest="address", type="string", default="name=b100a",
        help="Set Device Address [default=%default]")
    (options, args) = parser.parse_args()
    tb = cognitiva_node_a(param_freq=options.param_freq, address=options.address)
    tb.Start(True)
    tb.Wait()
