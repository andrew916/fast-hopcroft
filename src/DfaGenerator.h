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


	TDfa GenerateBridge(int alpha, int states, int initial_states, int final_states, 
		int max_steps, float density,
		TRandomGen& gen)
	{
		using namespace std;
		using namespace boost::random;

		typedef typename uniform_int_distribution<TState> TDState;
		typedef typename uniform_int_distribution<TSymbol> TDSymbol;
		typedef typename uniform_int_distribution<> TD;
		typedef TDfa::TSet TSet;

		TDfa dfa(alpha, states);
		TDState ud(0, states-1);
		TDState td(0, final_states-1);
		TDSymbol sd(0, alpha-1);
		TD ssd(0, states * alpha);
		TSet reach(states);
		TSet edges(states*alpha);

		InitializeDfa(dfa, ud, gen,  initial_states, final_states);

		// los primeros estados que originan bridges son los iniciales
		TSet origin = dfa.Initial;
				
process_origin:
		// para cada estado origen
		for(auto i=origin.find_first(); i!=origin.npos; i=origin.find_next(i))
		{
			reach.set(i);

			// selecciona un estado final
			size_t selected_final = td(gen);

			// obtiene el estado final seleccionado
			TState k=dfa.Final.find_first();
			while(selected_final--) k=dfa.Final.find_next(k);

			TState j = i;
			TState t;
			TSymbol c;
			for(int step = 0; step<max_steps; step++)
			{
				t = ud(gen);
				c = sd(gen);

				if(t == k) break;

				dfa.SetTransition(j, c, t);

				reach.set(t);
				j = t;
			}
			// cerrar el bridge
			dfa.SetTransition(j, c, k);
			reach.set(k);
		}

		// alcanza los que faltaron
		TSet unreach = ~reach;
		for(auto i=unreach.find_first(); i!=unreach.npos; i=unreach.find_next(i))
		{
			// selecciona un estado final
			TState pred;
			do { 
				pred = ud(gen);
			}while(unreach.test(pred));
			// simbolo de transicion
			TSymbol c = sd(gen);
			dfa.SetTransition(pred, c, i);			
		}
		origin = unreach;
		if(origin.any()) goto process_origin;

		return dfa;
	}
};
