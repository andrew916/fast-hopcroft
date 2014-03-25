// June 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#include "../MinimizationHopcroft.h"
#include "../MinimizationBrzozowski.h"
#include "../MinimizationIncremental.h"
#include "../MinimizationHybrid.h"
#include "../MinimizationAtomic.h"
#include "../MinimizationAlgorithm.h"
#include "../Dfa.h"
#include "../Nfa.h"
#include "../Fsa.h"
#include "../Set.h"
#include "../FsaFormat.h"
#include "../FsaFormatReader.h"
#include "../FsaPlainTextReader.h"
#include "../AlmeidaPlainTextReader.h"
#include "../FsaGraphVizWriter.h"
#include "../FsaPlainTextWriter.h"
#include "../Determinization.h"
#include "../NfaGenerator.h"
#include <fstream>
#include <map>
#include <boost/timer/timer.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <array>
#include <stdexcept>

using namespace std;
using boost::format;

char** global_argv;
int global_argc;

// Utilities

template<typename TFsa>
void write_dot(TFsa fsa, string filename)
{
	FsaGraphVizWriter<TFsa> writer;
	ofstream s(filename);
	if(!s.is_open()) 
	{
		throw invalid_argument("El archivo no pudo ser abierto");
	}
	writer.Write(fsa, s);
	s.close();
}

template<typename TFsa>
void write_text(TFsa fsa, string filename)
{
	FsaPlainTextWriter<TFsa> writer;
	ofstream s(filename);
	if(!s.is_open()) 
	{
		throw invalid_argument("El archivo no pudo ser abierto");
	}
	writer.Write(fsa, s);
	s.close();
}

template<typename TDfa>
void write_dfa_info(const TDfa& dfa)
{
	typedef typename TDfa::TState TState;
	typedef typename TDfa::TSymbol TSymbol;
	// Transitions
	cout << "Transitions" << endl;
	for(TState s=0; s<dfa.GetStates(); s++)
	{
		size_t state = static_cast<size_t>(s);
		cout << state << " | ";
		for(TSymbol a=0; a<dfa.GetAlphabetLength(); a++)
		{
			TState tgt = dfa.GetSuccessor(s, a);
			size_t target = static_cast<size_t>(tgt);
			cout << target;
			cout << "\t"; // padding
		}
		cout << endl;
	}
	cout << endl;
	cout << "Inverse delta" << endl;
	for(TState s=0; s<dfa.GetStates(); s++)
	{
		size_t state = static_cast<size_t>(s);
		cout << state << " | ";
		for(TSymbol a=0; a<dfa.GetAlphabetLength(); a++)
		{
			auto pred = dfa.GetPredecessors(s, a);
			int cont = 0;
			cout << "{";
			for(auto i=pred.GetIterator(); !i.IsEnd(); i.MoveNext())
			{
				if(cont++ > 0) cout << ",";
				size_t target = static_cast<size_t>(i.GetCurrent());
				cout << target;
			}			
			cout << "}";
			cont -= 3 + cont;
			while(cont++ < dfa.GetStates()) cout << " "; // padding			
		}
		cout << endl;
	}
	cout << endl;
}

template<typename TFsa>
TFsa read_text(const string& filename)
{
	FsaPlainTextReader<TFsa> reader;
	ifstream fsa_input(filename);
	if(!fsa_input.is_open()) throw logic_error("file could not be opened");
	auto fsa = reader.Read(fsa_input);
	fsa_input.close();
	return fsa;
}

template<typename TFsa>
TFsa read_text_one_based(const string& filename)
{
	FsaPlainTextReaderOneBased<TFsa> reader;
	ifstream fsa_input(filename);
	if(!fsa_input.is_open()) throw logic_error("file could not be opened");
	auto fsa = reader.Read(fsa_input);
	fsa_input.close();
	return fsa;
}

template<typename TFsa> 
void begin_read_text_almeida(ifstream& fsa_input, const string& filename, typename TFsa::TState* n, typename TFsa::TSymbol* k)
{
	// format "n<number>k<number>" skip ".n"
	string ext(filename.begin() + filename.find_last_of('.') + 2, filename.end());
	vector<string> vc;
	boost::split(vc, ext, boost::is_any_of("nk"), boost::algorithm::token_compress_on);

	*n = boost::lexical_cast<typename TFsa::TState>(vc[0]);
	*k = boost::lexical_cast<typename TFsa::TSymbol>(vc[1]);
	AlmeidaPlainTextReader<TFsa> reader(*n, *k);

	if(!fsa_input.is_open()) throw logic_error("file could not be opened");
	reader.ReadHeader(fsa_input);	
}

template<typename TFsa>
TFsa read_text_almeida(istream& fsa_input, typename TFsa::TState states, typename TFsa::TSymbol alpha)
{
	AlmeidaPlainTextReader<TFsa> reader(alpha, states);
	return reader.Read(fsa_input);
}

// Tests Hopcroft 100-199

int test100()
{	
	cout << "Esta prueba minimiza un automata sencillo usando Hopcroft y escribe el resultado en un archivo test1.dot" << endl;

	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	typedef Dfa<TState, TSymbol> TDfa;
	MinimizationHopcroft<TDfa> mini;	
	TDfa dfa(2, 4);

	//   / 2 \
	// 0 - 1 - 3
	dfa.SetInitial(0);
	dfa.SetFinal(3);

	dfa.SetTransition(0, 0, 1);
	dfa.SetTransition(0, 1, 2);

	dfa.SetTransition(1, 0, 3);
	dfa.SetTransition(1, 1, 1);

	dfa.SetTransition(2, 0, 3);
	dfa.SetTransition(2, 1, 2);

	dfa.SetTransition(3, 0, 3);
	dfa.SetTransition(3, 1, 3);

	MinimizationHopcroft<TDfa>::NumericPartition out_partitions;
	mini.ShowConfiguration=true;
	mini.Minimize(dfa, out_partitions);

	// asegura que la cantidad de estados al final es menor
	assert(out_partitions.GetSize() < dfa.GetStates());
	assert(out_partitions.GetSize() == 3);

	cout << endl;

	return 0;
}

int test101()
{
	cout << "Esta prueba minimiza un automata sencillo usando Hopcroft y escribe el resultado en un archivo test2.dot" << endl;

	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	typedef Dfa<TState,TSymbol> TDfa;
	MinimizationHopcroft<TDfa> mini;	
	//    / 1 - 3
	//  0
	//    \ 2 - 4
	TDfa dfa(2, 5);

	dfa.SetInitial(0);
	dfa.SetFinal(3);
	dfa.SetFinal(4);

	dfa.SetTransition(0, 0, 1);
	dfa.SetTransition(0, 1, 2);

	dfa.SetTransition(1, 0, 3);
	dfa.SetTransition(1, 1, 1);

	dfa.SetTransition(2, 0, 4);
	dfa.SetTransition(2, 1, 2);

	dfa.SetTransition(3, 0, 3);
	dfa.SetTransition(3, 1, 3);

	dfa.SetTransition(4, 0, 4);
	dfa.SetTransition(4, 1, 4);

	MinimizationHopcroft<TDfa>::NumericPartition out_partitions;
	mini.ShowConfiguration=true;
	mini.Minimize(dfa, out_partitions);

	// asegura que la cantidad de estados al final es menor
	assert(out_partitions.GetSize() < dfa.GetStates());
	assert(out_partitions.GetSize() == 3);

	cout << endl;

	return 0;
}

int test102()
{
	cout << "Esta prueba minimiza un automata sencillo usando Hopcroft y escribe el resultado en un archivo test3.dot" << endl;

	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	typedef Dfa<TState, TSymbol> TDfa;
	MinimizationHopcroft<TDfa> mini;	
	// uses zero as invisible null-sink state
	//      2 - 5 - 8  - 11
	//    /
	//   1 -3 - 6 - 9  - 12
	//    \
	//      4 - 7 - 10 - 13
	TDfa dfa(3, 14);

	dfa.SetInitial(1);
	dfa.SetFinal(11);
	dfa.SetFinal(12);
	dfa.SetFinal(13);

	dfa.SetTransition(1, 0, 2);
	dfa.SetTransition(1, 1, 3);
	dfa.SetTransition(1, 2, 4);

	dfa.SetTransition(2, 0, 5);
	dfa.SetTransition(3, 0, 6);
	dfa.SetTransition(4, 0, 7);

	dfa.SetTransition(5, 0, 8);
	dfa.SetTransition(6, 1, 9);
	dfa.SetTransition(7, 2, 10);

	dfa.SetTransition(8, 0, 11);
	dfa.SetTransition(9, 1, 12);
	dfa.SetTransition(10, 2, 13);

	MinimizationHopcroft<TDfa>::NumericPartition out_partitions;
	mini.ShowConfiguration=true;
	mini.Minimize(dfa, out_partitions);

	// asegura que la cantidad de estados al final es menor
	assert(out_partitions.GetSize() < dfa.GetStates());
	assert(out_partitions.GetSize() == 12);

	cout << endl;

	return 0;
}

