#!/usr/bin/env python
##################################################
# Gnuradio Python Flow Graph
# Title: Test Cognitiva Streaming
# Generated: Wed Sep 24 21:16:59 2014
##################################################

execfile("/home/tchiwewe/.grc_gnuradio/cognitiva_phy.py")
from gnuradio import blocks
from gnuradio import eng_notation
from gnuradio import gr
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from grc_gnuradio import wxgui as grc_wxgui
from optparse import OptionParser
import cognitiva
import foo
import wx

class test_cognitiva_streaming(grc_wxgui.top_block_gui):

    def __init__(self):
        grc_wxgui.top_block_gui.__init__(self, title="Test Cognitiva Streaming")

        ##################################################
        # Variables
        ##################################################
        self.samp_rate = samp_rate = 8e6
        self.phy_ver = phy_ver = 0

        ##################################################
        # Blocks
        ##################################################
        self.foo_packet_pad_0_0_0 = foo.packet_pad(False, False, 0.001, 100, 100)
        self.foo_packet_pad_0_0 = foo.packet_pad(False, False, 0.001, 100, 100)
        self.cognitiva_phy_0_0 = cognitiva_phy(
            debug_mask=0,
            phy_ver=phy_ver,
        )
        self.cognitiva_phy_0 = cognitiva_phy(
            debug_mask=0,
            phy_ver=phy_ver,
        )
        self.cognitiva_cognitiva_mac_1 = cognitiva.cognitiva_mac(
          "::3040", 
          False, 
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
          
        self.cognitiva_cognitiva_mac_0 = cognitiva.cognitiva_mac(
          "::1020", 
          False, 
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
        self.blocks_tagged_stream_to_pdu_0 = blocks.tagged_stream_to_pdu(blocks.byte_t, "packet_len_stream")
        self.blocks_stream_to_tagged_stream_0 = blocks.stream_to_tagged_stream(gr.sizeof_char, 1, 1024, "packet_len_stream")
        self.blocks_pdu_to_tagged_stream_0 = blocks.pdu_to_tagged_stream(blocks.byte_t, "packet_len")
        self.blocks_file_source_0 = blocks.file_source(gr.sizeof_char*1, "../../../data/music_.mp3", True)
        self.blocks_file_sink_0 = blocks.file_sink(gr.sizeof_char*1, "/home/tchiwewe/Documents/music_fifo.mp3", False)
        self.blocks_file_sink_0.set_unbuffered(False)

        ##################################################
        # Connections
        ##################################################
        self.connect((self.foo_packet_pad_0_0, 0), (self.cognitiva_phy_0_0, 0))
        self.connect((self.cognitiva_phy_0, 0), (self.blocks_throttle_1, 0))
        self.connect((self.blocks_throttle_1, 0), (self.foo_packet_pad_0_0, 0))
        self.connect((self.cognitiva_phy_0_0, 0), (self.foo_packet_pad_0_0_0, 0))
        self.connect((self.blocks_throttle_0, 0), (self.cognitiva_phy_0, 0))
        self.connect((self.foo_packet_pad_0_0_0, 0), (self.blocks_throttle_0, 0))
        self.connect((self.blocks_file_source_0, 0), (self.blocks_stream_to_tagged_stream_0, 0))
        self.connect((self.blocks_stream_to_tagged_stream_0, 0), (self.blocks_tagged_stream_to_pdu_0, 0))
        self.connect((self.blocks_pdu_to_tagged_stream_0, 0), (self.blocks_file_sink_0, 0))

        ##################################################
        # Asynch Message Connections
        ##################################################
        self.msg_connect(self.cognitiva_cognitiva_mac_0, "mpdu out", self.cognitiva_phy_0, "from_mac")
        self.msg_connect(self.cognitiva_phy_0_0, "to_mac", self.cognitiva_cognitiva_mac_1, "mpdu in")
        self.msg_connect(self.cognitiva_phy_0, "to_mac", self.cognitiva_cognitiva_mac_0, "mpdu in")
        self.msg_connect(self.cognitiva_cognitiva_mac_1, "mpdu out", self.cognitiva_phy_0_0, "from_mac")
        self.msg_connect(self.blocks_tagged_stream_to_pdu_0, "pdus", self.cognitiva_cognitiva_mac_0, "payload in")
        self.msg_connect(self.cognitiva_cognitiva_mac_1, "payload out", self.blocks_pdu_to_tagged_stream_0, "pdus")


    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.blocks_throttle_0.set_sample_rate(self.samp_rate)
        self.blocks_throttle_1.set_sample_rate(self.samp_rate)

    def get_phy_ver(self):
        return self.phy_ver

    def set_phy_ver(self, phy_ver):
        self.phy_ver = phy_ver
        self.cognitiva_phy_0.set_phy_ver(self.phy_ver)
        self.cognitiva_phy_0_0.set_phy_ver(self.phy_ver)

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
    (options, args) = parser.parse_args()
    tb = test_cognitiva_streaming()
    tb.Start(True)
    tb.Wait()
