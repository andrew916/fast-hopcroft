// July 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include <vector>
#include <algorithm>
#include <unordered_map>

template<typename TDfa, typename TNfa, typename TSet=typename TNfa::TSet, typename TSetHash=typename TSet::hash>
class Determinization
{
public:	
	typedef typename TNfa::TState TNfaState;
	typedef typename TDfa::TState TDfaState;
	typedef typename TNfa::TSymbol TSymbol;	

private:

public:
	TDfa Determinize(TNfa nfa)
	{	
		assert(!nfa.GetInitials().IsEmpty());

		using namespace std;

		typedef tuple<TDfaState,TSymbol,TDfaState> TEdge;
		typedef unordered_map<TSet, TDfaState, TSetHash> TStatesMap;
		
		// hash map para almacenar como llave cada conjunto de estados diferente
		// y como valor el indice que lo identifica como estado unico.
		// Sirve para obtener el indice de un conjunto de estados
		TStatesMap new_states_map;
		// Vector para almacenar con orden cada conjunto de estados.
		// Sirve para convertir un indice en un conjunto de estados.
		vector<TSet> new_states_vec;

		vector<TEdge> new_edges;
		vector<TDfaState> final_states;
		
		// inserta los estados iniciales en ambas colecciones
		// como un solo conjunto de estados
		new_states_map.insert(TStatesMap::value_type(nfa.GetInitials(), 0));
		new_states_vec.push_back(nfa.GetInitials());

		if(!TNfa::TSet::Intersect(nfa.GetInitials(), nfa.GetFinals()).IsEmpty()) 
		{
			// si alguno de los estados iniciales es tambien final
			// este primer estado es final
			final_states.push_back(0);
		}
		
		TSet next(nfa.GetStates());
		
		TDfaState current_state_index = 0;
		
		while(current_state_index < new_states_vec.size())
		{
			const auto current = new_states_vec[current_state_index];
			for(TSymbol c=0; c<nfa.GetAlphabetLength(); c++)
			{
				next.Clear();				
				for(auto s=current.GetIterator(); !s.IsEnd(); s.MoveNext())
				{
					TNfaState qs = s.GetCurrent();
					next.UnionWith(nfa.GetSuccessors(qs, c));
				}
				TDfaState target_state_index;
				target_state_index = static_cast<TDfaState>(new_states_vec.size());
				// intenta insertar el conjunto de estados, si ya lo contiene no hace nada
				auto fn = new_states_map.insert(TStatesMap::value_type(next, target_state_index));
				auto was_inserted = fn.second;
				if(was_inserted)
				{
					new_states_vec.push_back(next);
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

		// el dfa solo puede ser creado aqui porque no es mutable
		TDfa dfa(nfa.GetAlphabetLength(), new_states_vec.size());
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