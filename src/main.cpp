// June 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#include "Dfa.h"
#include "BitSet.h"
#include "Dfa.h"
#include "MinimizationHopcroft.h"
#include "AfdParser.h"
#include <fstream>
#include <boost/timer/timer.hpp>

int main(int argc, char** argv)
{	
	MinimizationHopcroft<uint32_t, uint8_t> mini;
	{
		Dfa<uint32_t, uint8_t> dfa(2, 4);

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
		Dfa<uint32_t, uint8_t> dfa2(2, 5);

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
		AfdParser<uint32_t, uint8_t> parser;
		std::vector<std::string> files;
		
		files.push_back("afd\\000_n512k2.afd");
		files.push_back("afd\\000_n1024k2.afd");
		files.push_back("afd\\000_n2046k2.afd");
		files.push_back("afd\\000_n4092k2.afd");
		files.push_back("afd\\000_n8192k2.afd");
		files.push_back("afd\\000_n16384k2.afd");

		files.push_back("afd\\001_n512k2.afd");
		files.push_back("afd\\001_n1024k2.afd");
		files.push_back("afd\\001_n2046k2.afd");
		files.push_back("afd\\001_n4092k2.afd");
		files.push_back("afd\\001_n8192k2.afd");
		files.push_back("afd\\001_n16384k2.afd");

		files.push_back("afd\\002_n512k2.afd");
		files.push_back("afd\\002_n1024k2.afd");
		files.push_back("afd\\002_n2046k2.afd");
		files.push_back("afd\\002_n4092k2.afd");
		files.push_back("afd\\002_n8192k2.afd");
		files.push_back("afd\\002_n16384k2.afd");

		files.push_back("afd\\003_n512k2.afd");
		files.push_back("afd\\003_n1024k2.afd");
		files.push_back("afd\\003_n2046k2.afd");
		files.push_back("afd\\003_n4092k2.afd");
		files.push_back("afd\\003_n8192k2.afd");
		files.push_back("afd\\003_n16384k2.afd");

		files.push_back("afd\\004_n512k2.afd");
		files.push_back("afd\\004_n1024k2.afd");
		files.push_back("afd\\004_n2046k2.afd");
		files.push_back("afd\\004_n4092k2.afd");
		files.push_back("afd\\004_n8192k2.afd");
		files.push_back("afd\\004_n16384k2.afd");

		files.push_back("afd\\005_n512k2.afd");
		files.push_back("afd\\005_n1024k2.afd");
		files.push_back("afd\\005_n2046k2.afd");
		files.push_back("afd\\005_n4092k2.afd");
		files.push_back("afd\\005_n8192k2.afd");
		files.push_back("afd\\005_n16384k2.afd");

		files.push_back("afd\\006_n512k2.afd");
		files.push_back("afd\\006_n1024k2.afd");
		files.push_back("afd\\006_n2046k2.afd");
		files.push_back("afd\\006_n4092k2.afd");
		files.push_back("afd\\006_n8192k2.afd");
		files.push_back("afd\\006_n16384k2.afd");

		files.push_back("afd\\007_n512k2.afd");
		files.push_back("afd\\007_n1024k2.afd");
		files.push_back("afd\\007_n2046k2.afd");
		files.push_back("afd\\007_n4092k2.afd");
		files.push_back("afd\\007_n8192k2.afd");
		files.push_back("afd\\007_n16384k2.afd");

		files.push_back("afd\\008_n512k2.afd");
		files.push_back("afd\\008_n1024k2.afd");
		files.push_back("afd\\008_n2046k2.afd");
		files.push_back("afd\\008_n4092k2.afd");
		files.push_back("afd\\008_n8192k2.afd");
		files.push_back("afd\\008_n16384k2.afd");

		files.push_back("afd\\009_n512k2.afd");
		files.push_back("afd\\009_n1024k2.afd");
		files.push_back("afd\\009_n2046k2.afd");
		files.push_back("afd\\009_n4092k2.afd");
		files.push_back("afd\\009_n8192k2.afd");
		files.push_back("afd\\009_n16384k2.afd");

		std::ifstream afd;
		Dfa<uint32_t, uint8_t> dfa(0,0);
		mini.ShowConfiguration = false;
		for(auto filename : files) 
		{
			afd.open(filename);
			dfa = parser.Parse(afd);
			afd.close();
			std::cout << "Begin, states: " << dfa.GetMaxStates() << ", alpha: " << dfa.GetAlphabethLength() << std::endl;
			boost::timer::cpu_timer timer;
			timer.start();			
			mini.Minimize(dfa);			
			timer.stop();						
			std::cout << "Done with " << filename << " elapsed (ms) " << timer.elapsed().wall / 1000000UL << std::endl;
		}
	}
	return 0;
}
