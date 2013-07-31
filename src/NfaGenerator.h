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
	
	template<typename TDist>
	TState RandomChoice(const TSet& reach, TRandGen& gen, TDist& state_dist)
	{
		size_t q;
		do
		{
			q = state_dist(gen);
			if(reach.test(q)) break;
			q = reach.find_next(q);
		} while(q==reach.npos);
		return static_cast<TState>(q);
	}

	TNfa Generate(size_t states, size_t alpha, size_t initials, size_t finals, float density, TRandGen& gen)
	{
		using namespace std;
		uniform_int_distribution<TState> state_dist(0, static_cast<TState>(states-1));
		uniform_int_distribution<TSymbol> sym_dist(0, static_cast<TSymbol>(alpha-1));
		uniform_real_distribution<float> p_dist;

		TNfa nfa(alpha, states);
		
		for(size_t i=0; i<initials;)
		{
			TState qi = state_dist(gen);
			if(nfa.IsInitial(qi))
			{
				continue;
			}
			nfa.SetInitial(qi);
			i++;
		}
		
		for(size_t i=0; i<finals;)
		{
			TState qi = state_dist(gen);
			if(nfa.IsFinal(qi))
			{
				continue;
			}
			nfa.SetFinal(qi);
			i++;
		}

		TSet reach = nfa.Initial;
		TSet nreach = nfa.Final;
		for(TState i=0; i<states; i++)
		{
			if(!reach.test(i))
			{
				TState qs = RandomChoice(reach, gen, state_dist);
				reach.set(i);
				TSymbol c = sym_dist(gen);
				nfa.SetTransition(qs, c, i);
			}
			if(!nreach.test(i))
			{
				TState qt = RandomChoice(nreach, gen, state_dist);
				nreach.set(i);
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