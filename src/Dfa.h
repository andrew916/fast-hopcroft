// June 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include "dynamic_bitset.h"
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
	typedef dynamic_bitset<TToken> TSet;
	typedef TState TState;
	typedef TSymbol TSymbol;
	typedef TToken TToken;

private:
	/// number of symbols in alphabet
	size_t Alphabet;	

	/// states back field
	size_t States;

public:
	/// Function to calculate next state
	std::vector<TState> Succesors;

	/// Inverse function
	std::vector<TSet> Predecessors;

	/// Initial states set
	TSet Initial;

	/// Final states set
	TSet Final;	

	/// <param ref="alpha" /> is the number of symbols in alphabet.	
	Dfa(size_t alpha, size_t states)
		: States(states), Alphabet(alpha), Succesors(alpha*states), Predecessors(alpha * states, TSet(states)), Initial(states), Final(states)
	{
		// At boot, each state go to state zero with every symbol
		for(TSymbol sym=0; sym<alpha; sym++)
		{
			Predecessors[sym].set();
		}		
	}
		
	/// Get the number of symbols in alphabet
	/// O(1)
	size_t GetAlphabetLength() const { return Alphabet; }

	/// Get maximum number of states of this DFA
	/// O(1)
	size_t GetStates() const { return States; }

	/// Set or unset one state as Final
	/// O(1)
	void SetFinal(TState state, bool st = true)
	{
		if(st) Final.set(state);
		else Final.reset(state);
	}

	/// Set or unset one state as Initial
	/// O(1)
	void SetInitial(TState state, bool st = true)
	{
		if(st) Initial.set(state);
		else Initial.reset(state);
	}

	/// Adjust the transition from <param ref="source_state" /> consuming <param ref="symbol" /> to <param ref="target_state" />
	/// O(1)
	void SetTransition(TState source_state, TSymbol symbol, TState target_state)
	{
		auto index1 = Alphabet * source_state + symbol;
		auto prev_target = Succesors[index1];
		Succesors[index1] = target_state;
		
		auto index2 = Alphabet * prev_target + symbol;
		Predecessors[index2].reset(source_state);

		auto index3 = Alphabet * target_state + symbol;				
		Predecessors[index3].set(source_state);
	}
	
	/// Get the target state transitioned from <param ref="source" /> consuming <param ref="symbol" />
	/// O(1)
	TState GetSuccessor(TState source, TSymbol symbol) const
	{
		auto index = source * Alphabet + symbol;
		return Succesors[index];
	}

	/// Get the source state transitioned to <param ref="target" /> consuming <param ref="symbol"/>
	/// O(1)
	TSet GetPredecessors(TState target, TSymbol symbol) const
	{
		auto index = target * Alphabet + symbol;
		return Predecessors[index];
	}

	/// Check if state <param ref="target" /> is reach from state <param ref="source" /> consuming symbol <param ref="symbol" />
	/// O(1)
	bool IsSuccessor(TState source, TSymbol symbol, TState target) const 
	{
		auto r = GetSuccessor(source, symbol) == target;
		return r;
	}

	/// Indicates if <param ref="state" /> is a Final State
	/// O(1)
	bool IsFinal(TState state) const { return Final.test(state); }

	/// Indicates if <param ref="state" /> is an Initial State
	/// O(1)
	bool IsInitial(TState state) const { return Initial.test(state); }		
};

