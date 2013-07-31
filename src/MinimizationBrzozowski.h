// July 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include "Dfa.h"
#include "Nfa.h"
#include "Determinization.h"

template<typename TNfa>
TNfa Invert(const TNfa& nfa)
{
	TNfa r(nfa);
	std::swap(r.Initial, r.Final);
	std::swap(r.Predecessors, r.Succesors);
	return r;
}

template<typename TDfa, typename TNfa>
TNfa Invert(TDfa dfa)
{
	TNfa nfa(dfa.GetAlphabetLength(), dfa.GetStates());
	nfa.Initial = dfa.Final;
	nfa.Final = dfa.Initial;
	nfa.Sucessors = dfa.Predecessors;
	for(TSymbol c=0; c<dfa.GetAlphabetLength(); c++)
	{
		for(TState qs=0; qs<dfa.GetStates(); qs++)
		{
			nfa.SetTransition(
		}
	}
	return nfa;
}

/// Brzozowski's FSM Minimization Algorithm.
template<typename TFsm, typename TDfa = Dfa<TState, TSymbol>>
class MinimizationBrzozowski
{
public:	
	typedef TFsm TFsm;
	typedef typename TFsm::TState TState;
	typedef typename TFsm::TSymbol TSymbol;

	typedef TDfa TDfa;
	typedef Determinization<TDfa, TFsm> TDeterminization;
private:


public:

	TDfa Minimize(const TFsm& fsm)
	{
		TDeterminization determinizer;
		auto p1 = Invert(fsm);
		auto p2 = determinizer.Determinize(p1);
		auto p3 = Invert(p2);
		auto p4 = determinizer.Determinize(p3);
		return p4;
	}
};
