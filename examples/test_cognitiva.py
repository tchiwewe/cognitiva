#!/usr/bin/env python
##################################################
# Gnuradio Python Flow Graph
# Title: Test Cognitiva
# Generated: Mon Dec 29 22:40:14 2014
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
import pmt
import wx

class test_cognitiva(grc_wxgui.top_block_gui):

    def __init__(self):
        grc_wxgui.top_block_gui.__init__(self, title="Test Cognitiva")

        ##################################################
        # Variables
        ##################################################
        self.samp_rate = samp_rate = 4e6
        self.phy_ver = phy_ver = 2

        ##################################################
        # Blocks
        ##################################################
        self.foo_packet_pad_1 = foo.packet_pad(False, False, 0.001, 2000, 2000)
        self.foo_packet_pad_0 = foo.packet_pad(False, False, 0.001, 2000, 2000)
        self.cognitiva_phy_1 = cognitiva_phy(
            phy_ver=phy_ver,
            samp_rate=samp_rate,
            parameter_fft_size=1024,
            debug_mask=0,
            parameter_dwell_delay=0.001,
            parameter_tune_delay=0.1,
        )
        self.cognitiva_phy_0 = cognitiva_phy(
            phy_ver=phy_ver,
            samp_rate=samp_rate,
            parameter_fft_size=1024,
            debug_mask=0,
            parameter_dwell_delay=0.001,
            parameter_tune_delay=0.1,
        )
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
          
        self.cognitiva_cognitiva_mac_0 = cognitiva.cognitiva_mac(
          "::1020", 
          2,
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
          1)
          
        self.blocks_throttle_1 = blocks.throttle(gr.sizeof_gr_complex*1, samp_rate,True)
        self.blocks_throttle_0 = blocks.throttle(gr.sizeof_gr_complex*1, samp_rate,True)
        self.blocks_socket_pdu_1 = blocks.socket_pdu("UDP_SERVER", "localhost", "52002", 10000, False)
        self.blocks_socket_pdu_0 = blocks.socket_pdu("UDP_SERVER", "localhost", "52001", 10000, False)
        self.blocks_random_pdu_0_0 = blocks.random_pdu(1400, 1400, chr(0xFF), 2)
        self.blocks_random_pdu_0 = blocks.random_pdu(1024, 1024, chr(0xFF), 2)
        self.blocks_message_strobe_0_0_1 = blocks.message_strobe(pmt.intern("TEST"), 500)
        self.blocks_message_strobe_0_0 = blocks.message_strobe(pmt.intern("TEST"), 1000)

        ##################################################
        # Connections
        ##################################################
        self.connect((self.foo_packet_pad_0, 0), (self.blocks_throttle_0, 0))
        self.connect((self.cognitiva_phy_0, 0), (self.foo_packet_pad_0, 0))
        self.connect((self.cognitiva_phy_1, 0), (self.foo_packet_pad_1, 0))
        self.connect((self.foo_packet_pad_1, 0), (self.blocks_throttle_1, 0))
        self.connect((self.blocks_throttle_1, 0), (self.cognitiva_phy_0, 0))
        self.connect((self.blocks_throttle_0, 0), (self.cognitiva_phy_1, 0))

        ##################################################
        # Asynch Message Connections
        ##################################################
        self.msg_connect(self.blocks_random_pdu_0_0, "pdus", self.cognitiva_cognitiva_mac_1, "payload_in")
        self.msg_connect(self.blocks_message_strobe_0_0_1, "strobe", self.blocks_random_pdu_0_0, "generate")
        self.msg_connect(self.cognitiva_cognitiva_mac_0, "payload_out", self.blocks_socket_pdu_0, "pdus")
        self.msg_connect(self.cognitiva_cognitiva_mac_1, "payload_out", self.blocks_socket_pdu_1, "pdus")
        self.msg_connect(self.blocks_socket_pdu_1, "pdus", self.cognitiva_cognitiva_mac_1, "payload_in")
        self.msg_connect(self.blocks_socket_pdu_0, "pdus", self.cognitiva_cognitiva_mac_0, "payload_in")
        self.msg_connect(self.blocks_message_strobe_0_0, "strobe", self.blocks_random_pdu_0, "generate")
        self.msg_connect(self.blocks_random_pdu_0, "pdus", self.cognitiva_cognitiva_mac_0, "payload_in")
        self.msg_connect(self.cognitiva_cognitiva_mac_0, "mpdu_out", self.cognitiva_phy_0, "psdu_in")
        self.msg_connect(self.cognitiva_phy_0, "psdu_out", self.cognitiva_cognitiva_mac_0, "mpdu_in")
        self.msg_connect(self.cognitiva_phy_1, "psdu_out", self.cognitiva_cognitiva_mac_1, "mpdu_in")
        self.msg_connect(self.cognitiva_cognitiva_mac_1, "mpdu_out", self.cognitiva_phy_1, "psdu_in")
        self.msg_connect(self.cognitiva_cognitiva_mac_0, "control_out", self.cognitiva_phy_0, "control_in")
        self.msg_connect(self.cognitiva_phy_0, "control_out", self.cognitiva_cognitiva_mac_0, "control_in")


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
    tb = test_cognitiva()
    tb.Start(True)
    tb.Wait()
