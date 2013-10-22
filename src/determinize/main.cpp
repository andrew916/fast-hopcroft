// August 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#include "../Dfa.h"
#include "../Nfa.h"
#include "../FsaGraphVizWriter.h"
#include "../FsaPlainTextReader.h"
#include "../FsaPlainTextWriter.h"
#include "../FsaFormat.h"
#include "../Determinization.h"
#include <fstream>
#include <boost/timer/timer.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <string>

namespace determinize 
{
	using namespace std;
	using boost::format;

	class Options 
	{
	public:
		string InputFile;
		string OutputFile;
		FsaFormat Format;
		bool ShowHelp;
		bool Verbose;

		Options() : Verbose(false), ShowHelp(false), Format(FsaFormat::ZeroBasedPlainText)
		{
		}
	};

	void Convert(const Options& opt)
	{
		typedef uint32_t TState;
		typedef uint8_t TSymbol;
		typedef Dfa<TState, TSymbol> TDfa;
		typedef Nfa<TState, TSymbol> TNfa;
				
		ifstream ifs(opt.InputFile);
		if(!ifs.is_open()) 
		{
			cout << "Error opening file " << opt.InputFile << endl;
			return;
		}

		auto reader = new_reader<TNfa>(opt.Format);
		reader->ReadHeader(ifs);

		TNfa nfa = reader->Read(ifs);

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

		auto writer = new_writer<TDfa>(opt.Format);
		writer->WriteHeader(ofs);
		writer->Write(dfa, ofs);
		ofs.close();

		if(opt.Verbose)
		{
			cout << "Written " << opt.OutputFile << endl;
		}
	}

}

using namespace determinize;

int main(int argc, char** argv)
{	
	using namespace boost::program_options;

	Options o;

	options_description opt_desc("Allowed options");
	opt_desc.add_options()
		("help,?", bool_switch(&o.ShowHelp)->default_value(false), "Show this information")
		("input,i", value(&o.InputFile), "Input FSA file")
		("output,o", value(&o.OutputFile), "Output FSA file")
		("format,f", value(&o.Format), "FSA file format to be used")
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

	Convert(o);

	return 0;
}