int test103()
{
	cout << "Esta prueba minimiza un automata sencillo usando Hopcroft y escribe el resultado en un archivo test4.dot" << endl;

	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	typedef Dfa<TState, TSymbol> TDfa;
	MinimizationHopcroft<TDfa> mini;	
	// uses zero as invisible null-sink state
	//      2 - 5 - 8  - 11
	//    /       \   /
	//   1 -3 - 6 - 9  - 12
	//    \       \   \
	//      4 - 7 - 10 - 13
	Dfa<TState, TSymbol> dfa(3, 14);

	dfa.SetInitial(1);
	dfa.SetFinal(11);
	dfa.SetFinal(12);
	dfa.SetFinal(13);

	dfa.SetTransition(1, 0, 2);
	dfa.SetTransition(1, 1, 3);
	dfa.SetTransition(1, 2, 4);

	dfa.SetTransition(2, 0, 5);
	dfa.SetTransition(3, 0, 6);
	dfa.SetTransition(4, 0, 7);

	dfa.SetTransition(5, 0, 8);		
	dfa.SetTransition(5, 1, 9);		
	dfa.SetTransition(5, 2, 9);		

	dfa.SetTransition(6, 1, 9);
	dfa.SetTransition(6, 0, 10);
	dfa.SetTransition(6, 2, 10);

	dfa.SetTransition(7, 0, 10);
	dfa.SetTransition(7, 1, 10);
	dfa.SetTransition(7, 2, 10);

	dfa.SetTransition(8, 0, 11);
	dfa.SetTransition(8, 1, 11);
	dfa.SetTransition(8, 2, 11);

	dfa.SetTransition(9, 0, 11);
	dfa.SetTransition(9, 1, 12);
	dfa.SetTransition(9, 2, 13);

	dfa.SetTransition(10, 0, 13);
	dfa.SetTransition(10, 1, 13);
	dfa.SetTransition(10, 2, 13);

	MinimizationHopcroft<TDfa>::NumericPartition out_partitions;
	mini.ShowConfiguration=true;
	mini.Minimize(dfa, out_partitions);

	// asegura que la cantidad de estados al final es menor
	assert(out_partitions.GetSize() < dfa.GetStates());
	assert(out_partitions.GetSize() == 6);

	cout << endl;

	return 0;
}

int test104()
{
	cout << "Esta prueba minimiza diferentes automatas desde archivos en la carpeta AFD usando Hopcroft. Se omite la reconstruccion del DFA minimo" << endl;

	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	typedef Dfa<TState, TSymbol> TDfa;
	MinimizationHopcroft<TDfa> mini;	
	MinimizationHopcroft<TDfa>::NumericPartition np;
	FsaFormatReader<TDfa> parser;
	vector<string> files;

	files.push_back("afd\\000_n512k2.afd");
	files.push_back("afd\\000_n1024k2.afd");
	files.push_back("afd\\000_n2046k2.afd");
	files.push_back("afd\\000_n4092k2.afd");
	files.push_back("afd\\000_n8192k2.afd");
	files.push_back("afd\\000_n16384k2.afd");

	files.push_back("afd\\001_n512k2.afd");
	files.push_back("afd\\001_n1024k2.afd");
	files.push_back("afd\\001_n2046k2.afd");
	files.push_back("afd\\001_n4092k2.afd");
	files.push_back("afd\\001_n8192k2.afd");
	files.push_back("afd\\001_n16384k2.afd");

	files.push_back("afd\\002_n512k2.afd");
	files.push_back("afd\\002_n1024k2.afd");
	files.push_back("afd\\002_n2046k2.afd");
	files.push_back("afd\\002_n4092k2.afd");
	files.push_back("afd\\002_n8192k2.afd");
	files.push_back("afd\\002_n16384k2.afd");

	files.push_back("afd\\003_n512k2.afd");
	files.push_back("afd\\003_n1024k2.afd");
	files.push_back("afd\\003_n2046k2.afd");
	files.push_back("afd\\003_n4092k2.afd");
	files.push_back("afd\\003_n8192k2.afd");
	files.push_back("afd\\003_n16384k2.afd");

	files.push_back("afd\\004_n512k2.afd");
	files.push_back("afd\\004_n1024k2.afd");
	files.push_back("afd\\004_n2046k2.afd");
	files.push_back("afd\\004_n4092k2.afd");
	files.push_back("afd\\004_n8192k2.afd");
	files.push_back("afd\\004_n16384k2.afd");

	files.push_back("afd\\005_n512k2.afd");
	files.push_back("afd\\005_n1024k2.afd");
	files.push_back("afd\\005_n2046k2.afd");
	files.push_back("afd\\005_n4092k2.afd");
	files.push_back("afd\\005_n8192k2.afd");
	files.push_back("afd\\005_n16384k2.afd");

	files.push_back("afd\\006_n512k2.afd");
	files.push_back("afd\\006_n1024k2.afd");
	files.push_back("afd\\006_n2046k2.afd");
	files.push_back("afd\\006_n4092k2.afd");
	files.push_back("afd\\006_n8192k2.afd");
	files.push_back("afd\\006_n16384k2.afd");

	files.push_back("afd\\007_n512k2.afd");
	files.push_back("afd\\007_n1024k2.afd");
	files.push_back("afd\\007_n2046k2.afd");
	files.push_back("afd\\007_n4092k2.afd");
	files.push_back("afd\\007_n8192k2.afd");
	files.push_back("afd\\007_n16384k2.afd");

	files.push_back("afd\\008_n512k2.afd");
	files.push_back("afd\\008_n1024k2.afd");
	files.push_back("afd\\008_n2046k2.afd");
	files.push_back("afd\\008_n4092k2.afd");
	files.push_back("afd\\008_n8192k2.afd");
	files.push_back("afd\\008_n16384k2.afd");

	files.push_back("afd\\009_n512k2.afd");
	files.push_back("afd\\009_n1024k2.afd");
	files.push_back("afd\\009_n2046k2.afd");
	files.push_back("afd\\009_n4092k2.afd");
	files.push_back("afd\\009_n8192k2.afd");
	files.push_back("afd\\009_n16384k2.afd");

	ifstream afd;
	ofstream report ("report.txt");
	Dfa<uint16_t, uint8_t> dfa(0,0);
	mini.ShowConfiguration = false;

	for(auto filename : files) 
	{
		afd.open(filename);
		if(afd.fail())  
		{
			cout << "No se pudo abrir el archivo: " << filename << endl;
			cout << endl;
			continue;
		}
		dfa = parser.Read(afd);
		afd.close();

		{
			stringstream str;
			str << "Begin, states: " << dfa.GetStates() << ", alpha: " << dfa.GetAlphabetLength() << endl;
			cout << str.str();
			report << str.str();
		}

		boost::timer::cpu_timer timer;
		timer.start();
		mini.Minimize(dfa, np);
		timer.stop();

		{
			stringstream str;
			str << "Done with " << filename << " elapsed (ms) " << timer.elapsed().wall / 1000000UL << endl;
			cout << str.str();
			report << str.str();
			cout << endl;
		}
	}
	return 0;
}


// Tests Brzozowski 200-299

int test200()
{	
	cout << "Esta prueba minimiza un automata sencillo usando Brzozowski y escribe el resultado en un archivo test6.dot" << endl;

	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	typedef Nfa<TState, TSymbol> TDfa;
	MinimizationBrzozowski<TDfa> mini;	
	TDfa dfa(2, 4);

	//   / 2 \
	// 0 - 1 - 3
	dfa.SetInitial(0);
	dfa.SetFinal(3);

	dfa.SetTransition(0, 0, 1);
	dfa.SetTransition(0, 1, 2);

	dfa.SetTransition(1, 0, 3);
	dfa.SetTransition(1, 1, 1);

	dfa.SetTransition(2, 0, 3);
	dfa.SetTransition(2, 1, 2);

	dfa.SetTransition(3, 0, 3);
	dfa.SetTransition(3, 1, 3);

	auto dfa_min = mini.Minimize(dfa);

	// asegura que la cantidad de estados al final es menor
	assert(dfa_min.GetStates() < dfa.GetStates());
	assert(dfa_min.GetStates() == 3);

	cout << endl;

	return 0;
}

