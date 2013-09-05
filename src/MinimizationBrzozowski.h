// July 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include "Dfa.h"
#include "Nfa.h"
#include "Determinization.h"

/// Brzozowski's FSA Minimization Algorithm.
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
		TFsa p1 = fsm;
		p1.Invert();
		auto p2 = determinizer.Determinize(p1);
		p2.Invert();
		auto p3 = determinizer.Determinize(p2);
		return p3;
	}
};
