#pragma once

enum class FsaFormat
{
	None,
	ZeroBasedPlainText,
	OneBasedPlainText,
	GraphViz
};

#include <stdexcept>
#include <iostream>

std::istream& operator>>(std::istream& in, FsaFormat& fmt)
{
	std::string token;
	in >> token;
	if (token == "none") fmt = FsaFormat::None;
	else if (token == "graphviz") fmt = FsaFormat::GraphViz;
	else if (token == "one-based-text") fmt = FsaFormat::OneBasedPlainText;
	else if (token == "zero-based-text") fmt = FsaFormat::ZeroBasedPlainText;
	else throw std::invalid_argument("unknown format");
	return in;
}

std::ostream& operator<<(std::ostream& on, const FsaFormat& fmt)
{
	std::string token;
	if(fmt == FsaFormat::None) token = "none";
	else if(fmt == FsaFormat::GraphViz) token = "graphviz";
	else if(fmt == FsaFormat::OneBasedPlainText) token = "one-based-text";
	else if(fmt == FsaFormat::ZeroBasedPlainText) token = "zero-based-text";
	else throw std::invalid_argument("unknown format");    
	return on << token;
}
