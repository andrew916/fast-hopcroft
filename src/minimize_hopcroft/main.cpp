// June 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#include "../MinimizationHopcroft.h"
#include "../Dfa.h"
#include "../Nfa.h"
#include "../FsmGraphVizWriter.h"
#include "../FsaFormatReader.h"
#include "../FsmPlainTextReader.h"
#include "../FsmPlainTextWriter.h"
#include <fstream>
#include <boost/timer/timer.hpp>
#include <boost/format.hpp>
#include <string>

namespace minimize_hopcroft
{

using namespace std;
using boost::format;

class Options
{
public:
	string InputFile;
	string OutputFile;
	string DotInputFile;
	string DotOutputFile;
	bool EmitDotOutputFile;
	bool EmitDotInputFile;
	bool SkipSynthOutput;
	bool ShowHelp;
	bool Verbose;

	Options() : EmitDotOutputFile(false), EmitDotInputFile(false), SkipSynthOutput(true), ShowHelp(false), Verbose(false)
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
			opt.InputFile = argv[i+1];
			i++;
		}
		else if(arg == "-o")
		{
			opt.SkipSynthOutput = false;
			opt.OutputFile = argv[i+1];
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

void Minimization(Options opt)
{
	typedef uint16_t TState;
	typedef uint8_t TSymbol;	
	typedef Dfa<TState, TSymbol> TDfa;
		
	FsmPlainTextReader<TDfa> reader;
	FsmPlainTextWriter<TDfa> writer;


	ifstream ifs(opt.InputFile);
	TDfa dfa = reader.Read(ifs);
	ifs.close();

	if(opt.Verbose) 
	{
		cout << "Read " << opt.InputFile << endl;
		cout << "Found DFA with " << dfa.GetStates() << " states and " << dfa.GetAlphabetLength() << " symbols" << endl;
	}
		
	TDfa min_dfa(0,0);
	MinimizationHopcroft<TDfa> min;
	MinimizationHopcroft<TDfa>::TPartitionVector partitions;
	MinimizationHopcroft<TDfa>::TStateToPartition state_to_partition;
	min.Minimize(dfa, partitions, state_to_partition);
	if(!opt.SkipSynthOutput)
	{
		min_dfa = min.Synthetize(dfa, partitions, state_to_partition);
	}

	if(opt.Verbose)
	{
		cout << "Hopcroft Minimization done, FSA with " << partitions.size() << " states" << endl;
	}

	if(!opt.SkipSynthOutput)
	{
		ofstream ofs(opt.OutputFile);
		writer.Write(min_dfa, ofs);
		ofs.close();
	
		if(opt.Verbose)
		{
			cout << "Written " << opt.OutputFile << endl;
 		}
	}

	if(opt.EmitDotInputFile)
	{
		ofstream ofs(opt.DotInputFile);
		FsmGraphVizWriter<TDfa> wnfa;
		
		wnfa.Write(dfa, ofs, false);
		ofs.close();
		if(opt.Verbose)
		{
			cout << "Written DOT " << opt.DotInputFile << endl;
 		}
	}
	if(opt.EmitDotOutputFile && !opt.SkipSynthOutput)
	{
		ofstream ofs(opt.DotOutputFile);
		FsmGraphVizWriter<TDfa> wdfa;
		wdfa.Write(min_dfa, ofs, false);
		ofs.close();
		if(opt.Verbose)
		{
			cout << "Written DOT " << opt.DotOutputFile << endl;
 		}
	}
}

}

using namespace minimize_hopcroft;

int main(int argc, char** argv)
{
	Options opt;

	ParseCommandLine(argc, argv, opt);

	if(opt.ShowHelp)
	{
		cout << "Usage:" << endl 
			<< argv[0] << " -i <infile> [-o <outfile>] [-dot-in <dotinfile>] [-dot-out <dotoutfile>] [-h|-?] [-v]" << endl
			<< endl
			<< "\t-i <infile>            Input filename" << endl
			<< "\t-o <outfile>           Output filename" << endl
			<< "\t-dot-out <dotoutfile>  DOT output filename" << endl
			<< "\t-dot-in <dotinfile>    DOT input filename" << endl
			<< "\t-h,-?                  Show this help message" << endl
			;
	}

	Minimization(opt);


	return 0;
}
