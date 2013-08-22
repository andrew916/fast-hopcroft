// June 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#include "../MinimizationBrzozowski.h"
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

namespace minimize_brzozowski
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
	bool ShowHelp;
	bool Verbose;

	Options() : EmitDotOutputFile(false), EmitDotInputFile(false), ShowHelp(false), Verbose(false)
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
	typedef Nfa<TState, TSymbol> TNfa;
		
	FsmPlainTextReader<TNfa> reader;
	FsmPlainTextWriter<TNfa> writer;


	ifstream ifs(opt.InputFile);
	TNfa nfa = reader.Read(ifs);
	ifs.close();

	if(opt.Verbose) 
	{
		cout << "Read " << opt.InputFile << endl;
		cout << "Found FSA with " << nfa.GetStates() << " states and " << nfa.GetAlphabetLength() << " symbols" << endl;
	}
		
	MinimizationBrzozowski<TNfa> min;
	TNfa min_nfa = min.Minimize(nfa);

	if(opt.Verbose)
	{		
		cout << "Brzozowski Minimization done, FSA with " << nfa.GetStates() << " states and " << nfa.GetAlphabetLength() << " symbols" << endl;
	}

	ofstream ofs(opt.OutputFile);
	writer.Write(min_nfa, ofs);
	ofs.close();

	if(opt.Verbose)
	{
		cout << "Written " << opt.OutputFile << endl;
 	}

	if(opt.EmitDotInputFile)
	{
		FsmGraphVizWriter<TNfa> wnfa;
		ofs.open(opt.DotInputFile);
		wnfa.Write(nfa, ofs, false);
		ofs.close();
		if(opt.Verbose)
		{
			cout << "Written DOT " << opt.DotInputFile << endl;
 		}
	}
	if(opt.EmitDotOutputFile)
	{
		ofs.open(opt.DotOutputFile);
		FsmGraphVizWriter<TNfa> wdfa;
		wdfa.Write(min_nfa, ofs, false);
		ofs.close();
		if(opt.Verbose)
		{
			cout << "Written DOT " << opt.DotOutputFile << endl;
 		}
	}
}

}

using namespace minimize_brzozowski;

int main(int argc, char** argv)
{
	Options opt;

	ParseCommandLine(argc, argv, opt);

	if(opt.ShowHelp)
	{
		cout << "Usage:" << endl 
			<< argv[0] << " -i <infile> -o <outfile> [-dot-in <dotinfile>] [-dot-out <dotoutfile>] [-h|-?] [-v]" << endl
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
