#pragma once

#include <ostream>

template<typename TFsa>
class IFsaWriter 
{
public:
	typedef typename TFsa::TSymbol TSymbol;
	typedef typename TFsa::TState TState;

	virtual void WriteHeader(std::ostream& str) = 0;
	virtual void Write(const TFsa& fsa, std::ostream& str) = 0;
};
