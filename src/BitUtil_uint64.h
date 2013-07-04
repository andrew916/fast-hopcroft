// June 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include <stdint.h>
#include <assert.h>
#include <memory.h>
#include "BitUtil.h"

#if defined(_MSC_VER)
#include <intrin.h>
#endif

#include <immintrin.h>

template<bool UseAVX256, bool UsePOPCNT>
class BitUtil<uint64_t, UseAVX256, UsePOPCNT>
{
public:

#if defined(_MSC_VER)

	static bool BitScanForward(unsigned long* idx, uint64_t v)
	{
		return _BitScanForward64(idx, v) != 0;
	}
	
	static bool SetBit(uint64_t* vec, unsigned bit)
	{
		return _bittestandset64((int64_t*)vec, bit) != 0;
	}
	
	static bool ClearBit(uint64_t* vec, unsigned bit)
	{
		return _bittestandreset64((int64_t*)vec, bit) != 0;
	}

	static bool TestBit(const uint64_t* vec, unsigned bit)
	{
		return _bittest64((const int64_t*)vec, bit) != 0;
	}
		
	static void OrAndClearSecondBit(uint64_t* vec, unsigned b1, unsigned b2) 
	{	
		auto c2 = _bittestandreset64((int64_t*)vec, b2);
		if(c2) _bittestandset64((int64_t*)vec, b1);	
	}

	static void OrVector(uint64_t* o, const uint64_t* v1, const uint64_t* v2, unsigned s)
	{
		if(UseAVX256)
		{
			assert(s * 64 % 256 == 0); // tokens debe ser multiplo de 256
			const int incr = 256 / 64; // AVX use 256-bit registers
			for(unsigned i=0; i<s; i+=incr)
			{
				__m256 rs = _mm256_loadu_ps((const float*)&v1[i]);
				__m256 rd = _mm256_loadu_ps((const float*)&v2[i]);		
				rd = _mm256_or_ps(rs, rd);
				_mm256_storeu_ps((float*)&o[i], rd);
			}
		} else {
			while(s--)
			{
				*o++ = *v1++ | *v2++;
			}
		}
	}

	static void AndVector(uint64_t* o, const uint64_t* v1, const uint64_t* v2, unsigned s)
	{
		if(UseAVX256)
		{
			assert(s * 64 % 256 == 0); // tokens debe ser multiplo de 256
			const int incr = 256 / 64; // AVX use 256-bit registers
			for(unsigned i=0; i<s; i+=incr)
			{
				__m256 rs = _mm256_loadu_ps((const float*)&v1[i]);
				__m256 rd = _mm256_loadu_ps((const float*)&v2[i]);		
				rd = _mm256_and_ps(rs, rd);
				_mm256_storeu_ps((float*)&o[i], rd);
			}
		} else {
			while(s--)
			{
				*o++ = *v1++ & *v2++;
			}
		}
	}

	static bool AnyBitOfAndVector(const uint64_t* v1, const uint64_t* v2, unsigned s)
	{
		if(UseAVX256) 
		{
			assert(s * 64 % 256 == 0); // tokens debe ser multiplo de 256
			const int incr = 256 / 64; // AVX use 256-bit registers
			for(unsigned i=0; i<s; i+=incr)
			{
				__m256i rs = _mm256_loadu_si256((__m256i*)&v1[i]);
				__m256i rd = _mm256_loadu_si256((__m256i*)&v2[i]);
				if(!_mm256_testz_si256(rd, rs)) return true;
			}
		} else {
			while(s--)
			{
				if(*v1++ & *v2++) return true;
			}
			return false;
		}
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
		bool r = TestBit(vec, bit);
		*vec |= (uint64_t)1 << bit;
		return r;
	}

	static bool ClearBit(uint64_t* vec, unsigned bit)
	{
		bool r = TestBit(vec, bit);
		*vec &= ~((uint64_t)1 << bit);
		return r;
	}

	static bool TestBit(const uint64_t* vec, unsigned bit)
	{
		return ((*vec >> bit) & 1) != 0;
	}
	
	static void OrAndClearSecondBit(uint64_t* vec, unsigned b1, unsigned b2) 
	{	
		auto c2 = TestBit(vec, b2);
		if(c2) 
		{
			ClearBit(vec, b2);
			SetBit(vec, b1);
		}
	}

	static void OrVector(uint64_t* o, const uint64_t* v1, const uint64_t* v2, unsigned s)
	{
			while(s--)
			{
				*o++ = *v1++ | *v2++;
			}
	}

	static void AndVector(uint64_t* o, const uint64_t* v1, const uint64_t* v2, unsigned s)
	{
			while(s--)
			{
				*o++ = *v1++ & *v2++;
			}
	}

	static bool AnyBitOfAndVector(const uint64_t* v1, const uint64_t* v2, unsigned s)
	{
			while(s--)
			{
				if(*v1++ & *v2++) return true;
			}
			return false;
	}
		
#endif
	
	static void ClearAllBits(uint64_t* vec, unsigned tokens)
	{
		memset(vec, 0, tokens * sizeof(uint64_t));	
	}
	
	static unsigned CountSet(const uint64_t* v, unsigned s)
	{
		unsigned r = 0;
		while(s--)
		{
			uint64_t x = *v++;
			if(UsePOPCNT) 
			{
				r += (unsigned)__popcnt64(x);
			} else {
				const uint64_t k1 = 0x5555555555555555; /*  -1/3   */
				const uint64_t k2 = 0x3333333333333333; /*  -1/5   */
				const uint64_t k4 = 0x0f0f0f0f0f0f0f0f; /*  -1/17  */
				const uint64_t kf = 0x0101010101010101; /*  -1/255 */
				// from http://chessprogramming.wikispaces.com/Population+Count			
				x =  x       - ((x >> 1)  & k1); /* put count of each 2 bits into those 2 bits */
				x = (x & k2) + ((x >> 2)  & k2); /* put count of each 4 bits into those 4 bits */
				x = (x       +  (x >> 4)) & k4 ; /* put count of each 8 bits into those 8 bits */
				x = (x * kf) >> 56; /* returns 8 most significant bits of x + (x<<8) + (x<<16) + (x<<24) + ...  */
				r += (unsigned)x;
			}
		}
		return r;
	}	
};

