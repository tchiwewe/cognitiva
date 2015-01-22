#ifndef INCLUDED_COGNITIVA_ADDRESS_UTILS_H
#define INCLUDED_COGNITIVA_ADDRESS_UTILS_H

#include "packet_utils.h"

// Handle IPv4 and IPv6 addresses
#include <arpa/inet.h>

// From http://lxr.free-electrons.com/source/net/core/utils.c#L189
// Linux/net/core/utils.c

namespace gr {
namespace cognitiva {

// Representation of MAC address
class MacAddress {
public:
	MacAddress() {
		sa_addr.sin6_addr = in6addr_loopback;
		std::memcpy(mac_address, sa_addr.sin6_addr.s6_addr + 16 - MAC_ADDR_LEN,
				MAC_ADDR_LEN);
	}

	MacAddress(uint8_t address[]) {
		sa_addr.sin6_addr = in6addr_any;
		std::memcpy(mac_address, address, MAC_ADDR_LEN);
		std::memcpy(sa_addr.sin6_addr.s6_addr + 16 - MAC_ADDR_LEN, mac_address,
				MAC_ADDR_LEN);
	}
	MacAddress(const char *addr) {
		sa_addr.sin6_addr = in6addr_loopback;
		int ok = inet_pton(AF_INET6, addr, &(sa_addr.sin6_addr));
		//std::cout << "*** ok: " << ok <<std::endl; 
		if (!ok) {
			std::cout << "Invalid MAC address given\n";
			//assert(false);
			//assert(true);
		}
		std::memcpy(mac_address, sa_addr.sin6_addr.s6_addr + 16 - MAC_ADDR_LEN,
				MAC_ADDR_LEN);
	}
	bool operator<(const MacAddress compare) const {
		return (std::memcmp(mac_address, compare.mac_address, MAC_ADDR_LEN) < 0);
	}
	bool operator>(const MacAddress compare) const {
		return (std::memcmp(mac_address, compare.mac_address, MAC_ADDR_LEN) > 0);
	}
	bool operator==(const MacAddress compare) const {
		return (std::memcmp(mac_address, compare.mac_address, MAC_ADDR_LEN) == 0);
	}
	bool operator()(const MacAddress lhs, const MacAddress rhs) const {
		return (std::memcmp(lhs.mac_address, rhs.mac_address, MAC_ADDR_LEN) < 0);
	}
	const uint8_t* bytes() {
		return mac_address;
	}
	std::string tostring() {
		char str_addr[INET6_ADDRSTRLEN];
		inet_ntop(AF_INET6, &(sa_addr.sin6_addr), str_addr, INET6_ADDRSTRLEN); // printable format
		return std::string(str_addr);
	}
	std::string tobytestring() {
		std::stringstream ss;
		for (int i = 0; i < MAC_ADDR_LEN; i++) {
			ss << std::setfill('0') << std::hex << std::setw(2)
			<< (int) mac_address[i] << " ";
		}
		return ss.str();
	}
	static std::string tostring(uint8_t address[]) {
		struct sockaddr_in6 sa_addr_;
		sa_addr_.sin6_addr= in6addr_any;		
		std::memcpy(sa_addr_.sin6_addr.s6_addr + 16 - MAC_ADDR_LEN, address,
				MAC_ADDR_LEN);
		char str_addr[INET6_ADDRSTRLEN];
		inet_ntop(AF_INET6, &(sa_addr_.sin6_addr), str_addr, INET6_ADDRSTRLEN); // printable format
		return std::string(str_addr);
	}
	static std::string tobytestring(uint8_t address[]) {
		std::stringstream ss;
		for (int i = 0; i < MAC_ADDR_LEN; i++) {
			ss << std::setfill('0') << std::hex << std::setw(2)
			<< (int) address[i] << " ";
		}
		return ss.str();
	}
private:
	uint8_t mac_address[MAC_ADDR_LEN];
	struct sockaddr_in6 sa_addr; // Used to hold MAC address bytes
};

} // namespace cognitiva
} // namespace gr

#endif /* INCLUDED_COGNITIVA_ADDRESS_UTILS_H */
