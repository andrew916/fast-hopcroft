// June 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#include "../MinimizationHopcroft.h"
#include "../MinimizationBrzozowski.h"
#include "../MinimizationIncremental.h"
#include "../MinimizationHybrid.h"
#include "../MinimizationAtomic.h"
#include "../MinimizationAlgorithm.h"
#include "../Dfa.h"
#include "../Nfa.h"
#include "../FsaGraphVizWriter.h"
#include "../FsaFormatReader.h"
#include "../FsaPlainTextReader.h"
#include "../FsaPlainTextWriter.h"
#include <fstream>
#include <boost/timer/timer.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <string>

#include <stdexcept>
#include <iostream>

namespace minimize
{

	using namespace std;
	using namespace boost::program_options;
	using boost::format;

	class Options
	{
	public:
		MinimizationAlgorithm Algorithm;
		string InputFile;
		string OutputFile;
		bool SkipSynthOutput;
		bool ShowHelp;
		bool Verbose;

		Options() : 
			SkipSynthOutput(true), 
			ShowHelp(false), 
			Verbose(false), 
			Algorithm(MinimizationAlgorithm::Hopcroft)
		{
		}
	};


	int Minimization(Options opt)
	{
		using boost::timer::cpu_timer;
		typedef uint16_t TState;
		typedef uint8_t TSymbol;	
		typedef Dfa<TState, TSymbol> TDfa;
		typedef Nfa<TState, TSymbol> TFsa;

		FsaPlainTextReader<TDfa> reader_dfa;
		FsaPlainTextReader<TFsa> reader_fsa;
		FsaPlainTextWriter<TDfa> writer;

		ifstream ifs(opt.InputFile);
		if(!ifs.is_open())
		{
			cout << "Error opening file " << opt.InputFile << endl;
			return -1;
		}

		TFsa fsa = reader_fsa.Read(ifs);
		ifs.close();
		ifs.open(opt.InputFile);
		TDfa dfa = reader_dfa.Read(ifs);
		ifs.close();

		if(opt.Verbose) 
		{
			cout << "Read " << opt.InputFile << endl;
			cout << "Found FSA with " << static_cast<size_t>(dfa.GetStates()) << " states and " << static_cast<size_t>(dfa.GetAlphabetLength()) << " symbols" << endl;
		}

		cpu_timer timer;
		// TODO: apply polymorphism to reduce code lines
		TDfa min_dfa(0,0);
		timer.start();
		if(opt.Algorithm == MinimizationAlgorithm::Hopcroft)
		{
			MinimizationHopcroft<TDfa> min;
			MinimizationHopcroft<TDfa>::NumericPartition partition;
			min.ShowConfiguration = false;
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
			if(opt.Verbose)
			{
				cout << "Minimum state count: " << static_cast<size_t>(states) << endl;
			}
			if(!opt.SkipSynthOutput) min_dfa = min.BuildDfa(fsa.GetAlphabetLength(), states, vfinal, vedges);
		}
		else if(opt.Algorithm == MinimizationAlgorithm::Incremental)
		{
			MinimizationIncremental<TDfa> min;
			MinimizationIncremental<TDfa>::NumericPartition partition;
			min.ShowConfiguration = false;
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
			min.ShowConfiguration = false;
			timer.start();
			min.Minimize(dfa, partition);
			timer.stop();
			if(opt.Verbose) {
				cout << "Partition count: " << static_cast<size_t>(partition.GetSize()) << endl;
			}
			if(!opt.SkipSynthOutput) min_dfa = min.BuildDfa(dfa, partition);
		} else if(opt.Algorithm == MinimizationAlgorithm::Atomic)
		{
			MinimizationAtomic<TDfa> min;
			min.ShowConfiguration = false;
			timer.start();
			min_dfa = min.Minimize(dfa);
			timer.stop();
			if(opt.Verbose)
			{
				cout << "Minimum state count: " << static_cast<size_t>(min_dfa.GetStates()) << endl;
			}
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
				return -1;
			}
			writer.Write(min_dfa, ofs);
			ofs.close();

			if(opt.Verbose)
			{
				cout << "Written " << opt.OutputFile << endl;
			}
		}
		return 0;
	}

}

using namespace minimize;

int main(int argc, char** argv)
{
	Options o;
	options_description opt_desc("allowed options");
	opt_desc.add_options()
		("help,?", bool_switch(&o.ShowHelp)->default_value(false), "Show this information")
		("algorithm,a", value(&o.Algorithm)->default_value(MinimizationAlgorithm::Hopcroft), "Minimization algorithm")
		("input,i", value(&o.InputFile), "Input file")
		("output,o", value(&o.OutputFile), "Output file")
		("skip_synth,s", bool_switch(&o.SkipSynthOutput)->default_value(false), "Skip synthetize output")
		("verbose,v", bool_switch(&o.Verbose), "Verbose mode")
		;

	variables_map vm;
	command_line_parser parser(argc, argv);
	auto po = parser.options(opt_desc).run();
	store(po, vm);
	notify(vm);

	if(o.ShowHelp)
	{
		cout << opt_desc << endl;
		return 0;
	}

	Minimization(o);

	return 0;
}