int test201()
{
	cout << "Esta prueba minimiza un automata sencillo usando Brzozowski y escribe el resultado en un archivo test2.dot" << endl;

	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	typedef Nfa<TState,TSymbol> TNfa;
	typedef Dfa<TState,TSymbol> TDfa;
	MinimizationBrzozowski<TNfa, TDfa> mini;	
	//    / 1 - 3
	//  0
	//    \ 2 - 4
	TNfa dfa(2, 5);

	dfa.SetInitial(0);
	dfa.SetFinal(3);
	dfa.SetFinal(4);

	dfa.SetTransition(0, 0, 1);
	dfa.SetTransition(0, 1, 2);

	dfa.SetTransition(1, 0, 3);
	dfa.SetTransition(1, 1, 1);

	dfa.SetTransition(2, 0, 4);
	dfa.SetTransition(2, 1, 2);

	dfa.SetTransition(3, 0, 3);
	dfa.SetTransition(3, 1, 3);

	dfa.SetTransition(4, 0, 4);
	dfa.SetTransition(4, 1, 4);

	auto dfa_min = mini.Minimize(dfa);

	// asegura que la cantidad de estados al final es menor
	assert(dfa_min.GetStates() < dfa.GetStates());
	assert(dfa_min.GetStates() == 3);

	cout << endl;

	return 0;
}

int test202()
{
	cout << "Esta prueba minimiza un automata sencillo usando Brzozowski y escribe el resultado en un archivo test3.dot" << endl;

	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	typedef Dfa<TState, TSymbol> TDfa;
	typedef Nfa<TState,TSymbol> TNfa;
	MinimizationBrzozowski<TNfa, TDfa> mini;	
	// uses zero as invisible null-sink state
	//      2 - 5 - 8  - 11
	//    /
	//   1 -3 - 6 - 9  - 12
	//    \
	//      4 - 7 - 10 - 13
	TNfa dfa(3, 14);

	dfa.SetInitial(1);
	dfa.SetFinal(11);
	dfa.SetFinal(12);
	dfa.SetFinal(13);

	dfa.SetTransition(1, 0, 2);
	dfa.SetTransition(1, 1, 3);
	dfa.SetTransition(1, 2, 4);

	dfa.SetTransition(2, 0, 5);
	dfa.SetTransition(3, 0, 6);
	dfa.SetTransition(4, 0, 7);

	dfa.SetTransition(5, 0, 8);
	dfa.SetTransition(6, 1, 9);
	dfa.SetTransition(7, 2, 10);

	dfa.SetTransition(8, 0, 11);
	dfa.SetTransition(9, 1, 12);
	dfa.SetTransition(10, 2, 13);

	auto dfa_min = mini.Minimize(dfa);

	// asegura que la cantidad de estados al final es menor
	assert(dfa_min.GetStates() < dfa.GetStates());
	assert(dfa_min.GetStates() == 12);

	cout << endl;

	return 0;
}

int test203()
{
	cout << "Esta prueba minimiza un automata sencillo usando Brzozowski y escribe el resultado en un archivo test4.dot" << endl;

	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	typedef Dfa<TState, TSymbol> TDfa;
	typedef Nfa<TState,TSymbol> TNfa;
	MinimizationBrzozowski<TNfa, TDfa> mini;	
	// uses zero as invisible null-sink state
	//      2 - 5 - 8  - 11
	//    /       \   /
	//   1 -3 - 6 - 9  - 12
	//    \       \   \
	//      4 - 7 - 10 - 13
	TNfa dfa(3, 14);

	dfa.SetInitial(1);
	dfa.SetFinal(11);
	dfa.SetFinal(12);
	dfa.SetFinal(13);

	dfa.SetTransition(1, 0, 2);
	dfa.SetTransition(1, 1, 3);
	dfa.SetTransition(1, 2, 4);

	dfa.SetTransition(2, 0, 5);
	dfa.SetTransition(3, 0, 6);
	dfa.SetTransition(4, 0, 7);

	dfa.SetTransition(5, 0, 8);		
	dfa.SetTransition(5, 1, 9);		
	dfa.SetTransition(5, 2, 9);		

	dfa.SetTransition(6, 1, 9);
	dfa.SetTransition(6, 0, 10);
	dfa.SetTransition(6, 2, 10);

	dfa.SetTransition(7, 0, 10);
	dfa.SetTransition(7, 1, 10);
	dfa.SetTransition(7, 2, 10);

	dfa.SetTransition(8, 0, 11);
	dfa.SetTransition(8, 1, 11);
	dfa.SetTransition(8, 2, 11);

	dfa.SetTransition(9, 0, 11);
	dfa.SetTransition(9, 1, 12);
	dfa.SetTransition(9, 2, 13);

	dfa.SetTransition(10, 0, 13);
	dfa.SetTransition(10, 1, 13);
	dfa.SetTransition(10, 2, 13);

	auto dfa_min = mini.Minimize(dfa);

	// asegura que la cantidad de estados al final es menor
	assert(dfa_min.GetStates() < dfa.GetStates());
	assert(dfa_min.GetStates() == 6);

	cout << endl;

	return 0;
}

// Tests Incremental 300-399

int test300()
{	
	cout << "Esta prueba minimiza un automata sencillo usando Incremental y escribe el resultado en un archivo test6.dot" << endl;

	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	typedef Dfa<TState, TSymbol> TDfa;
	MinimizationIncremental<TDfa> mini;	
	TDfa dfa(2, 4);

	//   / 2 \
	// 0 - 1 - 3
	dfa.SetInitial(0);
	dfa.SetFinal(3);

	dfa.SetTransition(0, 0, 1);
	dfa.SetTransition(0, 1, 2);

	dfa.SetTransition(1, 0, 3);
	dfa.SetTransition(1, 1, 1);

	dfa.SetTransition(2, 0, 3);
	dfa.SetTransition(2, 1, 2);

	dfa.SetTransition(3, 0, 3);
	dfa.SetTransition(3, 1, 3);

	mini.ShowConfiguration=true;
	auto dfa_min = mini.Minimize(dfa);

	// asegura que la cantidad de estados al final es menor
	assert(dfa_min.GetStates() < dfa.GetStates());
	assert(dfa_min.GetStates() == 3);

	cout << endl;

	return 0;
}

int test301()
{
	cout << "Esta prueba minimiza un automata sencillo usando Brzozowski y escribe el resultado en un archivo test2.dot" << endl;

	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	typedef Dfa<TState,TSymbol> TDfa;
	MinimizationIncremental<TDfa> mini;	
	//    / 1 - 3
	//  0
	//    \ 2 - 4
	TDfa dfa(2, 5);

	dfa.SetInitial(0);
	dfa.SetFinal(3);
	dfa.SetFinal(4);

	dfa.SetTransition(0, 0, 1);
	dfa.SetTransition(0, 1, 2);

	dfa.SetTransition(1, 0, 3);
	dfa.SetTransition(1, 1, 1);

	dfa.SetTransition(2, 0, 4);
	dfa.SetTransition(2, 1, 2);

	dfa.SetTransition(3, 0, 3);
	dfa.SetTransition(3, 1, 3);

	dfa.SetTransition(4, 0, 4);
	dfa.SetTransition(4, 1, 4);

	mini.ShowConfiguration=true;
	TDfa dfa_min = mini.Minimize(dfa);

	// asegura que la cantidad de estados al final es menor
	assert(dfa_min.GetStates() < dfa.GetStates());
	assert(dfa_min.GetStates() == 3);

	cout << endl;

	return 0;
}

int test310()
{

	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	typedef Dfa<TState,TSymbol> TDfa;
	MinimizationIncremental<TDfa> mini;	
	TDfa dfa(2, 10);

	dfa.SetInitial(0);

	dfa.SetFinal(1);
	dfa.SetFinal(2);
	dfa.SetFinal(3);
	dfa.SetFinal(5);
	dfa.SetFinal(6);

	dfa.SetTransition(0,0,1);
	dfa.SetTransition(0,1,2);
	dfa.SetTransition(1,0,3);
	dfa.SetTransition(1,1,4);
	dfa.SetTransition(2,0,5);
	dfa.SetTransition(2,1,4);
	dfa.SetTransition(3,0,3);
	dfa.SetTransition(3,1,1);
	dfa.SetTransition(4,0,6);
	dfa.SetTransition(4,1,2);
	dfa.SetTransition(5,0,7);
	dfa.SetTransition(5,1,2);
	dfa.SetTransition(6,0,3);
	dfa.SetTransition(6,1,8);
	dfa.SetTransition(7,0,7);
	dfa.SetTransition(7,1,7);
	dfa.SetTransition(8,0,6);
	dfa.SetTransition(8,1,9);
	dfa.SetTransition(9,0,7);
	dfa.SetTransition(9,1,9);

	mini.ShowConfiguration=true;
	auto dfa_min = mini.Minimize(dfa);

	write_dot(dfa, "test_310.dot");
	write_dot(dfa_min, "test_310_min.dot");

	return 0;
}

