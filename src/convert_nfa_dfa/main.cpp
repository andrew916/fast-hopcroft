// August 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#include "../MinimizationHopcroft.h"
#include "../MinimizationBrzozowski.h"
#include "../Dfa.h"
#include "../Nfa.h"
#include "../DfaGenerator.h"
#include "../FsmGraphVizWriter.h"
#include "../FsaFormatReader.h"
#include "../FsmPlainTextReader.h"
#include "../FsmPlainTextWriter.h"
#include "../Determinization.h"
#include "../NfaGenerator.h"
#include <fstream>
#include <boost/timer/timer.hpp>
#include <boost/format.hpp>
#include <string>

using namespace std;
using boost::format;

class Options 
{
public:
	string input_file;
	string output_file;
	string dot_input_file;
	string dot_output_file;
	bool emit_dot_input;
	bool emit_dot_output;
	bool show_help;
};


void ParseCommandLine(int argc, char** argv, Options& opt)
{	
	for(int i=1; i<argc; i++)
	{
		string arg = argv[i];
		if(arg == "-i")
		{
			opt.input_file = argv[i+1];
			i++;
		}
		else if(arg == "-o")
		{
			opt.output_file = argv[i+1];
			i++;
		}
		else if(arg == "-dot-in")
		{
			opt.emit_dot_input = true;
			opt.dot_input_file = argv[i+1];
			i++;
		}
		else if(arg == "-dot-out")
		{
			opt.emit_dot_output = true;
			opt.dot_output_file = argv[i+1];
			i++;
		}
		else if(arg == "-h" || arg == "-?")
		{
			opt.show_help = true;
			break;
		}
	}
}

void Convert(Options opt)
{
	
}

int main(int argc, char** argv)
{	
	Options opt;

	ParseCommandLine(argc, argv, opt);

	if(opt.show_help)
	{
		cout << "Usage: " << endl
			<< argv[0] << "-i infile -o outfile -dot-in dotinfile -dot-out dotoutfile" << endl
			<< endl
			<< "\t-i        infile      Input FSA file" << endl
			<< "\t-o        outfile     Output FSA file" << endl
			<< "\t-dot-in   dotinfile   Filename to write DOT file from input FSA" << endl
			<< "\t-dot-out  dotoutfile  Filename to write DOT file from output FSA" << endl
			<< "\t-h                    Show this help message" << endl
			;

		return 0;
	}

	Convert(opt);

	return 0;
}
