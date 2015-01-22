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
#include "bits_to_chips_new_impl.h"

#include <stdio.h>
#include <bitset>

namespace gr {
  namespace cognitiva {

    static const unsigned int PN_SEQUENCE[] = { 3653456430, 3986437410, 786023250,
            585997365, 1378802115, 891481500, 3276943065, 2620728045, 2358642555,
            3100205175, 2072811015, 2008598880, 125537430, 1618458825, 2517072780,
            3378542520 };

    bits_to_chips_new::sptr
    bits_to_chips_new::make(unsigned int debug_level)
    {
      return gnuradio::get_initial_sptr
        (new bits_to_chips_new_impl(debug_level));
    }

    /*
     * The private constructor
     */
    bits_to_chips_new_impl::bits_to_chips_new_impl(unsigned int debug_level) : d_debug_level(debug_level),
      gr::block("bits_to_chips_new",
              gr::io_signature::make(1, 1, sizeof(unsigned char)),
              gr::io_signature::make(1, 1, sizeof(unsigned char)))
    {}

    /*
     * Our virtual destructor.
     */
    bits_to_chips_new_impl::~bits_to_chips_new_impl()
    {
    }

    void
    bits_to_chips_new_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
        /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
        if (noutput_items < 8)
        {
            ninput_items_required[0] = 1;
        }
        else
        {
            ninput_items_required[0] = noutput_items / 8;
        }
        
        //if (ninput_items_required[0] == 0)
          //  ninput_items_required[0] = 1;
        if (d_debug_level & 2)
	    printf("********** Forecast wants %d needs %d\n",noutput_items,ninput_items_required[0]);
    }

    int
    bits_to_chips_new_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
        const unsigned char *in = (const unsigned char *) input_items[0];
        unsigned char *out = (unsigned char *) output_items[0];

        if(noutput_items >= 8)
        {
            if((noutput_items % 8) == 0)
            {
                // Do <+signal processing+>
                uint i_out = 0;
                for (int i_in = 0; i_in < (noutput_items / 8); i_in++) {
                        unsigned char chip_bytes[4];
                        const unsigned char in_byte = in[i_in];

			  if (d_debug_level & 2)
			    std::cout << std::setfill('0') << "hex in: " << std::hex << std::setw(4) << (int)in_byte << ", " <<  "dec in: " << std::dec << (int)in_byte  << ", " <<  "bin in: " << std::bitset< 8 >(in_byte) << std::endl;

                        // lower nibble first
                        unsigned int chips = PN_SEQUENCE[(in_byte & 0xF)];
                        chips_to_bytes(chips, chip_bytes);
                        //std::memcpy(&out[i_out], chip_bytes, 4);
                        out[i_out++] = chip_bytes[0];
                        out[i_out++] = chip_bytes[1];
                        out[i_out++] = chip_bytes[2];
                        out[i_out++] = chip_bytes[3];
                        //i_out += 4;
			
			if (d_debug_level & 2)
			  std::cout << std::setfill('0') << "\tlower nibble in: " << std::bitset< 4 >(in_byte & 0xF) << " | " << std::dec << (int)(in_byte & 0xF) << ", bin out: "<< std::bitset< 32 >(chips) << std::endl;

                        assert(chip_bytes[0]);
                        assert(chip_bytes[1]);
                        assert(chip_bytes[2]);
                        assert(chip_bytes[3]);
                        
                        // upper nibble second
                        chips = PN_SEQUENCE[(in_byte & 0xF0) >> 4];
                        chips_to_bytes(chips, chip_bytes);
                        //std::memcpy(&out[i_out], chip_bytes, 4);
                        //i_out += 4;
                        out[i_out++] = chip_bytes[0];
                        out[i_out++] = chip_bytes[1];
                        out[i_out++] = chip_bytes[2];
                        out[i_out++] = chip_bytes[3];
                       
			  if (d_debug_level & 2)
			    std::cout << std::setfill('0') << "\tupper nibble in: " << std::bitset< 4 >((in_byte & 0xF0)>>4) << " | " << std::dec << (int)((in_byte & 0xF0) >>4) << ", bin out: "<< std::bitset< 32 >(chips) << std::endl;
                        
                        assert(chip_bytes[0]);
                        assert(chip_bytes[1]);
                        assert(chip_bytes[2]);
                        assert(chip_bytes[3]);
                }

                // Tell runtime system how many input items we consumed on
                // each input stream.        
                consume_each (noutput_items/8);

		if (d_debug_level & 1)
		  printf("********** Produced %d, consumed %d\n",noutput_items, noutput_items/4);

                // Tell runtime system how many output items we produced.
                return noutput_items;
            }
            else
            {
                consume_each (0);
                return 0;
            }
        }
        else
        {
            consume_each (0);
            return 0;
        }
    }

    void bits_to_chips_new_impl::chips_to_bytes(unsigned int chips,
                unsigned char chip_bytes[]) {
        chip_bytes[3] = (chips & 0xFF);
        chip_bytes[2] = ((chips & 0xFF00) >> 8);
        chip_bytes[1] = ((chips & 0xFF0000) >> 16);
        chip_bytes[0] = ((chips & 0xFF000000) >> 24);
    }

  } /* namespace cognitiva */
} /* namespace gr */

