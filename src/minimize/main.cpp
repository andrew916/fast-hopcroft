// June 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#include "../MinimizationHopcroft.h"
#include "../MinimizationBrzozowski.h"
#include "../MinimizationIncremental.h"
#include "../MinimizationHybrid.h"
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

namespace minimize
{

	using namespace std;
	using boost::format;

	enum class MinimizationAlgorithm {
		Hopcroft,
		Brzozowski,
		Incremental,
		Hybrid
	};

	class Options
	{
	public:
		MinimizationAlgorithm Algorithm;
		string InputFile;
		string OutputFile;
		string DotInputFile;
		string DotOutputFile;
		bool EmitDotOutputFile;
		bool EmitDotInputFile;
		bool SkipSynthOutput;
		bool ShowHelp;
		bool Verbose;

		Options() : 
			EmitDotOutputFile(false), 
			EmitDotInputFile(false), 
			SkipSynthOutput(true), 
			ShowHelp(false), 
			Verbose(false), 
			Algorithm(MinimizationAlgorithm::Hopcroft)
		{
		}
	};


	void ParseCommandLine(int argc, char** argv, Options& opt)
	{
		for(int i=1; i<argc; i++)
		{
			string arg = argv[i];
			if(arg == "-m")
			{
				if(stricmp(argv[i+1], "hopcroft") == 0) {
					opt.Algorithm = MinimizationAlgorithm::Hopcroft;
				} else if(stricmp(argv[i+1], "brzozowski") == 0) {
					opt.Algorithm = MinimizationAlgorithm::Brzozowski;
				} else if(stricmp(argv[i+1], "incremental") == 0) {
					opt.Algorithm = MinimizationAlgorithm::Incremental;
				} else if(stricmp(argv[i+1], "hybrid") == 0) {
					opt.Algorithm = MinimizationAlgorithm::Hybrid;
				} else {
					cout << "error: invalid algorithm specification" << endl;
					abort();
				}
				i++;
			}
			else if(arg == "-i")
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
		using boost::timer::cpu_timer;
		typedef uint16_t TState;
		typedef uint8_t TSymbol;	
		typedef Dfa<TState, TSymbol> TDfa;
		typedef Nfa<TState, TSymbol> TFsa;

		FsmPlainTextReader<TDfa> reader_dfa;
		FsmPlainTextReader<TFsa> reader_fsa;
		FsmPlainTextWriter<TDfa> writer;


		ifstream ifs(opt.InputFile);
		if(!ifs.is_open())
		{
			cout << "Error opening file " << opt.InputFile << endl;
			return;
		}

		TFsa fsa = reader_fsa.Read(ifs);
		TDfa dfa = reader_dfa.Read(ifs);
		ifs.close();

		if(opt.Verbose) 
		{
			cout << "Read " << opt.InputFile << endl;
			cout << "Found FSA with " << dfa.GetStates() << " states and " << dfa.GetAlphabetLength() << " symbols" << endl;
		}

		cpu_timer timer;
		// TODO: apply polymorphism to reduce code lines
		TDfa min_dfa(0,0);
		timer.start();
		if(opt.Algorithm == MinimizationAlgorithm::Hopcroft)
		{
			MinimizationHopcroft<TDfa> min;
			MinimizationHopcroft<TDfa>::NumericPartition partition;
			timer.start();
			min.Minimize(dfa, partition);
			timer.stop();
			if(opt.Verbose) {
				cout << "Partition count: " << static_cast<size_t>(partition.GetSize()) << endl;
			}
			if(!opt.SkipSynthOutput) min_dfa = min.BuildDfa(dfa, partition);
		}
		else if(opt.Algorithm == MinimizationAlgorithm::Brzozowski)
		{
			MinimizationBrzozowski<TFsa, TDfa> min;			
			MinimizationBrzozowski<TFsa, TDfa>::TVectorDfaState vfinal;
			MinimizationBrzozowski<TFsa, TDfa>::TVectorDfaEdge vedges;
			TState states;
			timer.start();
			min.Minimize(fsa, &states, vfinal, vedges);
			timer.stop();
			if(!opt.SkipSynthOutput) min_dfa = min.BuildDfa(fsa.GetAlphabetLength(), states, vfinal, vedges);
		}
		else if(opt.Algorithm == MinimizationAlgorithm::Incremental)
		{
			MinimizationIncremental<TDfa> min;
			MinimizationIncremental<TDfa>::NumericPartition partition;
			timer.start();
			min.Minimize(dfa, partition);
			timer.stop();
			if(opt.Verbose) {
				cout << "Partition count: " << static_cast<size_t>(partition.GetSize()) << endl;
			}
			if(!opt.SkipSynthOutput) min_dfa = min.BuildDfa(dfa, partition);
		}
		else if(opt.Algorithm == MinimizationAlgorithm::Hybrid)
		{
			MinimizationHybrid<TDfa> min;
			MinimizationHybrid<TDfa>::NumericPartition partition;
			timer.start();
			min.Minimize(dfa, partition);
			timer.stop();
			if(opt.Verbose) {
				cout << "Partition count: " << static_cast<size_t>(partition.GetSize()) << endl;
			}
			if(!opt.SkipSynthOutput) min_dfa = min.BuildDfa(dfa, partition);
		}		

		if(opt.Verbose)
		{
			cout << "Minimization took: " << timer.format() << endl;
		}

		if(!opt.SkipSynthOutput)
		{
			ofstream ofs(opt.OutputFile);
			if(!ofs.is_open())
			{
				cout << "Error with output file: " << opt.OutputFile << endl;
				return;
			}
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
			if(!ofs.is_open())
			{
				cout << "Error opening file " << opt.DotInputFile << endl;
				return;
			}

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
			if(!ofs.is_open())
			{
				cout << "Error opening file " << opt.DotOutputFile << endl;
				return;
			}

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

using namespace minimize;

int main(int argc, char** argv)
{
	Options opt;

	ParseCommandLine(argc, argv, opt);

	if(opt.ShowHelp)
	{
		cout << "Usage:" << endl 
			<< argv[0] << " -i <infile> [-o <outfile>] [-dot-in <dotinfile>] [-dot-out <dotoutfile>] [-h|-?] [-v]" << endl
			<< endl
			<< "\t-a <algorithm>         Minimization algorithm" << endl
			<< "\t-i <infile>            Input filename" << endl
			<< "\t-o <outfile>           Output filename" << endl
			<< "\t-dot-out <dotoutfile>  DOT output filename" << endl
			<< "\t-dot-in <dotinfile>    DOT input filename" << endl
			<< "\t-h,-?                  Show this help message" << endl
			<< endl
			<< "Algorithms available: hopcroft, incremental, hybrid" << endl
			;
	}

	Minimization(opt);


	return 0;
}
