// July 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include "Determinization.h"
#include "Dfa.h"

/// Atomic FSA Minimization Algorithm.
template<typename _TFsa, typename _TDfa = Dfa<typename _TFsa::TState, typename _TFsa::TSymbol>>
class MinimizationAtomic
{
public:	
	typedef _TFsa TFsa;
	typedef _TDfa TDfa;
	typedef typename TFsa::TState TState;
	typedef typename TFsa::TSymbol TSymbol;

private:

public:

	void Minimize(const TFsa& fsa)
	{
		using namespace std;
		TState states = fsa.GetStates();
	}
	
		
};