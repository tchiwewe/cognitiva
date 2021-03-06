#ifndef INCLUDED_COGNITIVA_PN_SEQUENCE_H
#define INCLUDED_COGNITIVA_PN_SEQUENCE_H

namespace gr {
namespace cognitiva {

// 802.15.4 PN sequence
static const unsigned int PN_SEQUENCE_V0_0[] =  {
	3653456430, 
	3986437410, 
	786023250,
	585997365,
	1378802115, 
	891481500, 
	3276943065, 
	2620728045, 
	2358642555,
	3100205175, 
	2072811015, 
	2008598880, 
	125537430, 
	1618458825, 
	2517072780,
	3378542520
	};

// PN sequence for decoding 802.15.4 with O-QPSK
static const unsigned int PN_SEQUENCE_V0_1[] =  {
	1618456172,
	1309113062,
	1826650030,
	1724778362,
	778887287,
	2061946375,
	2007919840,
	125494990,
	529027475,
	838370585,
	320833617,
	422705285,
	1368596360,
	85537272,
	139563807,
	2021988657
	};

// Custom generated Kasami sequence for Cognitiva
static const unsigned int PN_SEQUENCE_V1[] =  {
	471289783,
	151029956,
	2916173109,
	202904059,
	3754827888,
	1371244189,
	3336916267,
	2844461333,
	120285084,
	265660717,
	2979836007,
	2515104994,
	2049276677,
	3328322954,
	1870384081,
	1693262953
	};
} // namespace cognitiva
} // namespace gr

#endif /* INCLUDED_COGNITIVA_PN_SEQUENCE_H */
