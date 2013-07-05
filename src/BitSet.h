// June 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include <stdint.h>
#include <assert.h>
#include <memory.h>
#include <functional>
#include "BitUtil_uint32.h"
#include "BitUtil_uint64.h"

/// Represents a Set. Internal storage uses bit vectors.
/// Every member of the Set is an integer number in a zero-based range.
/// It allows Union, Intersect, Add, Remove, Copy, Complement oprations to run in constant time.
/// <param ref="TElement" /> is the integer type for each state. Note that it affects maximum elements number.
/// <param ref="TToken" /> is the integer type to internal storage.
template<class TElement, class TToken, bool UseAVX256 = false, bool UsePOPCNT=true>
class BitSet
{
	static const unsigned int ElementsPerToken = sizeof(TToken)*8;
	typedef BitUtil<TToken, UseAVX256, UsePOPCNT> BU;

	TToken* TokenArray;
	// Ones means used bits, Zeros unused bits
	TToken LastTokenMask;
	unsigned Tokens;
	unsigned MaxElements;
		
	std::size_t ReqSize(unsigned r) 
	{ 
		if(UseAVX256 && r != 0)
		{
			assert(ElementsPerToken < 256);
			auto m = 256/ElementsPerToken;
			r = (r - 1)/m + m;
		}
		return sizeof(TToken)*r; 
	}

public:

	/// <param ref="maxElements" /> Indicates the maximum number of elements
	BitSet(unsigned maxElements)
	{
		Tokens = maxElements == 0 ? 0 : (maxElements - 1) / ElementsPerToken + 1;
		TokenArray = (TToken*)malloc(ReqSize(Tokens));
		MaxElements = maxElements;
				
		LastTokenMask = 0;
		auto offset = maxElements % ElementsPerToken;
		for(unsigned i=0; i<offset; i++) BU::SetBit(&LastTokenMask, i);

		Clear();
	}

	/// Copy constructor
	BitSet(const BitSet& copyFrom)
	{
		Tokens = copyFrom.Tokens;
		LastTokenMask = copyFrom.LastTokenMask;
		MaxElements = copyFrom.MaxElements;
		TokenArray = (TToken*)malloc(ReqSize(Tokens));
		CopyFrom(copyFrom);
	}

	/// Move constructor
	BitSet(BitSet&& rhs)
	{
		TokenArray = rhs.TokenArray;
		LastTokenMask = rhs.LastTokenMask;
		Tokens = rhs.Tokens;
		MaxElements = rhs.MaxElements;

		rhs.TokenArray = nullptr;
	}

	/// Destructor
	~BitSet()
	{
		if(TokenArray != nullptr) 
		{ 
			free(TokenArray);
			TokenArray = nullptr;
		}
	}

	/// Assign operator
	BitSet& operator= (const BitSet& rh)
	{		
		LastTokenMask = rh.LastTokenMask;
		MaxElements = rh.MaxElements;
		if(Tokens != rh.Tokens) 
		{
			Tokens = rh.Tokens;
			TokenArray = (TToken*)realloc(TokenArray, ReqSize(Tokens));
		}

		CopyFrom(rh);
		return *this;
	}

	/// Assign operator
	BitSet& operator= (BitSet&& rhs)
	{
		if(&rhs != this)
		{
			TokenArray = rhs.TokenArray;
			LastTokenMask = rhs.LastTokenMask;
			MaxElements = rhs.MaxElements;
			Tokens = rhs.Tokens;

			rhs.TokenArray = nullptr;
		}
		return *this;
	}

	/// Removes all members of the set
	void Clear()
	{
		BU::ClearAllBits(TokenArray, Tokens);
	}

	/// Copy other set defined under same universe.
	/// It means that source Set hold the same maximum number of elements.
	/// O(1)
	void CopyFrom(const BitSet& copyFrom)
	{
		assert(copyFrom.Tokens == Tokens);
		assert(copyFrom.LastTokenMask == LastTokenMask);
		assert(copyFrom.MaxElements == MaxElements);
		memcpy(TokenArray, copyFrom.TokenArray, sizeof(TToken)*Tokens);
	}

