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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include <gnuradio/blocks/count_bits.h>
#include "packet_sink_impl.h"
#include "pn_sequence.h"
#include <bitset>

// very verbose output for almost each sample
// d_debug_level 1

// less verbose output for higher level debugging
//d_debug_level 2

namespace gr {
namespace cognitiva {

packet_sink::sptr packet_sink::make(unsigned int threshold, unsigned int phy_version, unsigned int debug_level) {
	return gnuradio::get_initial_sptr(new packet_sink_impl(threshold, phy_version, debug_level));
}

/*
 * The private constructor
 */
packet_sink_impl::packet_sink_impl(int threshold, int phy_version, unsigned int debug_level) :
								block("packet_sink", gr::io_signature::make(1, 1, sizeof(unsigned char)),
										gr::io_signature::make(0, 0, 0)), d_threshold(threshold), d_phy_version(phy_version), d_debug_level(debug_level) {
	//**
	d_sync_vector = 0xF3A0;

	if(d_phy_version == 0)
	{
		std::copy(PN_SEQUENCE_V0_1, PN_SEQUENCE_V0_1+16, PN_SEQUENCE);
		mask1 = 0x7FFFFFFE;
		mask2 = 0xFFFFFFFE;
	}
	else 
	{
		std::copy(PN_SEQUENCE_V1, PN_SEQUENCE_V1+16, PN_SEQUENCE);
		mask1 = 0xFFFFFFFF;
		mask2 = 0xFFFFFFFF;
	}

	if(d_debug_level & 2)
		std::cout << "PHY version: " << d_phy_version << std::endl;
	// Link Quality Information
	d_lqi = 0;
	d_lqi_sample_count = 0;

	if ( d_debug_level & 1)
		fprintf(stderr, "syncvec: %x, threshold: %d\n", d_sync_vector,
				d_threshold), fflush(stderr);
	enter_search();

	message_port_register_out(pmt::mp("out"));
	message_port_register_out(pmt::mp("cca_cs"));
}

/*
 * Our virtual destructor.
 */
packet_sink_impl::~packet_sink_impl() {
}

int packet_sink_impl::general_work(int noutput, gr_vector_int& ninput_items,
		gr_vector_const_void_star& input_items,
		gr_vector_void_star& output_items) {

	const unsigned char *inbuf = (const unsigned char*) input_items[0];
	int ninput = ninput_items[0];
	int count = 0;
	int i = 0;

	if (d_debug_level & 1)
		fprintf(stderr, ">>> Entering state machine\n"), fflush(stderr);

	while (count < ninput) {
		switch (d_state) {
		case STATE_SYNC_SEARCH:    // Look for sync vector
			if (d_debug_level & 1)
				fprintf(stderr, "SYNC Search, ninput=%d syncvec=%x\n", ninput,
						d_sync_vector), fflush(stderr);

			while (count < ninput) {

				if (inbuf[count++])
					d_shift_reg = (d_shift_reg << 1) | 1;
				else
					d_shift_reg = d_shift_reg << 1;

				if (d_preamble_cnt > 0) {
					d_chip_cnt = d_chip_cnt + 1;
				}

				// The first if block syncronizes to chip sequences.
				if (d_preamble_cnt == 0) {
					unsigned int threshold;
					threshold = gr::blocks::count_bits32(
							(d_shift_reg & mask1)
							^ (PN_SEQUENCE[0] & mask1));
					if (threshold < d_threshold) {
						//  fprintf(stderr, "Threshold %d d_preamble_cnt: %d\n", threshold, d_preamble_cnt);
						//if ((d_shift_reg&0xFFFFFE) == (PN_SEQUENCE[0]&0xFFFFFE)) {
						if (d_debug_level & 1)
							fprintf(stderr, "Found 0 in chip sequence\n"), fflush(
									stderr);
						// we found a 0 in the chip sequence
						d_preamble_cnt += 1;
						//fprintf(stderr, "Threshold %d d_preamble_cnt: %d\n", threshold, d_preamble_cnt);
					}
				} else {
					// we found the first 0, thus we only have to do the calculation every 32 chips
					if (d_chip_cnt == 32) {
						d_chip_cnt = 0;

						if (d_packet_byte == 0) {
							if (gr::blocks::count_bits32(
									(d_shift_reg & mask1)
									^ (PN_SEQUENCE[0] & mask2))
							<= d_threshold) {
								if (d_debug_level & 1)
									fprintf(stderr,
											"Found %d 0 in chip sequence\n",
											d_preamble_cnt), fflush(stderr);
								// we found an other 0 in the chip sequence
								d_packet_byte = 0;
								d_preamble_cnt++;
							} else if (gr::blocks::count_bits32(
									(d_shift_reg & mask1)
									^ (PN_SEQUENCE[0xA] & mask2))
							<= d_threshold) {
								d_packet_byte = 0xA; // First SFD nibble of 0 occured already, so 0xA is 2nd nibble
								if (d_debug_level & 1)
									fprintf(stderr, "Found second SFD nibble 0x%x, %d\n", d_packet_byte, d_packet_byte), fflush(stderr);

								//break;
							} else {
								// we are not in the synchronization header
								if (d_debug_level & 1)
									fprintf(stderr,
											"Wrong 2nd nibble of SFD. %u\n",
											d_shift_reg), fflush(stderr);
								enter_search();
								break;
							}

						} else {
							if (d_packet_byte == 0xA) // looking for 3rd nibble
							{
								if (gr::blocks::count_bits32(
										(d_shift_reg & mask1)
										^ (PN_SEQUENCE[0x3] & mask2))
								<= d_threshold) {
									d_packet_byte = 0x3;
									if (d_debug_level & 1)
										fprintf(stderr, "Found 3rd SFD nibble 0x%x, %d\n", d_packet_byte, d_packet_byte), fflush(stderr);
									//break;
								} else {
									if (d_debug_level & 1)
										fprintf(stderr,
												"Wrong third nibble of SFD. %u\n",
												d_shift_reg), fflush(stderr);
									enter_search();
									break;
								}
							}
							else //if ((d_packet_byte & 0xF000) == 0) // looking for 4th nibble
							{
								//printf(stderr, "Found sync, 0x%x\n", d_packet_byte), fflush(stderr);
								if (gr::blocks::count_bits32(
										(d_shift_reg & mask1)
										^ (PN_SEQUENCE[0xF] & mask2))
								<= d_threshold) {
									d_packet_byte = 0xF;
									if (d_debug_level & 2)
									{
										fprintf(stderr, "Found 4th SFD nibble 0x%x, %d\n", d_packet_byte, d_packet_byte), fflush(stderr);
										fprintf(stderr, "Found sync\n"), fflush(stderr);
									}
									// found SFD
									
									// send carrier sensed message
									/* send it after frame length is known
									pmt::pmt_t msg = pmt::make_tuple(
									pmt::mp("cca"),
									pmt::mp("carrier sensed"));
									message_port_pub(pmt::mp("cca_cs"), msg);
									*/

									// setup for header decode
									enter_have_sync();
									break;
								} else {
									if (d_debug_level & 2)
										fprintf(stderr,
												"Wrong third nibble of SFD. %u\n",
												d_shift_reg), fflush(stderr);
									enter_search();
									break;
								}
							}

						}
					}
				}
			}
			break;

		case STATE_HAVE_SYNC:
			if (d_debug_level & 2)
				fprintf(stderr, "Header Search bitcnt=%d, header=0x%08x\n",
						d_headerbitlen_cnt, d_header), fflush(stderr);

			while (count < ninput) {	// Decode the bytes one after another.
				if (inbuf[count++])
					d_shift_reg = (d_shift_reg << 1) | 1;
				else
					d_shift_reg = d_shift_reg << 1;

				d_chip_cnt = d_chip_cnt + 1;

				if (d_chip_cnt == 32) {
					d_chip_cnt = 0;
					unsigned char c = decode_chips(d_shift_reg);
					if(d_debug_level & 2)
					{					  
						std::cout << "Frame length nibble " << (int)d_packet_byte_index << ", Nibble: " << std::bitset< 8 > ((int)(c)) << std::endl;
					}

					if (c == 0xFF) {
						// something is wrong. restart the search for a sync
						if (d_debug_level & 2)
							fprintf(stderr,
									"Found a not valid chip sequence! %u\n",
									d_shift_reg), fflush(stderr);

						enter_search();
						break;
					}

					if (d_packet_byte_index == 0) {
						d_packet_byte = c;						  
					} else if (d_packet_byte_index == 1) {
						// c is always < 15
						d_packet_byte |= c << 4;
					} else if (d_packet_byte_index == 2) {
						// c is always < 15
						d_packet_byte |= c << 8;
					} else if (d_packet_byte_index == 3) {
						// c is always < 15
						d_packet_byte |= c << 12;
					}
					d_packet_byte_index = d_packet_byte_index + 1;
					if (d_packet_byte_index % 4 == 0) {
						// we have a complete 2 bytes which represents the frame length.
						if(d_debug_level & 2)
							std::cout << "Frame length : " << (int)d_packet_byte << ", Lower length nibble: " << std::bitset< 8 > ((int)(d_packet_byte & 0xFF)) << ", Upper length nibble: " << std::bitset< 8 > ((int)(d_packet_byte >> 8)) << std::endl;

						int frame_len = d_packet_byte;
						
						// send carrier sensed message
						pmt::pmt_t msg = pmt::make_tuple(
						pmt::mp("cca"),
						pmt::mp("cs"),
						pmt::mp(frame_len));
						message_port_pub(pmt::mp("cca_cs"), msg);
						
						if (frame_len <= MAX_PKT_LEN) {
							enter_have_header(frame_len);
						} else {
							enter_search();
						}
						break;
					}
				}
			}
			break;

		case STATE_HAVE_HEADER:
			if (d_debug_level & 2)
				fprintf(stderr,
						"Packet Build count=%d, ninput=%d, packet_len=%d\n",
						count, ninput, d_packetlen), fflush(stderr);

			while (count < ninput) { // shift bits into bytes of packet one at a time
				if (inbuf[count++])
					d_shift_reg = (d_shift_reg << 1) | 1;
				else
					d_shift_reg = d_shift_reg << 1;

				d_chip_cnt = (d_chip_cnt + 1) % 32;

				if (d_chip_cnt == 0) {
					unsigned char c = decode_chips(d_shift_reg);
					if (c == 0xff) {
						// something is wrong. restart the search for a sync
						if (d_debug_level & 2)
							fprintf(stderr,
									"Found a not valid chip sequence! %u\n",
									d_shift_reg), fflush(stderr);

						enter_search();
						break;
					}
					// the first symbol represents the first part of the byte.
					if (d_packet_byte_index == 0) {
						d_packet_byte = c;
					} else {
						// c is always < 15
						d_packet_byte |= c << 4;
					}
					//fprintf(stderr, "%d: 0x%x\n", d_packet_byte_index, c);
					d_packet_byte_index = d_packet_byte_index + 1;
					if (d_packet_byte_index % 2 == 0) {
						// we have a complete byte
						if (d_debug_level & 2)
							fprintf(stderr,
									"packetcnt: %d, payloadcnt: %d, payload 0x%x, d_packet_byte_index: %d\n",
									d_packetlen_cnt, d_payload_cnt,
									d_packet_byte, d_packet_byte_index), fflush(
											stderr);

						d_packet[d_packetlen_cnt++] = d_packet_byte;
						d_payload_cnt++;
						d_packet_byte_index = 0;

						if (d_payload_cnt >= d_packetlen) {	// packet is filled, including CRC. might do check later in here
							unsigned int scaled_lqi = (d_lqi / MAX_LQI_SAMPLES)
															<< 3;
							unsigned char lqi = (
									scaled_lqi >= 256 ? 255 : scaled_lqi);

							pmt::pmt_t meta = pmt::make_dict();
							meta = pmt::dict_add(meta, pmt::mp("lqi"),
									pmt::from_long(lqi));

							std::memcpy(buf, d_packet, d_packetlen_cnt);
							pmt::pmt_t payload = pmt::make_blob(buf,
									d_packetlen_cnt);

							message_port_pub(pmt::mp("out"),
									pmt::cons(meta, payload));

							if (d_debug_level & 2)
								fprintf(stderr,
										"Adding message of size %d to queue\n",
										d_packetlen_cnt);
							enter_search();
							break;
						}
					}
				}
			}
			break;

		default:
			assert(0);
			break;

		}
	}

	if (d_debug_level & 1)
		fprintf(stderr, "Samples Processed: %d\n", ninput_items[0]), fflush(
				stderr);

	consume(0, ninput_items[0]);

	return 0;
}

void packet_sink_impl::enter_search() {
	if (d_debug_level & 1)
		fprintf(stderr, "@ enter_search\n");

	d_state = STATE_SYNC_SEARCH;
	d_shift_reg = 0;
	d_preamble_cnt = 0;
	d_chip_cnt = 0;
	d_packet_byte = 0;
}

void packet_sink_impl::enter_have_sync() {
	if (d_debug_level & 1)
		fprintf(stderr, "@ enter_have_sync\n");

	d_state = STATE_HAVE_SYNC;
	d_packetlen_cnt = 0;
	d_packet_byte = 0;
	d_packet_byte_index = 0;

	// Link Quality Information
	d_lqi = 0;
	d_lqi_sample_count = 0;
}

void packet_sink_impl::enter_have_header(int payload_len) {
	if (d_debug_level & 1)
		fprintf(stderr, "@ enter_have_header (payload_len = %d)\n",
				payload_len);

	d_state = STATE_HAVE_HEADER;
	d_packetlen = payload_len;
	d_payload_cnt = 0;
	d_packet_byte = 0;
	d_packet_byte_index = 0;
}

unsigned char packet_sink_impl::decode_chips(unsigned int chips) {
	int i;
	int best_match = 0xFF;
	int min_threshold = 33; // Matching to 32 chips, could never have a error of 33 chips

	for (i = 0; i < 16; i++) {
		// FIXME: we can store the last chip
		// ignore the first and last chip since it depends on the last chip.
		unsigned int threshold = gr::blocks::count_bits32(
				(chips & mask1) ^ (PN_SEQUENCE[i] & mask1));

		if (threshold < min_threshold) {
			best_match = i;
			min_threshold = threshold;
		}
	}

	if (min_threshold < d_threshold) {
		if (d_debug_level & 1)
			fprintf(stderr, "Found sequence with %d errors at 0x%x\n",
					min_threshold,
					(chips & mask1)
					^ (PN_SEQUENCE[best_match] & mask1)), fflush(
							stderr);
		// LQI: Average number of chips correct * MAX_LQI_SAMPLES
		//
		if (d_lqi_sample_count < MAX_LQI_SAMPLES) {
			d_lqi += 32 - min_threshold;
			d_lqi_sample_count++;
		}

		return (char) best_match & 0xF;
	}

	return 0xFF;
}

} /* namespace cognitiva */
} /* namespace gr */

