// July 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include <vector>
#include <algorithm>

template<typename TDfa, typename TNfa>
class Determinization
{
public:
	typedef typename TNfa::TState typedef typename TNfa::TSet TSet;;
	typedef typename TDfa::TState TDfaState;
	typedef typename TNfa::TSymbol TSymbol;
	typedef typename TNfa::TSet TSet;

private:

public:
	TDfa Determinize(TNfa nfa)
	{	
		using namespace std;
		
		vector<TSet> new_states;
		vector<tuple<TState,TSymbol,TState>>

		TSet current=nfa.Initial;
		TSet next(nfa.GetStates());
		
		for(TSymbol c=0; c<nfa.GetAlphabetLength(); c++)
		{
			TState qs = (TState)s;
			next.clear();
			for(auto s=current.find_first(); s!=current.npos; s=current.find_next(s))
			{
				next |= nfa.GetSuccessors(qs, c);
			}
			auto found = find(new_states.begin(), new_states.end(), next);
		}		
	}
};