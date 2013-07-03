#pragma once

#include <string>
#include <map>
#include <unordered_set>
#include <vector>
#include <istream>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <stdint.h>
#include "Dfa.h"

template<class TState, class TSymbol, class TToken = uint64_t>
class AfdParser
{
public:
	typedef Dfa<TState, TSymbol, TToken> TDfa;

protected:
	int state;
	bool second_step;
	std::string token;

	int number_states;
	std::unordered_set<int> initial_states;
	std::unordered_set<int> final_states;
	std::map<char,int> alphabet;
	std::vector<std::tuple<int,char,int>> transitions;	

	void expect(std::string ref)
	{
		if(token == ref) state++;
		else 
		{
			auto msg = std::string("Se esperaba el literal: ")+ref; 
			throw std::exception(msg.c_str()); 
		}
	}

	void read(int& i)
	{
		if(!std::all_of(token.begin(), token.end(), isdigit))
		{
			throw std::exception("Se esperaba token numerico");
		}
		i = boost::lexical_cast<int,std::string>(token);
		state++;
	}

	void read(char& i)
	{
		if(token.size() != 1 || !isalpha(token[0]))
		{
			throw std::exception("Se esperaba token caracter");
		}
		i = token[0];
		state++;
	}

	bool is_numeric()
	{
		return isdigit(token[0]) != 0;
	}
	
	void process_token()
	{
		int i;
		char a;
		switch (state)
		{
		case 0: expect("fa"); break;
		case 1: expect("("); break;
		case 2: expect("r"); break;
		case 3: expect("("); break;
		case 4: expect("fsa_frozen"); break;
		case 5: expect(")"); break;
		case 6: expect(","); break;
		case 7: read(number_states); break;
		case 8: expect(","); break;
		
		case 9: expect("["); break;
		case 10: read(i); break;
		case 11: if(token == ",") { expect(","); state = 10; } else { expect("]"); } break;
		case 12: expect(","); if(second_step) state = 13; else { second_step = true; state = 9; } break;

		case 13:  expect("["); break;
		case 14: expect("trans"); break;
		case 15: expect("("); break;
		case 16: read(i); break;
		case 17: expect(","); break;
		case 18: read(a); break;
		case 19: expect(","); break;
		case 20: read(i); break;
		case 21: expect(")"); break;			
		case 22: if(token == ",") { expect(","); state = 14; } else { expect("]"); } break;

		case 23: expect(","); break;
		case 24: expect("["); break;
		case 25: expect("]"); break;
		case 26: expect(")"); break;
		case 27: expect("."); break;

		default:
			break;
		}
	}

public:
	AfdParser()
	{
	}

	~AfdParser()
	{
	}

		
	TDfa Parse(std::istream& is)
	{
		state = 0;
		second_step = false;
				
		// lexer
		while(is.good())
		{
			char c;
			is.read(&c, 1);

			if(c == '%') 
			{
				if(!token.empty()) process_token();
				std::string line;
				std::getline(is, line);	// skip line			
				token.clear();
			}
			else if(isspace(c))
			{
				if(!token.empty()) process_token();
				token.clear();
			}
			else if(isalpha(c) || c=='_') 
			{
				if( !token.empty() && !(isalpha(token.back())||(token.back()=='_')) ) 
				{
					process_token();
					token.clear();
				}
				token.push_back(c);
			}
			else if(isdigit(c)) 
			{
				if(!token.empty() && !isdigit(token.back())) 
				{
					process_token();
					token.clear();
				}
				token.push_back(c);
			} else {
				if(!token.empty()) process_token();
				token.clear();
				token.push_back(c);
			}
		}
		return TDfa(alphabet.size(), number_states);
	}
};
