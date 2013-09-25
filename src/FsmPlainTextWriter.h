#pragma once

#include <string>
#include <ostream>
#include <stdint.h>
#include "Dfa.h"

template<typename TFsm>
class FsmPlainTextWriter
{
public:
	typedef TFsm TFsm;
	typedef typename TFsm::TState TState;
	typedef typename TFsm::TSymbol TSymbol;

protected:

public:
	
	void Write(const TFsm& dfa, std::ostream& output)
	{
		using namespace std;
		
		output << "# Estados DFA" << endl;
		output << static_cast<size_t>(dfa.GetStates()) << endl;

		output << "# Simbolos" << endl;
		output << static_cast<size_t>(dfa.GetAlphabetLength()) << endl;
		
		output << "# Iniciales" << endl;		
		auto first = dfa.GetInitials().GetIterator();
		for(auto i=first; !i.IsEnd(); i.MoveNext())
		{
			if(i!=first) output << " ";
			output << i.GetCurrent();
		}
		output << endl;

		output << "# Finales" << endl;
		first = dfa.GetFinals().GetIterator();
		for(auto i=first; !i.IsEnd(); i.MoveNext())
		{
			if(i!=first) output << " ";
			output << i.GetCurrent();
		}
		output << endl;

		output << "# Transiciones (qs, c, qt)" << endl;
		for(TState qs=0; qs<dfa.GetStates(); qs++)
		{
			for(TState qt=0; qt<dfa.GetStates(); qt++)
			{
				for(TSymbol c=0; c<dfa.GetAlphabetLength(); c++)
				{
					if(dfa.IsSuccessor(qs, c, qt))
						output << static_cast<size_t>(qs) << " " << static_cast<size_t>(c) << " " << static_cast<size_t>(qt) << endl;
				}
			}
		}
	}
};