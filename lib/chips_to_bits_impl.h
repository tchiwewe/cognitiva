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

#ifndef INCLUDED_COGNITIVA_CHIPS_TO_BITS_IMPL_H
#define INCLUDED_COGNITIVA_CHIPS_TO_BITS_IMPL_H

#include <cognitiva/chips_to_bits.h>

namespace gr {
namespace cognitiva {

class chips_to_bits_impl : public chips_to_bits
{
private:
	// Nothing to declare in this block.

public:
	chips_to_bits_impl();
	~chips_to_bits_impl();

	// Where all the action really happens
	int work(int noutput_items,
			gr_vector_const_void_star &input_items,
			gr_vector_void_star &output_items);
};

} // namespace cognitiva
} // namespace gr

#endif /* INCLUDED_COGNITIVA_CHIPS_TO_BITS_IMPL_H */

