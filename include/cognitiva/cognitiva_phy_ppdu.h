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


#ifndef INCLUDED_COGNITIVA_COGNITIVA_PHY_PPDU_H
#define INCLUDED_COGNITIVA_COGNITIVA_PHY_PPDU_H

#include <cognitiva/api.h>
#include <gnuradio/block.h>

namespace gr {
  namespace cognitiva {

    /*!
     * \brief <+description of block+>
     * \ingroup cognitiva
     *
     */
    class COGNITIVA_API cognitiva_phy_ppdu : virtual public gr::block
    {
     public:
      typedef boost::shared_ptr<cognitiva_phy_ppdu> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of cognitiva::cognitiva_phy_ppdu.
       *
       * To avoid accidental use of raw pointers, cognitiva::cognitiva_phy_ppdu's
       * constructor is in a private implementation
       * class. cognitiva::cognitiva_phy_ppdu::make is the public interface for
       * creating new instances.
       */
      static sptr make(unsigned int debug_level = 0);
    };

  } // namespace cognitiva
} // namespace gr

#endif /* INCLUDED_COGNITIVA_COGNITIVA_PHY_PPDU_H */

