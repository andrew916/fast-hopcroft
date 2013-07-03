// June 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#include "Dfa.h"
#include "BitSet.h"
#include "Dfa.h"
#include "MinimizationHopcroft.h"
#include "AfdParser.h"
#include <fstream>

int main(int argc, char** argv)
{	
	MinimizationHopcroft<uint32_t, uint32_t> mini;
	{
		Dfa<uint32_t, uint32_t> dfa(2, 4);

		//   / 2 \
		// 0 - 1 - 3
		dfa.SetInitial(0);
		dfa.SetFinal(3);

		dfa.SetTransition(0, 0, 1);
		dfa.SetTransition(0, 1, 0);

		dfa.SetTransition(1, 0, 3);
		dfa.SetTransition(1, 1, 1);

		dfa.SetTransition(2, 0, 3);
		dfa.SetTransition(2, 1, 2);

		dfa.SetTransition(3, 0, 3);
		dfa.SetTransition(3, 1, 3);


		mini.Minimize(dfa);
	}
	{
		//    / 1 - 3
		//  0
		//    \ 2 - 4
		Dfa<uint32_t, uint32_t> dfa2(2, 5);

		dfa2.SetInitial(0);
		dfa2.SetFinal(3);
		dfa2.SetFinal(4);

		dfa2.SetTransition(0, 0, 1);
		dfa2.SetTransition(0, 1, 2);

		dfa2.SetTransition(1, 0, 3);
		dfa2.SetTransition(1, 1, 1);

		dfa2.SetTransition(2, 0, 4);
		dfa2.SetTransition(2, 1, 2);

		dfa2.SetTransition(3, 0, 3);
		dfa2.SetTransition(3, 1, 3);

		dfa2.SetTransition(4, 0, 4);
		dfa2.SetTransition(4, 1, 4);

		mini.Minimize(dfa2);
	}
	{
		AfdParser parser;
		std::ifstream afd("afd\\000_n512k2.afd");
		parser.Parse(afd);
	}
	return 0;
}
