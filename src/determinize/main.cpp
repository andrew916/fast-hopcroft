// August 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#include "../Dfa.h"
#include "../Nfa.h"
#include "../FsmGraphVizWriter.h"
#include "../FsmPlainTextReader.h"
#include "../FsmPlainTextWriter.h"
#include "../Determinization.h"
#include <fstream>
#include <boost/timer/timer.hpp>
#include <boost/format.hpp>
#include <string>

namespace convert_nfa_dfa 
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
		bool EmitDotInputFile;
		bool EmitDotOutputFile;
		bool ShowHelp;
		bool Verbose;

		Options() : EmitDotInputFile(false), EmitDotOutputFile(false), Verbose(false), ShowHelp(false)
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
			else if(arg == "-dot-in")
			{
				opt.EmitDotInputFile = true;
				opt.DotInputFile = argv[i+1];
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

	void Convert(Options opt)
	{
		typedef uint32_t TState;
		typedef uint8_t TSymbol;
		typedef Dfa<TState, TSymbol> TDfa;
		typedef Nfa<TState, TSymbol> TNfa;

		FsmPlainTextReader<TNfa> reader;
		FsmPlainTextWriter<TDfa> writer;

		ifstream ifs(opt.InputFile);
		if(!ifs.is_open()) 
		{
			cout << "Error opening file " << opt.InputFile << endl;
			return;
		}

		TNfa nfa = reader.Read(ifs);

		if(opt.Verbose) 
		{		
			cout << "Read " << opt.InputFile << endl;
			cout << "Found FSA with " << static_cast<size_t>(nfa.GetStates()) << " states and " << static_cast<size_t>(nfa.GetAlphabetLength()) << " symbols" << endl;
		}
		ifs.close();

		Determinization<TDfa, TNfa> det;
		TDfa dfa = det.Determinize(nfa);

		if(opt.Verbose)
		{
			cout << "Determinization done, DFA with " << static_cast<size_t>(dfa.GetStates()) << " states and " << static_cast<size_t>(dfa.GetAlphabetLength()) << " symbols" << endl;
		}

		ofstream ofs(opt.OutputFile);
		if(!ofs.is_open()) 
		{
			cout << "Error opening file " << opt.OutputFile << endl;
			return;
		}
		writer.Write(dfa, ofs);
		ofs.close();

		if(opt.Verbose)
		{
			cout << "Written " << opt.OutputFile << endl;
		}

		if(opt.EmitDotInputFile)
		{
			FsmGraphVizWriter<TNfa> wnfa;
			ofs.open(opt.DotInputFile);
			if(!ofs.is_open())
			{
				cout << "Error opening file " << opt.DotInputFile << endl;
				return;
			}
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
			if(!ofs.is_open())
			{
				cout << "Error opening file " << opt.DotOutputFile << endl;
				return;
			}
			FsmGraphVizWriter<TDfa> wdfa;
			wdfa.Write(dfa, ofs, false);
			ofs.close();
			if(opt.Verbose)
			{
				cout << "Written DOT " << opt.DotOutputFile << endl;
			}
		}
	}

}

using namespace convert_nfa_dfa;

int main(int argc, char** argv)
{	
	Options opt;

	ParseCommandLine(argc, argv, opt);

	if(opt.ShowHelp)
	{
		cout 
			<< "Apply determinization algorithm to a NFA" << endl
			<< "Usage: " << endl
			<< argv[0] << " -i <infile> -o <outfile> [-dot-in <dotinfile>] [-dot-out <dotoutfile>] [-h|-?] [-v]" << endl
			<< endl
			<< "\t-i        <infile>      Input FSA file" << endl
			<< "\t-o        <outfile>     Output DFA file" << endl
			<< "\t-dot-in   <dotinfile>   Filename to write DOT file from input FSA" << endl
			<< "\t-dot-out  <dotoutfile>  Filename to write DOT file from output DFA" << endl
			<< "\t-h                      Show this help message" << endl
			<< "\t-v                      Verbose mode" << endl
			;

		return 0;
	}

	Convert(opt);

	return 0;
}