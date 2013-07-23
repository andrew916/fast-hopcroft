#pragma once

#include <string>
#include <istream>
#include "Dfa.h"
#include <boost\algorithm\string\split.hpp>

template<class TState, class TSymbol, class TToken = uint64_t>
class AfdParser2
{
public:
	typedef Dfa<TState, TSymbol, TToken> TDfa;

protected:	

public:
	TDfa Parse(std::istream& str)
	{
		using namespace std;

		if(str.eof()) throw exception();

		string line;

		// states
		getline(str, line);		
		if(line[0] != '#') throw exception();

		getline(str, line);
		long states = stol(line);

		// alpha
		getline(str, line);
		if(line[0] != '#') throw exception();

		getline(str, line);
		long alpha = stol(line);

		// initialization
		TDfa dfa(alpha, states);

		// initial
		getline(str, line);
		if(line[0] != '#') throw exception();

		auto ini = boost::split(line, ' ');
		for(auto i : ini)
		{
			TState st = (TState)stol(st);
			dfa.SetInitial(st);
		}

		// final
		getline(str, line);
		if(line[0] != '#') throw exception();

		auto fin = boost::split(line, ' ');
		for(auto i : fin)
		{
			TState st = (TState)stol(st);
			dfa.SetFinal(st);
		}

		// transitions
		getline(str, line);
		if(line[0] != '#') throw exception();

		while(!str.eof())
		{
			getline(str, line);
			auto parts = boost::split(line, ' ');
			if(parts.size() != 3) throw exception();

			TState qs = (TState)stol(parts[0]);
			TSymbol c = (TSymbol)stol(parts[1]);
			TState qt = (TState)stol(parts[2]);
			
			dfa.SetTransition(qs, c, qt);
		}

		return dfa;
	}
};