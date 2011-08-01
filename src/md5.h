#ifndef MD5_H
#define MD5_H

#include <stdint.h>
#include <string>

struct MD5Context
{
	uint32_t buf[4];
	uint32_t bits[2];
	uint8_t in[64];
};

struct MD5Digest
{
	uint8_t digest[16];
};

// Initialize MD5 context.
extern void MD5Init(MD5Context *);

// Update MD5 context with the provided data.
extern void MD5Update(MD5Context *, const void *, size_t length);

// Finalize MD5 context and provide the computed digest.
extern void MD5Final(MD5Digest *, MD5Context *);

// Convert digest to base-16 string.
extern std::string MD5DigestToBase16(const MD5Digest &);

#endif
