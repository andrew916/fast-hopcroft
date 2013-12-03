// June 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#include "../MinimizationHopcroft.h"
#include "../MinimizationBrzozowski.h"
#include "../MinimizationIncremental.h"
#include "../MinimizationHybrid.h"
#include "../Dfa.h"
#include "../Nfa.h"
#include "../FsaGraphVizWriter.h"
#include "../FsaFormatReader.h"
#include "../FsaPlainTextReader.h"
#include "../FsaPlainTextWriter.h"
#include "../FsaFormat.h"
#include <boost/timer/timer.hpp>
#include <boost/format.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <string>

namespace transcode
{

	using namespace std;
	using namespace boost::program_options;
	using boost::filesystem::ifstream;
	using boost::filesystem::ofstream;
	using boost::format;	

	class Options
	{
	public:
		string InputFile;
		string OutputFile;
		FsaFormat InputFormat;
		FsaFormat OutputFormat;
		bool ShowHelp;
		bool Verbose;

		Options() : 		
			ShowHelp(false), 
			Verbose(false),
			InputFormat(FsaFormat::None),
			OutputFormat(FsaFormat::None)
		{
		}
	};

	// TODO: Take advantage of polymorph code when it is available
	void Transcode(Options opt)
	{
		ifstream input_file(opt.InputFile);
		ofstream output_file(opt.OutputFile);

		typedef Nfa<uint32_t, uint32_t> TFsa;

		if(opt.InputFormat == FsaFormat::GraphViz) 
			throw invalid_argument("GraphViz in not supported as input format");

		if(opt.OutputFormat == FsaFormat::OneBasedPlainText) 
			throw invalid_argument("One-Based-plain-text format is not supported as output format");

		if(opt.InputFormat == FsaFormat::OneBasedPlainText)
		{			
			FsaPlainTextReaderOneBased<TFsa> reader;
			auto fsa = reader.Read(input_file);
			if(opt.OutputFormat == FsaFormat::ZeroBasedPlainText) 
			{
				FsaPlainTextWriter<TFsa> writer;
				writer.Write(fsa, output_file);
			} 
			else if(opt.OutputFormat == FsaFormat::GraphViz)
			{
				FsaGraphVizWriter<TFsa> writer;
				writer.Write(fsa, output_file);
			}
			else
			{
				throw invalid_argument("This transcode scenario is not supported");
			}
		}
		else if(opt.InputFormat == FsaFormat::ZeroBasedPlainText)
		{
			FsaPlainTextReader<TFsa> reader;
			auto fsa = reader.Read(input_file);
			if(opt.OutputFormat == FsaFormat::GraphViz)
			{
				FsaGraphVizWriter<TFsa> writer;
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
	Options o;	
	options_description opt_desc("Allowed options");
	opt_desc.add_options() 
		("help,?", bool_switch(&o.ShowHelp)->default_value(false), "Show this information")
		("output,o", value(&o.OutputFile), "Output file")
		("input,i", value(&o.InputFile), "Output file")
		("iformat,k", value(&o.InputFormat), "input file format")
		("oformat,t", value(&o.OutputFormat), "output file format")
		("verbose,v", value(&o.Verbose)->default_value(false), "Verbose mode")
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
	try
	{
		if(o.InputFile.empty()) throw invalid_argument("missing input file param");
		if(o.OutputFile.empty()) throw invalid_argument("missing output file param");
		if(o.InputFormat == FsaFormat::None) throw invalid_argument("missing input format param");
		if(o.OutputFormat == FsaFormat::None) throw invalid_argument("missing input format param");

		Transcode(o);
	}
	catch(exception ex)
	{
		cout << "Error: " << ex.what() << endl;
		return -1;
	}

	return 0;
}
