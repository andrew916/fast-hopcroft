// July 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include <vector>
#include <algorithm>
#include <unordered_map>
#include <map>
#include <tuple>

template<typename TDfa, typename TNfa, typename TSet=typename TNfa::TSet, typename TSetHash=typename TSet::hash>
class Determinization
{
public:	
	typedef typename TNfa::TState TNfaState;
	typedef typename TDfa::TState TDfaState;
	typedef typename TNfa::TSymbol TSymbol;	
	typedef std::tuple<TDfaState,TSymbol,TDfaState> TEdge;

	typedef std::vector<TDfaState> TVectorDfaState;
	typedef std::vector<TEdge> TVectorDfaEdge;
private:

public:
	void Determinize(const TNfa& nfa, TDfaState* new_states_count, TVectorDfaState& final_states, TVectorDfaEdge& new_edges)
	{	
		using namespace std;
		
		typedef unordered_map<TSet, TDfaState, TSetHash> TStatesMap;
		//typedef map<TSet, TDfaState> TStatesMap;
		
		// hash map para almacenar como llave cada conjunto de estados diferente
		// y como valor el indice que lo identifica como estado unico.
		// Sirve para obtener el indice de un conjunto de estados
		TStatesMap new_states_map;
		// Vector para almacenar con orden cada conjunto de estados.
		// Sirve para convertir un indice en un conjunto de estados.
		list<TSet> new_states_lst;

		final_states.clear();
		new_edges.clear();
		
		// inserta los estados iniciales en ambas colecciones
		// como un solo conjunto de estados
		if(!nfa.GetInitials().IsEmpty()) 
		{
			new_states_map.insert(make_pair(nfa.GetInitials(), 0));
			new_states_lst.push_back(nfa.GetInitials());
		
			if(!TNfa::TSet::Intersect(nfa.GetInitials(), nfa.GetFinals()).IsEmpty()) 
			{
				// si alguno de los estados iniciales es tambien final
				// este primer estado es final
				final_states.push_back(0);
			}
		}
		
		TSet next(nfa.GetStates());
		
		TDfaState current_state_index = 0;
		for(auto current_it=new_states_lst.cbegin(); current_it != new_states_lst.cend(); current_it++)
		{			
			for(TSymbol c=0; c<nfa.GetAlphabetLength(); c++)
			{				
				next.Clear();
				for(auto s=current_it->GetIterator(); !s.IsEnd(); s.MoveNext())
				{
					TNfaState qs = s.GetCurrent();
					next.UnionWith(nfa.GetSuccessors(qs, c));
				}
				TDfaState target_state_index = static_cast<TDfaState>(new_states_lst.size());
				// intenta insertar el conjunto de estados, si ya lo contiene no hace nada
				auto fn = new_states_map.insert(typename TStatesMap::value_type(next, target_state_index));
				auto was_inserted = fn.second;
				if(was_inserted)
				{					
					new_states_lst.push_back(next);
					// detecta si contiene un final, para marcarlo como final
					next.IntersectWith(nfa.GetFinals());					
					if(!next.IsEmpty())
					{
						final_states.push_back(target_state_index);
					}					
				}
				else
				{
					target_state_index = fn.first->second;
				}
				new_edges.push_back(TEdge(current_state_index, c, target_state_index));
			}
			current_state_index++;
		}
		assert(current_state_index == new_states_lst.size());	
		*new_states_count = current_state_index;
	}
		
	TDfa BuildDfa(TSymbol symbols, TDfaState states, const TVectorDfaState& final_states, const TVectorDfaEdge& new_edges)
	{
		using namespace std;
		// el dfa solo puede ser creado aqui porque no es mutable
		TDfa dfa(symbols, states);
		if(states > 0) dfa.SetInitial(0);
		for(auto f : final_states)
		{
			dfa.SetFinal(f);
		}
		for(auto edge : new_edges)
		{
			TDfaState qs, qt;
			TSymbol c;
			tie(qs, c, qt) = edge;
			dfa.SetTransition(qs, c, qt);
		}
		return dfa;
	}

	TDfa Determinize(TNfa nfa)
	{
		TDfaState states;
		TVectorDfaState fstates;
		TVectorDfaEdge edges;
		Determinize(nfa, &states, fstates, edges);
		TDfa dfa = BuildDfa(nfa.GetAlphabetLength(), states, fstates, edges);
		return dfa;
	}
};