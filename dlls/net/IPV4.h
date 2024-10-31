#pragma once
#include "extdll.h"
#include <string>
#include <stdint.h>

class EXPORT IPV4
{
public:
	uint8_t b1, b2, b3, b4;
	uint16_t port;

	IPV4();

	IPV4(const char * addr);

	IPV4(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4);

	IPV4(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint16_t port);

	// 4 octets and port
	std::string getString() const;

	// 4 octets - no port
	std::string getHostString() const;

	// is everything 0?
	bool isEmpty() const;
};

EXPORT bool operator==( IPV4 a, IPV4 b );
EXPORT bool operator!=( IPV4 a, IPV4 b );