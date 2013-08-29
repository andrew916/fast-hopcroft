// August 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include <tuple>
#include <set>

///	Represents a Finite Automata.
///	<param ref="TState" /> is the integer type representing states.
///	<param ref="TSymbol" /> is the integer type representing symbols.
/// Represenation:
/// Each state is a zero-based integer index.
/// Each symbol is a zero-based integer index.
template<typename TState, typename TSymbol>
class Fsa
{
public:
	typedef std::tuple<TState,TSymbol,TState> TEdge;
	typedef std::set<TState> TSet;
private:
	size_t Alphabet;
	size_t States;
	std::set<TEdge> Edges;
	std::vector<TSet> Successors;
	std::vector<TSet> Predecessors;
	TSet Final;
	TSet Initial;

public:
	bool IsInitial(TState state) const { return Initial.count(state) == 1; }
	
	bool IsFinal(TState state) const { return Final.count(state) == 1; }

	bool IsSuccessor(TState source, TSymbol c, TState target) const
	{
		return GetSuccessors(source, c).count(target) == 1;
	}

	bool IsPredecessor(TState source, TSymbol c, TState target)
	{
		return GetPredecessors(source, c).count(target) == 1;
	}

	void SetInitial(TState state, bool st = true)
	{
		if(st) Initial.insert(state);
		else Initial.erase(state);
	}

	void SetFinal(TState state, bool st = true)
	{
		if(set) Final.insert(state);
		else Final.erase(state);
	}

	void SetTransition(TState source, TSymbol symbol, TState target, bool add=true)
	{		
		auto p = std::make_pair(source, symbol, target);
		if(add)
		{
			Edges.insert(p);			
		} 
		else
		{
			Edges.erase(p);
		}
	}

	const TSet& GetSuccessors(TState source, TSymbol c) const
	{
		return Successors[source*Alphabet+c];
	}

	const TSet& GetPredecessors(TState target, TSymbol c) const
	{
		return Predecessors[target*Alphabet+c];
	}

	const std::set<TEdge>& GetEdges() const { return Edges; } 
};
