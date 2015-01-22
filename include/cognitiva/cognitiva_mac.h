/* -*- c++ -*- */
/* 
 * Copyright 2014 <+YOU OR YOUR COMPANY+>.
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


#ifndef INCLUDED_COGNITIVA_COGNITIVA_MAC_H
#define INCLUDED_COGNITIVA_COGNITIVA_MAC_H

#include <cognitiva/api.h>
#include <gnuradio/block.h>

namespace gr {
  namespace cognitiva {

    /*!
     * \brief <+description of block+>
     * \ingroup cognitiva
     *
     */
    class COGNITIVA_API cognitiva_mac : virtual public gr::block
    {
     public:
      typedef boost::shared_ptr<cognitiva_mac> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of cognitiva::cognitiva_mac.
       *
       * To avoid accidental use of raw pointers, cognitiva::cognitiva_mac's
       * constructor is in a private implementation
       * class. cognitiva::cognitiva_mac::make is the public interface for
       * creating new instances.
       */
      static sptr make(const char *mac_address_, 
        unsigned int cca_mode,
		bool use_arq_,
		unsigned int mac_version_ = 0,
		unsigned int max_attempts_ = 10,
		float timeout_ = 0.1,
		float broadcast_interval_ = 2.0,
		bool exp_backoff_ = true,
		float backoff_randomness_ = 0.05,
		float node_expiry_delay_ = 10,
		bool expire_on_arq_failure_ = false,
		bool only_send_if_alive_ = false,
		unsigned int debug_level_ = 0);
    };
  } // namespace cognitiva
} // namespace gr

#endif /* INCLUDED_COGNITIVA_COGNITIVA_MAC_H */

