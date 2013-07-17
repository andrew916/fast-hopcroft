// June 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#include "MinimizationHopcroft.h"
#include "Dfa.h"
#include "BitSet.h"
#include "DfaGraphVizExporter.h"
#include "AfdParser.h"
#include <fstream>
#include <boost/timer/timer.hpp>

using namespace std;

int main(int argc, char** argv)
{	
	MinimizationHopcroft<uint32_t, uint8_t> mini;
	DfaGraphVizExporter<uint32_t, uint8_t> exporter;
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


		mini.Minimize2(dfa);
		exporter.Export(dfa, std::ofstream("dfa1.dot"));
		cout << endl;
	}
	{
		//    / 1 - 3
		//  0
		//    \ 2 - 4
		Dfa<uint32_t, uint8_t> dfa(2, 5);

		dfa.SetInitial(0);
		dfa.SetFinal(3);
		dfa.SetFinal(4);

		dfa.SetTransition(0, 0, 1);
		dfa.SetTransition(0, 1, 2);

		dfa.SetTransition(1, 0, 3);
		dfa.SetTransition(1, 1, 1);

		dfa.SetTransition(2, 0, 4);
		dfa.SetTransition(2, 1, 2);

		dfa.SetTransition(3, 0, 3);
		dfa.SetTransition(3, 1, 3);

		dfa.SetTransition(4, 0, 4);
		dfa.SetTransition(4, 1, 4);

		mini.Minimize2(dfa);
		exporter.Export(dfa, std::ofstream("dfa2.dot"));
		cout << endl;
	}
	{
		// uses zero as invisible null-sink state
		//      2 - 5 - 8  - 11
		//    /
		//   1 -3 - 6 - 9  - 12
		//    \
		//      4 - 7 - 10 - 13
		Dfa<uint32_t, uint8_t> dfa(3, 14);

		dfa.SetInitial(1);
		dfa.SetFinal(11);
		dfa.SetFinal(12);
		dfa.SetFinal(13);

		dfa.SetTransition(1, 0, 2);
		dfa.SetTransition(1, 1, 3);
		dfa.SetTransition(1, 2, 4);

		dfa.SetTransition(2, 0, 5);
		dfa.SetTransition(3, 0, 6);
		dfa.SetTransition(4, 0, 7);

		dfa.SetTransition(5, 0, 8);
		dfa.SetTransition(6, 1, 9);
		dfa.SetTransition(7, 2, 10);

		dfa.SetTransition(8, 0, 11);
		dfa.SetTransition(9, 1, 12);
		dfa.SetTransition(10, 2, 13);

		mini.Minimize2(dfa);
		exporter.Export(dfa, std::ofstream("dfa3.dot"), false);		
		cout << endl;
	}
	{
		// uses zero as invisible null-sink state
		//      2 - 5 - 8  - 11
		//    /       \   /
		//   1 -3 - 6 - 9  - 12
		//    \       \   \
		//      4 - 7 - 10 - 13
		Dfa<uint32_t, uint8_t> dfa(3, 14);

		dfa.SetInitial(1);
		dfa.SetFinal(11);
		dfa.SetFinal(12);
		dfa.SetFinal(13);

		dfa.SetTransition(1, 0, 2);
		dfa.SetTransition(1, 1, 3);
		dfa.SetTransition(1, 2, 4);

		dfa.SetTransition(2, 0, 5);
		dfa.SetTransition(3, 0, 6);
		dfa.SetTransition(4, 0, 7);

		dfa.SetTransition(5, 0, 8);		
		dfa.SetTransition(5, 1, 9);		
		dfa.SetTransition(5, 2, 9);		

		dfa.SetTransition(6, 1, 9);
		dfa.SetTransition(6, 0, 10);
		dfa.SetTransition(6, 2, 10);

		dfa.SetTransition(7, 0, 10);
		dfa.SetTransition(7, 1, 10);
		dfa.SetTransition(7, 2, 10);

		dfa.SetTransition(8, 0, 11);
		dfa.SetTransition(8, 1, 11);
		dfa.SetTransition(8, 2, 11);

		dfa.SetTransition(9, 0, 11);
		dfa.SetTransition(9, 1, 12);
		dfa.SetTransition(9, 2, 13);

		dfa.SetTransition(10, 0, 13);
		dfa.SetTransition(10, 1, 13);
		dfa.SetTransition(10, 2, 13);

		mini.Minimize2(dfa);
		exporter.Export(dfa, std::ofstream("dfa4.dot"), false);		
		cout << endl;
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

		ifstream afd;
		ofstream report ("report.txt");
		Dfa<uint32_t, uint8_t> dfa(0,0);
		mini.ShowConfiguration = false;

		for(auto filename : files) 
		{
			afd.open(filename);
			dfa = parser.Parse(afd);
			afd.close();

			{
				stringstream str;
				str << "Begin, states: " << dfa.GetStates() << ", alpha: " << dfa.GetAlphabethLength() << endl;
				cout << str.str();
				report << str.str();
			}

			boost::timer::cpu_timer timer;
			timer.start();
			mini.Minimize2(dfa);
			timer.stop();

			{
				stringstream str;
				str << "Done with " << filename << " elapsed (ms) " << timer.elapsed().wall / 1000000UL << endl;
				cout << str.str();
				report << str.str();
				cout << endl;
			}
		}
	}
	return 0;
}
