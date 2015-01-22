#!/usr/bin/env python
##################################################
# Gnuradio Python Flow Graph
# Title: Test Cognitiva With Hardware
# Generated: Thu Nov  6 16:12:10 2014
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

class test_cognitiva_with_hardware(grc_wxgui.top_block_gui):

    def __init__(self, address="name=b100a", param_freq=2400e6):
        grc_wxgui.top_block_gui.__init__(self, title="Test Cognitiva With Hardware")

        ##################################################
        # Parameters
        ##################################################
        self.address = address
        self.param_freq = param_freq

        ##################################################
        # Variables
        ##################################################
        self.samp_rate = samp_rate = 4e6
        self.phy_ver = phy_ver = 2

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
            phy_ver=phy_ver,
            samp_rate=samp_rate,
            parameter_fft_size=1024,
            debug_mask=0,
            parameter_dwell_delay=0.001,
            parameter_tune_delay=0.1,
        )
        self.cognitiva_cognitiva_mac_0 = cognitiva.cognitiva_mac(
          "::1020", 
          3,
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
          
        self.blocks_socket_pdu_0 = blocks.socket_pdu("UDP_SERVER", "localhost", "52001", 10000, False)

        ##################################################
        # Connections
        ##################################################
        self.connect((self.cognitiva_phy_0, 0), (self.uhd_usrp_sink_0, 0))
        self.connect((self.uhd_usrp_source_0, 0), (self.cognitiva_phy_0, 0))

        ##################################################
        # Asynch Message Connections
        ##################################################
        self.msg_connect(self.cognitiva_cognitiva_mac_0, "payload_out", self.blocks_socket_pdu_0, "pdus")
        self.msg_connect(self.blocks_socket_pdu_0, "pdus", self.cognitiva_cognitiva_mac_0, "payload_in")
        self.msg_connect(self.cognitiva_phy_0, "control_out", self.cognitiva_cognitiva_mac_0, "control_in")
        self.msg_connect(self.cognitiva_cognitiva_mac_0, "control_out", self.cognitiva_phy_0, "control_in")
        self.msg_connect(self.cognitiva_cognitiva_mac_0, "mpdu_out", self.cognitiva_phy_0, "psdu_in")
        self.msg_connect(self.cognitiva_phy_0, "psdu_out", self.cognitiva_cognitiva_mac_0, "mpdu_in")


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
        self.cognitiva_phy_0.set_samp_rate(self.samp_rate)
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
    parser.add_option("-a", "--address", dest="address", type="string", default="name=b100a",
        help="Set Device Address [default=%default]")
    parser.add_option("-f", "--param-freq", dest="param_freq", type="eng_float", default=eng_notation.num_to_str(2400e6),
        help="Set Default Frequency [default=%default]")
    (options, args) = parser.parse_args()
    tb = test_cognitiva_with_hardware(address=options.address, param_freq=options.param_freq)
    tb.Start(True)
    tb.Wait()
