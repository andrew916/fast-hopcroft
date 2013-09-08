// June 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include "Set.h"
#include <vector>
#include <limits>

///	Represents a Non-Deterministic Finite Automata.
///	<param ref="TState" /> is the integer type representing states.
///	<param ref="TSymbol" /> is the integer type representing symbols.
///	<param ref="TToken" /> is the integer type used in order to manage internal storage of BitSets.
/// Represenation:
/// Each state is a zero-based integer index.
/// Each symbol is a zero-based integer index.
template<typename TState, typename TSymbol, typename TToken = uint64_t>
class Nfa
{
public:	
	typedef Nfa<TState,TSymbol,TToken> TNfa;
	typedef BitSet<TState, TToken> TSet;
	typedef TState TState;
	typedef TSymbol TSymbol;
	typedef std::tuple<TState,TSymbol,TState> TEdge;
	
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
	std::vector<TSet> Succesors;

	/// Inverse function
	std::vector<TSet> Predecessors;

public:
	
	const TSet& GetInitials() const
	{
		return Initial;
	}

	const TSet& GetFinals() const
	{
		return Final;
	}

	Nfa(TSymbol alpha, TState states)
		:States(states), Alphabet(alpha), Predecessors(alpha * states, TSet(states)), Succesors(alpha * states, TSet(states)), Initial(states), Final(states)
	{		
	}

	Nfa(TSymbol alpha, TState states, const TSet& initials, const TSet& finals, const std::vector<TSet>& succesors, const std::vector<TSet>& predecessors)
		: States(states), Alphabet(alpha), Initial(initials), Final(finals), Succesors(succesors), Predecessors(predecessors)
	{
		assert(initials.Count() <= states);
		assert(finals.Count() <= states);
		assert(states*alpha == succesors.size());
		assert(states*alpha == predecessors.size());
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

		auto index1 = Alphabet * source_state + symbol;		
		if(add)
			Succesors[index1].Add(target_state);
		else 
			Succesors[index1].Remove(target_state);
		
		auto index2 = Alphabet * target_state + symbol;
		if(add)
			Predecessors[index2].Add(source_state);
		else
			Predecessors[index2].Remove(source_state);
	}
	
	/// Get the target state transitioned from <param ref="source" /> consuming <param ref="symbol" />
	/// O(1)
	virtual const TSet& GetSuccessors(TState source, TSymbol symbol) const
	{
		assert(source < States);		
		assert(symbol < Alphabet);

		auto index = source * Alphabet + symbol;
		return Succesors[index];
	}

	/// Check if state <param ref="target" /> is reach from state <param ref="source" /> consuming symbol <param ref="symbol" />
	/// O(1)
	virtual bool IsSuccessor(TState source, TSymbol symbol, TState target) const 
	{
		assert(source < States);
		assert(target < States);
		assert(symbol < Alphabet);

		auto r = GetSuccessors(source, symbol).Contains(target);
		return r;
	}
		
	/// Get the source state transitioned to <param ref="target" /> consuming <param ref="symbol"/>
	/// O(1)
	virtual const TSet& GetPredecessors(TState target, TSymbol symbol) const
	{
		assert(target < States);
		assert(symbol < Alphabet);

		auto index = target * Alphabet + symbol;
		return Predecessors[index];
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
		std::swap(Succesors, Predecessors);
	}

	class EdgeSuccessorIterator
	{
	public:
	private:
		TState s;
		TSymbol a;
		typename TSet::Iterator i;
		TNfa* nfa;

		EdgeSuccessorIterator(TNfa* tnfa, TState st, TSymbol sa, typename TSet::Iterator it)
			: nfa(tnfa), s(st), a(sa), i(it)
		{
			if(i.IsEnd()) MoveNext();
		}

		friend class TNfa;

	public:
		TEdge GetCurrent() const
		{
			return TEdge(s, a, i.GetCurrent());
		}

		bool IsEnd() const
		{
			return i.IsEnd();
		}

		void MoveNext()
		{
			i.MoveNext();		
			while(i.IsEnd())
			{
				a++;
				if(a == nfa->GetAlphabetLength()) 
				{ 
					a=0; s++; 
					if(s == nfa->GetStates()) break;
				}
				i = nfa->GetSuccessors(s, a).GetIterator();
			}
		}
	};

	EdgeSuccessorIterator GetEdgeIterator() const
	{
		TState s=0;
		TSymbol a=0;
		TSet::Iterator i=GetSuccessors(s,a).GetIterator();
		return EdgeSuccessorIterator(this, s, a, i);
	}
};