	/// Check membership of <param ref="element" />
	/// O(1)
	bool Contains(TElement element) const
	{
		assert(element < Tokens*ElementsPerToken);
		assert(element < MaxElements);

		// Optmizer will reduce integer divisions to bit shifts
		auto token = element / ElementsPerToken;
		auto offset = element % ElementsPerToken;

		return BU::TestBit(&TokenArray[token], offset);
	}

	/// Add one element 
	/// O(1)
	void Add(TElement element)
	{
		assert(element < Tokens*ElementsPerToken);
		assert(element < MaxElements);

		// Optmizer will reduce integer divisions to bit shifts
		auto token = element / ElementsPerToken;
		auto offset = element % ElementsPerToken;

		BU::SetBit(&TokenArray[token], offset);
	}

	/// Remove one element 
	// O(1)
	void Remove(TElement element)
	{
		assert(element < Tokens*ElementsPerToken);
		assert(element < MaxElements);

		// Optmizer will reduce integer divisions to bit shifts
		auto token = element / ElementsPerToken;
		auto offset = element % ElementsPerToken;

		BU::ClearBit(&TokenArray[token], offset);
	}

	/// Check if the Set is empty
	/// O(Nmax) -> Nmax no es el numero de elementos en el conjunto. 
	/// Es el numero MAXIMO de elementos en el conjunto.
	/// Respecto al numero de elementos se ejecuta en tiempo constante
	bool IsEmpty() const
	{
		for(unsigned s=0; s<Tokens; s++)
		{
			if(TokenArray[s]) return false;
		}
		return true;
	}

	/// Get the maximum number of elements
	unsigned GetMaxElements() const
	{
		return MaxElements;
	}

	/// Add every member from other set
	/// O(Nmax) -> Nmax no es el numero de elementos en el conjunto. 
	/// Es el numero MAXIMO de elementos en el conjunto.
	/// Respecto al numero de elementos la complejidad es constante
	void UnionWith(const BitSet& c)
	{
		assert(c.Tokens == Tokens);
		BU::OrVector(TokenArray, TokenArray, c.TokenArray, Tokens);
	}

	/// Remove elements do not contained in both sets
	/// O(Nmax) -> Nmax no es el numero de elementos en el conjunto. 
	/// Es el numero MAXIMO de elementos en el conjunto.
	/// Respecto al numero de elementos la complejidad es constante
	void IntersectWith(const BitSet& c)
	{
		assert(c.Tokens == Tokens);
		BU::AndVector(TokenArray, TokenArray, c.TokenArray, Tokens);
	}

	/// Calculates the complement
	void Complement()
	{
		for(unsigned s=0; s<Tokens; s++)
		{
			TokenArray[s] = ~TokenArray[s];
			if(s == Tokens-1)
			{
				TokenArray[s] &= LastTokenMask;
			}
		}
	}
	
	/// Iterates for each member and applies the function <param ref="cb" />
	/// If function applied return false, iteration is stopped.
	/// Enumerates in O(N) -> N es el numero de elementos
	void ForEachMember(const std::function< bool(TElement) >& cb) const
	{	
		int offset = 0;
		for(unsigned i=0; i<Tokens; i++) // Tokens can not change, it is a O(1) contributing loop
		{
			auto token = TokenArray[i];
			if(i == Tokens-1)  // skip out of range bits
			{
				token &= LastTokenMask;
			}
			unsigned long idx;
			while(BU::BitScanForward(&idx, token)) // each cycle is one member always
			{
				auto st = idx + offset;
				auto cont = cb(st);
				if(!cont) return;
				BU::ClearBit(&token, idx);
			}
			offset += ElementsPerToken;
		}
	}
		
	unsigned Count() const
	{		
		return BU::CountSet(TokenArray, Tokens);		
	}

	struct hash
	{
		std::size_t operator()(const BitSet<TElement,TToken>& rh)
		{		
			std::size_t r;
			r = std::_Hash_seq((const unsigned char*)rh.TokenArray, rh.Tokens*sizeof(TToken));
			r ^= rh.MaxElements;
			return r;
		}
	};

	bool operator==(const BitSet<TElement, TToken> &other) const 
	{
		if(Tokens != other.Tokens) return false;
		if(MaxElements != other.MaxElements) return false;		
		for(unsigned i=0; i<Tokens; i++)
		{
			if(TokenArray[i] != other.TokenArray[i]) return false;
		}
		return true;
	}
};
