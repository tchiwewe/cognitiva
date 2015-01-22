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

#ifndef INCLUDED_COGNITIVA_COGNITIVA_PHY_PPDU_IMPL_H
#define INCLUDED_COGNITIVA_COGNITIVA_PHY_PPDU_IMPL_H

#include <cognitiva/cognitiva_phy_ppdu.h>
#include "packet_utils.h"

namespace gr {
namespace cognitiva {

class cognitiva_phy_ppdu_impl: public cognitiva_phy_ppdu {
public:
	cognitiva_phy_ppdu_impl(unsigned int debug_level = 0);
	~cognitiva_phy_ppdu_impl();

private:
	void make_ppdu(pmt::pmt_t msg);
	void add_phy_fields(const uint8_t *buf, int len);

	typedef struct ppdu_struct_ {
		// PPDU fields
		uint8_t sync[6];         // Synchronisation header
		uint8_t SFD[2];  // Start of frame delimiter / access code
		uint8_t length[2];       // Payload (MPDU) length
		uint8_t payload[MAX_PPDU_LEN];    // Payload (MPDU)
		//uint8_t        padding[4];    // Padding

	} ppdu_struct;

	uint8_t m_PPDU[MAX_PPDU_LEN];               // PHY protocol data unit    

	unsigned int d_debug_level;
};

} // namespace cognitiva
} // namespace gr

#endif /* INCLUDED_COGNITIVA_COGNITIVA_PHY_PPDU_IMPL_H */