int test302()
{
	cout << "Esta prueba minimiza un automata sencillo usando Incremental y escribe el resultado en un archivo test3.dot" << endl;

	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	typedef Dfa<TState, TSymbol> TDfa;
	MinimizationIncremental<TDfa> mini;	
	// uses zero as invisible null-sink state
	//      2 - 5 - 8  - 11
	//    /
	//   1 -3 - 6 - 9  - 12
	//    \
	//      4 - 7 - 10 - 13
	TDfa dfa(3, 14);

	dfa.SetInitial(1);
	dfa.SetFinal(11);
	dfa.SetFinal(12);
	dfa.SetFinal(13);

	dfa.SetTransition(1, 0, 2);
	dfa.SetTransition(1, 1, 3);
	dfa.SetTransition(1, 2, 4);

	dfa.SetTransition(2, 0, 5);
	dfa.SetTransition(3, 0, 6);
	dfa.SetTransition(4, 0, 7);

	dfa.SetTransition(5, 0, 8);
	dfa.SetTransition(6, 1, 9);
	dfa.SetTransition(7, 2, 10);

	dfa.SetTransition(8, 0, 11);
	dfa.SetTransition(9, 1, 12);
	dfa.SetTransition(10, 2, 13);

	mini.ShowConfiguration=true;
	auto dfa_min = mini.Minimize(dfa);

	write_dot(dfa, "test_302.dot");
	write_dot(dfa_min, "test_302_min.dot");

	// asegura que la cantidad de estados al final es menor
	assert(dfa_min.GetStates() < dfa.GetStates());
	assert(dfa_min.GetStates() == 12);

	cout << endl;

	return 0;
}

int test303()
{
	cout << "Esta prueba minimiza un automata sencillo usando Incremental y escribe el resultado en un archivo test4.dot" << endl;

	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	typedef Dfa<TState, TSymbol> TDfa;
	MinimizationIncremental<TDfa> mini;	
	// uses zero as invisible null-sink state
	//      2 - 5 - 8  - 11
	//    /       \   /
	//   1 -3 - 6 - 9  - 12
	//    \       \   \
	//      4 - 7 - 10 - 13
	TDfa dfa(3, 14);

	dfa.SetInitial(1);
	dfa.SetFinal(11);
	dfa.SetFinal(12);
	dfa.SetFinal(13);

	dfa.SetTransition(1, 0, 2);
	dfa.SetTransition(1, 1, 3);
	dfa.SetTransition(1, 2, 4);

	dfa.SetTransition(2, 0, 5);
	dfa.SetTransition(3, 0, 6);
	dfa.SetTransition(4, 0, 7);

	dfa.SetTransition(5, 0, 8);		
	dfa.SetTransition(5, 1, 9);		
	dfa.SetTransition(5, 2, 9);		

	dfa.SetTransition(6, 1, 9);
	dfa.SetTransition(6, 0, 10);
	dfa.SetTransition(6, 2, 10);

	dfa.SetTransition(7, 0, 10);
	dfa.SetTransition(7, 1, 10);
	dfa.SetTransition(7, 2, 10);

	dfa.SetTransition(8, 0, 11);
	dfa.SetTransition(8, 1, 11);
	dfa.SetTransition(8, 2, 11);

	dfa.SetTransition(9, 0, 11);
	dfa.SetTransition(9, 1, 12);
	dfa.SetTransition(9, 2, 13);

	dfa.SetTransition(10, 0, 13);
	dfa.SetTransition(10, 1, 13);
	dfa.SetTransition(10, 2, 13);

	mini.ShowConfiguration=true;
	auto dfa_min = mini.Minimize(dfa);

	// asegura que la cantidad de estados al final es menor
	assert(dfa_min.GetStates() < dfa.GetStates());
	assert(dfa_min.GetStates() == 6);

	cout << endl;

	return 0;
}

// Test Hybrid 600-699

int test600()
{	
	cout << "Esta prueba minimiza un automata sencillo usando Incremental y escribe el resultado en un archivo test6.dot" << endl;

	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	typedef Dfa<TState, TSymbol> TDfa;
	MinimizationHybrid<TDfa> mini;	
	TDfa dfa(2, 4);

	//   / 2 \
	// 0 - 1 - 3
	dfa.SetInitial(0);
	dfa.SetFinal(3);

	dfa.SetTransition(0, 0, 1);
	dfa.SetTransition(0, 1, 2);

	dfa.SetTransition(1, 0, 3);
	dfa.SetTransition(1, 1, 1);

	dfa.SetTransition(2, 0, 3);
	dfa.SetTransition(2, 1, 2);

	dfa.SetTransition(3, 0, 3);
	dfa.SetTransition(3, 1, 3);

	mini.ShowConfiguration=true;
	auto dfa_min = mini.Minimize(dfa);

	// asegura que la cantidad de estados al final es menor
	assert(dfa_min.GetStates() < dfa.GetStates());
	assert(dfa_min.GetStates() == 3);

	cout << endl;

	return 0;
}

int test601()
{
	cout << "Esta prueba minimiza un automata sencillo usando Brzozowski y escribe el resultado en un archivo test2.dot" << endl;

	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	typedef Dfa<TState,TSymbol> TDfa;
	MinimizationHybrid<TDfa> mini;	
	//    / 1 - 3
	//  0
	//    \ 2 - 4
	TDfa dfa(2, 5);

	dfa.SetInitial(0);
	dfa.SetFinal(3);
	dfa.SetFinal(4);

	dfa.SetTransition(0, 0, 1);
	dfa.SetTransition(0, 1, 2);

	dfa.SetTransition(1, 0, 3);
	dfa.SetTransition(1, 1, 1);

	dfa.SetTransition(2, 0, 4);
	dfa.SetTransition(2, 1, 2);

	dfa.SetTransition(3, 0, 3);
	dfa.SetTransition(3, 1, 3);

	dfa.SetTransition(4, 0, 4);
	dfa.SetTransition(4, 1, 4);

	mini.ShowConfiguration=true;
	TDfa dfa_min = mini.Minimize(dfa);

	// asegura que la cantidad de estados al final es menor
	assert(dfa_min.GetStates() < dfa.GetStates());
	assert(dfa_min.GetStates() == 3);

	cout << endl;

	return 0;
}

int test610()
{

	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	typedef Dfa<TState,TSymbol> TDfa;
	MinimizationHybrid<TDfa> mini;	
	TDfa dfa(2, 10);

	dfa.SetInitial(0);

	dfa.SetFinal(1);
	dfa.SetFinal(2);
	dfa.SetFinal(3);
	dfa.SetFinal(5);
	dfa.SetFinal(6);

	dfa.SetTransition(0,0,1);
	dfa.SetTransition(0,1,2);
	dfa.SetTransition(1,0,3);
	dfa.SetTransition(1,1,4);
	dfa.SetTransition(2,0,5);
	dfa.SetTransition(2,1,4);
	dfa.SetTransition(3,0,3);
	dfa.SetTransition(3,1,1);
	dfa.SetTransition(4,0,6);
	dfa.SetTransition(4,1,2);
	dfa.SetTransition(5,0,7);
	dfa.SetTransition(5,1,2);
	dfa.SetTransition(6,0,3);
	dfa.SetTransition(6,1,8);
	dfa.SetTransition(7,0,7);
	dfa.SetTransition(7,1,7);
	dfa.SetTransition(8,0,6);
	dfa.SetTransition(8,1,9);
	dfa.SetTransition(9,0,7);
	dfa.SetTransition(9,1,9);

	mini.ShowConfiguration=true;
	write_dot(dfa, "test_610.dot");
	auto dfa_min = mini.Minimize(dfa);
	write_dot(dfa_min, "test_610_min.dot");

	return 0;
}

