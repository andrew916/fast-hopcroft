#pragma once

#include <boost/static_assert.hpp>
#include <cstdint>
#include "BitUtil.h"

#include <intrin.h>

/// Specialized implementation for 32-bit tokens
template<bool UseAVX256>
class BitUtil<uint32_t, UseAVX256>
{
public:
	// No se permite usar AVX256 con tokens de 32 bits
	BOOST_STATIC_ASSERT(!UseAVX256);

#if defined(_MSC_VER)
	static bool BitScanForward(unsigned long* idx, uint32_t v)
	{
		return _BitScanForward(idx, v) != 0;
	}

	static bool SetBit(uint32_t* vec, unsigned bit)
	{
		return _bittestandset((long*)vec, bit) != 0;
	}
	
	static bool ClearBit(uint32_t* vec, unsigned bit)
	{
		return _bittestandreset((long*)vec, bit) != 0;
	}

	static bool TestBit(const uint32_t* vec, unsigned bit)
	{
		return _bittest((const long*)vec, bit) != 0;
	}
	
	static void OrAndClearSecondBit(uint32_t* vec, unsigned b1, unsigned b2) 
	{
		auto c2 = _bittestandreset((long*)vec, b2);
		if(c2) _bittestandset((long*)vec, b1);	
	}
		
#else
	static bool BitScanForward(unsigned long* idx, uint64_t v)
	{
		auto l = __builtin_ffsll(v);
		if(l == 0) return false;
		*idx = l-1;
		return true;
	}
	
	static bool SetBit(uint64_t* vec, unsigned bit)
	{
		*vec |= (uint64_t)1 << bit;
	}

	static bool ClearBit(uint64_t* vec, unsigned bit)
	{
		*vec &= ~((uint64_t)1 << bit);
	}

	static bool TestBit(const uint64_t* vec, unsigned bit)
	{
		return ((*vec >> bit) & 1) != 0;
	}

	static void OrAndClearSecondBit(uint32_t* vec, unsigned b1, unsigned b2) 
	{	
		auto c2 = TestBit(vec, b2);
		if(c2) 
		{
			ClearBit(vec, b2);
			SetBit(vec, b1);
		}
	}

#endif
		
	static void ClearAllBits(uint32_t* vec, unsigned tokens)
	{
		memset(vec, 0, tokens * sizeof(uint32_t));	
	}

	static void OrVector(uint32_t* o, const uint32_t* v1, const uint32_t* v2, unsigned s)
	{
		while(s--)
		{
			*o++ = *v1++ | *v2++;
		}
	}

	static void AndVector(uint32_t* o, const uint32_t* v1, const uint32_t* v2, unsigned s)
	{
		while(s--)
		{
			*o++ = *v1++ & *v2++;
		}
	}

	static bool AnyBitOfAndVector(const uint32_t* v1, const uint32_t* v2, unsigned s)
	{		
		while(s--)
		{
			if(*v1++ & *v2++) return true;
		}
		return false;
	}
};
