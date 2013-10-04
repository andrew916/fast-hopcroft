// July 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include "Determinization.h"
#include "Dfa.h"

/// Brzozowski's FSA Minimization Algorithm.
template<typename _TFsa, typename _TDfa = Dfa<typename _TFsa::TState, typename _TFsa::TSymbol>>
class MinimizationBrzozowski
{
public:	
	typedef _TFsa TFsa;
	typedef _TDfa TDfa;
	typedef typename TFsa::TState TState;
	typedef typename TFsa::TSymbol TSymbol;
		
	typedef Determinization<TFsa, TFsa> TDeterminization;
	typedef Determinization<TDfa, TFsa> TDeterminization2;
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

	TDfa BuildDfa(TSymbol alpha, TDfaState states, const TVectorDfaState& final_states, const TVectorDfaEdge& edges)
	{
		TDeterminization2 det;
		TDfa dfa = det.BuildDfa(alpha, states, final_states, edges);
		return dfa;
	}

	TDfa Minimize(const TFsa& fsa)
	{
		TDfaState states;
		TVectorDfaState fstates;
		TVectorDfaEdge edges;
		Minimize(fsa, &states, fstates, edges);
		TDfa min_fsa = BuildDfa(fsa.GetAlphabetLength(), states, fstates, edges);
		return min_fsa;
	}
};
