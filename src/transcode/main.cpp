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
#include <boost/timer/timer.hpp>
#include <boost/format.hpp>
#include <boost/filesystem/fstream.hpp>
#include <string>

namespace transcode
{

	using namespace std;
	using boost::filesystem::ifstream;
	using boost::filesystem::ofstream;
	using boost::format;

	enum class Format
	{
		None,
		ZeroBasedPlainText,
		OneBasedPlainText,
		GraphViz
	};

	class Options
	{
	public:
		string InputFile;
		string OutputFile;
		Format InputFormat;
		Format OutputFormat;
		bool ShowHelp;
		bool Verbose;

		Options() : 		
			ShowHelp(false), 
			Verbose(false),
			InputFormat(Format::None),
			OutputFormat(Format::None)
		{
		}
	};

	Format ParseFormat(string fmt)
	{
		if(fmt == "zero-based-text") return Format::ZeroBasedPlainText;
		else if(fmt == "one-based-text") return Format::OneBasedPlainText;
		else if(fmt == "graphviz") return Format::GraphViz;
		else if(fmt == "none") return Format::None;
		else throw invalid_argument("Unknown format: " + fmt);
	}

	string to_string(Format fmt)
	{
		switch (fmt)
		{
		case Format::None: return "none";
		case Format::ZeroBasedPlainText: return "zero-based-text";
		case Format::OneBasedPlainText: return "one-based-text";
		case Format::GraphViz: return "graphviz";		
		}
		throw invalid_argument("Unknown format");
	}

	void ParseCommandLine(int argc, char** argv, Options& opt)
	{
		for(int i=1; i<argc; i++)
		{
			string arg = argv[i];
			if(arg == "-if")
			{
				string format_str = argv[i + 1];
				opt.InputFormat = ParseFormat(format_str);
				i++;
			}
			if(arg == "-of")
			{
				string format_str = argv[i + 1];
				opt.OutputFormat = ParseFormat(format_str);
				i++;
			}
			else if(arg == "-i")
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
		if(!opt.ShowHelp && opt.InputFile.empty()) throw invalid_argument("missing input file param");
		if(!opt.ShowHelp && opt.OutputFile.empty()) throw invalid_argument("missing output file param");
		if(!opt.ShowHelp && opt.InputFormat == Format::None) throw invalid_argument("missing input format param");
		if(!opt.ShowHelp && opt.OutputFormat == Format::None) throw invalid_argument("missing input format param");
	}

	// TODO: Take advantage of polymorph code when it is available
	void Transcode(Options opt)
	{
		ifstream input_file(opt.InputFile);
		ofstream output_file(opt.OutputFile);

		typedef Nfa<uint32_t, uint32_t> TFsa;

		if(opt.InputFormat == Format::GraphViz) 
			throw invalid_argument("GraphViz in not supported as input format");

		if(opt.OutputFormat == Format::OneBasedPlainText) 
			throw invalid_argument("One-Based-plain-text format is not supported as output format");

		if(opt.InputFormat == Format::OneBasedPlainText)
		{			
			FsmPlainTextReaderOneBased<TFsa> reader;
			auto fsa = reader.Read(input_file);
			if(opt.OutputFormat == Format::ZeroBasedPlainText) 
			{
				FsmPlainTextWriter<TFsa> writer;
				writer.Write(fsa, output_file);
			} 
			else if(opt.OutputFormat == Format::GraphViz)
			{
				FsmGraphVizWriter<TFsa> writer;
				writer.Write(fsa, output_file);
			}
			else
			{
				throw invalid_argument("This transcode scenario is not supported");
			}
		}
		else if(opt.InputFormat == Format::ZeroBasedPlainText)
		{
			FsmPlainTextReader<TFsa> reader;
			auto fsa = reader.Read(input_file);
			if(opt.OutputFormat == Format::GraphViz)
			{
				FsmGraphVizWriter<TFsa> writer;
				writer.Write(fsa, output_file);
			}
			else 
			{
				throw invalid_argument("This transcode scenario is not supported");
			}
		}
		else 
		{
			throw invalid_argument("input format not supported");
		}
	}

}

using namespace transcode;

int main(int argc, char** argv)
{
	Options opt;

	ParseCommandLine(argc, argv, opt);

	if(opt.ShowHelp)
	{
		cout << "Usage:" << endl 
			<< argv[0] << " -i <infile> -o <outfile> -if <format> -of <format> [-h|-?] [-v]" << endl
			<< endl
			<< "\t-i <infile>         Input filename" << endl
			<< "\t-o <outfile>        Output filename" << endl
			<< "\t-if <format>        Input file format" << endl
			<< "\t-of <format>        Output file format" << endl
			<< "\t-h,-?               Show this help message" << endl
			<< "\t-v                  Verbose mode" << endl
			<< endl
			<< "Available formats: zero-based-text, one-based-text, graphviz" << endl
			;
	}
	else Transcode(opt);

	return 0;
}
