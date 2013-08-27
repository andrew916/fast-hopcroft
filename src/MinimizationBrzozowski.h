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


/// Brzozowski's FSM Minimization Algorithm.
template<typename TFsa>
class MinimizationBrzozowski
{
public:	
	typedef TFsa TFsa;
	typedef typename TFsa::TState TState;
	typedef typename TFsa::TSymbol TSymbol;
		
	typedef Determinization<TFsa, TFsa> TDeterminization;
private:


public:

	TFsa Minimize(const TFsa& fsm)
	{
		TDeterminization determinizer;
		auto p1 = Invert(fsm);
		auto p2 = determinizer.Determinize(p1);
		auto p3 = Invert(p2);
		auto p4 = determinizer.Determinize(p3);
		return p4;
	}
};
