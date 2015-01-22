/* -*- c++ -*- */

#define COGNITIVA_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "cognitiva_swig_doc.i"

%{
#include "cognitiva/cognitiva_mac.h"
#include "cognitiva/cognitiva_phy_ppdu.h"
#include "cognitiva/bits_to_chips.h"
#include "cognitiva/packet_sink.h"
#include "cognitiva/burst_tagger.h"
#include "cognitiva/chips_to_bits.h"
#include "cognitiva/bits_to_chips_new.h"
#include "cognitiva/bin_statistics.h"
%}

%include "cognitiva/cognitiva_mac.h"
GR_SWIG_BLOCK_MAGIC2(cognitiva, cognitiva_mac);
%include "cognitiva/cognitiva_phy_ppdu.h"
GR_SWIG_BLOCK_MAGIC2(cognitiva, cognitiva_phy_ppdu);

%include "cognitiva/bits_to_chips.h"
GR_SWIG_BLOCK_MAGIC2(cognitiva, bits_to_chips);
%include "cognitiva/packet_sink.h"
GR_SWIG_BLOCK_MAGIC2(cognitiva, packet_sink);
%include "cognitiva/burst_tagger.h"
GR_SWIG_BLOCK_MAGIC2(cognitiva, burst_tagger);
%include "cognitiva/chips_to_bits.h"
GR_SWIG_BLOCK_MAGIC2(cognitiva, chips_to_bits);
%include "cognitiva/bits_to_chips_new.h"
GR_SWIG_BLOCK_MAGIC2(cognitiva, bits_to_chips_new);

%include "cognitiva/bin_statistics.h"
GR_SWIG_BLOCK_MAGIC2(cognitiva, bin_statistics);
