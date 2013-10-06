// June 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#include "../MinimizationHopcroft.h"
#include "../MinimizationBrzozowski.h"
#include "../MinimizationIncremental.h"
#include "../MinimizationHybrid.h"
#include "../Dfa.h"
#include "../Nfa.h"
#include "../Fsa.h"
#include "../Set.h"
#include "../FsmGraphVizWriter.h"
#include "../FsaFormatReader.h"
#include "../FsmPlainTextReader.h"
#include "../FsmPlainTextWriter.h"
#include "../Determinization.h"
#include "../NfaGenerator.h"
#include <fstream>
#include <boost/timer/timer.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <array>

using namespace std;
using boost::format;

// Utilities

template<typename TFsa>
void write_dot(TFsa fsa, string filename)
{
	FsmGraphVizWriter<TFsa> writer;
	ofstream s(filename);
	if(!s.is_open()) 
	{
		throw exception("El archivo no pudo ser abierto");
	}
	writer.Write(fsa, s);
	s.close();
}

template<typename TFsa>
void write_text(TFsa fsa, string filename)
{
	FsmPlainTextWriter<TFsa> writer;
	ofstream s(filename);
	if(!s.is_open()) 
	{
		throw exception("El archivo no pudo ser abierto");
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
TFsa read_text(string filename)
{
	FsmPlainTextReader<TFsa> reader;
	ifstream fsa_input(filename);
	if(!fsa_input.is_open()) throw exception("El archivo no pudo ser abierto");
	auto fsa = reader.Read(fsa_input);
	fsa_input.close();
	return fsa;
}

template<typename TFsa>
TFsa read_text_one_based(string filename)
{
	FsmPlainTextReaderOneBased<TFsa> reader;
	ifstream fsa_input(filename);
	if(!fsa_input.is_open()) throw exception("El archivo no pudo ser abierto");
	auto fsa = reader.Read(fsa_input);
	fsa_input.close();
	return fsa;
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
		mini.Minimize(dfa, MinimizationHopcroft<TDfa>::NumericPartition());
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
	typedef uint8_t TSymbol;
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

	typedef uint16_t TState;
	typedef uint16_t TSymbol;
	typedef Dfa<TState, TSymbol> TDfa;
	typedef Nfa<TState, TSymbol> TNfa;

	Determinization<TDfa, TNfa> determ;
	NfaGenerator<TNfa, mt19937> nfagen;

	MinimizationHopcroft<TDfa> min_h;
	MinimizationHopcroft<TDfa>::NumericPartition p_h;
	min_h.ShowConfiguration = false;

	MinimizationHybrid<TDfa> min_hi;
	MinimizationHybrid<TDfa>::NumericPartition p_hi;
	min_hi.ShowConfiguration = false;

	MinimizationIncremental<TDfa> min_i;
	MinimizationIncremental<TDfa>::NumericPartition p_i;
	min_i.ShowConfiguration = false;

	cpu_timer timer;

	mt19937 rgen(5000);
	ofstream report("report_401_mod.csv");
	if(!report.is_open()) throw exception("No se pudo abrir el archivo");

	report << "states,alpha,d,states_dfa,t_h,c_h,t_i,c_i,t_hi,c_hi" << endl;

	const array<TSymbol,5> alphas = { 2, 10, 25, 50, 500 };
	const array<TState,2> states_set = { 10, 11 };
	for(TSymbol alpha : alphas)
	for(TState states : states_set)
	for(float d=0.00f; d<1.0f; d+=0.05f)
	for(int i=0; i<20; i++)
	{
		float den = d;
		cout << "states: "<< states << " alpha: " << alpha << " d:" << d << " i:" << i << endl;
		auto nfa = nfagen.Generate_v2(states, alpha, 1, 1, &den, rgen);
		auto dfa = determ.Determinize(nfa);

		//write_text(dfa, "automata_test.txt");
		//write_dot(dfa, "automata_test.dot");

		timer.start();
		min_h.Minimize(dfa, p_h);
		timer.stop();
		auto t_h = timer.elapsed().wall;
		auto c_h = p_h.GetSize();
/*
		timer.start();
		min_i.Minimize(dfa, p_i);
		timer.stop();
		auto t_i = timer.elapsed().wall;
		auto c_i = p_i.GetSize();

		timer.start();
		min_hi.Minimize(dfa, p_hi);
		timer.stop();
		auto t_hi = timer.elapsed().wall;
		auto c_hi = p_hi.GetSize();*/

		auto fmt = boost::format("%1%,%2%,%3%,%4%,%5%,%6%,%7%,%8%,%9%,%10%")
			% states 
			% alpha
			% den
			% dfa.GetStates()
			% t_h
			% c_h
			% 0//t_i
			% 0//c_i
			% 0//t_hi
			% 0//c_hi
			;
		report << fmt.str() << endl;
/*
		if(!(c_h == c_i)) throw exception("");
		if(!(c_h == c_hi)) throw exception("");
		if(!(c_i == c_hi)) throw exception("");*/

	}

	return 0;
}

// Test performance 500-599

int test500()
{
	cout << "Prueba la determinizacion y la minimizacion Hopcroft, Brzozowski e Incremental" << endl;

	typedef uint16_t TState;
	typedef uint8_t TSymbol;
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

int test501()
{
	using namespace boost::filesystem;
	using namespace boost::timer;

	cpu_timer timer;
	path root_path("CorpusAlmeida");

	typedef uint16_t TState;
	typedef uint16_t TSymbol;
	typedef Dfa<TState, TSymbol> TDfa;
	typedef Nfa<TState, TSymbol> TNfa;

	MinimizationHopcroft<TDfa> min2;
	MinimizationIncremental<TDfa> min3;
	MinimizationHybrid<TDfa> min4;

	min2.ShowConfiguration = false;
	min3.ShowConfiguration = false;
	min4.ShowConfiguration = false;

	ofstream report("report_501_almeida.csv");
	if(!report.is_open()) throw exception("No se pudo abrir el reporte");

	report << "alg,n,k,t,file,min_st" << endl;

	vector<TState> vfinal;
	vector<TNfa::TEdge> vedges;
	MinimizationIncremental<TDfa>::NumericPartition part_i;
	MinimizationHybrid<TDfa>::NumericPartition part_hi;
	MinimizationHopcroft<TDfa>::NumericPartition part_h;

	try 
	{
		for(auto root_path_iter=directory_iterator(root_path); root_path_iter != directory_iterator(); root_path_iter++)
		{
			uint64_t acum_time_b = 0;
			uint64_t acum_time_h = 0;
			uint64_t acum_time_i = 0;
			uint64_t acum_time_hi = 0;
			uint64_t automata_count = 0;
			if(!is_directory(root_path_iter->status())) continue;
			cout << "Entering " << root_path_iter->path() << endl;
			for(auto i=directory_iterator(root_path_iter->path()); i!=directory_iterator(); i++)
			{
				if(!is_regular_file(i->status())) continue;

				auto dfa_filename = i->path().string();
				auto nfa = read_text_one_based<TNfa>(dfa_filename);
				auto dfa = read_text_one_based<TDfa>(dfa_filename);

				//cout << "Read " << dfa_filename << endl;

				size_t n = nfa.GetStates();
				size_t k = nfa.GetAlphabetLength();
				/*
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
				acum_time_b += timer.elapsed().wall;
				*/
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
				acum_time_h += timer.elapsed().wall;

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
				acum_time_i += timer.elapsed().wall;

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
				acum_time_hi += timer.elapsed().wall;

				if(part_h.GetSize() != part_i.GetSize()) throw exception();
				if(part_h.GetSize() != part_hi.GetSize()) throw exception();
				if(part_i.GetSize() != part_hi.GetSize()) throw exception();

				automata_count++;
			}
			// skip empty folders
			if(automata_count == 0) continue;
			acum_time_b /= automata_count;
			acum_time_h /= automata_count;
			acum_time_i /= automata_count;
			acum_time_hi /= automata_count;
			cout << (boost::format("folder: %1% | Promedios: B:%2% H:%3% I:%4% HI:%5%")
				% root_path_iter->path().string()
				% acum_time_b
				% acum_time_h
				% acum_time_i
				% acum_time_hi
				).str() << endl;
		}
	}
	catch(const filesystem_error& ex)
	{
		cout << "ERROR" << endl;
		cout << ex.what() << endl;
		return -1;
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

	ofstream report("report_501.csv");
	if(!report.is_open()) throw exception("No se pudo abrir el reporte");

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

	return 0;
}

// Test Set 50-60

void test50()
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
}


int main(int argc, char** argv)
{	
	int i = stoi(argv[1]);

#define MACRO_TEST(N) case N: test##N(); break
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
		MACRO_TEST(501);
		MACRO_TEST(502);
	default:
		cout << "La prueba indicada no existe" << endl;
		throw exception("La prueba no existe");
		return -1;
	};
#undef MACRO_TEST

	return 0;
}
