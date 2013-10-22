#pragma once

#include <istream>

template<typename TFsa>
class IFsaReader
{
public:
	typedef typename TFsa::TSymbol TSymbol;
	typedef typename TFsa::TState TState;

	virtual void ReadHeader(std::istream& stream) = 0;
	virtual TFsa Read(std::istream& stream) = 0;
};