int test602()
{
	cout << "Esta prueba minimiza un automata sencillo usando Incremental y escribe el resultado en un archivo test3.dot" << endl;

	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	typedef Dfa<TState, TSymbol> TDfa;
	MinimizationHybrid<TDfa> mini;	
	// uses zero as invisible null-sink state
	//      2 - 5 - 8  - 11
	//    /
	//   1 -3 - 6 - 9  - 12
	//    \
	//      4 - 7 - 10 - 13
	TDfa dfa(3, 14);

	dfa.SetInitial(1);
	dfa.SetFinal(11);
	dfa.SetFinal(12);
	dfa.SetFinal(13);

	dfa.SetTransition(1, 0, 2);
	dfa.SetTransition(1, 1, 3);
	dfa.SetTransition(1, 2, 4);

	dfa.SetTransition(2, 0, 5);
	dfa.SetTransition(3, 0, 6);
	dfa.SetTransition(4, 0, 7);

	dfa.SetTransition(5, 0, 8);
	dfa.SetTransition(6, 1, 9);
	dfa.SetTransition(7, 2, 10);

	dfa.SetTransition(8, 0, 11);
	dfa.SetTransition(9, 1, 12);
	dfa.SetTransition(10, 2, 13);

	mini.ShowConfiguration=true;
	auto dfa_min = mini.Minimize(dfa);

	write_dot(dfa, "test_302.dot");
	write_dot(dfa_min, "test_302_min.dot");

	// asegura que la cantidad de estados al final es menor
	assert(dfa_min.GetStates() < dfa.GetStates());
	assert(dfa_min.GetStates() == 12);

	cout << endl;

	return 0;
}

int test603()
{
	cout << "Esta prueba minimiza un automata sencillo usando Incremental y escribe el resultado en un archivo test4.dot" << endl;

	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	typedef Dfa<TState, TSymbol> TDfa;
	MinimizationHybrid<TDfa> mini;	
	// uses zero as invisible null-sink state
	//      2 - 5 - 8  - 11
	//    /       \   /
	//   1 -3 - 6 - 9  - 12
	//    \       \   \
	//      4 - 7 - 10 - 13
	TDfa dfa(3, 14);

	dfa.SetInitial(1);
	dfa.SetFinal(11);
	dfa.SetFinal(12);
	dfa.SetFinal(13);

	dfa.SetTransition(1, 0, 2);
	dfa.SetTransition(1, 1, 3);
	dfa.SetTransition(1, 2, 4);

	dfa.SetTransition(2, 0, 5);
	dfa.SetTransition(3, 0, 6);
	dfa.SetTransition(4, 0, 7);

	dfa.SetTransition(5, 0, 8);		
	dfa.SetTransition(5, 1, 9);		
	dfa.SetTransition(5, 2, 9);		

	dfa.SetTransition(6, 1, 9);
	dfa.SetTransition(6, 0, 10);
	dfa.SetTransition(6, 2, 10);

	dfa.SetTransition(7, 0, 10);
	dfa.SetTransition(7, 1, 10);
	dfa.SetTransition(7, 2, 10);

	dfa.SetTransition(8, 0, 11);
	dfa.SetTransition(8, 1, 11);
	dfa.SetTransition(8, 2, 11);

	dfa.SetTransition(9, 0, 11);
	dfa.SetTransition(9, 1, 12);
	dfa.SetTransition(9, 2, 13);

	dfa.SetTransition(10, 0, 13);
	dfa.SetTransition(10, 1, 13);
	dfa.SetTransition(10, 2, 13);

	mini.ShowConfiguration=true;
	auto dfa_min = mini.Minimize(dfa);

	// asegura que la cantidad de estados al final es menor
	assert(dfa_min.GetStates() < dfa.GetStates());
	assert(dfa_min.GetStates() == 6);

	cout << endl;

	return 0;
}

// Test automata generation 400-499

int test400()
{
	cout << "Genera multiples NFA con diferentes numeros de estados y simbolos, los minimiza, los guarda y escribe un reporte." << endl;

	typedef uint16_t TState;
	typedef uint16_t TSymbol;
	typedef Dfa<TState, TSymbol> TDfa;
	typedef Nfa<TState, TSymbol> TNfa;
	typedef MinimizationHopcroft<TDfa> TMinimizer;

	mt19937 rgen(5000);
	ofstream report("report_test_400.txt");
	report << "nfa_nQ,dfa_nQ,min_nQ,alpha,density,filename,t_wall,t_user,t_system" << endl;


	for(int states=10; states<30; states*=2)
	{
		for(int symbols=2; symbols<9; symbols*=2)
		{	
			for(int redundancy=0; redundancy<100; redundancy++)
			{
				for(float density=0.005f; density<0.16f; density *= 2.0f) 
				{
					auto filename1 = format("nfa\\nfa_n%1%_a%2%_d%3%_r%4%") % states % symbols % density % redundancy;
					auto filename = filename1.str();

					cout << "Generando " << filename << "... " << endl;
					NfaGenerator<TNfa, mt19937> nfagen;
					TNfa nfa = nfagen.Generate(states, symbols, 1, 1, density, rgen);

					cout << "Determinizando " << filename << "... ";
					Determinization<TDfa, TNfa> determ;
					auto dfa = determ.Determinize(nfa);
					cout << "Done." << endl;

					TMinimizer mini;
					TMinimizer::NumericPartition part;
					mini.ShowConfiguration = false;

					boost::timer::cpu_timer timer;
					timer.start();
					mini.Minimize(dfa, part);
					timer.stop();

					auto fmt = format("%1%, %2%, %3%, %4%, %5%, %6%, %7%, %8%, %9%") 
						% states					// 1
						% dfa.GetStates()			// 2
						% part.GetSize()			// 3
						% symbols					// 4
						% density					// 5
						% filename					// 6
						% timer.elapsed().wall		// 7
						% timer.elapsed().user		// 8
						% timer.elapsed().system	// 9
						;
					report << fmt.str() << endl;

					auto dfa2 = mini.BuildDfa(dfa, part);

					//write_dot(nfa, filename + ".dot");
					//write_dot(dfa, filename + "_dfa.dot");
					//write_dot(dfa2, filename + "_mini.dot");
					//write_text(nfa, filename + ".txt");
					//write_text(dfa, filename + "_dfa.txt");
					//write_text(dfa2, filename + "_mini.txt");
				}
			}
		}
	}
	return 0;
}

