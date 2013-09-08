// June 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#include "../MinimizationHopcroft.h"
#include "../MinimizationBrzozowski.h"
#include "../MinimizationIncremental.h"
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

using namespace std;
using boost::format;

int test1()
{	
	cout << "Esta prueba minimiza un automata sencillo usando Hopcroft y escribe el resultado en un archivo test1.dot" << endl;

	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	typedef Dfa<TState, TSymbol> TDfa;
	MinimizationHopcroft<TDfa> mini;	
	FsmGraphVizWriter<TDfa> exporter;	
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
	
	MinimizationHopcroft<TDfa>::TPartitionVector out_partitions;
	mini.Minimize(dfa, out_partitions);

	// asegura que la cantidad de estados al final es menor
	assert(out_partitions.size() < dfa.GetStates());
	assert(out_partitions.size() == 3);

	exporter.Write(dfa, ofstream("test1.dot"));
	cout << endl;
	
	return 0;
}

int test2()
{
	cout << "Esta prueba minimiza un automata sencillo usando Hopcroft y escribe el resultado en un archivo test2.dot" << endl;

	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	typedef Dfa<TState,TSymbol> TDfa;
	MinimizationHopcroft<TDfa> mini;	
	FsmGraphVizWriter<TDfa> exporter;
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

	MinimizationHopcroft<TDfa>::TPartitionVector out_partitions;
	mini.Minimize(dfa, out_partitions);
	
	// asegura que la cantidad de estados al final es menor
	assert(out_partitions.size() < dfa.GetStates());

	exporter.Write(dfa, ofstream("test2.dot"));
	cout << endl;

	return 0;
}

int test3()
{
	cout << "Esta prueba minimiza un automata sencillo usando Hopcroft y escribe el resultado en un archivo test3.dot" << endl;

	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	typedef Dfa<TState, TSymbol> TDfa;
	MinimizationHopcroft<TDfa> mini;	
	FsmGraphVizWriter<TDfa> exporter;	
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

	MinimizationHopcroft<TDfa>::TPartitionVector out_partitions;
	mini.Minimize(dfa, out_partitions);

	// asegura que la cantidad de estados al final es menor
	assert(out_partitions.size() < dfa.GetStates());
	assert(out_partitions.size() == 12);

	exporter.Write(dfa, ofstream("test3.dot"), false);		
	cout << endl;

	return 0;
}

int test4()
{
	cout << "Esta prueba minimiza un automata sencillo usando Hopcroft y escribe el resultado en un archivo test4.dot" << endl;

	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	typedef Dfa<TState, TSymbol> TDfa;
	MinimizationHopcroft<TDfa> mini;	
	FsmGraphVizWriter<TDfa> exporter;	
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

	MinimizationHopcroft<TDfa>::TPartitionVector out_partitions;
	mini.Minimize(dfa, out_partitions);

	// asegura que la cantidad de estados al final es menor
	assert(out_partitions.size() < dfa.GetStates());
	assert(out_partitions.size() == 6);

	exporter.Write(dfa, ofstream("test4.dot"), false);		
	cout << endl;

	return 0;
}

int test5()
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
		mini.Minimize(dfa);
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


int test6()
{	
	cout << "Esta prueba minimiza un automata sencillo usando Brzozowski y escribe el resultado en un archivo test6.dot" << endl;

	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	typedef Nfa<TState, TSymbol> TNfa;
	MinimizationBrzozowski<TNfa> mini;	
	FsmGraphVizWriter<TNfa> exporter;	
	TNfa dfa(2, 4);

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

	exporter.Write(dfa, ofstream("test6.dot"));
	exporter.Write(dfa_min, ofstream("test6_min_brzozowski.dot"));
	cout << endl;
	
	return 0;
}

int test7()
{
	cout << "Esta prueba minimiza un automata sencillo usando Hopcroft y escribe el resultado en un archivo test3.dot" << endl;

	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	typedef Nfa<TState, TSymbol> TDfa;
	MinimizationBrzozowski<TDfa> mini;	
	FsmGraphVizWriter<TDfa> exporter;	
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

	auto dfa_min = mini.Minimize(dfa);

	// asegura que la cantidad de estados al final es menor
	assert(dfa_min.GetStates() < dfa.GetStates());
	assert(dfa_min.GetStates() == 12);

	exporter.Write(dfa, ofstream("test7.dot"), false);
	exporter.Write(dfa_min, ofstream("test7_min_brzozowski.dot"), false);
	cout << endl;

	return 0;
}

int test7b()
{
	cout << "Esta prueba minimiza un automata sencillo usando Hopcroft y escribe el resultado en un archivo test4.dot" << endl;

	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	typedef Nfa<TState, TSymbol> TDfa;
	MinimizationBrzozowski<TDfa> mini;	
	FsmGraphVizWriter<TDfa> exporter;	
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

	
	auto dfa_min = mini.Minimize(dfa);

	// asegura que la cantidad de estados al final es menor
	assert(dfa_min.GetStates() < dfa.GetStates());
	assert(dfa_min.GetStates() == 6);

	exporter.Write(dfa, ofstream("test7b.dot"), false);		
	exporter.Write(dfa_min, ofstream("test7b_min_brzozowski.dot"), false);		
	cout << endl;

	return 0;
}

