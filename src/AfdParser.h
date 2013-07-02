#pragma once

#include <string>
#include <iostream>
#include <exception>
#include <boost/spirit/include/qi.hpp>
#include "Dfa.h"

template<typename _Iterator>
struct AfdParserGrammar : boost::spirit::qi::grammar<_Iterator, boost::spirit::ascii::space_type>
{
	boost::spirit::qi::rule<_Iterator> fa_args;
	boost::spirit::qi::rule<_Iterator> fa_arg1;
	boost::spirit::qi::rule<_Iterator> initial_states;
	boost::spirit::qi::rule<_Iterator> final_states;
	boost::spirit::qi::rule<_Iterator> transitions;
	boost::spirit::qi::rule<_Iterator> transition;
	boost::spirit::qi::rule<_Iterator> integer_list;
	boost::spirit::qi::rule<_Iterator> jumps;
	boost::spirit::qi::rule<_Iterator> start;

	AfdParserGrammar() : AfdParserGrammar::base_type(start)
	{
		using boost::spirit::qi::int_;
		using boost::spirit::qi::char_;
		using boost::spirit::qi::lit;
		using boost::spirit::qi::lexeme;
		using boost::spirit::qi::alpha;
		using boost::spirit::qi::string;
		
		start %= lit("fa") 
			>> '('
			>> fa_args
			>> ')'
			>> '.';

		fa_args %= fa_arg1
			>> num_states >> ','
			>> initial_states >> ','			
			>> final_states >> ','
			>> transitions >> ','
			>> jumps;

		num_states %= int_;

		initial_states %= integer_list;

		final_states %= integer_list;

		transitions %= lit('[') 
			>> transition 
			>> *( ',' transition )
			>> ']';

		transition %= lit("trans") 
			>> '('
			>> int_ // state 1
			>> ','
			>> char_ // symbol
			>> ','
			>> int_ // state 2
			>> ')';

		integer_list %= lit('[') 
			>> int_ >> *( ',' >> int_ );
			>> ']';

		fa_arg1 %= lit("r(fsa_frozen)");
	}
};

template<class TState, class TSymbol, class TToken = uint64_t>
class AfdParser 
{
public:
	typedef Dfa<TState, TSymbol, TToken> TDfa;
	
public:	
	typedef std::string::const_iterator iterator_type;
	typedef AfdParserGrammar<iterator_type> AfdParserGrammar;

	AfdParserGrammar g; // Our grammar
		
	template<typename _Iter>
	TDfa& Parse(_Iter first, _Iter last)
	{			
		bool r = boost::spirit::qi::phrase_parse(first, last, g, skipper, ast);
		return TDfa(1,1);
	}

	AfdParser()
	{
	}

	~AfdParser()
	{
	}
};
