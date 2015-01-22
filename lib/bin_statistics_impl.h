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

#ifndef INCLUDED_COGNITIVA_BIN_STATISTICS_IMPL_H
#define INCLUDED_COGNITIVA_BIN_STATISTICS_IMPL_H

#include <cognitiva/bin_statistics.h>

namespace gr {
namespace cognitiva {

class bin_statistics_impl : public bin_statistics
{
private:
	enum state_t {ST_IDLE, ST_INIT, ST_TUNE_DELAY, ST_DWELL_DELAY };

	size_t d_vlen;
	size_t d_tune_delay;
	size_t d_dwell_delay;    
	state_t d_state;
	size_t d_delay;    // nsamples remaining to state transition
	std::vector<float> d_max;	// per bin maxima
	float sample_rate;
	float d_center_freq;
	unsigned int d_debug_level;

	void enter_init();
	void enter_tune_delay();
	void enter_dwell_delay();
	void leave_dwell_delay();
	void enter_idle();

	size_t vlen() const { return d_vlen; }

	void reset_stats();
	void accrue_stats(const float *input);
	void send_stats();

	void process_command(pmt::pmt_t msg);

public:
	bin_statistics_impl(unsigned int vlen, float tune_delay, float dwell_delay, float samp_rate, unsigned int debug_level_);
	~bin_statistics_impl();

	// Where all the action really happens
	int work(int noutput_items,
			gr_vector_const_void_star &input_items,
			gr_vector_void_star &output_items);
};

} // namespace cognitiva
} // namespace gr

#endif /* INCLUDED_COGNITIVA_BIN_STATISTICS_IMPL_H */

