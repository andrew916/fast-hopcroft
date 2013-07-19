// June 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include <stdint.h>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include "Dfa.h"

template<typename TState, typename TSymbol, typename TToken = uint64_t, typename TRandomGen = boost::random::mt19937>
class DfaGenerator
{
public:
	typedef Dfa<TState, TSymbol, TToken> TDfa;

protected:

	template<typename TDist>
	void InitializeDfa(TDfa& dfa, TDist& ud, TRandomGen& gen,  int initial, int final)
	{
		int i=initial;
		while(i > 0)
		{
			int st = ud(gen);
			if(!dfa.IsInitial(st))
			{
				dfa.SetInitial(st);
				i--;
			}
		}

		i=final;
		while(i > 0)
		{
			int st = ud(gen);
			if(!dfa.IsFinal(st))
			{
				dfa.SetFinal(st);
				i--;
			}
		}
	}

public:

	TDfa Generate(int alpha, int states, int initial_states, int final_states, float density, TRandomGen& gen=boost::random::mt19937(), bool use_null_state=true)
	{
		using namespace std;
		using namespace boost::random;

		TState min_state = use_null_state ? 1 : 0;

		TDfa dfa(alpha, states);
		uniform_int_distribution<TState> ud(min_state, states-1);

		InitializeDfa(dfa, ud, gen,  initial_states, final_states);

		uniform_int_distribution<TSymbol> sd(0, alpha-1);		
		int max = (int)(states * alpha * density); 
		while(max--)
		{
			TState src = ud(gen);
			TState dst = ud(gen);
			TSymbol sym = sd(gen);
			dfa.SetTransition(src, sym, dst);
		}

		return dfa;
	}

	TDfa Generate2(int alpha, int states, int initial_states, int final_states, float density, TRandomGen& gen=boost::random::mt19937(), bool use_null_state=true)
	{
		using namespace std;
		using namespace boost::random;

		TDfa dfa(alpha, states);
		uniform_int_distribution<TState> ud(min_state, states-1);
		InitializeDfa(dfa, ud, gen,  initial_states, final_states);

		uniform_int_distribution<TSymbol> sd(0, alpha-1);

		return dfa;
	}


	TDfa GenerateBridge(int alpha, int states, int initial_states, int final_states, 
		int max_steps, float density,
		TRandomGen& gen)
	{
		using namespace std;
		using namespace boost::random;

		typedef typename uniform_int_distribution<TState> TDState;
		typedef typename uniform_int_distribution<TSymbol> TDSymbol;
		typedef typename uniform_int_distribution<> TD;

		TDfa dfa(alpha, states);
		TDState ud(0, states-1);
		TDState td(0, final_states-1);
		TDState sd(0, alpha-1);
		
		InitializeDfa(dfa, ud, gen,  initial_states, final_states);

		for(auto i=dfa.Initial.find_first(); i!=dfa.Initial.npos; i=dfa.Initial.find_next(i))
		{
			int selected_final = td(gen);
			for(auto k=dfa.Final.find_first(); k!=dfa.Final.npos; k=dfa.Final.find_next(k))
			{
				if(selected_final-- != 0) continue;
				TState j = i;
				TState t;
				TSymbol s;
				for(int step = 0; step<max_steps; step++)
				{
					t = ud(gen);
					s = sd(gen);
					dfa.SetTransition(j, s, t);
					if(t==k) break;
					j = t;
				}
				if(t!=k) dfa.SetTransition(t, s, k);
				break;
			}
		}
		return dfa;
	}
};
