#pragma once

#include <string>
#include <ostream>
#include <stdint.h>
#include "Dfa.h"

template<class TState, class TSymbol, class TToken = uint64_t>
class DfaPlainTextExporter
{
public:
	typedef Dfa<TState, TSymbol, TToken> TDfa;

protected:

public:
	
	void Export(const TDfa& dfa, std::ostream str)
	{
	}
};