int test401()
{
	using namespace std;
	using namespace boost::timer;
	using namespace boost::program_options;

	typedef uint16_t TState;
	typedef uint16_t TSymbol;
	typedef Dfa<TState, TSymbol> TDfa;
	typedef Nfa<TState, TSymbol> TNfa;

	bool show_help;
	bool enable_all;
	bool hoproft_verbose, incremental_verbose, hybrid_verbose, atomic_verbose;
	bool hopcroft_enable, incremental_enable, hybrid_enable, atomic_enable;
	string output_file;
	int seed, redundancy;

	vector<TState> states_set;
	vector<TSymbol> alphas;
	vector<float> finals_densities;
	float min_density, max_density, step_density;

	options_description opt_desc("Allowed options");
	opt_desc.add_options()
		("help,?", bool_switch(&show_help)->default_value(false), "Show this information")
		("all", bool_switch(&enable_all)->default_value(false), "Enable use all algorithms produces the same results")
		("hopcroft", bool_switch(&hopcroft_enable)->default_value(true), "Enable use Hopcroft algorithm")
		("incremental", bool_switch(&incremental_enable)->default_value(false), "Enable use Incremental algorithm")
		("hybrid", bool_switch(&hybrid_enable)->default_value(false), "Enable use Hybrid algorithm")
		("atomic", bool_switch(&atomic_enable)->default_value(false), "Enable use Atomic algorithm")
		("seed", value(&seed)->default_value(5000), "Seed for MT19937 random number generator")
		("hopcroft-verbose", value(&hoproft_verbose)->default_value(false), "Verbosity for Hopcroft")
		("incremental-verbose", value(&incremental_verbose)->default_value(false), "Verbosity for Incremental algorithm")
		("hybrid-verbose", value(&hybrid_verbose)->default_value(false), "Verbosity for Hybrid algorithm")
		("atomic-verbose", value(&atomic_verbose)->default_value(false), "Verbosity for Atomic algorithm")
		("output,o", value(&output_file)->default_value("report_401.csv"), "Output file")
		("alphas,a", value(&alphas)->multitoken(), "Alphabet to test")
		("states,s", value(&states_set)->multitoken(), "States number to test")
		("finals-density,f", value(&finals_densities)->multitoken(), "Final states density")
		("min-density", value(&min_density)->default_value(0.0f), "Minimum density")
		("max-density", value(&max_density)->default_value(1.0f), "Maximum density")
		("step-density", value(&step_density)->default_value(0.05f), "Density step increment")
		("redundancy,r", value(&redundancy)->default_value(50), "How many tests per configuration scenario")
		;

	variables_map vm;
	command_line_parser parser(global_argc, global_argv);
	auto po = parser.options(opt_desc).run();
	store(po, vm);
	notify(vm);

	if(show_help)
	{
		cout << opt_desc << endl;
		return 0;
	}
	if(enable_all) hopcroft_enable = incremental_enable = hybrid_enable = atomic_enable = true; 
	else if(!hopcroft_enable && !incremental_enable && !hybrid_enable) throw invalid_option_value("None algorithm enabled");

	if(redundancy == 0) throw invalid_option_value("Invalid redundancy");
	if(max_density > 1.0f) throw invalid_option_value("Invalid max density");
	if(min_density < 0.0f) throw invalid_option_value("Invalid min density");	
	if(step_density < 0.0f || step_density>1.0f) throw invalid_option_value("Invalid step density");
	if(max_density < min_density) throw invalid_option_value("max density must be greater than min density");
	if(hoproft_verbose || hybrid_verbose || incremental_verbose) cout << "WARNING: Verbosity affects time measurements" << endl;

	Determinization<TDfa, TNfa> determ;
	NfaGenerator<TNfa, mt19937> nfagen;

	MinimizationHopcroft<TDfa> min_h;
	MinimizationHopcroft<TDfa>::NumericPartition p_h;
	min_h.ShowConfiguration = hoproft_verbose;

	MinimizationHybrid<TDfa> min_hi;
	MinimizationHybrid<TDfa>::NumericPartition p_hi;
	min_hi.ShowConfiguration = hybrid_verbose;

	MinimizationIncremental<TDfa> min_i;
	MinimizationIncremental<TDfa>::NumericPartition p_i;
	min_i.ShowConfiguration = incremental_verbose;

	MinimizationAtomic<TDfa> min_at;
	min_at.ShowConfiguration = atomic_verbose;

	cpu_timer timer;

	mt19937 rgen(seed);
	ofstream report(output_file);
	if(!report.is_open()) throw invalid_argument("No se pudo abrir el archivo");

	report << "states,alpha,d,fd,states_dfa,t_h,c_h,t_i,c_i,t_hi,c_hi" << endl;

	for(TSymbol alpha : alphas)
		for(TState states : states_set)
			for(float d=min_density; d<max_density; d+=step_density)
				for(float finals_density : finals_densities)
					for(int i=0; i<50; i++)
					{
						float den = d;
						cout << "states: "<< states << " alpha: " << alpha << " d:" << d << " i:" << i << " fd:" << finals_density << endl;
						auto nfa = nfagen.Generate_v2(states, alpha, 1, static_cast<TState>(states*finals_density), &den, rgen);
						auto dfa = determ.Determinize(nfa);

						//write_text(dfa, "automata_test.txt");
						//write_dot(dfa, "automata_test.dot");

						nanosecond_type t_h=0, t_i=0, t_hy=0, t_at=0;
						TState c_h=0, c_i=0, c_hy=0, c_at=0;

						if(hopcroft_enable)
						{
							timer.start();
							min_h.Minimize(dfa, p_h);
							timer.stop();
							t_h = timer.elapsed().wall;
							c_h = p_h.GetSize();
						}
						if(incremental_enable)
						{
							timer.start();
							min_i.Minimize(dfa, p_i);
							timer.stop();
							t_i = timer.elapsed().wall;
							c_i = p_i.GetSize();
						}
						if(hybrid_enable)
						{
							timer.start();
							min_hi.Minimize(dfa, p_hi);
							timer.stop();
							t_hy = timer.elapsed().wall;
							c_hy = p_hi.GetSize();
						}
						if(atomic_enable)
						{
							timer.start();
							auto d = min_at.Minimize(dfa);
							timer.stop();
							t_at = timer.elapsed().wall;
							c_at = d.GetStates();
						}
						auto fmt = boost::format("%1%,%2%,%3%,%4%,%5%,%6%,%7%,%8%,%9%,%10%,%11%,%12%,%13%")
							% states 
							% alpha
							% den
							% finals_density
							% dfa.GetStates()
							% t_h
							% c_h
							% t_i
							% c_i
							% t_hy
							% c_hy
							% t_at
							% c_at
							;
						report << fmt.str() << endl;

						if(hopcroft_enable && incremental_enable && (c_h != c_i))  throw invalid_argument("Hopcroft differs of Incremental");
						if(hopcroft_enable && hybrid_enable &&      (c_h != c_hy)) throw invalid_argument("Hopcroft differs of Hybrid");
						if(incremental_enable && hybrid_enable &&   (c_i != c_hy)) throw invalid_argument("Incremental differs of Hybrid");
						if(atomic_enable && hopcroft_enable &&      (c_h != c_at)) throw invalid_argument("Atomic differs of Hopcroft");
						if(atomic_enable && hybrid_enable &&      (c_hy != c_at)) throw invalid_argument("Atomic differs of Hybrid");
					}

					return 0;
}

// Test performance 500-599

int test500()
{
	cout << "Prueba la determinizacion y la minimizacion Hopcroft, Brzozowski e Incremental" << endl;

	typedef uint16_t TState;
	typedef uint16_t TSymbol;
	typedef Dfa<TState, TSymbol> TDfa;
	typedef Nfa<TState, TSymbol> TNfa;
	NfaGenerator<TNfa, mt19937> nfagen;
	boost::timer::cpu_timer timer;	

	long seed = 5000;
	mt19937 rgen(seed);	
	//int states = 80;
	int states = 5;
	int symbols = 2;
	float density = 0.1f;
	TNfa nfa = nfagen.Generate(states, symbols, 1, 1, density, rgen);

	cout << "Generado NFA con " << (size_t)nfa.GetStates() << " estados, " << (size_t)nfa.GetAlphabetLength() << " simbolos, d=" << density << endl;
	write_dot(nfa, "nfa\\t500_nfa_org.dot");	
	write_text(nfa, "nfa\\t500_nfa_org.txt");

	Determinization<TDfa, TNfa> determh;
	auto dfa = determh.Determinize(nfa);
	cout << "Determinizado con " << (size_t)dfa.GetStates() << " estados, " << (size_t)dfa.GetAlphabetLength() << " simbolos" << endl;
	write_dot(dfa, "nfa\\t500_dfa.dot");
	write_text(dfa, "nfa\\t500_dfa_h.txt");

	MinimizationHopcroft<TDfa> minh;
	MinimizationHopcroft<TDfa>::NumericPartition hpartitions;
	minh.ShowConfiguration = false;
	timer.start();
	minh.Minimize(dfa, hpartitions);	
	timer.stop();
	cout << "Minimizado Hopcroft con " << hpartitions.GetSize() << " estados, " << (size_t)dfa.GetAlphabetLength() << " simbolos" << endl;
	cout << "Minimizacion tomo " << timer.format(5) << endl;
	//write_dot(dfa_minh, "nfa\\t500_dfa_min_h.dot");
	//write_text(dfa_minh, "nfa\\t500_dfa_min_h.txt");

	typedef Fsa<TState,TSymbol> TFsa;
	Determinization<TNfa, TNfa> determb;
	auto nfab = determb.Determinize(nfa);
	cout << "Determinizado con " << (size_t)nfab.GetStates() << " estados, " << (size_t)nfab.GetAlphabetLength() << " simbolos" << endl;
	write_dot(nfab, "nfa\\t500_dfa.dot");	
	write_text(nfab, "nfa\\t500_dfa_b.txt");

	MinimizationBrzozowski<TNfa> minb;	
	MinimizationBrzozowski<TNfa>::TVectorDfaEdge edges;
	MinimizationBrzozowski<TNfa>::TVectorDfaState fstates;
	MinimizationBrzozowski<TNfa>::TState nstates;
	timer.start();
	minb.Minimize(nfab, &nstates, fstates, edges);
	timer.stop();
	auto nfa_minb = minb.BuildDfa(nfab.GetAlphabetLength(), nstates, fstates, edges);

	cout << "Minimizado Brzozowski con " << (size_t)nfa_minb.GetStates() << " estados, " << (size_t)nfa_minb.GetAlphabetLength() << " simbolos" << endl;
	cout << "Minimizacion tomo " << timer.format(5) << endl;
	write_dot(nfa_minb, "nfa\\t500_dfa_min_b.dot");
	write_text(nfa_minb, "nfa\\t500_dfa_min_b.txt");

	MinimizationIncremental<TDfa> mini;
	MinimizationIncremental<TDfa>::NumericPartition part;
	mini.ShowConfiguration = false;
	timer.start();
	mini.Minimize(dfa, part);
	timer.stop();
	auto dfa_mini = mini.BuildDfa(dfa, part);

	cout << "Minimizado Incremental con " << (size_t)part.GetSize() << " estados, " << (size_t)dfa.GetAlphabetLength() << " simbolos" << endl;
	cout << "Minimizacion tomo " << timer.format(5) << endl;
	write_dot(dfa_mini, "nfa\\t500_dfa_min_i.dot");
	write_text(dfa_mini, "nfa\\t500_dfa_min_i.txt");

	MinimizationHybrid<TDfa> minhi;
	MinimizationHybrid<TDfa>::NumericPartition parthi;
	minhi.ShowConfiguration = false;
	timer.start();
	minhi.Minimize(dfa, parthi);
	timer.stop();
	auto dfa_minhi = mini.BuildDfa(dfa, part);

	cout << "Minimizado Hibrido con " << (size_t)part.GetSize() << " estados, " << (size_t)dfa.GetAlphabetLength() << " simbolos" << endl;
	cout << "Minimizacion tomo " << timer.format(5) << endl;
	write_dot(dfa_minhi, "nfa\\t500_dfa_min_hi.dot");
	write_text(dfa_minhi, "nfa\\t500_dfa_min_hi.txt");

	return 0;
}

