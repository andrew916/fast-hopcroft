#pragma once

#include <stdint.h>
#include <assert.h>
#include <memory.h>
#include "BitUtil.h"

#if defined(_MSC_VER)
#include <intrin.h>
#endif

#include <immintrin.h>

/// Specialized implementation for 32-bit tokens
template<bool UseAVX256, bool UsePOPCNT>
class BitUtil<uint32_t, UseAVX256, UsePOPCNT>
{
public:
	// No se permite usar AVX256 con tokens de 32 bits
	static_assert(!UseAVX256, "No se permite usar AVX256 con tokens de 32 bits");

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
	static bool BitScanForward(unsigned long* idx, uint32_t v)
	{
		auto l = __builtin_ffsl(v);
		if(l == 0) return false;
		*idx = l-1;
		return true;
	}
	
	static bool SetBit(uint32_t* vec, unsigned bit)
	{
		bool r = TestBit(vec, bit);
		*vec |= (uint32_t)1 << bit;
		return r;
	}

	static bool ClearBit(uint32_t* vec, unsigned bit)
	{
		bool r = TestBit(vec, bit);
		*vec &= ~((uint32_t)1 << bit);
		return r;
	}

	static bool TestBit(const uint32_t* vec, unsigned bit)
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

	static unsigned CountSet(const uint32_t* v, unsigned s)
	{
		unsigned r = 0;
		while(s--)
		{
			uint32_t x = *v++;
			if(UsePOPCNT) 
			{
				r += __popcnt(x);
			} else {
				const uint32_t k1 = 0x55555555; /*  -1/3   */
				const uint32_t k2 = 0x33333333; /*  -1/5   */
				const uint32_t k4 = 0x0f0f0f0f; /*  -1/17  */
				const uint32_t kf = 0x01010101; /*  -1/255 */
				// from http://chessprogramming.wikispaces.com/Population+Count			
				x =  x       - ((x >> 1)  & k1); /* put count of each 2 bits into those 2 bits */
				x = (x & k2) + ((x >> 2)  & k2); /* put count of each 4 bits into those 4 bits */
				x = (x       +  (x >> 4)) & k4 ; /* put count of each 8 bits into those 8 bits */
				x = (x * kf) >> 24; /* returns 8 most significant bits of x + (x<<8) + (x<<16) + (x<<24) + ...  */
				r += (unsigned)x;
			}
		}
		return r;
	}	
};
