// June 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include "dynamic_bitset.h"
#include <vector>
#include <limits>

///	Represents a Non-Deterministic Finite Automata.
///	<param ref="TState" /> is the integer type representing states.
///	<param ref="TSymbol" /> is the integer type representing symbols.
///	<param ref="TToken" /> is the integer type used in order to manage internal storage of BitSets.
/// Represenation:
/// Each state is a zero-based integer index.
/// Each symbol is a zero-based integer index.
template<class TState, class TSymbol, class TToken = uint64_t>
class Nfa
{
public:	
	typedef dynamic_bitset<TToken> TSet;
	typedef TState TState;
	typedef TSymbol TSymbol;
	typedef TToken TToken;
	
private:
	/// number of symbols in alphabet
	size_t Alphabet;

	/// States backing field
	size_t States;
public:
	
	/// Function to calculate next state
	std::vector<TSet> Succesors;

	/// Inverse function
	std::vector<TSet> Predecessors;

	/// Initial states set
	TSet Initial;

	/// Final states set
	TSet Final;

	

	Nfa(size_t alpha, size_t states)
		:States(states), Alphabet(alpha), Predecessors(alpha * states, TSet(states)), Succesors(alpha * states, TSet(states)), Initial(states), Final(states)
	{
		
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
	void SetTransition(TState source_state, TSymbol symbol, TState target_state, bool add=true)
	{
		auto index1 = Alphabet * source_state + symbol;		
		Succesors[index1].set(target_state, add);
		
		auto index2 = Alphabet * target_state + symbol;
		Predecessors[index2].set(source_state, add);
	}
	
	/// Get the target state transitioned from <param ref="source" /> consuming <param ref="symbol" />
	/// O(1)
	TSet GetSuccessors(TState source, TSymbol symbol) const
	{
		auto index = source * Alphabet + symbol;
		return Succesors[index];
	}

	/// Check if state <param ref="target" /> is reach from state <param ref="source" /> consuming symbol <param ref="symbol" />
	/// O(1)
	bool IsSuccesor(TState source, TSymbol symbol, TState target) const 
	{
		auto r = GetSuccessors(source, symbol).test(target);
		return r;
	}
		
	/// Get the source state transitioned to <param ref="target" /> consuming <param ref="symbol"/>
	/// O(1)
	TSet GetPredecessors(TState target, TSymbol symbol) const
	{
		auto index = target * Alphabet + symbol;
		return Predecessors[index];
	}

	/// Indicates if <param ref="state" /> is a Final State
	/// O(1)
	bool IsFinal(TState state) const { return Final.test(state); }

	/// Indicates if <param ref="state" /> is an Initial State
	/// O(1)
	bool IsInitial(TState state) const { return Initial.test(state); }
};
