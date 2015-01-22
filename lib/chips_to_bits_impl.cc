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
#include "chips_to_bits_impl.h"

namespace gr {
namespace cognitiva {

static const unsigned int PN_SEQUENCE[] = { 3653456430, 3986437410, 786023250,
		585997365, 1378802115, 891481500, 3276943065, 2620728045, 2358642555,
		3100205175, 2072811015, 2008598880, 125537430, 1618458825, 2517072780,
		3378542520 };


inline unsigned char decode_chips(unsigned int chips) {
	int i;
	int best_match = 0xFF;
	int min_threshold = 33; // Matching to 32 chips, could never have a error of 33 chips

	for (i = 0; i < 16; i++) {
		// FIXME: we can store the last chip
		// ignore the first and last chip since it depends on the last chip.
		unsigned int threshold = gr::blocks::count_bits32(
				(chips & 0xFFFFFFFF) ^ (PN_SEQUENCE[i] & 0xFFFFFFFF));

		if (threshold < min_threshold) {
			best_match = i;
			min_threshold = threshold;
		}
	}

	if (min_threshold < 33) {
		/*if (VERBOSE)
			fprintf(stderr, "Found sequence with %d errors at 0x%x\n",
					min_threshold,
					(chips & 0x7FFFFFFE)
							^ (PN_SEQUENCE[best_match] & 0x7FFFFFFE)), fflush(
					stderr);*/
		// LQI: Average number of chips correct * MAX_LQI_SAMPLES
		//
		/*if (d_lqi_sample_count < MAX_LQI_SAMPLES) {
			d_lqi += 32 - min_threshold;
			d_lqi_sample_count++;
		}*/

		return (unsigned char) best_match & 0xF;
	}

	return 0xFF;
}

chips_to_bits::sptr
chips_to_bits::make()
{
	return gnuradio::get_initial_sptr
			(new chips_to_bits_impl());
}

/*
 * The private constructor
 */
chips_to_bits_impl::chips_to_bits_impl()
: gr::sync_decimator("chips_to_bits",
		gr::io_signature::make(1, 1, sizeof(unsigned char) * 8),
		gr::io_signature::make(1, 1, sizeof(unsigned char)), 8)
{}

/*
 * Our virtual destructor.
 */
chips_to_bits_impl::~chips_to_bits_impl()
{
}

int
chips_to_bits_impl::work(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items)
{
	const unsigned char *in = (const unsigned char *) input_items[0];
	unsigned char *out = (unsigned char *) output_items[0];

	// Do <+signal processing+>
	uint i_out = 0;
	unsigned int chips;
	for (int i_in = 0; i_in < (noutput_items * 8); i_in+=8) {
		chips = (in[i_in] << 24) | (in[i_in+1] << 16) | (in[i_in+2] << 8) | (in[i_in+3] << 0);
		out[i_out] = decode_chips(chips) << 4;	  
		//std::cout << std::setfill('0') << "\tupper nibble out: " << std::bitset< 4 > ((int)((out[i_out] & 0xF0) >> 4)) << " | " << std::dec << (int)((out[i_out] & 0xF0)>>4)  << ", bin in: "<< std::bitset< 32 >(chips) << std::endl;

		chips = (in[i_in+4] << 24) | (in[i_in+5] << 16) | (in[i_in+6] << 8) | (in[i_in+7] << 0);
		out[i_out] |= decode_chips(chips);	  
		//std::cout << std::setfill('0') << "\tlower nibble out: " << std::bitset< 4 >  ((int)(out[i_out] & 0xF)) << " | " << std::dec << (int)(out[i_out] & 0xF)  << ", bin in: "<< std::bitset< 32 >(chips) << std::endl;

		i_out++;
	}



	// Tell runtime system how many output items we produced.
	return noutput_items;
}

} /* namespace cognitiva */
} /* namespace gr */
