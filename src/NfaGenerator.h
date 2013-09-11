// June 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include <random>
#include <algorithm>

template<typename TNfa, typename TRandGen>
class NfaGenerator
{
public:
	typedef typename TNfa::TState TState;
	typedef typename TNfa::TSymbol TSymbol;
	typedef typename TNfa::TSet TSet;

private:

public:
	
	template<typename TSet, typename TRandGen>
	typename TSet::TElement RandomChoice(const TSet& reach, TRandGen& gen, std::uniform_int<typename TSet::TElement>& state_dist)
	{
		TState q;
		do
		{
			q = state_dist(gen);
			if(reach.Contains(q)) break;
			//q = reach.find_next(q);
		} while(true); //while(q==reach.npos);
		return q;
	}

	TNfa Generate_v2(TState states, TSymbol alpha, TState initials, TState finals, float* density, TRandGen& gen)
	{
		using namespace std;
		uniform_int_distribution<TState> state_dist(0, states-1);
		uniform_int_distribution<TSymbol> sym_dist(0, alpha-1);
		uniform_real_distribution<float> p_dist;

		typedef tuple<TState,TSymbol,TState> TEdge;

		TNfa nfa(alpha, states);		
								
		// Determina estados iniciales
		for(TState i=0; i<initials;)
		{
			TState qi = state_dist(gen);
			if(nfa.IsInitial(qi))
			{
				continue;
			}
			nfa.SetInitial(qi);
			i++;
		}
		
		// Determina estados finales
		for(TState i=0; i<finals;)
		{
			TState qi = state_dist(gen);
			if(nfa.IsFinal(qi))
			{
				continue;
			}
			nfa.SetFinal(qi);
			i++;
		}

		size_t total_transitions = states*states*alpha;
		// transitions counter
		size_t n_transitions = 0;
		// required transitions
		size_t r_transitions = static_cast<size_t>(total_transitions * (*density));
		TSet reach = nfa.GetInitials();
		TSet nreach = nfa.GetFinals();
		for(TState i=0; i<states; i++)
		{
			if(!reach.Contains(i))
			{
				TState qs = RandomChoice(reach, gen, state_dist);
				reach.Add(i);
				nreach.Add(qs);
				TSymbol c = sym_dist(gen);
				nfa.SetTransition(qs, c, i);
				n_transitions++;
			}
			if(!nreach.Contains(i))
			{
				TState qt = RandomChoice(nreach, gen, state_dist);
				nreach.Add(i);
				reach.Add(qt);
				TSymbol c = sym_dist(gen);
				nfa.SetTransition(i, c, qt);
				n_transitions++;
			}
		}

		if(n_transitions > r_transitions) 
		{
			*density = (float)n_transitions / (float)total_transitions;
			return nfa;
		}

		vector<TEdge> remaining(states*states*alpha);
		auto i = remaining.begin();
		for(TState qs=0; qs<states; qs++)
			for(TState qt=0; qt<states; qt++)
				for(TSymbol s=0; s<states; s++)
					*i++ = make_tuple(qs,s,qt);

		shuffle(remaining.begin(), remaining.end(), gen);
		auto iter = remaining.begin();
		while(n_transitions < r_transitions)
		{			
			TState qs, qt; TSymbol s;
			tie(qs, s, qt) = *iter++;
			if(!nfa.IsSuccessor(qs, s, qt))
			{
				nfa.SetTransition(qs, s, qt);
				n_transitions++;
			}
		}
		return nfa;
	}

	TNfa Generate(TState states, TSymbol alpha, TState initials, TState finals, float density, TRandGen& gen)
	{
		using namespace std;
		uniform_int_distribution<TState> state_dist(0, states-1);
		uniform_int_distribution<TSymbol> sym_dist(0, alpha-1);
		uniform_real_distribution<float> p_dist;

		TNfa nfa(alpha, states);
		
		// Determina estados iniciales
		for(TState i=0; i<initials;)
		{
			TState qi = state_dist(gen);
			if(nfa.IsInitial(qi))
			{
				continue;
			}
			nfa.SetInitial(qi);
			i++;
		}
		
		// Determina estados finales
		for(TState i=0; i<finals;)
		{
			TState qi = state_dist(gen);
			if(nfa.IsFinal(qi))
			{
				continue;
			}
			nfa.SetFinal(qi);
			i++;
		}

		TSet reach = nfa.GetInitials();
		TSet nreach = nfa.GetFinals();
		for(TState i=0; i<states; i++)
		{
			if(!reach.Contains(i))
			{
				TState qs = RandomChoice(reach, gen, state_dist);
				reach.Add(i);
				TSymbol c = sym_dist(gen);
				nfa.SetTransition(qs, c, i);
			}
			if(!nreach.Contains(i))
			{
				TState qt = RandomChoice(nreach, gen, state_dist);
				nreach.Add(i);
				TSymbol c = sym_dist(gen);
				nfa.SetTransition(i, c, qt);
			}
		}

		for(TState qs=0; qs<states; qs++)
		{
			for(TSymbol c=0; c<alpha; c++)
			{
				for(TState qt=0; qt<states; qt++)
				{
					auto p = p_dist(gen);
					if(p < density)
					{
						nfa.SetTransition(qs, c, qt);
					}
				}
			}
		}
		return nfa;
	}
};