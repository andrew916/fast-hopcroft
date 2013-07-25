// July 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include <vector>
#include <algorithm>

template<typename TDfa, typename TNfa>
class Determinization
{
public:
	typedef typename TNfa::TSet TSet;
	typedef typename TNfa::TState TNfaState;	
	typedef typename TDfa::TState TDfaState;
	typedef typename TNfa::TSymbol TSymbol;	

private:

public:
	TDfa Determinize(TNfa nfa)
	{	
		assert(nfa.Initial.any());

		using namespace std;

		typedef tuple<TDfaState,TSymbol,TDfaState> TEdge;
		
		vector<TSet> new_states;
		vector<TEdge> new_edges;
		vector<TDfaState> final_states;

		new_states.push_back(nfa.Initial);
		
		TSet next(nfa.GetStates());
				
		TDfaState new_state_index = 0;
		TDfaState current_state_index = 0;
		
		for(auto current_iterator=new_states.begin(); current_iterator != new_states.end(); current_iterator++)
		{
			for(TSymbol c=0; c<nfa.GetAlphabetLength(); c++)
			{				
				next.clear();
				auto current = *current_iterator;
				for(auto s=current.find_first(); s!=current.npos; s=current.find_next(s))
				{
					TNfaState qs = (TNfaState)s;
					next |= nfa.GetSuccessors(qs, c);
				}
				auto found = find(new_states.begin(), new_states.end(), next);
				if(found == new_states.end())
				{
					new_state_index = (TDfaState)new_states.size();
					new_states.push_back(next);
					// detecta si contiene un final, para marcarlo como final
					auto finalMask = nfa.Final & next;
					if(finalMask.any())
					{
						final_states.push_back(new_state_index);
					}
				} 
				else 
				{
					new_state_index = (TDfaState)(found - new_states.begin());
				}
				new_edges.push_back(TEdge(current_state_index, c, new_state_index));
			}
			current_state_index++;
		}

		TDfa dfa(nfa.GetAlphabetLength(), new_states.size());
		dfa.SetInitial(0);
		for(auto f : final_states)
		{
			dfa.SetFinal(f);
		}
		for(auto edge : new_edges)
		{
			TDfaState qs = get<0>(edge);
			TSymbol c = get<1>(edge);
			TDfaState qt = get<1>(edge);
			dfa.SetTransition(qs, c, qt);
		}
		return dfa;
	}
};