#pragma once

#include <string>
#include <istream>
#include "Dfa.h"
#include <boost\algorithm\string\split.hpp>
#include <boost\algorithm\string\trim.hpp>

template<typename TFsm>
class FsmPlainTextReader
{
public:
	typedef typename TFsm::TState TState;
	typedef typename TFsm::TSymbol TSymbol;

protected:	

public:
	TFsm Read(std::istream& str)
	{
		using namespace std;
		using boost::split;
		using boost::trim;

		if(str.eof()) throw exception();

		string line;
		vector<string> col;

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
		TFsm dfa(alpha, states);

		// initial
		getline(str, line);
		if(line[0] != '#') throw exception();

		col.clear();
		getline(str, line);
		trim(line);
		split(col, line, isspace);		
		for(string i : col)
		{
			TState st = (TState)stol(i);
			dfa.SetInitial(st);
		}

		// final
		getline(str, line);
		if(line[0] != '#') throw exception();

		col.clear();
		getline(str, line);	
		trim(line);
		split(col, line, isspace);
		for(string i : col)
		{
			TState st = (TState)stol(i);
			dfa.SetFinal(st);
		}

		// transitions
		getline(str, line);
		if(line[0] != '#') throw exception();
				
		size_t transitionsRead=0;
		while(!str.eof())
		{
			getline(str, line);

			if(line.empty()) break;

			col.clear();
			trim(line);
			split(col, line, isspace);
			if(col.size() != 3) throw exception();

			TState qs = (TState)stol(col[0]);
			TSymbol c = (TSymbol)stol(col[1]);
			TState qt = (TState)stol(col[2]);
			
			dfa.SetTransition(qs, c, qt);
			transitionsRead++;
		}

		return dfa;
	}
};