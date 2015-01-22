# Copyright 2011 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
install(FILES
    cognitiva_cognitiva_mac.xml
    cognitiva_cognitiva_phy_ppdu.xml
    cognitiva_bits_to_chips.xml
    cognitiva_packet_sink.xml
    cognitiva_burst_tagger.xml
    cognitiva_chips_to_bits.xml
    cognitiva_bits_to_chips_new.xml
    cognitiva_bin_statistics.xml
    cognitiva_spectrum_sense.xml DESTINATION share/gnuradio/grc/blocks
)
