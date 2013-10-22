#pragma once

#include <stdexcept>
#include <string>
#include <istream>
#include <limits>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/classification.hpp>
#include "IFsaReader.h"

template<typename TFsa>
class FsaPlainTextReader : IFsaReader<TFsa>
{
public:
	virtual void ReadHeader(std::istream& str) override
	{
	}

	virtual TFsa Read(std::istream& str) override
	{
		using namespace std;
		using boost::split;
		using boost::trim;
		using boost::algorithm::is_space;

		if(str.eof()) throw invalid_argument("end of file premature");

		string line;
		vector<string> col;

		// states
		getline(str, line);		
		if(line[0] != '#') throw invalid_argument("Formato invalido");

		getline(str, line);
		unsigned long states = stoul(line);
		if(states >= numeric_limits<TState>::max())
		{
			throw invalid_argument("El numero de estados supera el maximo representable");
		}

		// alpha
		getline(str, line);
		if(line[0] != '#') throw invalid_argument("Formato invalido");

		getline(str, line);
		unsigned long alpha = stoul(line);
		if(alpha >= numeric_limits<TSymbol>::max())
		{
			throw invalid_argument("El numero de simbolos supera el maximo representable");
		}

		// initialization
		TFsa dfa(static_cast<TSymbol>(alpha), static_cast<TState>(states));

		// initial
		getline(str, line);
		if(line[0] != '#') throw invalid_argument("Formato invalido");

		col.clear();
		getline(str, line);
		trim(line);
		split(col, line, is_space());		
		for(string i : col)
		{
			TState st = (TState)stol(i);
			dfa.SetInitial(st);
		}

		// final
		getline(str, line);
		if(line[0] != '#') throw invalid_argument("Formato invalido");

		col.clear();
		getline(str, line);	
		trim(line);
		split(col, line, is_space());
		for(string i : col)
		{
			TState st = (TState)stol(i);
			dfa.SetFinal(st);
		}

		// transitions
		getline(str, line);
		if(line[0] != '#') throw invalid_argument("Formato invalido");
				
		size_t transitionsRead=0;
		while(!str.eof())
		{
			getline(str, line);

			if(line.empty()) break;

			col.clear();
			trim(line);
			split(col, line, is_space());
			if(col.size() != 3) throw invalid_argument("Formato invalido");

			TState qs = (TState)stol(col[0]);
			TSymbol c = (TSymbol)stol(col[1]);
			TState qt = (TState)stol(col[2]);
			
			dfa.SetTransition(qs, c, qt);
			transitionsRead++;
		}

		return dfa;
	}
};

/** One based files do not have initial states.
*/
template<typename TFsa>
class FsaPlainTextReaderOneBased : IFsaReader<TFsa>
{
public:
	virtual void ReadHeader(std::istream& str) override
	{
	}

	virtual TFsa Read(std::istream& str) override
	{
		using namespace std;
		using boost::split;
		using boost::trim;
		using boost::algorithm::is_space;

		if(str.eof()) throw invalid_argument("end of file premature");

		string line;
		vector<string> col;

		// states
		getline(str, line);		
		if(line[0] != '#') throw invalid_argument("Formato invalido");

		getline(str, line);
		unsigned long states = stoul(line);
		if(states >= numeric_limits<TState>::max())
		{
			throw invalid_argument("El numero de estados supera el maximo representable");
		}

		// alpha
		getline(str, line);
		if(line[0] != '#') throw invalid_argument("Formato invalido");

		getline(str, line);
		vector<string> alpha_set_line;
		trim(line);
		split(alpha_set_line, line, is_space());
		size_t alpha = alpha_set_line.size();
		if(alpha >= numeric_limits<TSymbol>::max())
		{
			throw invalid_argument("El numero de simbolos supera el maximo representable");
		}

		// initialization
		TFsa dfa(static_cast<TSymbol>(alpha), static_cast<TState>(states));
				
		dfa.SetInitial(0);

		// final
		getline(str, line);
		if(line[0] != '#') throw invalid_argument("Formato invalido");

		col.clear();
		getline(str, line);	
		trim(line);
		split(col, line, is_space());
		for(string i : col)
		{
			TState st = (TState)stol(i);
			if(st == 0) throw invalid_argument("Formato invalido, no basado en 1");
			dfa.SetFinal(st-1);
		}

		// transitions
		getline(str, line);
		if(line[0] != '#') throw invalid_argument("Formato invalido");
				
		size_t transitionsRead=0;
		while(!str.eof())
		{
			getline(str, line);

			if(line.empty()) break;

			col.clear();
			trim(line);
			split(col, line, is_space());
			if(col.size() != 3) throw invalid_argument("Formato invalido");

			TState qs = (TState)stol(col[0]);
			TSymbol c = (TSymbol)stol(col[1]);
			TState qt = (TState)stol(col[2]);
			
			dfa.SetTransition(qs-1, c-1, qt-1);
			transitionsRead++;
		}

		return dfa;
	}
};
