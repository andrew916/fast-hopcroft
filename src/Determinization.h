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
		if((nfa.Initial & nfa.Final).any()) 
		{
			// si alguno de los estados iniciales es tambien final
			// este primer estado es final
			final_states.push_back(0);
		}
		
		TSet next(nfa.GetStates());
		
		TDfaState current_state_index = 0;
		
		while(current_state_index < new_states.size())
		{
			auto current = new_states[current_state_index];
			for(TSymbol c=0; c<nfa.GetAlphabetLength(); c++)
			{				
				next.reset();				
				for(auto s=current.find_first(); s!=current.npos; s=current.find_next(s))
				{
					TNfaState qs = static_cast<TNfaState>(s);
					next |= nfa.GetSuccessors(qs, c);
				}
				TDfaState target_state_index;
				auto found = find(new_states.begin(), new_states.end(), next);
				if(found == new_states.end())
				{
					target_state_index = static_cast<TDfaState>(new_states.size());
					new_states.push_back(next);
					// detecta si contiene un final, para marcarlo como final
					auto finalMask = nfa.Final & next;
					if(finalMask.any())
					{
						final_states.push_back(target_state_index);
					}					
				} 
				else
				{
					target_state_index = static_cast<TDfaState>(found - new_states.begin());
				}
				new_edges.push_back(TEdge(current_state_index, c, target_state_index));
			}
			current_state_index++;
		}

		// el dfa solo puede ser creado aqui porque no es mutable
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
			TDfaState qt = get<2>(edge);
			dfa.SetTransition(qs, c, qt);
		}
		return dfa;
	}
};