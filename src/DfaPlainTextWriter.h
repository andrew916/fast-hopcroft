#pragma once

#include <string>
#include <ostream>
#include <stdint.h>
#include "Dfa.h"

template<class TState, class TSymbol, class TToken = uint64_t>
class DfaPlainTextWriter
{
public:
	typedef Dfa<TState, TSymbol, TToken> TDfa;

protected:

public:
	
	void Write(const TDfa& dfa, std::ostream& output)
	{
		using namespace std;
		
		output << "# Estados DFA" << endl;
		output << dfa.GetStates() << endl;

		output << "# Simbolos" << endl;
		output << dfa.GetAlphabetLength() << endl;
		
		output << "# Iniciales" << endl;		
		auto first = dfa.Initial.find_first();
		for(auto i=first; i!=dfa.Initial.npos; i=dfa.Initial.find_next(i))
		{
			if(i!=first) output << " ";
			output << i;
		}
		output << endl;

		output << "# Finales" << endl;
		first = dfa.Final.find_first();
		for(auto i=first; i!=dfa.Final.npos; i=dfa.Final.find_next(i))
		{
			if(i!=first) output << " ";
			output << i;
		}
		output << endl;

		output << "# Transiciones (qs, c, qt)" << endl;
		for(TState qs=0; qs<dfa.GetStates(); qs++)
		{
			for(TState qt=0; qt<dfa.GetStates(); qt++)
			{
				for(TSymbol c=0; c<dfa.GetAlphabetLength(); c++)
				{
					output << (size_t)qs << " " << (size_t)c << " " << (size_t)qt << endl;
				}
			}
		}
	}
};