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
 * the Free Software Foundation, Inc., 51 f(msgq()->full_p())    // if the queue is full, don't block, drop the data...
        return;

      // build & send a message
      message::sptr msg = message::make(0, center_freq(), vlen(), vlen() * sizeof(float));
      memcpy(msg->msg(), &d_max[0], vlen() * sizeof(float));
      msgq()->insert_tail(msg);Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "bin_statistics_impl.h"

namespace gr {
namespace cognitiva {

bin_statistics::sptr
bin_statistics::make(unsigned int vlen, float tune_delay, float dwell_delay, float samp_rate, unsigned int debug_level_)
{
	return gnuradio::get_initial_sptr
			(new bin_statistics_impl(vlen, tune_delay, dwell_delay, samp_rate, debug_level_));
}

/*
 * The private constructor
 */
bin_statistics_impl::bin_statistics_impl(unsigned int vlen, float tune_delay, float dwell_delay, float samp_rate, unsigned int debug_level_)
: gr::sync_block("bin_statistics",
		gr::io_signature::make(1, 1, sizeof(float) * vlen),
		gr::io_signature::make(0,0,0)),
		d_vlen(vlen),
		d_tune_delay(std::max(0, (int)round(tune_delay * samp_rate / vlen))), 
		d_dwell_delay(std::max (1, (int)std::ceil(dwell_delay * samp_rate / vlen))),
		d_delay(0),
		d_max(vlen),
		d_center_freq(0),
		sample_rate(samp_rate),
		d_debug_level(debug_level_)
{
	if (d_debug_level & 1)
	{
		std::cout << "Tune delay: " << tune_delay << " " << d_tune_delay << std::endl; 
		std::cout << "Dwell delay: " << dwell_delay << " " << d_dwell_delay << std::endl;
		std::cout << "Vlen: " << d_vlen << std::endl;
		std::cout << "Sample rate: " << sample_rate << std::endl;
	}
	//self.tune_delay  = max(0, int(round(self.tune_delay * self.samp_rate / self.fft_size)))
	//self.dwell_delay = max(1, int(round(self.dwell_delay * self.samp_rate / self.fft_size)))

	message_port_register_out(pmt::mp("bins"));	
	message_port_register_out(pmt::mp("cca_ed"));
	message_port_register_out(pmt::mp("command_out"));	
	message_port_register_in(pmt::mp("command_in"));
	set_msg_handler(pmt::mp("command_in"),
			boost::bind(&bin_statistics_impl::process_command, this, _1));

	// Start in init state
	enter_init();
}

void bin_statistics_impl::process_command(pmt::pmt_t msg)
{
	if (d_state == ST_IDLE)
	{
		if (d_debug_level & 2)
			std::cout << "Got spectrum sensing command " << msg << std::endl;
		if ((pmt::length(msg) == 2) && pmt::is_tuple(msg))
		{
			pmt::pmt_t msg_param1 = pmt::tuple_ref(msg, 0);
			pmt::pmt_t msg_param2 = pmt::tuple_ref(msg, 1);
			
			if (d_debug_level & 2)
			{	
				std::cout << "Msg param 1: " << msg_param1 << std::endl;
				std::cout << "Msg param 2: " << msg_param2 << std::endl;
			}
			if (pmt::symbol_to_string(msg_param1) == "spectrum_sense")
			{
				if (pmt::symbol_to_string(msg_param2) == "tune")
				{
					enter_init();
				}
				else if (pmt::symbol_to_string(msg_param2) == "continue")
				{
					enter_dwell_delay();
				}				
				else
					std::cout << "what****\n";
			}
			else
				std::cout << "huh****\n";
		}
		   
	}
	else
	{
		std::cout << "Attempt to start bin statistics while not in IDLE state\n";
		abort();
	}
	//** state checks
}

/*
 * Our virtual destructor.
 */
bin_statistics_impl::~bin_statistics_impl()
{
}

int
bin_statistics_impl::work(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items)
{
	const float *input = (const float*)input_items[0];
	size_t vlen = d_max.size();

	int n = 0;
	int t;

	while(n < noutput_items) {
		switch(d_state) {
		case ST_IDLE:	    
			n = noutput_items;
			break;
		case ST_INIT:
			enter_tune_delay();
			break;

		case ST_TUNE_DELAY:
			t = std::min(noutput_items - n, int(d_delay));
			n += t;
			d_delay -= t;
			assert(d_delay >= 0);
			if(d_delay == 0)
				enter_dwell_delay();
			break;

		case ST_DWELL_DELAY:
			t = std::min(noutput_items - n, int(d_delay));
			for(int i = 0; i < t; i++) {
				accrue_stats(&input[n * vlen]);
				n++;
			}
			d_delay -= t;
			assert(d_delay >= 0);
			if(d_delay == 0) {
				leave_dwell_delay();
				enter_idle();
			}
			break;

		default:
			assert(0);
		}
	}

	return noutput_items;
}

void
bin_statistics_impl::enter_init()
{
	if (d_debug_level & 1)
		std::cout << "*** Entered INIT state" << std::endl; 
	d_state = ST_INIT;
	d_delay = 0;
}

void
bin_statistics_impl::enter_tune_delay()
{
	if (d_debug_level & 1)
		std::cout << "*** Entered TUNE DELAY state" << std::endl; 

	d_state = ST_TUNE_DELAY;
	d_delay = d_tune_delay;
	//**d_center_freq = d_tune->calleval(0);
}

void
bin_statistics_impl::enter_dwell_delay()
{
	if (d_debug_level & 1)
		std::cout << "*** Entered DWELL DELAY state" << std::endl; 
	d_state = ST_DWELL_DELAY;
	d_delay = d_dwell_delay;
	reset_stats();
}

void
bin_statistics_impl::leave_dwell_delay()
{
	send_stats();
}

void
bin_statistics_impl::enter_idle()
{
	if (d_debug_level & 1)
		std::cout << "*** Entered IDLE state" << std::endl; 
	d_state = ST_IDLE;
	d_delay = 0;
}


//////////////////////////////////////////////////////////////////////////
//                  methods for gathering stats
//////////////////////////////////////////////////////////////////////////

void
bin_statistics_impl::reset_stats()
{
	for (size_t i = 0; i < vlen(); i++){
		d_max[i] = 0;
	}
}

void
bin_statistics_impl::accrue_stats(const float *input)
{
	for(size_t i = 0; i < vlen(); i++) {
		d_max[i] = std::min(d_max[i], input[i]);    // compute per bin maxima
	}
}

void
bin_statistics_impl::send_stats()
{
	if (d_debug_level & 1)
		std::cout << "*** Sending stats\n";

	// send bin statistics message
	/*
	pmt::pmt_t command = pmt::make_tuple( // We make a pair, but pmt::make_tuple() is also valid!
			pmt::mp("freq"), // Use the 'freq' command, which sets the frequency
			pmt::mp(d_center_freq), // Set the frequency to 1.1 GHz
			pmt::init_f32vector(vlen(), (const std::vector<float>) d_max)			);

	message_port_pub(pmt::mp("bins"),
			command);
	*/
	
	// Calculate average power for channel
	int bin_start = (int)(vlen() * 0.25);
    int bin_stop = (vlen() - bin_start);
    if (d_debug_level & 2)
		std::cout << "Num bins: " << vlen() << ", bin start: " << bin_start << ", bin stop: " << bin_stop << std::endl;
    
    float pow_total = 0;
    float pow_avg = 0;
    for(int bin = bin_start; bin < bin_stop; bin++) {
		pow_total += d_max[bin];
	}
	pow_avg = pow_total / (bin_stop - bin_start);
	if (d_debug_level & 2)
		std::cout << "Total power: " << pow_total << ", Avg power: " << pow_avg << std::endl;

	// send energy detected message
	//if (pow_avg > -174) //** set energy detection threshold
	{
		pmt::pmt_t msg = pmt::make_tuple(
		pmt::mp("cca"),
		pmt::mp("ed"),
		pmt::mp(pow_avg));
		message_port_pub(pmt::mp("cca_ed"), msg);
	}
}

} /* namespace cognitiva */
} /* namespace gr */

