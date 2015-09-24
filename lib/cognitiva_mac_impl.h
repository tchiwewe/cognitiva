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

#ifndef INCLUDED_COGNITIVA_COGNITIVA_MAC_IMPL_H
#define INCLUDED_COGNITIVA_COGNITIVA_MAC_IMPL_H

#include <cognitiva/cognitiva_mac.h>
#include <address_utils.h>
#include <time.h>
#include "packet_utils.h"

/* Packet types:
 * 	0: Data
 * 		Sub types:
 * 			0: Default
 * 	1: Control
 * 		Sub types:
 * 			0: ACK
 * 			1: RTS
 * 			2: CTS
 * 	2: Management
 * 		Sub types
 * 			0: Power control
 * 			1: Sensing
 * 			2: Discovery
 */

namespace gr {
namespace cognitiva {

// A node in the network
class Node {
public:
	Node() {
		id = MacAddress();
		//last_heard = 0;
		alive = false;
	}
	Node(const MacAddress id_) {
		id = id_;
		//last_heard = 0;
		alive = false;
	}
	bool update(struct timeval time) {
		was_alive = alive;
		alive = true;
		last_heard = time;
		return (was_alive == false) && (alive);
	}

	void expire() {
		alive = false;
	}
	MacAddress id;
	struct timeval last_heard;
	bool alive;
	bool was_alive;

private:
};

#pragma pack(push, 1)
typedef struct mpdu_struct_ {
	// MPDU fields
	uint8_t FCF[MAC_FCF_LEN];             		// Frame control field
	uint8_t seq_nr[MAC_SEQ_LEN];          		// Sequence number
	uint8_t src_address[MAC_ADDR_LEN];    		// Source address
	uint8_t dest_address[MAC_ADDR_LEN];   		// Destination address
	uint8_t src_network_address[MAC_ADDR_LEN]; 	// Source network address
	uint8_t dest_network_address[MAC_ADDR_LEN];	// Destination network address
	uint8_t payload[MAX_MAC_PAYLOAD_LEN]; 		// Data payload
	uint8_t FCS[MAC_FCS_LEN];             		// Frame check sequence

	// FCF Fields
	uint8_t version;            // version
	uint8_t type;               // frame type
	uint8_t sub_type;           // frame sub-type
	uint8_t use_ARQ;            // whether or not to use ARQ
	uint8_t retransmission;     // if frame is a retransmission
	uint8_t security;           // security type
	uint8_t coding;             // coding type      
	uint8_t reserved;           // reserved

	// Payload length
	uint16_t payload_len;
} mpdu_struct;
#pragma pack(pop)

class cognitiva_mac_impl: public cognitiva_mac {
public:
	cognitiva_mac_impl(const char *mac_address_, 
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
			unsigned int debug_level_);
	~cognitiva_mac_impl();
private:
	void make_mpdu(pmt::pmt_t msg);
	void read_mpdu(pmt::pmt_t msg);
	void read_cca(pmt::pmt_t msg);
	mpdu_struct create_mac_frame(const uint8_t *buf, int len,
			MacAddress dest_address, uint16_t seq_nr, uint8_t version,
			uint8_t type, uint8_t sub_type, uint8_t use_ARQ,
			uint8_t retransmission = 0, uint8_t security = 0,
			uint8_t coding = 0, uint8_t reserved = 0);
	void process_mac_frame(mpdu_struct mac_frame);
	void send_mpdu_from_queue(bool empty = false);
	void send_mpdu(mpdu_struct mac_frame);
	void check_message_queue();
	void perform_message_arq(mpdu_struct mac_frame);
	int compare_address(const uint8_t* address_1, const uint8_t* address_2);
	void check_nodes();
	void run();

	// MPDU
	uint8_t m_MPDU[MAX_MAC_PAYLOAD_LEN + MAC_FCF_LEN + MAC_SEQ_LEN
	               + 2 * MAC_ADDR_LEN + MAC_FCS_LEN];  // MAC protocol data unit       
	uint16_t m_seq_count;
	int m_MPDU_len;
	MacAddress local_address;    // Local address 
	MacAddress broadcast_address;

	// Version
	unsigned int d_mac_version;
	
	// CCA mode
	unsigned int d_cca_mode;

	// Debugging
	unsigned int d_debug_level;

	// Helper
	void print_mac_frame(mpdu_struct mac_frame);
	void print_arq_stats();

	// ARQ	
	bool use_arq;
	uint16_t expected_arq_id;       //arq id we're expected to get ack for      
	uint16_t arq_pkts_txed;             //how many arq packets we've transmitted
	uint16_t arq_retxed;                    //how many times we've retransmitted
	uint16_t failed_arq;
	uint16_t succeeded_arq;
	uint16_t total_arq;
	uint16_t max_attempts;
	uint16_t retries;
	float timeout;                            //arq timeout parameter
	float backoff_randomness;
	float next_random_backoff_percentage;
	float backedoff_timeout;	
	float node_expiry_delay;
	bool expire_on_arq_failure;
	bool only_send_if_alive;
	float broadcast_interval; //**
	struct timeval time_of_tx;                   //time of last arq transmission
	struct timeval last_tx_time;			// time of last transmission in general	
	enum {
		ARQ_CHANNEL_IDLE, ARQ_CHANNEL_BUSY
	} arq_channel_state;		
	enum {
		ARQ_BACKOFF_EXPONENTIAL, ARQ_BACKOFF_LINEAR
	} exp_backoff;

	// CCA
	enum cca_mode_t {
	MODE_NONE = 0, // Medium always considered idle
	MODE_ED = 1,   // Medium busy if energy detected above threshold
	MODE_CS = 2,   // Medium busy if carrier is sensed
	MODE_ED_CS = 3 // Medium is busy if energy is detected above threshold and carrier is sensed
	};
	enum {
		CCA_CHANNEL_IDLE, CCA_CHANNEL_BUSY
	} cca_channel_state;
	enum {
		TX_INIT, TX_WAIT, TX_DEFER, TX_PROCEED
	} csma_tx_state;
	float cca_ed_threshold;
	struct timeval time_cca_busy;	// time channel marked busy
	float timeout_cca_busy;			// timeout for channel busy state
	struct timeval time_csma_tx_wait;	// time transmitting wait started
	float timeout_csma_tx_wait;			// timeout for transmit wait
	struct timeval time_csma_tx_defer;	// time transmitting was deferred
	float timeout_csma_tx_defer;			// timeout for defering transmitting
	float timeout_difs;					// DIFS time, time to receive maximum length packet and an ACK
	
	// List of messages to send
	std::list<mpdu_struct> msg_queue;

	// List of known network nodes
	std::map<MacAddress, Node> node_list;

	// Threading
	boost::shared_ptr<boost::thread> d_thread;
	gr::thread::mutex d_mutex;
	bool d_finished;
	float d_period_ms;
};

} // namespace cognitiva
} // namespace gr

#endif /* INCLUDED_COGNITIVA_COGNITIVA_MAC_IMPL_H */
