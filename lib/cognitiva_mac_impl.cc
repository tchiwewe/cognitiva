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
#include "cognitiva_mac_impl.h"
#include "debug_utils.h"
#include <gnuradio/digital/crc32.h>

// Handle IPv4 and IPv6 addresses
#include <arpa/inet.h>

#include <stdio.h>
// 
//using namespace gr;
//using namespace gr::digital;

namespace gr {
namespace cognitiva {

cognitiva_mac::sptr cognitiva_mac::make(const char *mac_address_, 
		unsigned int cca_mode_,
		bool use_arq_,
		unsigned int mac_version_,
		unsigned int max_attempts_,
		float timeout_,
		float broadcast_interval_,
		bool exp_backoff_,
		float backoff_randomness_,
		float node_expiry_delay_,
		bool expire_on_arq_failure_,
		bool only_send_if_alive_,
		unsigned int debug_level_) {
	return gnuradio::get_initial_sptr(
			new cognitiva_mac_impl(mac_address_, 
					cca_mode_,
					use_arq_,
					mac_version_,
					max_attempts_,
					timeout_,
					broadcast_interval_,
					exp_backoff_,
					backoff_randomness_,
					node_expiry_delay_,
					expire_on_arq_failure_,
					only_send_if_alive_,
					debug_level_));
}

/*
 * The private constructor
 */
cognitiva_mac_impl::cognitiva_mac_impl(const char *mac_address_, 
		unsigned int cca_mode_,
		bool use_arq_,
		unsigned int mac_version_,
		unsigned int max_attempts_,
		float timeout_,
		float broadcast_interval_,
		bool exp_backoff_,
		float backoff_randomness_,
		float node_expiry_delay_,
		bool expire_on_arq_failure_,
		bool only_send_if_alive_,
		unsigned int debug_level_) :
										local_address(mac_address_), 
										d_cca_mode(cca_mode_),
										use_arq(use_arq_), 
										d_debug_level(debug_level_), 
										d_mac_version(mac_version_), 
										max_attempts(max_attempts_), 
										timeout(timeout_), 
										broadcast_interval(broadcast_interval_), 
										exp_backoff(exp_backoff_ ? ARQ_BACKOFF_EXPONENTIAL : ARQ_BACKOFF_LINEAR), 
										backoff_randomness(backoff_randomness_),
										node_expiry_delay(node_expiry_delay_),
										expire_on_arq_failure(expire_on_arq_failure_),
										only_send_if_alive(only_send_if_alive_),		
										gr::block("cognitiva_mac", gr::io_signature::make(0, 0, 0),gr::io_signature::make(0, 0, 0)) {
	message_port_register_out(pmt::mp("mpdu_out"));
	message_port_register_out(pmt::mp("payload_out"));
	message_port_register_out(pmt::mp("control_out"));
	message_port_register_in(pmt::mp("mpdu_in"));
	message_port_register_in(pmt::mp("payload_in"));
	message_port_register_in(pmt::mp("control_in"));

	set_msg_handler(pmt::mp("payload_in"),
			boost::bind(&cognitiva_mac_impl::make_mpdu, this, _1));
	set_msg_handler(pmt::mp("mpdu_in"),
			boost::bind(&cognitiva_mac_impl::read_mpdu, this, _1));
	set_msg_handler(pmt::mp("control_in"),
			boost::bind(&cognitiva_mac_impl::read_cca, this, _1));

	m_seq_count = 0;
	broadcast_address = MacAddress("::");

	// ARQ
	arq_pkts_txed = 0;
	arq_retxed = 0;
	failed_arq = 0;
	succeeded_arq = 0;
	total_arq = 0;
	arq_channel_state = ARQ_CHANNEL_IDLE;
	expected_arq_id = 0;	
	next_random_backoff_percentage = 0.0;	
	
	// CCA
	cca_channel_state = CCA_CHANNEL_IDLE;
	cca_ed_threshold = -90; //**
	csma_tx_state = TX_INIT;
	timeout_difs = (MAX_MAC_PAYLOAD_LEN + PPDU_OVERHEAD + MPDU_OVERHEAD) * 8 / 250e3;
		
	if (d_debug_level & DEBUG_INFO)
	{
		std::cout << "cca_mode: " << d_cca_mode << std::endl;
		std::cout << "use_arq: " << use_arq << std::endl;
		std::cout << "max_attempts: " << max_attempts << std::endl;
		std::cout << "timeout: " << timeout << std::endl;
		std::cout << "broadcast_interval: " << broadcast_interval << std::endl;
		std::cout << "exp_backoff: " << exp_backoff << std::endl;
		std::cout << "backoff_randomness: " << backoff_randomness << std::endl;
		std::cout << "node_expiry_delay: " << node_expiry_delay << std::endl;
		std::cout << "expire_on_arq_failure: " << expire_on_arq_failure << std::endl;
		std::cout << "only_send_if_alive: " << only_send_if_alive << std::endl;
		std::cout << "timeout_difs: " << timeout_difs << std::endl;
	}	
	
	//gettimeofday(&last_tx_time, NULL);	
	last_tx_time = (struct timeval){0};
	time_cca_busy = (struct timeval){0};
	time_csma_tx_wait = (struct timeval){0};
	time_csma_tx_defer = (struct timeval){0};

	// As pure message block work function isn't called, create thread for continuous, periodic update of MAC logic
	d_thread = boost::shared_ptr<boost::thread> (new boost::thread(boost::bind(&cognitiva_mac_impl::run, this)));
	d_finished = false;
	d_period_ms = 10; // 10ms interval
	
	/*// Need to initiate spectrum sensing if desired
	{
		// Continue spectrum sensing for energy detection
		pmt::pmt_t command = pmt::make_tuple(
		pmt::mp("spectrum_sense"),
		pmt::mp("continue"));
		message_port_pub(pmt::mp("control_out"), command);		
		if (d_debug_level & DEBUG_INFO)				
			std::cout << "Sent SS continue command" << std::endl;
	}*/
}

/*
 * Our virtual destructor.
 */
cognitiva_mac_impl::~cognitiva_mac_impl() {
	d_finished = true;
	d_thread->interrupt();
	d_thread->join();
}

/* 
 * Run loop 
 */
void cognitiva_mac_impl::run()
{
	while(!d_finished) {
		boost::this_thread::sleep(boost::posix_time::milliseconds(d_period_ms)); // Controls rate at which periodic checks are done
		if(d_finished) {
			return;
		}

		// Scope mutex
		{
			gr::thread::scoped_lock guard(d_mutex);

			// Broadcast packet check
			struct timeval now;
			gettimeofday(&now, NULL);
			if ( ((last_tx_time.tv_sec == 0) && (last_tx_time.tv_usec == 0))|| ((now.tv_sec + (now.tv_usec / 1000000.0)) - (last_tx_time.tv_sec
					+ (last_tx_time.tv_usec / 1000000.0)) >= broadcast_interval))
				//if (self.broadcast_interval > 0) and (self.last_tx_time is None or (time.time() - self.last_tx_time) >= self.broadcast_interval):
			{
				// Send broadcast discovery packet
				mpdu_struct mac_frame_discovery = create_mac_frame(NULL,
						0, broadcast_address, ++m_seq_count, 0, 2, 2, 0);

				// Send discovery packet
				send_mpdu(mac_frame_discovery);
			}

			// Update CSMA variables and states
			
			// CCA channel state
			if (cca_channel_state == CCA_CHANNEL_BUSY)
			{
				if ( ((now.tv_sec + (now.tv_usec / 1000000.0)) - (time_cca_busy.tv_sec
					+ (time_cca_busy.tv_usec / 1000000.0)) >= timeout_cca_busy))
				{
					timeout_cca_busy = 0.0;
					cca_channel_state = CCA_CHANNEL_IDLE;					
				}
			}
			
			// CSMA transmit
			if (csma_tx_state == TX_WAIT)
			{
				if ( ((now.tv_sec + (now.tv_usec / 1000000.0)) - (time_csma_tx_wait.tv_sec
					+ (time_csma_tx_wait.tv_usec / 1000000.0)) >= timeout_csma_tx_wait))
				{
					timeout_csma_tx_wait = 0.0;
					if (cca_channel_state == CCA_CHANNEL_IDLE)
					{
						csma_tx_state = TX_PROCEED;
					}
					else
					{
						gettimeofday(&time_csma_tx_defer, NULL);
						timeout_csma_tx_defer = timeout_difs * (1.0 + 0.5 * (double) (rand() % 1000) / 1000.0); // at most another 50% of DIFS
						csma_tx_state = TX_DEFER;
					}
				}
			}
			else if (csma_tx_state == TX_DEFER)
			{
				if ( ((now.tv_sec + (now.tv_usec / 1000000.0)) - (time_csma_tx_defer.tv_sec
					+ (time_csma_tx_defer.tv_usec / 1000000.0)) >= timeout_csma_tx_defer))
				{
					timeout_csma_tx_defer = 0.0;
					if (cca_channel_state == CCA_CHANNEL_IDLE)
					{
						csma_tx_state = TX_PROCEED;
					}
					else
					{						
						csma_tx_state = TX_DEFER; // increase contention window
					}
				}
			}
			else if (csma_tx_state == TX_PROCEED)
			{
			}
			
			// Check nodes
			check_nodes();
			
			// Check message queue
			check_message_queue();
			
			
		}
	}
}

/* 
 * Form a MAC PDU from upper layer message
 */
void cognitiva_mac_impl::make_mpdu(pmt::pmt_t msg) {
	gr::thread::scoped_lock guard(d_mutex);

	pmt::pmt_t blob;
	if (pmt::is_eof_object(msg)) {
		std::cout << "MAC: exiting" << std::endl;
		//detail().get()->set_done(true);
		return;
	} else if (pmt::is_blob(msg)) {
		blob = msg;
	} else if (pmt::is_pair(msg)) {
		blob = pmt::cdr(msg);
	} else {
		std::cout << "MAC: unknown input" << std::endl;
		return;
	}

	if (d_debug_level & DEBUG_INFO)
		std::cout << "\nMAC: received new message" << std::endl;

	if (d_debug_level & DEBUG_VERBOSE) {
		std::cout << "message length " << pmt::blob_length(blob) << std::endl;
	}

	if (pmt::blob_length(blob) > MAX_MAC_PAYLOAD_LEN)
	{
	  std::cout <<  "ERROR: Message too large. MAC message received, len: " << pmt::blob_length(blob) << std::endl;
	  return;
	}

	{
		if(node_list.size() > 0)
		{
			bool frame_ok;
			for (std::map<MacAddress, Node>::iterator it = node_list.begin(); it != node_list.end(); it++)
			{
				// Create MAC frame
				mpdu_struct mac_frame = create_mac_frame(
						(const uint8_t*) pmt::blob_data(blob), pmt::blob_length(blob),
						it->first, ++m_seq_count, 0, 0, 0, use_arq ? 1 : 0);
				frame_ok = true;
				
				if (only_send_if_alive)
				{
					// Check if destination is known and communicating
					if (compare_address(mac_frame.dest_address, broadcast_address.bytes()))
					{
						if (!node_list.count(MacAddress(mac_frame.dest_address)))
						{
							if (d_debug_level & DEBUG_INFO)
								std::cout << "Not sending packet to " << MacAddress::tobytestring(mac_frame.dest_address) << ", " << MacAddress::tostring(mac_frame.dest_address) << " as it hasn't been seen yet\n";
							frame_ok = false;							
						}
						else 
						{
							if (!node_list[MacAddress(mac_frame.dest_address)].alive)
							{
								if (d_debug_level & DEBUG_INFO )
									std::cout << "Not sending packet to " << MacAddress::tobytestring(mac_frame.dest_address) << ", " << MacAddress::tostring(mac_frame.dest_address) << " as it isn't communicating\n";
								frame_ok = false;
							}
							else
							{
								if (d_debug_level & DEBUG_INFO )
									std::cout << "Sending packet to known communicating node" << "\n";
							}
						}
					}
				}
				
				// Add frame to message queue	
				if (frame_ok)
				      msg_queue.push_back(mac_frame);
				else
				      m_seq_count--;
			}
		}
		else
		{
			 if (d_debug_level & DEBUG_INFO)
				std::cout << "No neighbour nodes ***" << std::endl;
		 }
	}
	
	if (d_debug_level & DEBUG_VERBOSE)
		std::cout << "queue length: " << msg_queue.size() << std::endl;
}

/* 
 * Construct a MAC frame given data payload and other MAC frame parameters
 */
mpdu_struct cognitiva_mac_impl::create_mac_frame(const uint8_t *buf, int len,
		MacAddress dest_address, uint16_t seq_nr, uint8_t version, uint8_t type,
		uint8_t sub_type, uint8_t use_ARQ, uint8_t retransmission,
		uint8_t security, uint8_t coding, uint8_t reserved) {
	mpdu_struct mac_frame;

	// No ARQ for broadcast
	if (!compare_address(dest_address.bytes(), broadcast_address.bytes()))
		use_ARQ = false;

	// FCF
	mac_frame.version = version;
	mac_frame.type = type;
	mac_frame.sub_type = sub_type;
	mac_frame.use_ARQ = use_ARQ;
	mac_frame.retransmission = retransmission;
	mac_frame.security = security;
	mac_frame.coding = coding;
	mac_frame.reserved = reserved;
	
	mac_frame.FCF[0] = ((mac_frame.version & 3) << 6)
	| ((mac_frame.type & 3) << 4) | ((mac_frame.sub_type & 3) << 2)
	| ((mac_frame.use_ARQ & 1) << 1) | (mac_frame.retransmission & 1);
	mac_frame.FCF[1] = ((mac_frame.security & 3) << 6)
	| ((mac_frame.coding & 3) << 4) | (mac_frame.reserved & 15);

	// Sequence number
	mac_frame.seq_nr[0] = seq_nr & 0xFF;
	mac_frame.seq_nr[1] = seq_nr >> 8;

	// Address info
	std::memcpy(mac_frame.src_address, local_address.bytes(), MAC_ADDR_LEN);
	std::memcpy(mac_frame.dest_address, dest_address.bytes(), MAC_ADDR_LEN);
	std::memcpy(mac_frame.src_network_address, MacAddress("::").bytes(), MAC_ADDR_LEN); // all zero address
	std::memcpy(mac_frame.dest_network_address, MacAddress("::").bytes(), MAC_ADDR_LEN); // all zero address

	// Payload
	mac_frame.payload_len = len;
	std::memcpy(mac_frame.payload, buf, len);

	// CRC32
	uint32_t data_crc = gr::digital::crc32(
			reinterpret_cast<const unsigned char*>(&mac_frame),
			len + MAC_FCF_LEN + MAC_SEQ_LEN + 4 * MAC_ADDR_LEN);
	mac_frame.FCS[0] = data_crc & 0xFF;
	mac_frame.FCS[1] = data_crc >> 8;
	mac_frame.FCS[2] = data_crc >> 16;
	mac_frame.FCS[3] = data_crc >> 24;
	
	return mac_frame;
}

/*
 * Send a MAC frame to PHY layer to be transmitted
 */
void cognitiva_mac_impl::send_mpdu(mpdu_struct mac_frame) {
	// copy over bytes of mac frame
	std::memcpy(m_MPDU, mac_frame.FCF, MAC_FCF_LEN);
	std::memcpy(m_MPDU + MAC_FCF_LEN, mac_frame.seq_nr, MAC_SEQ_LEN);
	std::memcpy(m_MPDU + MAC_FCF_LEN + MAC_SEQ_LEN, mac_frame.src_address,
			MAC_ADDR_LEN);
	std::memcpy(m_MPDU + MAC_FCF_LEN + MAC_SEQ_LEN + MAC_ADDR_LEN,
			mac_frame.dest_address, MAC_ADDR_LEN);
	std::memcpy(m_MPDU + MAC_FCF_LEN + MAC_SEQ_LEN + 2 * MAC_ADDR_LEN,
			mac_frame.src_network_address, MAC_ADDR_LEN);
	std::memcpy(m_MPDU + MAC_FCF_LEN + MAC_SEQ_LEN + 3 * MAC_ADDR_LEN,
			mac_frame.dest_network_address, MAC_ADDR_LEN);
	std::memcpy(m_MPDU + MAC_FCF_LEN + MAC_SEQ_LEN + 4 * MAC_ADDR_LEN,
			mac_frame.payload, mac_frame.payload_len);
	std::memcpy(
			m_MPDU + MAC_FCF_LEN + MAC_SEQ_LEN + 4 * MAC_ADDR_LEN
			+ mac_frame.payload_len, mac_frame.FCS, MAC_FCS_LEN);
	m_MPDU_len = MAC_FCF_LEN + MAC_SEQ_LEN + 4 * MAC_ADDR_LEN
			+ mac_frame.payload_len + MAC_FCS_LEN;

	// send message
	message_port_pub(pmt::mp("mpdu_out"),
			pmt::cons(pmt::PMT_NIL, pmt::make_blob(m_MPDU, m_MPDU_len)));

	// Update time of last mpdu sent
	gettimeofday(&last_tx_time, NULL);	
}

/*
 * Get message at front of message queue and send it to PHY layer
 */
void cognitiva_mac_impl::send_mpdu_from_queue(bool empty) {
	// get message at front of queue
	mpdu_struct mac_frame = msg_queue.front();
	send_mpdu(mac_frame);
}

/* 
 * Read a MAC layer frame
 */
void cognitiva_mac_impl::read_mpdu(pmt::pmt_t msg) {
	gr::thread::scoped_lock guard(d_mutex);

	mpdu_struct mac_frame;
	pmt::pmt_t blob;

	if (pmt::is_eof_object(msg)) {
		message_port_pub(pmt::mp("payload_out"), pmt::PMT_EOF);
		//detail().get()->set_done(true);
		return;
	} else if (pmt::is_pair(msg)) {
		blob = pmt::cdr(msg);
	} else {
		assert(false);
	}

	int data_len = pmt::blob_length(blob);
	if (data_len < MAC_FCF_LEN + MAC_SEQ_LEN + 4 * MAC_ADDR_LEN + MAC_FCS_LEN) {
		std::cout << "MAC: frame too short. Dropping!" << std::endl;
		return;
	}

	if (d_debug_level & DEBUG_INFO)
		std::cout << "\nReceived MPDU, length: " << data_len << std::endl;

	const uint8_t* buf = (const uint8_t*) pmt::blob_data(blob);
	std::memcpy(mac_frame.FCS, buf + data_len - MAC_FCS_LEN, MAC_FCS_LEN);
	uint32_t data_crc_sent = (mac_frame.FCS[0] & 0xFF) | (mac_frame.FCS[1] << 8)
											| (mac_frame.FCS[2] << 16) | (mac_frame.FCS[3] << 24);
	uint32_t data_crc = gr::digital::crc32((const unsigned char*) buf,
			data_len - MAC_FCS_LEN);

	if (data_crc != data_crc_sent) {
		std::cout << "MAC: wrong crc. Dropping packet!" << std::endl;
		std::cout << "CRC32 calc: " << data_crc << ", CRC32 recv: "
				<< data_crc_sent << std::endl;
		return;
	}

	uint16_t payload_len = data_len
			- (MAC_FCF_LEN + MAC_SEQ_LEN + 4 * MAC_ADDR_LEN + MAC_FCS_LEN);
	std::memcpy(mac_frame.FCF, buf, MAC_FCF_LEN);
	std::memcpy(mac_frame.seq_nr, buf + MAC_FCF_LEN, MAC_SEQ_LEN);
	std::memcpy(mac_frame.src_address, buf + MAC_FCF_LEN + MAC_SEQ_LEN,
			MAC_ADDR_LEN);
	std::memcpy(mac_frame.dest_address,
			buf + MAC_FCF_LEN + MAC_SEQ_LEN + MAC_ADDR_LEN, MAC_ADDR_LEN);
	std::memcpy(mac_frame.src_network_address,
			buf + MAC_FCF_LEN + MAC_SEQ_LEN + 2 * MAC_ADDR_LEN, MAC_ADDR_LEN);
	std::memcpy(mac_frame.dest_network_address,
			buf + MAC_FCF_LEN + MAC_SEQ_LEN + 3 * MAC_ADDR_LEN, MAC_ADDR_LEN);
	std::memcpy(mac_frame.payload,
			buf + MAC_FCF_LEN + MAC_SEQ_LEN + 4 * MAC_ADDR_LEN, payload_len);
	mac_frame.payload_len = payload_len;

	// FFC fields
	mac_frame.version = ((mac_frame.FCF[0] >> 6) & 3);
	mac_frame.type = ((mac_frame.FCF[0] >> 4) & 3);
	mac_frame.sub_type = ((mac_frame.FCF[0] >> 2) & 3);
	mac_frame.use_ARQ = ((mac_frame.FCF[0] >> 1) & 1);
	mac_frame.retransmission = ((mac_frame.FCF[0]) & 1);
	mac_frame.security = ((mac_frame.FCF[1] >> 6) & 3);
	mac_frame.coding = ((mac_frame.FCF[1] >> 4) & 3);
	mac_frame.reserved = ((mac_frame.FCF[1]) & 15);

	// Process the MAC frame
	process_mac_frame(mac_frame);
}

/*
 * Read a CCA message used to determine state of channel (BUSY, IDLE)
 */
void cognitiva_mac_impl::read_cca(pmt::pmt_t msg)
{
	gr::thread::scoped_lock guard(d_mutex);
	
	if (d_debug_level & DEBUG_INFO)
		std::cout << "Got CCA message:" << pmt::length(msg) <<std::endl;

	if ((pmt::length(msg) == 3) && pmt::is_tuple(msg))
	{
		pmt::pmt_t msg_param1 = pmt::tuple_ref(msg, 0);
		pmt::pmt_t msg_param2 = pmt::tuple_ref(msg, 1);
		pmt::pmt_t msg_param3 = pmt::tuple_ref(msg, 2);

		if (d_debug_level & DEBUG_VERBOSE)
		{	
			std::cout << "Msg param 1: " << msg_param1 << std::endl;
			std::cout << "Msg param 2: " << msg_param2 << std::endl;
			//std::cout << "Msg param 3: " << msg_param3 << std::endl;
		}
		
		if (pmt::symbol_to_string(msg_param1) == "cca")
		{
			if (d_debug_level & DEBUG_VERBOSE)	
				std::cout << "CCA message param 1 correct" << std::endl;
			if (pmt::symbol_to_string(msg_param2) == "ed")
			{
				if (d_debug_level & DEBUG_INFO)	
					std::cout << "Got CCA ED message" << std::endl;

				if ((d_cca_mode == MODE_ED) || (d_cca_mode == MODE_ED_CS))
				{
					if (pmt::is_real(msg_param3))
					{
						double pow_avg = pmt::to_double(msg_param3);
						
						if (d_debug_level & DEBUG_PERFORMANCE)	
							std::cout << "CCA ED avg pow, threshold: " << pow_avg <<  ", " << cca_ed_threshold << std::endl;
					
						if (pow_avg > cca_ed_threshold)
						{
							if (cca_channel_state != CCA_CHANNEL_BUSY)
							{
								cca_channel_state = CCA_CHANNEL_BUSY;
								gettimeofday(&time_cca_busy, NULL);
								
								timeout_cca_busy = timeout_difs * (1.0 + 0.5 * (double) (rand() % 1000) / 1000.0); // at most another 50% of DIFS 
								if (d_debug_level & DEBUG_PERFORMANCE)	
									std::cout << "Set CCA timeout: " << timeout_cca_busy << std::endl; //**
							}
							else
							{
								if (d_debug_level & DEBUG_PERFORMANCE)	
								{
									struct timeval now;
									gettimeofday(&now, NULL);

									float cca_elapsed = (now.tv_sec + (now.tv_usec / 1000000.0)) - (time_cca_busy.tv_sec + (time_cca_busy.tv_usec / 1000000.0));
									std::cout << "Elapsed CCA time : " << cca_elapsed << " of " << timeout_cca_busy << std::endl; //**
								}
							}
						}
						else
						{
							if (d_debug_level & DEBUG_PERFORMANCE)	
								std::cout << "CCA ED avg pow below threshold"<< std::endl;
						}
					}
					else
					{
						std::cout << "*** CCA ED 3rd parameter in incorrect format" << std::endl;
					}
					
					// Continue spectrum sensing for energy detection
					pmt::pmt_t command = pmt::make_tuple(
					pmt::mp("spectrum_sense"),
					pmt::mp("continue"));
					message_port_pub(pmt::mp("control_out"), command);		
					if (d_debug_level & DEBUG_INFO)	
						std::cout << "Sent SS continue command" << std::endl;
				}
			}
			else if (pmt::symbol_to_string(msg_param2) == "cs")
			{
				if (d_debug_level & DEBUG_INFO)	
					std::cout << "Got CCA CS message" << std::endl;
					
				if ((d_cca_mode == MODE_CS) || (d_cca_mode == MODE_ED_CS))
				{
					if(cca_channel_state != CCA_CHANNEL_BUSY)
					{
						cca_channel_state = CCA_CHANNEL_BUSY;
						gettimeofday(&time_cca_busy, NULL);
						
						timeout_cca_busy = timeout_difs * (1.0 + 0.5 * (double) (rand() % 1000) / 1000.0); // at most another 50% of DIFS
						if (d_debug_level & DEBUG_PERFORMANCE)	
							std::cout << "Set CCA timeout: " << timeout_cca_busy << std::endl; //**
					}
					else
					{
						if (d_debug_level & DEBUG_PERFORMANCE)	
						{
							struct timeval now;
							gettimeofday(&now, NULL);

							float cca_elapsed = (now.tv_sec + (now.tv_usec / 1000000.0)) - (time_cca_busy.tv_sec + (time_cca_busy.tv_usec / 1000000.0));
							std::cout << "Elapsed CCA time : " << cca_elapsed << " of " << timeout_cca_busy << std::endl; //**
						}
					}
					
				}
			}
		}
		else if (pmt::symbol_to_string(msg_param1) == "freq")
		{
			// if (d_debug_level & DEBUG_INFO)	
				//std::cout << "*** Got spectrum sensing message" << std::endl;
		}
	}
	else
	{		
		std::cout << "*** Received CCA message in incorrect format\n";
	}
}

/*
 * Check for messages in message queue to send to PHY layer for transmision
 * and performs ARQ
 */
void cognitiva_mac_impl::check_message_queue() {
	if (msg_queue.size() > 0)
	{
		if (cca_channel_state == CCA_CHANNEL_IDLE) // Only send if CCA state reports idle channel
		{
			if (csma_tx_state == TX_INIT)
			{
				gettimeofday(&time_csma_tx_wait, NULL);
				timeout_csma_tx_wait = timeout_difs * (1.0 + 0.5 * (double) (rand() % 1000) / 1000.0); // at most another 50% of DIFS
				csma_tx_state = TX_WAIT;				
			}
			else if (csma_tx_state == TX_WAIT)
			{}
			else if (csma_tx_state == TX_DEFER)
			{}
			else if (csma_tx_state == TX_PROCEED)
			{
				mpdu_struct mac_frame = msg_queue.front();
				if ((mac_frame.FCF[0] >> 1) & 1) // use ARQ for frame
				{
					perform_message_arq(mac_frame);				
				} else {
					send_mpdu_from_queue();
					msg_queue.pop_front();
				}
				csma_tx_state = TX_INIT;
			}			
		}
		else // Channel busy, defer sending
		{
			if ((csma_tx_state == TX_INIT) || (csma_tx_state == TX_PROCEED))
			{
				gettimeofday(&time_csma_tx_defer, NULL);
				timeout_csma_tx_defer = timeout_difs * (1.0 + 0.5 * (double) (rand() % 1000) / 1000.0); // at most another 50% of DIFS
				csma_tx_state = TX_DEFER;
			}
		}
	}
}

/*
 * Perform ARQ on message for delivery reliability
 */
void cognitiva_mac_impl::perform_message_arq(mpdu_struct mac_frame)
{
	if (arq_channel_state == ARQ_CHANNEL_IDLE) {
		expected_arq_id = ((mac_frame.seq_nr[0] & 0xFF)
				| ((mac_frame.seq_nr[1] << 8)));
		gettimeofday(&time_of_tx, NULL);
		send_mpdu_from_queue();
		arq_channel_state = ARQ_CHANNEL_BUSY;		
		arq_pkts_txed++;
		total_arq++;
		retries = 0;
		next_random_backoff_percentage = backoff_randomness
				* (double) (rand() % 1000) / 1000.0;
	} else {
		if (exp_backoff == ARQ_BACKOFF_EXPONENTIAL) {
			backedoff_timeout = timeout * pow(2, retries);
			if (d_debug_level & DEBUG_INFO)
				std::cout << "*** timeout: " << backedoff_timeout
				<< std::endl;
		} else {
			backedoff_timeout = timeout * (retries + 1);
			if (d_debug_level & DEBUG_INFO)
				std::cout << "*** timeout: " << backedoff_timeout
				<< std::endl;
		}
		backedoff_timeout *= (1.0 + next_random_backoff_percentage);
		if (d_debug_level & DEBUG_INFO)
			std::cout << ">>> timeout: " << backedoff_timeout << std::endl;

		struct timeval now;
		gettimeofday(&now, NULL);

		// Check elapsed time
		if ( (now.tv_sec + (now.tv_usec / 1000000.0)) - (time_of_tx.tv_sec
				+ (time_of_tx.tv_usec / 1000000.0)) > backedoff_timeout) { // ARQ timeout reached
			if (retries == max_attempts) { // Maximum attempts made, packet failed
				mpdu_struct mac_frame = msg_queue.front();
				MacAddress addr = MacAddress(mac_frame.dest_address);

				if (d_debug_level & DEBUG_VERBOSE)
					std::cout << "Destination address "
					<< node_list[addr].id.tobytestring() << ", "
					<< node_list[addr].id.tostring()
					<< " ARQ failed after " << max_attempts
					<< " attempts\n";
				retries = 0;
				arq_channel_state = ARQ_CHANNEL_IDLE;
				failed_arq++;
				if (expire_on_arq_failure) {
					node_list[addr].expire();
					if (d_debug_level & DEBUG_VERBOSE)
						std::cout << "Expired node "
						<< node_list[addr].id.tobytestring() << ", "
						<< node_list[addr].id.tostring()
						<< std::endl;
				}

				// remove unacknowledged message from the message queue
				msg_queue.pop_front();
				
				print_arq_stats();
			} else { // Resend the packet
				mpdu_struct mac_frame = msg_queue.front();
				MacAddress addr = MacAddress(mac_frame.dest_address);

				if (d_debug_level & DEBUG_VERBOSE)
					std::cout << "Destination address "
					<< node_list[addr].id.tobytestring() << ", "
					<< node_list[addr].id.tostring()
					<< " ARQ failed after " << retries
					<< " attempts and "
					<< (now.tv_sec + (now.tv_usec / 1000000.0))
					- (time_of_tx.tv_sec + (time_of_tx.tv_usec / 1000000.0)) 
					<< "s, retrying\n";

				retries++;
				gettimeofday(&time_of_tx, NULL);
				send_mpdu_from_queue();				
				next_random_backoff_percentage = backoff_randomness
						* (double) (rand() % 1000) / 1000.0;
				arq_retxed++;
				total_arq++;
				
				print_arq_stats();
			}
		}
		else // ARQ timer not yet timed out, continue waiting before attempting to transmit again
		{
		}
	}	
	
}

/* 
 * Process a MAC frame received from PHY layer
 */
void cognitiva_mac_impl::process_mac_frame(mpdu_struct mac_frame) {
	if (d_debug_level & DEBUG_INFO)
		print_mac_frame(mac_frame);

	uint16_t received_seq_nr = ((mac_frame.seq_nr[0] & 0xFF)
			| ((mac_frame.seq_nr[1] << 8)));

	// Check address
	if (compare_address(mac_frame.src_address, local_address.bytes())) {
		MacAddress mac_src = MacAddress(mac_frame.src_address);
		Node node = Node(mac_src);

		if (d_debug_level & DEBUG_INFO)
			std::cout << "Received packet from other node\n";

		if (node_list.count(mac_src)) {
			if (d_debug_level & DEBUG_INFO)
				std::cout << "Received packet from known node\n";
			node = node_list[mac_src];
		} else {
			if (d_debug_level & DEBUG_VERBOSE)
				std::cout << "Received packet from previously unknown node\n";
			//node = Node(mac_src);
			node_list[mac_src] = node;
		}

		struct timeval now;
		gettimeofday(&now, NULL);
		if (node_list[mac_src].update(now)) {
			if (d_debug_level & DEBUG_VERBOSE)
				std::cout << "Node " << node.id.tobytestring() << ", "
				<< node.id.tostring() << " alive\n";
		}

		// check if packet is for this node
		if (!compare_address(mac_frame.dest_address, local_address.bytes()) || // unicast
				!compare_address(mac_frame.dest_address,
						broadcast_address.bytes()) // broadcast
		) {
			if (d_debug_level & DEBUG_INFO)
				std::cout << "Packet destined for this node\n";

			// check mac version compatibility
			if (d_mac_version == mac_frame.version) {
				// check mac frame type

				// data frame
				if (mac_frame.type == 0) {
					// data
					if (mac_frame.sub_type == 0) {
						// check if ACK is requested
						if (mac_frame.use_ARQ) {
							if (d_debug_level & DEBUG_INFO)
								std::cout << "ARQ requested\n";

							// Create MAC frame for reply
							mpdu_struct mac_frame_reply = create_mac_frame(NULL,
									0, mac_src, received_seq_nr, 0, 1, 0, 0);

							// Send ACK
							send_mpdu(mac_frame_reply);

							if (d_debug_level & DEBUG_INFO)
								std::cout << "ACK sent " << std::endl;
						} else {
							if (d_debug_level & DEBUG_VERBOSE)
								std::cout << "No ARQ requested\n";
						}

						// Send data payload to upper layers
						pmt::pmt_t mac_payload = pmt::make_blob(
								mac_frame.payload, mac_frame.payload_len);
						message_port_pub(pmt::mp("payload_out"),
								pmt::cons(pmt::PMT_NIL, mac_payload));
					} else {
						if (d_debug_level & DEBUG_VERBOSE)
							std::cout << "Unrecognised data frame sub-type\n";
					}
				}
				// control frame
				else if (mac_frame.type == 1) {
					// ACK
					if (mac_frame.sub_type == 0) {
						if (arq_channel_state == ARQ_CHANNEL_IDLE) {
							if (d_debug_level & DEBUG_VERBOSE)
								std::cout << "Received ACK while idle: "
								<< received_seq_nr << std::endl;
						} else {
							if (expected_arq_id == received_seq_nr) {
								msg_queue.pop_front();
								succeeded_arq++;
								arq_channel_state = ARQ_CHANNEL_IDLE;

								struct timeval now;
								gettimeofday(&now, NULL);
								double delay = ((now.tv_sec
										+ (now.tv_usec / 1000000.0))
										- (time_of_tx.tv_sec
												+ (time_of_tx.tv_usec / 1000000.0)));
								//**
								if (d_debug_level & DEBUG_PERFORMANCE)
									std::cout << "ACK " << expected_arq_id << " received, time taken: "
									<< delay << std::endl;
								print_arq_stats();
							} else {
								// Out of seuqence ACK, stop--and-wait ARQ implemented for now
								if (d_debug_level & DEBUG_INFO) {
									std::cout
									<< "Received out of sequence ACK, expected "
									<< expected_arq_id << "got "
									<< received_seq_nr << std::endl;
								}								
							}
						}
					}
					// RTS
					else if (mac_frame.sub_type == 1) {
					}
					// CTS
					else if (mac_frame.sub_type == 2) {
					} else {
						if (d_debug_level & DEBUG_VERBOSE)
							std::cout
							<< "Unrecognised control frame sub-type\n";
					}
				}
				// management frame
				else if (mac_frame.type == 2) {
					// Power control
					if (mac_frame.sub_type == 0) {
					}
					// Sensing
					else if (mac_frame.sub_type == 1) {
					}
					// Discovery
					else if (mac_frame.sub_type == 2) {
						// Reply to discovery packet if it was send as broadcast packet
						if(!compare_address(mac_frame.dest_address, broadcast_address.bytes()))
						{
							if (d_debug_level & DEBUG_VERBOSE)
								std::cout << "Broadcast DISCOVERY packet received from node : " << node.id.tobytestring() << ", " << node.id.tostring() << std::endl;

							mpdu_struct mac_frame_discovery = create_mac_frame(NULL, 0, mac_src, received_seq_nr, 0, 2, 2, 0);

							// Send discovery packet
							send_mpdu(mac_frame_discovery);

							if (d_debug_level & DEBUG_VERBOSE)
								std::cout << "Sent unicast DISCOVERY packet to node : " << node.id.tobytestring() << ", " << node.id.tostring() << std::endl;

						}
						else
						{
							if (d_debug_level & DEBUG_VERBOSE)
								std::cout << "Unicast DISCOVERY packet received from node : " << node.id.tobytestring() << ", " << node.id.tostring() << std::endl;
						}
					} else {
						if (d_debug_level & DEBUG_VERBOSE)
							std::cout
							<< "Unrecognised management frame sub-type\n";
					}
				} else {
					if (d_debug_level & DEBUG_VERBOSE)
						std::cout << "MAC frame type unrecognised\n";
				}
			} else {
				if (d_debug_level & DEBUG_VERBOSE)
					std::cout << "MAC version mismatch\n";
			}
		} else {
			if (d_debug_level & DEBUG_VERBOSE)
				std::cout << "Packet destined for another node\n";
		}
	} else {
		if (d_debug_level & DEBUG_VERBOSE)
			std::cout << "Received own packet\n";
	}
}

/* 
 * Perform checks on nodes in the network to determine their status
 */
void cognitiva_mac_impl::check_nodes()
{
	if(node_list.size() > 0)
	{
		struct timeval now;
		gettimeofday(&now, NULL);

		for (std::map<MacAddress, Node>::iterator it = node_list.begin(); it != node_list.end(); it++)
		{
			if (it->second.alive)
			{
				double diff = (now.tv_sec + (now.tv_usec / 1000000.0)) - (it->second.last_heard.tv_sec + (it->second.last_heard.tv_usec / 1000000.0));
				if (diff > node_expiry_delay)
				{
					it->second.expire();
					if (d_debug_level & DEBUG_VERBOSE)
						std::cout<< "Node " << it->second.id.tobytestring() << ", " << it->second.id.tostring() << " disappeared\n";
				}
			}
		}
	}
}

/* 
 * Helper function to print out ARQ stats
 */
void cognitiva_mac_impl::print_arq_stats() {
	if (d_debug_level & DEBUG_PERFORMANCE)
		std::cout << "ARQ stats " << local_address.tostring() << ", Packet count: " << arq_pkts_txed << ", Transmissions: " << total_arq << ", Success: " << succeeded_arq << ", Failed: " << failed_arq << ",  Retransmitted: " << arq_retxed << ", Oustanding: " << ((arq_channel_state == ARQ_CHANNEL_IDLE) ? 0 : 1) << std::endl;
}

/* 
 * Helper function to print out the fields of a MAC frame
 */
void cognitiva_mac_impl::print_mac_frame(mpdu_struct mac_frame) {
	std::cout << std::endl;	
	std::cout << "~~~ MAC PDU Fields ~~~\n";
	
	// FCF
	std::cout << "Frame control field: " << std::endl;
	std::cout << "\tversion: " << ((mac_frame.FCF[0] >> 6) & 3) << std::endl;
	std::cout << "\tframe type: " << ((mac_frame.FCF[0] >> 4) & 3) << std::endl;
	std::cout << "\tframe sub-type: " << ((mac_frame.FCF[0] >> 2) & 3)
											<< std::endl;
	std::cout << "\tuse ARQ: " << ((mac_frame.FCF[0] >> 1) & 1) << std::endl;
	std::cout << "\tretransmission: " << ((mac_frame.FCF[0]) & 1) << std::endl;
	std::cout << "\tsecurity: " << ((mac_frame.FCF[1] >> 6) & 3) << std::endl;
	std::cout << "\tcode: " << ((mac_frame.FCF[1] >> 4) & 3) << std::endl;
	std::cout << "\treserved: " << ((mac_frame.FCF[1]) & 15) << std::endl;
	
	// Sequence number
	std::cout << "Sequence number: "
			<< ((mac_frame.seq_nr[0] & 0xFF) | ((mac_frame.seq_nr[1] << 8)))
			<< std::endl;
	
	// Source address
	std::cout << "Source address:\n\t";
	for (int i = 0; i < MAC_ADDR_LEN; i++) {
		printf("%02x ", mac_frame.src_address[i]);
	}
	MacAddress mac_src = MacAddress(mac_frame.src_address);
	std::cout << "\n\t" << mac_src.tostring() << std::endl;
	
	// Destination address
	std::cout << "Destination address:\n\t";
	for (int i = 0; i < MAC_ADDR_LEN; i++) {
		printf("%02x ", mac_frame.dest_address[i]);
	}
	MacAddress mac_dest = MacAddress(mac_frame.dest_address);
	std::cout << "\n\t" << mac_dest.tostring() << std::endl;
	
	// Source network address
	std::cout << "Source network address:\n\t";
	for (int i = 0; i < MAC_ADDR_LEN; i++) {
		printf("%02x ", mac_frame.src_network_address[i]);
	}
	MacAddress mac_src_network = MacAddress(mac_frame.src_network_address);
	std::cout << "\n\t" << mac_src_network.tostring() << std::endl;
	
	// Destination network address
	std::cout << "Destination network address:\n\t";
	for (int i = 0; i < MAC_ADDR_LEN; i++) {
		printf("%02x ", mac_frame.dest_network_address[i]);
	}
	MacAddress mac_dest_network = MacAddress(mac_frame.dest_network_address);
	std::cout << "\n\t" << mac_dest_network.tostring() << std::endl;
	
	//std::cout << "Data payload: " << mac_frame.payload << std::endl;
	std::cout << "Payload length: " << mac_frame.payload_len << std::endl;
	std::cout << "Frame check sequence: "
			<< (uint32_t)(
					(mac_frame.FCS[0] & 0xFF) | (mac_frame.FCS[1] << 8)
					| (mac_frame.FCS[2] << 16)
					| (mac_frame.FCS[3] << 24)) << std::endl
					<< std::endl;
}

/* 
 * Function to compare two MAC addresses
 */
int cognitiva_mac_impl::compare_address(const uint8_t* address_1,
		const uint8_t* address_2) {
	return std::memcmp(address_1, address_2, MAC_ADDR_LEN);
}

} /* namespace cognitiva */
} /* namespace gr */
