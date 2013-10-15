// August 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#include "../Nfa.h"
#include "../NfaGenerator.h"
#include "../FsmGraphVizWriter.h"
#include "../FsaFormatReader.h"
#include "../FsmPlainTextReader.h"
#include "../FsmPlainTextWriter.h"
#include "../FsaFormat.h"
#include <fstream>
#include <chrono>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <string>
#include <iostream>

namespace generate_nfa
{

	using namespace std;
	using namespace boost::program_options;
	using boost::format;

	class Options
	{
	public:
		string OutputFile;
		FsaFormat OutputFormat;
		long States;
		long Symbols;
		long Seed;
		bool Verbose;
		bool ShowHelp;
		float Density;
		float FinalsDensity;	

		Options() : States(10), Symbols(2), Seed(0), Verbose(false), ShowHelp(false), Density(0.05f), FinalsDensity(0.1f), OutputFormat(FsaFormat::None)
		{
		}
	};

	void GenerateNfa(const Options& opt)
	{
		typedef uint16_t TState;
		typedef uint8_t TSymbol;
		typedef Nfa<TState, TSymbol> TNfa;
		typedef mt19937 TRandGen;

		// Si la semilla es cero, usamos como semilla un valor derivado del temporizador
		auto seed = opt.Seed;
		if(seed == 0)
		{
			chrono::steady_clock::time_point p = chrono::steady_clock::now();
			chrono::steady_clock::duration d = p.time_since_epoch();
			seed = static_cast<long>(d.count());
		}

		NfaGenerator<TNfa, TRandGen> gen;
		TRandGen rgen(seed);
		FsmPlainTextWriter<TNfa> writer;
		
		TNfa nfa = gen.Generate(static_cast<TState>(opt.States), static_cast<TSymbol>(opt.Symbols), 1, 1, opt.Density, rgen);
		if(opt.Verbose) 
		{		
			cout << "Generated NFA with " << static_cast<size_t>(nfa.GetStates()) << " states and " << static_cast<size_t>(nfa.GetAlphabetLength()) << " symbols" << endl;
		}

		ofstream ofs(opt.OutputFile);
		if(!ofs.is_open())
		{
			cout << "Error opening " << opt.OutputFile << endl;
			return;
		}
		writer.Write(nfa, ofs);
		ofs.close();

		if(opt.Verbose)
		{
			cout << "Written " << opt.OutputFile << endl;
		}
	}

}

using namespace generate_nfa;

int main(int argc, char** argv)
{
	Options o;
	options_description opt_desc("Allowed options");
	opt_desc.add_options()
		("help,?", bool_switch(&o.ShowHelp)->default_value(false), "Show this information")
		("states,n", value(&o.States)->default_value(10), "Number of states")
		("alpha,k", value(&o.Symbols)->default_value(2), "Number of symbols")
		("density,d", value(&o.Density)->default_value(0.1f), "Transition function density")
		("finals,f", value(&o.FinalsDensity)->default_value(0.1f), "Final states density")
		("seed,s", value(&o.Seed)->default_value(0), "Zero means time-based")
		("verbose,v", bool_switch(&o.Verbose)->default_value(false), "Verbose mode")
		("output,o", value(&o.OutputFile), "Output file")
		("format,t", value(&o.OutputFormat)->default_value(FsaFormat::ZeroBasedPlainText), "Output format")
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

	GenerateNfa(o);

	return 0;
}
