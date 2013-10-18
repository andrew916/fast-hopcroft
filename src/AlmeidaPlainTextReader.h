#pragma once

#include <istream>
#include <stdexcept>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

template<typename TFsa>
class AlmeidaPlainTextReader
{
public:

	TFsa Read(std::istream& str, typename TFsa::TSymbol alpha, typename TFsa::TState states)
	{
		using namespace std;
		using namespace boost::algorithm;
		using boost::lexical_cast;
		string line;

		if(str.eof()) throw invalid_argument("end of file premature");
		getline(str, line);

		TFsa fsa(alpha, states);
		
		bool reading_finals = false;
		auto n = line.begin();
		TFsa::TState qs = 0;
		TFsa::TSymbol a = 0;
		for(auto i=line.begin(); i!=line.end(); )
		{
			if(*i == ',')
			{				
				if(reading_finals) 
				{
					auto l = lexical_cast<TFsa::TState>(string(n, i));
					fsa.SetFinal(l);
				}
				else 
				{
					auto qt = lexical_cast<TFsa::TSymbol>(string(n, i));
					fsa.SetTransition(qs, a, qt);					
					if(a++ == alpha - 1)
					{
						a = 0;
						qs++;						
						if(qs >= states) throw logic_error("wrong states or symbol number");
					}
				}
				n = ++i;
			}
			if(*i == ' ')
			{
				advance(i, 3);
				reading_finals = true;
			}
		}
		
		return fsa;
	}

private:
public:

};