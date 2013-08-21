// August 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#include "../Nfa.h"
#include "../NfaGenerator.h"
#include "../FsmGraphVizWriter.h"
#include "../FsaFormatReader.h"
#include "../FsmPlainTextReader.h"
#include "../FsmPlainTextWriter.h"
#include <fstream>
#include <boost/timer/timer.hpp>
#include <boost/format.hpp>
#include <string>

namespace generate_nfa
{

using namespace std;
using boost::format;

class Options
{
public:
	string OutputFile;
	string DotOutputFile;
	long States;
	long Symbols;
	long Seed;
	bool Verbose;
	bool ShowHelp;
	float Density;
	bool EmitDotOutputFile;

	Options() : States(10), Symbols(2), Seed(0), Verbose(false), ShowHelp(false), Density(0.05f), EmitDotOutputFile(false)
	{
	}
};

void ParseCommandLine(int argc, char** argv, Options& opt)
{
	for(int i=1; i<argc; i++)
	{
		string arg = argv[i];
		if(arg == "-i")
		{
			opt.OutputFile = argv[i+1];
			i++;				
		}
		else if(arg == "-s")
		{
			opt.States = stol(argv[i+1]);
			i++;
		}
		else if(arg == "-a")
		{
			opt.Symbols = stol(argv[i+1]);
			i++;
		}
		else if(arg == "-seed")
		{
			opt.Seed = stol(argv[i+1]);
			i++;
		}
		else if(arg == "-d")
		{
			opt.Density = stof(argv[i+1]);
			i++;
		}
		else if(arg == "-dot-out")
		{
			opt.EmitDotOutputFile = true;
			opt.DotOutputFile = argv[i+1];
			i++;
		}
		else if(arg == "-v")
		{
			opt.Verbose = true;
		}
		else if(arg == "-h" || arg == "-?")
		{
			opt.ShowHelp = true;
		}
	}

}

void GenerateNfa(Options opt)
{
	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	typedef Nfa<TState, TSymbol> TNfa;
	typedef mt19937 TRandGen;
	
	NfaGenerator<TNfa, TRandGen> gen;
	TRandGen rgen;
	FsmPlainTextWriter<TNfa> writer;

	TNfa nfa = gen.Generate(opt.States, opt.Symbols, 1, 1, opt.Density, rgen);
	if(opt.Verbose) 
	{		
		cout << "Generated NFA with " << nfa.GetStates() << " states and " << nfa.GetAlphabetLength() << " symbols" << endl;
	}

	ofstream ofs(opt.OutputFile);
	writer.Write(nfa, ofs);
	ofs.close();

	if(opt.Verbose)
	{
		cout << "Written " << opt.OutputFile << endl;
 	}

	if(opt.EmitDotOutputFile)
	{
		ofs.open(opt.DotOutputFile);
		FsmGraphVizWriter<TNfa> wnfa;
		wnfa.Write(nfa, ofs, false);
		ofs.close();
		cout << "Written " << opt.DotOutputFile << endl;
	}
}

}

using namespace generate_nfa;

int main(int argc, char** argv)
{		
	Options opt;
	ParseCommandLine(argc, argv, opt);
	
	if(opt.ShowHelp)
	{
		cout 
			<< "Generates a new NFA using a specified seed and transition function density." << endl
			<< "Use the same seed to get the same automata" << endl
			<< "Usage: " << endl
			<< argv[0] << "-o <outfile> -s <states> -a <symbols> -d <density> [-dot-out <dotoutfile>] [-s <seed>] [-h|-?] [-v]" << endl
			<< endl	
			<< "\t-o        outfile     Output FSA file" << endl
			<< "\t-dot-out  dotoutfile  Filename to write DOT file from output FSA" << endl
			<< "\t-s        <states>    Number of states" << endl
			<< "\t-a        <symbols>   Number of symbols" << endl
			<< "\t-d        <density>   Real number in zero to 1.0 range. Indicates how populated is the transition function" << endl
			<< "\t-seed     <seed>      Integer value for the numeric random generator" << endl
			<< "\t-h,-?                 Show this help message" << endl
			<< "\t-v                    Verbose mode" << endl
			;

		return 0;
	}

	GenerateNfa(opt);

	return 0;
}
