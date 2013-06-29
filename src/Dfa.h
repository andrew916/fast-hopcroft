#pragma once

#include "BitSet.h"
#include <vector>
#include <limits>
#include <cassert>
#include <functional>

template<class TState, class TSymbol, class TToken = uint64_t>
class Dfa
{
public:	
	unsigned Alphabet;	
	std::vector<TState> Succesors;
	std::vector<TState> Predecessors;
	BitSet<TState, TToken> Initial;
	BitSet<TState, TToken> Final;

	Dfa(unsigned alpha, unsigned states)
		: Alphabet(alpha), Succesors(alpha*states), Predecessors(alpha * states), Initial(states), Final(states)
	{	
	}

	unsigned GetAlphabethLength() const { return Alphabet; }
	unsigned GetMaxStates() const { return Initial.GetMaxElements(); }

	// O(1)
	void SetFinal(TState state, bool st = true)
	{
		if(st) Final.Add(state);
		else Final.Remove(state);
	}

	// O(1)
	void SetInitial(TState state, bool st = true)
	{
		if(st) Initial.Add(state);
		else Initial.Remove(state);
	}

	// O(1)
	void SetTransition(TState source_state, TSymbol symbol, TState target_state)
	{
		auto index1 = Alphabet * source_state + symbol;
		Succesors[index1] = target_state;
		auto index2 = Alphabet * target_state + symbol;
		Predecessors[index2] = source_state;
	}
	
	// O(1)
	TState GetSucessor(TState source, TSymbol symbol) const
	{
		auto index = source * Alphabet + symbol;
		return Succesors[index];
	}

	// O(1)
	TState GetPredecessor(TState target, TSymbol symbol) const
	{
		auto index = target * Alphabet + symbol;
		return Predecessors[index];
	}

	// O(1)
	bool IsFinal(TState state) const { return Final.Contains(state); }

	// O(1)
	bool IsInitial(TState state) const { return Initial.Contains(state); }		
};

