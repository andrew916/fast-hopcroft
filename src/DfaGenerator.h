// June 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include <algorithm>
#include <random>
#include "Dfa.h"

template<typename TState, typename TSymbol, typename TToken = size_t, typename TRandomGen = std::mt19937>
class DfaBridgeGenerator
{
public:
	typedef Dfa<TState, TSymbol, TToken> TDfa;
	typedef typename std::uniform_int_distribution<TState> TDState;
	typedef typename std::uniform_int_distribution<TSymbol> TDSymbol;
	typedef typename std::uniform_int_distribution<> TD;
	typedef typename TDfa::TSet TSet;

protected:

	TDfa dfa;
	TSet edges_unused;
	TSet reach;
	TDState state_distribution;
	TDState final_state_distribution;
	TDSymbol symbol_distribution;
	int max_steps;
	std::vector<int> shuffle_mapping;
	std::vector<int>::iterator shuffle_mapping_it;

	void InitializeDfa(TRandomGen& gen, int initial, int final)
	{
		int i=initial;
		while(i > 0)
		{
			int st = state_distribution(gen);
			if(!dfa.IsInitial(st))
			{
				dfa.SetInitial(st);
				i--;
			}
		}

		i=final;
		while(i > 0)
		{
			int st = state_distribution(gen);
			if(!dfa.IsFinal(st))
			{
				dfa.SetFinal(st);
				i--;
			}
		}
	}

	TState RandomState(TRandomGen& gen, TDState& dist, const TSet& states)
	{
		TState r = dist(gen);
		auto qf = states.find_first();
		while(r--)
		{
			assert(qf != states.npos);
			qf = states.find_next(qf);
		}
		assert(qf != states.npos);
		return (TState)qf;
	}

	TState RandomFinalState(TRandomGen& gen) 
	{
		return RandomState(gen, final_state_distribution, dfa.Final);
	}

	TSymbol RandomSymbol(TRandomGen& gen)
	{
		return symbol_distribution(gen);
	}

	bool TakeUnusedEdge(TState& state, TSymbol& sym)
	{
		int r;
		do {
			r = *shuffle_mapping_it;		
			shuffle_mapping_it++;
			if(shuffle_mapping_it == shuffle_mapping.end()) 
			{
				return false;
			}
		} while(!edges_unused.test(r));
		state = r / dfa.GetAlphabetLength();
		sym = r % dfa.GetAlphabetLength();
		return true;
	}

	void TakeUnusedInitialEdge(TState qi, TSymbol& sym)
	{		
		for(auto i=shuffle_mapping_it; i!=shuffle_mapping.end(); i++)
		{
			TState state = *i / dfa.GetAlphabetLength();
			sym = *i % dfa.GetAlphabetLength();
			if(state == qi && edges_unused.test(i-shuffle_mapping.begin())) 
			{
				return;
			}
		}
		assert(false);
	}

	void RegisterTransition(TState qs, TSymbol c, TState qt)
	{
		dfa.SetTransition(qs, c, qt);
		edges_unused.reset(qs * dfa.GetAlphabetLength() + c);
		reach.set(qt);
	}

	void GenerateBranch(TRandomGen& gen, TState qi)
	{
		TState qf = RandomFinalState(gen);
		TSymbol c;
		TakeUnusedInitialEdge(qi, c);
		TState qs = qi;
		int step=0;
		do
		{
			TState qt;
			TSymbol ct;
			bool found;
			do {
				found = TakeUnusedEdge(qt, ct);
				if(!found) break;
			}while(qt == qs && ct == c);
			if(!found) break;
			RegisterTransition(qs, c, qt);
			qs = qt;
			c = ct;
		} while (step++ < max_steps && qs != qf);
		if(qs != qf)
		{
			RegisterTransition(qs, c, qf);
		}
	}

	void GenerateIntialBranches(TRandomGen& gen)
	{
		for(auto qi=dfa.Initial.find_first(); qi!=dfa.Initial.npos; qi=dfa.Initial.find_next(qi))
		{
			GenerateBranch(gen, (TState)qi);
		}
	}

	void LinkOrphans()
	{
		auto unreach = ~reach;
		for(auto i=unreach.find_first(); i!=unreach.npos; i=unreach.find_next(i))
		{
			TState qs;
			TSymbol c;
			TState qt = (TState)i;
			bool found = TakeUnusedEdge(qs, c);
			if(!found) continue;
			RegisterTransition(qs, c, qt);
		}
	}

public:

	DfaBridgeGenerator()
		: dfa(0,0), edges_unused(0), reach(0), state_distribution(0), final_state_distribution(0), symbol_distribution(0)
	{
	}

	TDfa Generate(int alpha, int states, int initial_states, int final_states, 
		int max_steps, float density,
		TRandomGen& gen)
	{
		using namespace std;

		dfa = TDfa(alpha, states);
		reach = TSet(states);
		edges_unused = TSet(states * alpha, ~(unsigned long)0);

		state_distribution = TDState(0, states-1);
		final_state_distribution = TDState(0, final_states-1);
		symbol_distribution = TDSymbol(0, alpha-1);
		this->max_steps = max_steps;

		// mapea a un edge
		shuffle_mapping.resize(states * alpha);
		int d=0;
		generate(shuffle_mapping.begin(), shuffle_mapping.end(), [&d] { return d++; });
		shuffle(shuffle_mapping.begin(), shuffle_mapping.end(), gen);
		shuffle_mapping_it = shuffle_mapping.begin();

		// Crea estados iniciales y finales para el dfa
		InitializeDfa(gen, initial_states, final_states);

		// los primeros estados que originan bridges son los iniciales
		GenerateIntialBranches(gen);

		// Crea enlaces hacia los estados huerfanos y desde ellos
		LinkOrphans();

		return dfa;
	}
};
