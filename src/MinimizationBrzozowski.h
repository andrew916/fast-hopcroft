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
	typedef typename TDeterminization::TDfaState TDfaState;
	typedef typename TDeterminization::TVectorDfaState TVectorDfaState;
	typedef typename TDeterminization::TVectorDfaEdge TVectorDfaEdge;

private:


public:

	void Minimize(const TFsa& fsa, TDfaState* states, TVectorDfaState& vfinalstates, TVectorDfaEdge& vedges)
	{
		TDeterminization det;
		TFsa p1 = fsa;

		p1.Invert();
		det.Determinize(p1, states, vfinalstates, vedges);
		TFsa p2 = det.BuildDfa(fsa.GetAlphabetLength(), *states, vfinalstates, vedges);

		p2.Invert();
		det.Determinize(p2, states, vfinalstates, vedges);		
	}

	TFsa BuildDfa(TSymbol alpha, TDfaState states, const TVectorDfaState& final_states, const TVectorDfaEdge& edges)
	{
		TDeterminization det;
		TFsa dfa = det.BuildDfa(alpha, states, final_states, edges);
		return dfa;
	}

	TFsa Minimize(const TFsa& fsa)
	{
		TDfaState states;
		TVectorDfaState fstates;
		TVectorDfaEdge edges;
		Minimize(fsa, &states, fstates, edges);
		TFsa min_fsa = BuildDfa(fsa.GetAlphabetLength(), states, fstates, edges);
		return min_fsa;
	}
};
