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
#include "bits_to_chips_impl.h"
#include "pn_sequence.h"
#include <bitset>

namespace gr {
namespace cognitiva {

bits_to_chips::sptr bits_to_chips::make(unsigned int phy_version, unsigned int debug_level) {
	return gnuradio::get_initial_sptr(new bits_to_chips_impl(phy_version, debug_level));
}

/*
 * The private constructor
 */
bits_to_chips_impl::bits_to_chips_impl(unsigned int phy_version, unsigned int debug_level) : d_phy_version(phy_version), d_debug_level(debug_level),
		gr::sync_interpolator("bits_to_chips",
				gr::io_signature::make(1, 1, sizeof(unsigned char)),
				gr::io_signature::make(1, 1, sizeof(unsigned char)), 8) {
	if(d_phy_version == 0)
	{
		std::copy(PN_SEQUENCE_V0_0, PN_SEQUENCE_V0_0+16, PN_SEQUENCE);
	}
	else 
	{
		std::copy(PN_SEQUENCE_V1, PN_SEQUENCE_V1+16, PN_SEQUENCE);
	}

}

/*
 * Our virtual destructor.
 */
bits_to_chips_impl::~bits_to_chips_impl() {
}

int bits_to_chips_impl::work(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items) {
	const unsigned char *in = (const unsigned char *) input_items[0];
	unsigned char *out = (unsigned char *) output_items[0];

	uint i_out = 0;
	for (int i_in = 0; i_in < (noutput_items / 8); i_in++) {
		unsigned char chip_bytes[4];
		const unsigned char in_byte = in[i_in];

		if (d_debug_level & 2)
			std::cout << std::setfill('0') << "hex in: " << std::hex << std::setw(4) << (int)in_byte << ", " <<  "dec in: " << std::dec << (int)in_byte  << ", " <<  "bin in: " << std::bitset< 8 >(in_byte) << std::endl;

		// lower nibble first
		unsigned int chips = PN_SEQUENCE[(in_byte & 0xF)];
		chips_to_bytes(chips, chip_bytes);
		std::memcpy(&out[i_out], chip_bytes, 4);
		i_out += 4;
		if (d_debug_level & 2)
			std::cout << std::setfill('0') << "\tlower nibble in: " << std::bitset< 4 >(in_byte & 0xF) << " | " << std::dec << (int)(in_byte & 0xF) << ", bin out: "<< std::bitset< 32 >(chips) << std::endl;

		// upper nibble second
		chips = PN_SEQUENCE[(in_byte & 0xF0) >> 4];
		chips_to_bytes(chips, chip_bytes);
		std::memcpy(&out[i_out], chip_bytes, 4);
		i_out += 4;

		if (d_debug_level & 2)
			std::cout << std::setfill('0') << "\tupper nibble in: " << std::bitset< 4 >((in_byte & 0xF0)>>4) << " | " << std::dec << (int)((in_byte & 0xF0) >>4) << ", bin out: "<< std::bitset< 32 >(chips) << std::endl;
	}

	if (d_debug_level & 1)
		std::cout << "*Samples Produced: " << noutput_items << "\n";
	//fprintf(stderr, "Samples Produced: %d\n", noutput_items), fflush(
	//                        stderr);
	// Tell runtime system how many output items we produced.
	return noutput_items;
}

void bits_to_chips_impl::chips_to_bytes(unsigned int chips,
		unsigned char chip_bytes[]) {
	chip_bytes[3] = (chips & 0xFF);
	chip_bytes[2] = ((chips & 0xFF00) >> 8);
	chip_bytes[1] = ((chips & 0xFF0000) >> 16);
	chip_bytes[0] = ((chips & 0xFF000000) >> 24);
}

} /* namespace cognitiva */
} /* namespace gr */

