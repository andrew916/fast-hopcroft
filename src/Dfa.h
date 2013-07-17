// June 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include <boost/dynamic_bitset.hpp>
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
	typedef boost::dynamic_bitset<TToken> TSet;

	/// number of symbols in alphabet
	unsigned Alphabet;	

	/// Function to calculate next state
	std::vector<TState> Succesors;

	/// Inverse function
	std::vector<TSet> Predecessors;

	/// Initial states set
	TSet Initial;

	/// Final states set
	TSet Final;

	unsigned States;

	/// <param ref="alpha" /> is the number of symbols in alphabet.	
	Dfa(unsigned alpha, unsigned states)
		: States(states), Alphabet(alpha), Succesors(alpha*states), Predecessors(alpha * states, TSet(states)), Initial(states), Final(states)
	{
		// At boot, each state go to state zero with every symbol		
		for(TSymbol sym=0; sym<alpha; sym++)
		{
			Predecessors[sym] = ~Predecessors[sym];
		}		
	}
		
	/// Get the number of symbols in alphabet
	unsigned GetAlphabethLength() const { return Alphabet; }

	/// Get maximum number of states of this DFA
	unsigned GetStates() const { return States; }

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
	TState GetSucessor(TState source, TSymbol symbol) const
	{
		auto index = source * Alphabet + symbol;
		return Succesors[index];
	}

	/// Get the source state transitioned to <param ref="target" /> consuming <param ref="symbol"/>
	/// O(1)
	const TSet& GetPredecessor(TState target, TSymbol symbol) const
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