int test503()
{
	using namespace boost::filesystem;
	using namespace boost::timer;
	using namespace boost::program_options;
	using boost::format;

	cpu_timer timer;
	string root_path_string;

	bool show_help, almeida_format;
	string output_filename;
	vector<MinimizationAlgorithm> algorithms;

	options_description opt_desc("Allowed options");
	opt_desc.add_options()
		("help,?", bool_switch(&show_help)->default_value(false), "Show this information")
		("path,p", value(&root_path_string), "Root path to search")
		("almeida_format", bool_switch(&almeida_format)->default_value(false), "Use Almeida format")
		("output,o", value(&output_filename), "Output filename")
		("algorithm,a", value(&algorithms)->multitoken(), "Algorithms to test")
		;

	variables_map vm;
	command_line_parser parser(global_argc, global_argv);
	auto po = parser.options(opt_desc).run();
	store(po, vm);
	notify(vm);

	if(show_help)
	{
		cout << opt_desc << endl;
		return 0;
	}

	typedef uint16_t TState;
	typedef uint16_t TSymbol;
	typedef Dfa<TState, TSymbol> TDfa;
	typedef Nfa<TState, TSymbol> TNfa;

	ofstream report(output_filename);
	if(!report.is_open()) throw invalid_argument("No se pudo abrir el reporte");

	path root_path(root_path_string);

	report << "alg,n,k,t,file,min_st" << endl;

	vector<TState> vfinal;
	vector<TNfa::TEdge> vedges;

	map<MinimizationAlgorithm, size_t> statesCount;
	map<MinimizationAlgorithm, nanosecond_type> acumTime;

	vector<path> v;
	copy(directory_iterator(root_path), directory_iterator(), back_inserter(v));

	sort(v.begin(), v.end(), [](const path& a, const path& b){ 
		const string aa=a.string(), bb=b.string(); 
		return aa.size() < bb.size();
	});
	for(auto i=v.begin(); i!=v.end(); i++)
	{		
		size_t automata_count = 0;

		auto fnc = [&](const TDfa& dfa, const string& dfa_filename) 
		{
			MinimizationAlgorithm algo;
			TState n = dfa.GetStates();
			TSymbol k = dfa.GetAlphabetLength();
			statesCount.clear();

			if(dfa.GetFinals().IsEmpty()) 
			{
				cout << "Skip because ATOMIC can't handle DFA without finals" << endl;
				return;
			}

			// BRZOZOWSKI
			algo = MinimizationAlgorithm::Brzozowski;
			if(find(algorithms.begin(), algorithms.end(), algo) != algorithms.end())
			{
				throw invalid_argument("brozozowski not supported currently");
				/*
				//deshabilitado porque en ocasiones tarda demasiado
				TState min_states;
				timer.start();
				min1.Minimize(nfa, &min_states, vfinal, vedges);
				timer.stop();
				//cout << "Brzozowski " << dfa_filename << ": " << timer.elapsed().wall << endl;
				report << (boost::format("%1%,%2%,%3%,%4%,%5%,%6%") 
				% "Brzozowski"
				% n
				% k
				% timer.elapsed().wall
				% dfa_filename
				% static_cast<size_t>(part_b.GetSize())
				).str() << endl;
				if(acumTime.find(algo)==acumTime.end()) acumTime[algo] = 0;
				acumTime[algo] += timer.elapsed().wall;
				statesCount[algo] = min_states;
				//*/
			}

			// HOPCROFT
			algo = MinimizationAlgorithm::Hopcroft;
			if(find(algorithms.begin(), algorithms.end(), algo) != algorithms.end())
			{	
				MinimizationHopcroft<TDfa> min2;
				MinimizationHopcroft<TDfa>::NumericPartition part_h;
				min2.ShowConfiguration = false;

				timer.start();
				min2.Minimize(dfa, part_h);
				timer.stop();
				//cout << "Hopcroft " << dfa_filename << ": " << timer.elapsed().wall << endl;
				report << (boost::format("%1%,%2%,%3%,%4%,%5%,%6%") 
					% "Hopcroft"
					% n
					% k
					% timer.elapsed().wall
					% dfa_filename
					% static_cast<size_t>(part_h.GetSize())
					).str()	<< endl;
				if(acumTime.find(algo)==acumTime.end()) acumTime[algo] = 0;
				acumTime[algo] += timer.elapsed().wall;
				statesCount[algo] = part_h.GetSize();
			}

			// INCREMENTAL
			algo = MinimizationAlgorithm::Incremental;
			if(find(algorithms.begin(), algorithms.end(), algo) != algorithms.end())	
			{
				MinimizationIncremental<TDfa> min3;
				MinimizationIncremental<TDfa>::NumericPartition part_i;
				min3.ShowConfiguration = false;

				timer.start();
				min3.Minimize(dfa, part_i);
				timer.stop();
				//cout << "Incremental " << dfa_filename << ": " << timer.elapsed().wall << endl;
				report << (boost::format("%1%,%2%,%3%,%4%,%5%,%6%") 
					% "Incremental"
					% n
					% k
					% timer.elapsed().wall
					% dfa_filename
					% static_cast<size_t>(part_i.GetSize())
					).str() << endl;
				if(acumTime.find(algo)==acumTime.end()) acumTime[algo] = 0;
				acumTime[algo] += timer.elapsed().wall;
				statesCount[algo] = part_i.GetSize();
			}

			// HYBRID
			algo = MinimizationAlgorithm::Hybrid;
			if(find(algorithms.begin(), algorithms.end(), algo) != algorithms.end())
			{
				MinimizationHybrid<TDfa> min4;
				MinimizationHybrid<TDfa>::NumericPartition part_hi;
				min4.ShowConfiguration = false;	

				timer.start();
				min4.Minimize(dfa, part_hi);
				timer.stop();
				//cout << "Hybrid " << dfa_filename << ": " << timer.elapsed().wall << endl;
				report << (boost::format("%1%,%2%,%3%,%4%,%5%,%6%") 
					% "Hybrid"
					% n
					% k
					% timer.elapsed().wall
					% dfa_filename
					% static_cast<size_t>(part_hi.GetSize())
					).str() << endl;
				if(acumTime.find(algo)==acumTime.end()) acumTime[algo] = 0;
				acumTime[algo] += timer.elapsed().wall;
				statesCount[algo] = part_hi.GetSize();
			}

			// ATOMIC
			algo = MinimizationAlgorithm::Atomic;			
			if(find(algorithms.begin(), algorithms.end(), algo) != algorithms.end())
			{
				MinimizationAtomic<TDfa> min5;
				min5.ShowConfiguration = false;

				timer.start();
				auto mdfa = min5.Minimize(dfa);
				timer.stop();
				if(acumTime.find(algo)==acumTime.end()) acumTime[algo] = 0;
				acumTime[algo] += timer.elapsed().wall;
				statesCount[algo] = mdfa.GetStates();
			}

			bool fail = false;
			for(auto j=statesCount.begin(); j!=statesCount.end(); j++)
			{
				for(auto k=statesCount.begin(); k!=statesCount.end(); k++)
				{
					if(j->first == k->first) continue;
					if(j->second != k->second)
					{
						auto msg = format("ERROR: No coincide %1% (%2%) con %3% (%4%)") 
							% j->first
							% j->second
							% k->first
							% k->second;
						cout << msg.str() << endl;
						fail = true;
					}
				}
			}
			if(fail) throw logic_error("No conicide el numero de estados en los automatas minimos");

			automata_count++;					
		};

		if(almeida_format) 
		{
			if(!is_regular_file(*i)) continue;
			auto dfa_filename = i->string();
			cout << "Reading " << dfa_filename << endl;

			ifstream fsa_file(dfa_filename);
			string line;
			TState n; TSymbol k;
			begin_read_text_almeida<TDfa>(fsa_file, dfa_filename, &n, &k);
			while(!fsa_file.eof())
			{
				getline(fsa_file, line);
				if(line.empty() || line.front() == '(') continue;
				stringstream line_stream(line);
				auto dfa = read_text_almeida<TDfa>(line_stream, n, k);				
				fnc(dfa, dfa_filename);				
			}			
		} 
		else 
		{
			if(!is_directory(*i)) continue;
			cout << "Entering " << *i << endl;			
			for(auto j=directory_iterator(*i); j!=directory_iterator(); j++)
			{				
				if(!is_regular_file(*j)) continue;
				const auto dfa_filename = j->path().string();
				auto nfa = read_text_one_based<TNfa>(dfa_filename);
				auto dfa = read_text_one_based<TDfa>(dfa_filename);
				fnc(dfa, dfa_filename);
			}
		}	
		// skip empty folders
		if(automata_count == 0) continue;
		for(auto j=acumTime.begin(); j!=acumTime.end(); j++)
		{
			auto avg = j->second / automata_count;
			cout << j->first << ": " << avg << endl;
			cout << (boost::format("file %1% | time avg = %2%") % i->string() % avg).str() << endl;
		}
	}

	return 0;
}

