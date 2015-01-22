#ifndef INCLUDED_COGNITIVA_PACKET_UTILS_H
#define INCLUDED_COGNITIVA_PACKET_UTILS_H

#define MAX_PPDU_LEN 2048
#define PPDU_OVERHEAD 10
#define MAC_ADDR_LEN 6
#define MAC_FCF_LEN 2
#define MAC_SEQ_LEN 2
#define MPDU_OVERHEAD 32
// Max MAC payload len = 2048 (MAX PPDU) - 10 (PPDU overhead) - 32 (MPDU overhead) = 2006 
#define MAX_MAC_PAYLOAD_LEN MAX_PPDU_LEN - PPDU_OVERHEAD - MPDU_OVERHEAD // 2006
#define MAC_FCS_LEN 4

namespace gr {
namespace cognitiva {
} // namespace cognitiva
} // namespace gr

#endif /* INCLUDED_COGNITIVA_PACKET_UTILS_H */
