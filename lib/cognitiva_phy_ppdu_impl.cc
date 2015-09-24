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
#include "cognitiva_phy_ppdu_impl.h"
#include "debug_utils.h"

#include <bitset>

namespace gr {
namespace cognitiva {

cognitiva_phy_ppdu::sptr cognitiva_phy_ppdu::make(unsigned int debug_level) {
	return gnuradio::get_initial_sptr(new cognitiva_phy_ppdu_impl(debug_level));
}

/*
 * The private constructor
 */
cognitiva_phy_ppdu_impl::cognitiva_phy_ppdu_impl(unsigned int debug_level) : d_debug_level(debug_level),
		gr::block("cognitiva_phy_ppdu", gr::io_signature::make(0, 0, 0),
				gr::io_signature::make(0, 0, 0)) {
	message_port_register_out(pmt::mp("out"));
	message_port_register_in(pmt::mp("in"));

	set_msg_handler(pmt::mp("in"),
			boost::bind(&cognitiva_phy_ppdu_impl::make_ppdu, this, _1));
}

/*
 * Our virtual destructor.
 */
cognitiva_phy_ppdu_impl::~cognitiva_phy_ppdu_impl() {
}

void cognitiva_phy_ppdu_impl::make_ppdu(pmt::pmt_t msg) {
	if (pmt::is_eof_object(msg)) {
		std::cout << "PHY: exiting" << std::endl;
		message_port_pub(pmt::mp("out"), pmt::PMT_EOF);
		return;
	}

	assert(pmt::is_pair(msg));
	pmt::pmt_t blob = pmt::cdr(msg);

	size_t data_len = pmt::blob_length(blob);

	if (d_debug_level & DEBUG_VERBOSE)
		std::cout << "PHY message received, len: " << data_len << std::endl;

	assert(data_len);
	assert(data_len <= MAX_PPDU_LEN - PPDU_OVERHEAD);
	
	if (data_len > MAX_PPDU_LEN - PPDU_OVERHEAD)
	{
	  std::cout <<  "ERROR: Message too large. PHY message received, len: " << data_len << std::endl;
	  abort();
	}

	// PPDU fields
	ppdu_struct ppdu;
	ppdu.sync[0] = 0;
	ppdu.sync[1] = 0;
	ppdu.sync[2] = 0;
	ppdu.sync[3] = 0;
	ppdu.sync[4] = 0;
	ppdu.sync[5] = 0;
	ppdu.SFD[0] = 0xA0; // 1111001110100000 802.11 SFD
	ppdu.SFD[1] = 0xF3;
	ppdu.length[0] = data_len & 0xFF;
	ppdu.length[1] = data_len >> 8;
	
	std::memcpy(ppdu.payload, pmt::blob_data(blob), data_len);
	std::memcpy(m_PPDU, ppdu.sync, 6);
	std::memcpy(m_PPDU + 6, ppdu.SFD, 2);
	std::memcpy(m_PPDU + 8, ppdu.length, 2);
	std::memcpy(m_PPDU + 10, ppdu.payload, data_len);
	
	message_port_pub(pmt::mp("out"),
			pmt::cons(pmt::PMT_NIL, pmt::make_blob(m_PPDU, data_len + 10)));

	if (d_debug_level & 1)
		std::cout << "PHY payload length: " << data_len << ", Lower length nibble: " << std::bitset< 8 > ((int)(data_len & 0xFF)) << ", Upper length nibble: " << std::bitset< 8 > ((int)(data_len >> 8)) << std::endl;
}

void cognitiva_phy_ppdu_impl::add_phy_fields(const uint8_t *buf, int len) {
}

} /* namespace cognitiva */
} /* namespace gr */
