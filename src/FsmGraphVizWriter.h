#pragma once

#include <fstream>
#include <string>
#include "Dfa.h"

template<typename TFsm>
class FsmGraphVizWriter
{
public:	
	typedef typename TFsm::TSet TSet;
	typedef typename TFsm::TSymbol TSymbol;
	typedef typename TFsm::TState TState;
	
protected:
public:
	
	void Write(const TFsm& dfa, std::ofstream& out, bool ignoreZeroState = false)
	{
		using namespace std;

		static const string initialStyle = "style=\"filled\"";
		static const string finalStyle = "style=\"bold,dashed\"";
		static const string initialFinalStyle = "style=\"filled,bold,dashed\"";

		out << "/* Generated with FastHopcroft */" << endl;
		out << "digraph \"NDFA\" {" << endl;
		out << "  rankdir=LR" << endl;
		out << "  node [shape=box width=0.1 height=0.1 fontname=Arial]" << endl;
		out << "  edge [fontname=Arial]" << endl;

		out << "/* Estados */" << endl;
		for(TState st=0; st<dfa.GetStates(); st++)
		{			
			auto isInitial = dfa.IsInitial(st);
			auto isFinal = dfa.IsFinal(st);

			if(st==0)
			{
				if(isFinal || isInitial) ignoreZeroState = false;
				if(ignoreZeroState) continue;
			}

			string fmt;
			if (isInitial && !isFinal) fmt = initialStyle;
			else if (!isInitial && isFinal) fmt = finalStyle;
			else if (isInitial && isFinal) fmt = initialFinalStyle;

			out << " s" << (size_t)st << " [label=\"" << (size_t)st << "\" " << fmt << "] " 
				<< "/* I:"<< isInitial
				<< " F:" << isFinal					
				<< " */" << endl;
		}
		
		out << "/* Transiciones */" << endl;		
		for(TState src=0; src<dfa.GetStates(); src++)
		{
			if(ignoreZeroState && src == 0) continue;
			for(TState dst=0; dst<dfa.GetStates(); dst++)
			{
				if(ignoreZeroState && dst == 0) continue;
				int c = 0;
				for(TSymbol sym=0; sym<dfa.GetAlphabetLength(); sym++)
				{
					if(dfa.IsSuccessor(src, sym, dst))
					{
						if(c == 0) out << "  s" << (size_t)src << " -> s" << (size_t)dst << " [label=\"";
						if(c != 0) out << ", ";
						c++;						
						out << (size_t)sym;
					}					
				}
				if(c > 0) out << "\"]" << endl;
			}
		}
		out << "}" << endl;
	}
};