int test8()
{
	cout << "Genera multiples NFA con diferentes numeros de estados y simbolos, los minimiza, los guarda y escribe un reporte." << endl;

	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	typedef Dfa<TState, TSymbol> TDfa;
	typedef Nfa<TState, TSymbol> TNfa;
	typedef MinimizationHopcroft<TDfa> TMinimizer;

	mt19937 rgen(5000);
	ofstream report("report_test8.txt");
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
					TMinimizer::TPartitionVector part;
					TMinimizer::TStateToPartition conv_table;
					mini.ShowConfiguration = false;

					boost::timer::cpu_timer timer;
					timer.start();
					mini.Minimize(dfa, part, conv_table);
					timer.stop();
			
					auto fmt = format("%1%, %2%, %3%, %4%, %5%, %6%, %7%, %8%, %9%") 
						% states					// 1
						% dfa.GetStates()			// 2
						% part.size()				// 3
						% symbols					// 4
						% density					// 5
						% filename					// 6
						% timer.elapsed().wall		// 7
						% timer.elapsed().user		// 8
						% timer.elapsed().system	// 9
						;
					report << fmt.str() << endl;

					auto dfa2 = mini.Synthetize(dfa, part, conv_table);

					//write_dot(nfa, filename + ".dot");
					//write_dot(dfa, filename + "_dfa.dot");
					//write_dot(dfa2, filename + "_mini.dot");
					write_text(nfa, filename + ".txt");
					write_text(dfa, filename + "_dfa.txt");
					write_text(dfa2, filename + "_mini.txt");
				}
			}
		}
	}
	return 0;
}

int test9()
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
	write_dot(nfa, "nfa\\t9_nfa_org.dot");	
	write_text(nfa, "nfa\\t9_nfa_org.txt");

	Determinization<TDfa, TNfa> determh;
	auto dfa = determh.Determinize(nfa);
	cout << "Determinizado con " << (size_t)dfa.GetStates() << " estados, " << (size_t)dfa.GetAlphabetLength() << " simbolos" << endl;
	write_dot(dfa, "nfa\\t9_dfa.dot");
	write_text(dfa, "nfa\\t9_dfa_h.txt");

	MinimizationHopcroft<TDfa> minh;
	MinimizationHopcroft<TDfa>::TPartitionVector hpartitions;
	minh.ShowConfiguration = false;
	timer.start();
	minh.Minimize(dfa, hpartitions);	
	timer.stop();
	cout << "Minimizado Hopcroft con " << hpartitions.size() << " estados, " << (size_t)dfa.GetAlphabetLength() << " simbolos" << endl;
	cout << "Minimizacion tomo " << timer.format(5) << endl;
	//write_dot(dfa_minh, "nfa\\t9_dfa_min_h.dot");
	//write_text(dfa_minh, "nfa\\t9_dfa_min_h.txt");

	typedef Fsa<TState,TSymbol> TFsa;
	Determinization<TNfa, TNfa> determb;
	auto nfab = determb.Determinize(nfa);
	cout << "Determinizado con " << (size_t)nfab.GetStates() << " estados, " << (size_t)nfab.GetAlphabetLength() << " simbolos" << endl;
	write_dot(nfab, "nfa\\t9_dfa.dot");	
	write_text(nfab, "nfa\\t9_dfa_b.txt");
	
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
	write_dot(nfa_minb, "nfa\\t9_dfa_min_b.dot");
	write_text(nfa_minb, "nfa\\t9_dfa_min_b.txt");

	MinimizationIncremental<TDfa> mini;
	MinimizationIncremental<TDfa>::NumericPartition part;
	mini.ShowConfiguration = false;
	timer.start();
	mini.Minimize(dfa, part);
	timer.stop();
	auto dfa_mini = mini.BuildDfa(dfa, part);

	cout << "Minimizado Incremental con " << (size_t)part.GetSize() << " estados, " << (size_t)dfa.GetAlphabetLength() << " simbolos" << endl;
	cout << "Minimizacion tomo " << timer.format(5) << endl;
	write_dot(dfa_mini, "nfa\\t9_dfa_min_i.dot");
	write_text(dfa_mini, "nfa\\t9_dfa_min_i.txt");

	return 0;
}

void test10()
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

void test11()
{
	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	typedef Dfa<TState, TSymbol> TDfa;
	typedef Fsa<TState, TSymbol> TNfa;
	NfaGenerator<TNfa, mt19937> nfagen;
	boost::timer::cpu_timer timer;	

	long seed = 5000;
	mt19937 rgen(seed);	
	int states = 80;
	int symbols = 2;
	float density = 0.005f;
	TNfa nfa = nfagen.Generate(states, symbols, 1, 1, density, rgen);

}

int main(int argc, char** argv)
{	
	int i = stoi(argv[1]);
	switch(i)
	{
		case 1: test1(); break;
		case 2: test2(); break;
		case 3: test3(); break;
		case 4: test4(); break;
		case 5: test5(); break;
		case 6: test6(); break;
		case 7: test7(); break;
		case 8: test8(); break;
		case 9: test9(); break;
		case 10: test10(); break;
		case 11: test11(); break;
		case 12: test7b(); break;
	};
	
	return 0;
}
