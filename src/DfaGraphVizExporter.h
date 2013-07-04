#pragma once

#include <fstream>
#include <string>
#include "Dfa.h"

template<class TState, class TSymbol>
class DfaGraphVizExporter
{
public:
	typedef Dfa<TState, TSymbol> TDfa;
	typedef typename TDfa::TSet TSet;
	
protected:
public:
	
	void Export(const TDfa& dfa, std::ofstream& out, bool ignoreZeroState = false)
	{
		static const std::string initialStyle = "style=\"filled\"";
		static const std::string finalStyle = "style=\"bold,dashed\"";
		static const std::string initialFinalStyle = "style=\"filled,bold,dashed\"";

		out << "/* Generated with FastHopcroft */" << std::endl;
		out << "digraph \"NDFA\" {" << std::endl;
		out << "  rankdir=LR" << std::endl;
		out << "  node [shape=box width=0.1 height=0.1 fontname=Arial]" << std::endl;
		out << "  edge [fontname=Arial]" << std::endl;

		out << "/* Estados */" << std::endl;
		for(TState st=0; st<dfa.GetStates(); st++)
		{
			if(ignoreZeroState && st==0) continue;

			auto isInitial = dfa.IsInitial(st);
			auto isFinal = dfa.IsFinal(st);
			std::string fmt;
			if (isInitial && !isFinal) fmt = initialStyle;
			else if (!isInitial && isFinal) fmt = finalStyle;
			else if (isInitial && isFinal) fmt = initialFinalStyle;

			out << " s" << (uint64_t)st << " [label=\"" << (uint64_t)st << "\" " << fmt << "] " 
				<< "/* I:"<< isInitial
				<< " F:" << isFinal					
				<< " */" << std::endl;
		}
		
		out << "/* Transiciones */" << std::endl;		
		for(TState src=0; src<dfa.GetStates(); src++)
		{
			for(TState dst=0; dst<dfa.GetStates(); dst++)
			{
				if(ignoreZeroState && dst == 0) continue;
				int c = 0;
				for(TSymbol sym=0; sym<dfa.GetAlphabethLength(); sym++)
				{					
					if(dfa.GetSucessor(src, sym) == dst)
					{
						if(c == 0) out << "  s" << (uint64_t)src << " -> s" << (uint64_t)dst << " [label=\"";
						if(c != 0) out << ", ";
						c++;						
						out << (uint64_t)sym;
					}					
				}
				if(c > 0) out << "\"]" << std::endl;
			}
		}
		out << "}" << std::endl;
	}
};
