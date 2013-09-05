// June 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include <random>

template<typename TNfa, typename TRandGen>
class NfaGenerator
{
public:
	typedef typename TNfa::TState TState;
	typedef typename TNfa::TSymbol TSymbol;
	typedef typename TNfa::TSet TSet;

private:

public:
	
	TState RandomChoice(const TSet& reach, TRandGen& gen, std::uniform_int<TState>& state_dist)
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