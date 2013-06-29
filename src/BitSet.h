#pragma once

#include <cassert>
#include <memory>
#include <functional>
#include "BitUtil_uint32.h"
#include "BitUtil_uint64.h"

template<class TElement, class TToken>
class BitSet
{
	static const size_t ElementsPerToken = sizeof(TToken)*8;
	typedef BitUtil<TToken, false> BU;

	TToken* TokenArray;
	TToken LastTokenMask;
	unsigned Tokens;
	unsigned MaxElements;
		
public:

	BitSet(unsigned maxElements)
	{
		Tokens = maxElements / ElementsPerToken;
		if(Tokens * ElementsPerToken != maxElements) Tokens++;
		TokenArray = new TToken[Tokens];
		MaxElements = maxElements;
				
		LastTokenMask = 0;
		auto offset = maxElements % ElementsPerToken;
		for(int i=0; i<offset; i++) BU::SetBit(&LastTokenMask, i);

		Clear();
	}

	BitSet(const BitSet& copyFrom)
	{
		Tokens = copyFrom.Tokens;
		LastTokenMask = copyFrom.LastTokenMask;
		MaxElements = copyFrom.MaxElements;
		TokenArray = new TToken[Tokens];
		CopyFrom(copyFrom);
	}

	BitSet(BitSet&& rhs)
	{
		TokenArray = rhs.TokenArray;
		LastTokenMask = rhs.LastTokenMask;
		Tokens = rhs.Tokens;

		rhs.TokenArray = nullptr;
	}

	~BitSet()
	{
		if(TokenArray != nullptr) 
		{ 
			free(TokenArray);
			TokenArray = nullptr;
		}
	}

	BitSet& operator= (const BitSet& rh)
	{
		if(rh.Tokens != Tokens) 
		{
			Tokens = rh.Tokens;
			LastTokenMask = rh.LastTokenMask;
			MaxElements = rh.MaxElements;
			realloc(TokenArray, sizeof(TToken)*Tokens)		
		}
		CopyFrom(rh);		
		return *this;
	}

	BitSet& operator= (const BitSet&& rhs)
	{
		if(rhs != this)
		{
			TokenArray = rhs.TokenArray;
			LastTokenMask = rhs.LastTokenMask;
			Tokens = rhs.Tokens;

			rhs.TokenArray = nullptr;
		}
		return *this;
	}

	void Clear()
	{
		memset(TokenArray, 0, sizeof(TToken)*Tokens);
	}

	void CopyFrom(const BitSet& copyFrom)
	{
		assert(copyFrom.Tokens == Tokens);
		assert(copyFrom.LastTokenMask == LastTokenMask);
		assert(copyFrom.MaxElements == MaxElements);
		memcpy(TokenArray, copyFrom.TokenArray, sizeof(TToken)*Tokens);
	}
			
	// O(1)
	bool Contains(TElement element) const
	{
		assert(element < Tokens*ElementsPerToken);
		assert(element < MaxElements);

		// Optmizer will reduce integer divisions to bit shifts
		auto token = element / ElementsPerToken;
		auto offset = element % ElementsPerToken;

		return (TokenArray[token] >> offset) & 1;
	}

	// O(1)
	void Add(TElement element)
	{
		assert(element < Tokens*ElementsPerToken);
		assert(element < MaxElements);

		// Optmizer will reduce integer divisions to bit shifts
		auto token = element / ElementsPerToken;
		auto offset = element % ElementsPerToken;

		TokenArray[token] |= ((TToken)1) << offset;
	}

	// O(1)
	void Remove(TElement element)
	{
		assert(element < Tokens*ElementsPerToken);
		assert(element < MaxElements);

		// Optmizer will reduce integer divisions to bit shifts
		auto token = element / ElementsPerToken;
		auto offset = element % ElementsPerToken;

		TokenArray[token] &= ~(((TToken)1) << offset);
	}

	// O(Nmax) -> Nmax no es el numero de elementos en el conjunto. Es el numero maximo de elementos en el conjunto.
	// Respecto al numero de elementos la complejidad es constante
	bool IsEmpty() const
	{
		for(unsigned s=0; s<c.Tokens; s++)
		{
			if(TokenArray[s]) return false;
		}
		return true;
	}

	unsigned GetMaxElements() const
	{
		return MaxElements;
	}

	// O(Nmax) -> Nmax no es el numero de elementos en el conjunto. Es el numero maximo de elementos en el conjunto.
	// Respecto al numero de elementos la complejidad es constante
	void UnionWith(const BitSet& c)
	{
		assert(c.Tokens == Tokens);
		BU::OrVector(TokenArray, TokenArray, c.TokenArray, Tokens);
	}

	// O(Nmax) -> Nmax no es el numero de elementos en el conjunto. Es el numero maximo de elementos en el conjunto.
	// Respecto al numero de elementos la complejidad es constante
	void IntersectWith(const BitSet& c)
	{
		assert(c.Tokens == Tokens);
		BU::AndVector(TokenArray, TokenArray, c.TokenArray, Tokens);
	}

	void Complement()
	{
		for(unsigned s=0; s<Tokens; s++)
		{
			TokenArray[s] = ~TokenArray[s];
		}
	}
	
	// O(N) -> N es el numero de elementos
	void ForEachMember(std::function< bool(TElement) > cb) const
	{	
		int offset = 0;
		for(unsigned i=0; i<Tokens; i++)
		{
			auto token = TokenArray[i];
			if(i == Tokens-1)  // skip out of range bits
			{
				token &= LastTokenMask;
			}
			unsigned long idx;
			while(BU::BitScanForward(&idx, token))
			{
				auto st = idx + offset;
				auto cont = cb(st);
				if(!cont) return;
				BU::ClearBit(&token, idx);
			}
			offset += ElementsPerToken;
		}
	}
};
