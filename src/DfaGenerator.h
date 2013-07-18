// June 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include <stdint.h>
#include <boost/random/mersenne_twister.hpp>
#include "Dfa.h"

template<typename TState, typename TSymbol, typename TToken = uint64_t, typename TRandomGen = boost::random::mt19937>
class DfaGenerator
{
public:
	typedef Dfa<TState, TSymbol, TToken> TDfa;

protected:
public:
	
	TDfa Generate(int alpha, int states, TRandomGen& gen)
	{
		using namespace std;

		TDfa dfa(alpha, states);
		
	}

};