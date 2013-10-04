// August 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include <tuple>
#include "Set.h"

///	Represents a Finite Automata.
///	<param ref="TState" /> is the integer type representing states.
///	<param ref="TSymbol" /> is the integer type representing symbols.
/// Represenation:
/// Each state is a zero-based integer index.
/// Each symbol is a zero-based integer index.
template<typename _TState, typename _TSymbol, typename _TSet = Set<_TState>, typename _TEdge = std::tuple<_TState, _TSymbol, _TState>, typename _TEdgeSet = Set<_TEdge>>
class Fsa
{
public:
	typedef _TSet TSet;
	typedef _TState TState;
	typedef _TSymbol TSymbol;
	typedef _TEdge TEdge;
	typedef _TEdgeSet TEdgeSet;
	
private:
	/// number of symbols in alphabet
	TSymbol Alphabet;

	/// States backing field
	TState States;

	/// Initial states set
	TSet Initial;

	/// Final states set
	TSet Final;

	/// Function to calculate next state
	TEdgeSet Successors;

	/// Inverse function
	TEdgeSet Predecessors;

public:
	
	const TSet& GetInitials() const
	{
		return Initial;
	}

	const TSet& GetFinals() const
	{
		return Final;
	}

	Fsa(TSymbol alpha, TState states)
		: States(states), Alphabet(alpha), Initial(states), Final(states), Successors(states*states*alpha), Predecessors(states*states*alpha)
	{
	}

	Fsa(TSymbol alpha, TState states, const TSet& initials, const TSet& finals, const TEdgeSet& succesors, const TEdgeSet& predecessors)
		: States(states), Alphabet(alpha), Initial(initials), Final(finals), 
		Successors(succesors), Predecessors(predecessors)
	{
		assert(states >= initials.Count());
		assert(states >= finals.Count());
	}

	/// Get the number of symbols in alphabet
	/// O(1)
	virtual TSymbol GetAlphabetLength() const { return Alphabet; }

	/// Get maximum number of states of this DFA
	/// O(1)
	virtual TState GetStates() const { return States; }

	/// Set or unset one state as Final
	/// O(1)
	virtual void SetFinal(TState state, bool st = true)
	{
		assert(state < States);

		if(st) Final.Add(state);
		else Final.Remove(state);
	}

	/// Set or unset one state as Initial
	/// O(1)
	virtual void SetInitial(TState state, bool st = true)
	{
		assert(state < States);

		if(st) Initial.Add(state);
		else Initial.Remove(state);
	}

	/// Adjust the transition from <param ref="source_state" /> consuming <param ref="symbol" /> to <param ref="target_state" />
	/// O(1)
	virtual void SetTransition(TState source_state, TSymbol symbol, TState target_state, bool add=true)
	{
		assert(source_state < States);
		assert(target_state < States);
		assert(symbol < Alphabet);

		
		if(add)
			Successors.Add(TEdge(source_state, symbol, target_state));
		else 
			Successors.Remove(TEdge(source_state, symbol, target_state));
		
		
		if(add)
			Predecessors.Add(TEdge(target_state, symbol, source_state));
		else
			Predecessors.Remove(TEdge(target_state, symbol, source_state));
	}
	
	/// Check if state <param ref="target" /> is reach from state <param ref="source" /> consuming symbol <param ref="symbol" />
	/// O(1)
	virtual bool IsSuccessor(TState source, TSymbol symbol, TState target) const 
	{
		assert(source < States);
		assert(target < States);
		assert(symbol < Alphabet);
		
		auto r = Successors.Contains(TEdge(source, symbol, target));
		return r;
	}

	/// Indicates if <param ref="state" /> is a Final State
	/// O(1)
	virtual bool IsFinal(TState state) const 
	{		
		assert(state < States);

		return Final.Contains(state); 
	}

	/// Indicates if <param ref="state" /> is an Initial State
	/// O(1)
	virtual bool IsInitial(TState state) const 
	{ 
		assert(state < States);

		return Initial.Contains(state); 
	}

	// Inversion of each transition.
	// Final states are converted into initial states.
	// Initial states are converted into final states.
	// O(1)
	virtual void Invert()
	{
		std::swap(Initial, Final);
		std::swap(Successors, Predecessors);
	}

	virtual TSet GetSuccessors(TState source, TSymbol sym) const
	{
		auto rb = Successors.FindLower(TEdge(source, sym, 0));
		auto re = Successors.FindUpper(TEdge(source, sym, GetStates()-1));
		TSet s(0);
		while(rb != re)
		{
			auto t = rb.GetCurrent();
			auto e = std::get<2>(t);
			s.Add(e);
			rb.MoveNext();
		}
		return s;
	}

	virtual TSet GetPredecessors(TState target, TSymbol sym) const
	{
		auto rb = Predecessors.FindLower(TEdge(target, sym, 0));
		auto re = Predecessors.FindUpper(TEdge(target, sym, GetStates()-1));
		TSet s(0);
		while(rb != re)
		{
			auto t = rb.GetCurrent();
			auto e = std::get<2>(t);
			s.Add(e);
			rb.MoveNext();
		}
		return s;
	}

	virtual typename TEdgeSet::Iterator GetEdgeIterator() const
	{
		return Successors.GetIterator();
	}
};