int test502()
{
	using namespace boost::filesystem;
	using namespace boost::timer;

	cpu_timer timer;
	path root_path("dfa_501");

	typedef uint16_t TState;
	typedef uint16_t TSymbol;
	typedef Dfa<TState, TSymbol> TDfa;
	typedef Nfa<TState, TSymbol> TNfa;

	MinimizationHopcroft<TDfa> min_h;
	min_h.ShowConfiguration = true;
	MinimizationHopcroft<TDfa>::NumericPartition part_h;

	MinimizationIncremental<TDfa> min_i;
	min_i.ShowConfiguration = true;
	MinimizationIncremental<TDfa>::NumericPartition part_i;

	MinimizationHybrid<TDfa> min_hi;
	min_hi.ShowConfiguration = true;
	MinimizationHybrid<TDfa>::NumericPartition part_hi;

	MinimizationAtomic<TDfa> min_at;
	min_at.ShowConfiguration = true;

	ofstream report("report_501.csv");
	if(!report.is_open()) throw invalid_argument("No se pudo abrir el reporte");

	report << "alg,n,k,t,file" << endl;

	//string dfa_filename = "dfa_501\\n50k5\\0002.dfa";
	string dfa_filename = "automata_test.txt";
	auto dfa = read_text<TDfa>(dfa_filename);
	//auto dfa = read_text_one_based<TDfa>(dfa_filename);

	//cout << "Read " << dfa_filename << endl;

	size_t n = dfa.GetStates();
	size_t k = dfa.GetAlphabetLength();

	write_dot(dfa, "auto.dot");
	write_dfa_info(dfa);

	string r;

	timer.start();
	min_h.Minimize(dfa, part_h);
	timer.stop();
	cout << "Hopcroft: " << timer.elapsed().wall << endl;;
	ofstream report_hopcroft("report_hopcroft.txt");
	r = min_h.to_string(part_h);
	report_hopcroft << r;
	report_hopcroft.close();

	timer.start();
	min_i.Minimize(dfa, part_i);
	timer.stop();
	cout << "Incremental: " << timer.elapsed().wall << endl;
	ofstream report_incremental("report_incremental.txt");
	r = min_i.to_string(part_i);
	report_incremental << r;
	report_incremental.close();

	timer.start();
	min_hi.Minimize(dfa, part_hi);
	timer.stop();
	cout << "Hybrid: " << timer.elapsed().wall << endl;;
	ofstream report_hybrid("report_hybrid.txt");
	r = min_hi.to_string(part_hi);
	report_hybrid << r;
	report_hybrid.close();

	timer.start();
	auto dfa_at = min_at.Minimize(dfa);
	timer.stop();
	cout << "Atomic: " << timer.elapsed().wall << endl;;
	ofstream report_atomic("report_atomic.txt");
	report_atomic << static_cast<size_t>(dfa_at.GetStates()) << endl;
	report_atomic.close();

	return 0;
}

int test504()
{
	using namespace boost::filesystem;
	using namespace boost::timer;

	cpu_timer timer;	

	typedef uint16_t TState;
	typedef uint16_t TSymbol;
	typedef Dfa<TState, TSymbol> TDfa;
	typedef Nfa<TState, TSymbol> TNfa;
	
	ofstream report("report_504.csv");
	if (!report.is_open()) throw invalid_argument("No se pudo abrir el reporte");

	report << "alg,n,k,t,file" << endl;
	
	for (int i = 100; i <= 10000; i+=100)
	{
		MinimizationHopcroft<TDfa> min_h;
		min_h.ShowConfiguration = false;
		MinimizationHopcroft<TDfa>::NumericPartition part_h;

		MinimizationHybrid<TDfa> min_hi;
		min_hi.ShowConfiguration = false;
		MinimizationHybrid<TDfa>::NumericPartition part_hi;

		string dfa_filename = string("experimento_03-2013\\k10\\") + to_string(i) + ".afd";
		auto dfa = read_text_one_based<TDfa>(dfa_filename);
		cout << "Read " << dfa_filename << endl;

		TState n = dfa.GetStates();
		TSymbol k = dfa.GetAlphabetLength();
	
		timer.start();
		min_h.Minimize(dfa, part_h);
		timer.stop();
		cout << "Hopcroft: " << timer.elapsed().wall << " " << part_h.GetSize() << endl;;				

		report << "hopcroft," << n << "," << k << "," << timer.elapsed().wall << "," << dfa_filename << endl;

		timer.start();
		min_hi.Minimize(dfa, part_hi);
		timer.stop();
		cout << "Hybrid: " << timer.elapsed().wall << " " << part_hi.GetSize() << endl;;

		report << "hybrid," << n << "," << k << "," << timer.elapsed().wall << "," << dfa_filename << endl;
	}

	report.close();

	return 0;
}

// Test Set 50-60

int test50()
{
	cout << "Prueba de Set" << endl;
	Set<int> s(3);
	s.Add(0);
	s.Add(3);
	assert(s.Contains(0));
	assert(!s.Contains(1));
	assert(!s.Contains(2));
	assert(s.Contains(3));

	auto r = s.GetIterator();
	assert(!r.IsEnd());
	assert(r.GetCurrent() == 0);
	r.MoveNext();

	assert(!r.IsEnd());
	assert(r.GetCurrent() == 3);
	r.MoveNext();

	assert(r.IsEnd());

	cout << "Prueba de BitSet" << endl;
	BitSet<int> s2(10);
	s2.Add(0);
	s2.Add(3);
	assert(s2.Contains(0));
	assert(!s2.Contains(1));
	assert(!s2.Contains(2));
	assert(s2.Contains(3));

	auto r2 = s2.GetIterator();
	assert(!r2.IsEnd());
	assert(r2.GetCurrent() == 0);
	r2.MoveNext();

	assert(!r2.IsEnd());
	assert(r2.GetCurrent() == 3);
	r2.MoveNext();

	assert(r2.IsEnd());

	return 0;
}


int main(int argc, char** argv)
{	
	if(argc < 2)
	{
		cout << "Specify the test number" << endl;
		return -1;
	}
	int i = stoi(argv[1]);
	global_argc = argc - 1;
	global_argv = &argv[1];
	global_argv[0] = argv[0];
	//try
	{
#define MACRO_TEST(N) case N: return test##N(); break
		switch(i)
		{
			MACRO_TEST(50);

			MACRO_TEST(100);
			MACRO_TEST(101);
			MACRO_TEST(102);
			MACRO_TEST(103);
			MACRO_TEST(104);

			MACRO_TEST(200);
			MACRO_TEST(201);
			MACRO_TEST(202);
			MACRO_TEST(203);

			MACRO_TEST(300);
			MACRO_TEST(301);
			MACRO_TEST(302);
			MACRO_TEST(303);
			MACRO_TEST(310);

			MACRO_TEST(600);
			MACRO_TEST(601);
			MACRO_TEST(602);
			MACRO_TEST(603);
			MACRO_TEST(610);

			MACRO_TEST(400);
			MACRO_TEST(401);

			MACRO_TEST(500);
			MACRO_TEST(502);
			MACRO_TEST(503);
			MACRO_TEST(504);
		default:
			cout << "La prueba indicada no existe" << endl;
			throw invalid_argument("La prueba no existe");
			return -1;
		}
#undef MACRO_TEST
	}
	/*
	catch(exception ex)
	//{
	cout << "Error: " << endl;
	cout << ex.what() << endl;
	return -1;
	}//*/

	return 0;
}
