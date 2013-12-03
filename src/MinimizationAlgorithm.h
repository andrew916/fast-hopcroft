#pragma once

#include <iostream>
#include <string>


enum class MinimizationAlgorithm 
{
	Hopcroft,
	Brzozowski,
	Incremental,	
	Hybrid,
	Atomic,
};

std::istream& operator>>(std::istream& in, MinimizationAlgorithm& fmt)
{
	std::string token;
	in >> token;
	if (token == "hopcroft") fmt = MinimizationAlgorithm::Hopcroft;
	else if (token == "brzozowski") fmt = MinimizationAlgorithm::Brzozowski;
	else if (token == "incremental") fmt = MinimizationAlgorithm::Incremental;
	else if (token == "hybrid") fmt = MinimizationAlgorithm::Hybrid;
	else if (token == "atomic") fmt = MinimizationAlgorithm::Atomic;
	else throw std::invalid_argument("unknown format");
	return in;
}

std::ostream& operator<<(std::ostream& on, const MinimizationAlgorithm& fmt)
{
	std::string token;
	if(fmt == MinimizationAlgorithm::Hopcroft) token = "hopcroft";
	else if(fmt == MinimizationAlgorithm::Brzozowski) token = "brzozowski";
	else if(fmt == MinimizationAlgorithm::Incremental) token = "incremental";
	else if(fmt == MinimizationAlgorithm::Hybrid) token = "hybrid";
	else if(fmt == MinimizationAlgorithm::Atomic) token = "atomic";
	else throw std::invalid_argument("unknown format");    
	return on << token;
}
