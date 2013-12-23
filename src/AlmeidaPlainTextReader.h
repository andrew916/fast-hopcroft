#pragma once

#include <istream>
#include <stdexcept>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "IFsaReader.h"

template<typename TFsa>
class AlmeidaPlainTextReader : public IFsaReader<TFsa>
{
private:
	TSymbol alpha;
	TState states;

public:
	
	AlmeidaPlainTextReader(TSymbol _alpha, TState _states)
		: alpha(_alpha), states(_states)
	{
	}

	AlmeidaPlainTextReader() 
		: alpha(0), states(0)
	{
	}

	void SetParameters(TSymbol _alpha, TState _states)
	{
		alpha = _alpha;
		states = _states;
	}

	virtual void ReadHeader(std::istream& str) override
	{
		using namespace std;
		using namespace boost::algorithm;
		string line;
		getline(str, line);
		getline(str, line);
	}

	virtual TFsa Read(std::istream& str) override
	{
		using namespace std;
		using namespace boost::algorithm;
		using boost::lexical_cast;

		string line;

		if(str.eof()) throw logic_error("premature end of file");
		getline(str, line);
		trim_left(line);

		TFsa fsa(alpha, states);

		bool reading_finals = false;
		auto n = line.begin();

		TState qs = 0;
		TSymbol a = 0;

		for(auto i=line.begin(); i!=line.end(); i++)
		{
			if(*i != ',' && *i != ' ') continue;
			string token(n, i);
			if(reading_finals) 
			{
				auto qf = lexical_cast<TFsa::TState>(token);
				fsa.SetFinal(qf);
			}
			else 
			{
				auto qt = lexical_cast<TFsa::TSymbol>(token);
				fsa.SetTransition(qs, a, qt);					
				if(a++ == alpha - 1)
				{
					a = 0;
					qs++;					
				}
			}			
			if(*i == ' ') // begin of " | "
			{
				i = std::find(i, line.end(), '|');				
				advance(i, 2); // skip " | "
				n = i; // begin of number
				if(i == line.end()) break;
				reading_finals = true;				
			} else {
				n = ++i;
			}
		}
		if(n != line.end())
		{
			string token(n, line.end());
			auto qf = lexical_cast<TFsa::TState>(token);
			fsa.SetFinal(qf);
		}

		fsa.SetInitial(0);

		return fsa;
	}

};