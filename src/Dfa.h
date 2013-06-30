// June 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include "BitSet.h"
#include <vector>
#include <limits>
#include <cassert>
#include <functional>

///	Represents a Deterministic Finite Automata.
///	<param ref="TState" /> is the integer type representing states.
///	<param ref="TSymbol" /> is the integer type representing symbols.
///	<param ref="TToken" /> is the integer type used in order to manage internal storage of BitSets.
/// Represenation:
/// Each state is a zero-based integer index.
/// Each symbol is a zero-based integer index.
template<class TState, class TSymbol, class TToken = uint64_t>
class Dfa
{
public:	
	/// number of symbols in alphabet
	unsigned Alphabet;	

	/// Function to calculate next state
	std::vector<TState> Succesors;

	/// Inverse function
	std::vector<TState> Predecessors;

	/// Initial states set
	BitSet<TState, TToken> Initial;

	/// Final states set
	BitSet<TState, TToken> Final;

	/// <param ref="alpha" /> is the number of symbols in alphabet.	
	Dfa(unsigned alpha, unsigned states)
		: Alphabet(alpha), Succesors(alpha*states), Predecessors(alpha * states), Initial(states), Final(states)
	{	
	}

	/// Get the number of symbols in alphabet
	unsigned GetAlphabethLength() const { return Alphabet; }

	/// Get maximum number of states of this DFA
	unsigned GetMaxStates() const { return Initial.GetMaxElements(); }

	/// Set or unset one state as Final
	/// O(1)
	void SetFinal(TState state, bool st = true)
	{
		if(st) Final.Add(state);
		else Final.Remove(state);
	}

	/// Set or unset one state as Initial
	/// O(1)
	void SetInitial(TState state, bool st = true)
	{
		if(st) Initial.Add(state);
		else Initial.Remove(state);
	}

	/// Adjust the transition from <param ref="source_state" /> consuming <param ref="symbol" /> to <param ref="target_state" />
	/// O(1)
	void SetTransition(TState source_state, TSymbol symbol, TState target_state)
	{
		auto index1 = Alphabet * source_state + symbol;
		Succesors[index1] = target_state;
		auto index2 = Alphabet * target_state + symbol;
		Predecessors[index2] = source_state;
	}
	
	/// Get the target state transitioned from <param ref="source" /> consuming <param ref="symbol" />
	/// O(1)
	TState GetSucessor(TState source, TSymbol symbol) const
	{
		auto index = source * Alphabet + symbol;
		return Succesors[index];
	}

	/// Get the source state transitioned to <param ref="target" /> consuming <param ref="symbol"/>
	/// O(1)
	TState GetPredecessor(TState target, TSymbol symbol) const
	{
		auto index = target * Alphabet + symbol;
		return Predecessors[index];
	}

	/// Indicates if <param ref="state" /> is a Final State
	/// O(1)
	bool IsFinal(TState state) const { return Final.Contains(state); }

	/// Indicates if <param ref="state" /> is an Initial State
	/// O(1)
	bool IsInitial(TState state) const { return Initial.Contains(state); }		
};

