#pragma once

template<class T, bool UseAVX256>
class BitUtil
{
public:
	static bool BitScanForward(unsigned long* idx, T v);
	static bool BitScanReverse(unsigned long* idx, T v);
	static bool SetBit(T* vec, unsigned bit);
	static bool ClearBit(T* vec, unsigned bit);
	static bool TestBit(const T* vec, unsigned bit);
	static void ClearAllBits(T* vec, unsigned tokens);
	static void OrAndClearSecondBit(T* vec, unsigned b1, unsigned b2);
	static void OrVector(T* o, const T* v1, const T*v2, unsigned s);
	static bool AnyBitOfAndVector(const T* v1, const T*v2, unsigned s);
};