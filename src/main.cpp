#include "Dfa.h"
#include "BitSet.h"
#include "Dfa.h"
#include "MinimizationHopcroft.h"


int main(int argc, char** argv)
{	
	Dfa<uint32_t, uint32_t> dfa(2, 4);

	//   /  2  \
	// 0 -> 1 -> 3
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
		
	MinimizationHopcroft<uint32_t, uint32_t> mini;
	mini.Minimize(dfa);

	Dfa<uint32_t, uint32_t> dfa2(2, 5);

	dfa.SetInitial(0);
	dfa.SetFinal(3);
	dfa.SetFinal(4);

	return 0;
}