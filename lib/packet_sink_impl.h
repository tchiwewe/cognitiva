/* -*- c++ -*- */
/* 
 * Copyright 2014 Tapiwa M. Chiwewe <tapiwa.chiwewe@ieee.org>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_COGNITIVA_PACKET_SINK_IMPL_H
#define INCLUDED_COGNITIVA_PACKET_SINK_IMPL_H

#include <cognitiva/packet_sink.h>

#include <stdio.h>

#define MAX_PACKET_SIZE 2048

namespace gr {
namespace cognitiva {

class packet_sink_impl: public packet_sink {
private:
	void enter_search();
	void enter_have_sync();
	void enter_have_header(int payload_len);
	unsigned char decode_chips(unsigned int chips);
	enum {
		STATE_SYNC_SEARCH, STATE_HAVE_SYNC, STATE_HAVE_HEADER
	} d_state;
	//**
	static const int MAX_PKT_LEN = MAX_PACKET_SIZE - 10; // remove header
	static const int MAX_LQI_SAMPLES = 8; // Number of chip correlation samples to take

	unsigned int d_sync_vector;    // Cognitiva standard is 6x 0 bytes and 1x 0xF3A0
	unsigned int d_threshold;      // how many bits may be wrong in sync vector
	unsigned int d_shift_reg;      // used to look for sync_vector
	int d_preamble_cnt;          // count on where we are in preamble
	int d_chip_cnt;              // counts the chips collected
	unsigned int d_header;                // header bits
	int d_headerbitlen_cnt;      // how many so far
	unsigned char d_packet[MAX_PKT_LEN];   // assembled payload
	unsigned int d_packet_byte;           // byte being assembled
	int d_packet_byte_index;     // which bit of d_packet_byte we're working on
	int d_packetlen;             // length of packet
	int d_packetlen_cnt;         // how many so far
	int d_payload_cnt;           // how many bytes in payload
	unsigned int d_lqi;                   // Link Quality Information
	unsigned int d_lqi_sample_count;
	unsigned int d_phy_version;
	unsigned int d_debug_level;
	char buf[MAX_PACKET_SIZE];
	unsigned int PN_SEQUENCE[16];
	unsigned int mask1;
	unsigned int mask2;

public:
	packet_sink_impl(int threshold, int phy_version, unsigned int debug_level);
	~packet_sink_impl();

	// Where all the action really happens
	int general_work(int noutput_items, gr_vector_int &ninput_items,
			gr_vector_const_void_star &input_items,
			gr_vector_void_star &output_items);
};

} // namespace cognitiva
} // namespace gr

#endif /* INCLUDED_COGNITIVA_PACKET_SINK_IMPL_H */

