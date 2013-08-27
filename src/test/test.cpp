// June 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
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
	dfa.SetTransition(0, 1, 0);

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

int test6()
{
	cout << "Prueba la generacion de DFA experimental" << endl;

	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	typedef Dfa<TState, TSymbol> TDfa;
	typedef MinimizationHopcroft<TDfa> TMinimizer;

	FsmGraphVizWriter<TDfa> exporter;		
	DfaBridgeGenerator<TState, TSymbol> gen;
	TMinimizer mini;

	for(int a=2; a!=5; a++)
	{
		for(int n=2; n!=10; n++) 
		{
			auto rnd = mt19937();
			rnd.seed(1);
			auto dfa = gen.Generate(a, n, 1, 1, n/2, rnd);
			string filename = string("test6_a") + to_string((size_t)a) + "_n" + to_string((size_t)n);

			ofstream generated_stream(filename + ".dot");
			exporter.Write(dfa, generated_stream, false);
			generated_stream.close();

			TMinimizer::TDfa urdfa = mini.CleanUnreachable(dfa);

			ofstream clean_stream(filename + "_clean.dot");
			exporter.Write(urdfa, clean_stream, false);
			clean_stream.close();

			TMinimizer::TPartitionVector partitions;
			TMinimizer::TStateToPartition state_to_partition;
			mini.Minimize(urdfa, partitions, state_to_partition);
			TMinimizer::TDfa ndfa = mini.Synthetize(urdfa, partitions, state_to_partition);

			ofstream minimized_stream(filename + "_min.dot");
			exporter.Write(ndfa, minimized_stream, false);
			minimized_stream.close();
		}
	}
	return 0;
}

int test7()
{
	cout << "Prueba la generacion de DFA experimental con un solo automata" << endl;

	typedef uint16_t TState;
	typedef uint8_t TSymbol;				
	typedef DfaBridgeGenerator<TState, TSymbol> TGenerator;
	typedef TGenerator::TDfa TDfa;
	TGenerator gen;
	FsmPlainTextWriter<TDfa> exporter;
	FsmPlainTextReader<TDfa> reader;

	ofstream output("dfa_plain_text.txt");
	mt19937 rgen;
	TDfa dfa = gen.Generate(5, 10, 2, 2, 3, rgen);
	exporter.Write(dfa, output);
	output.close();	

	ifstream input("dfa_plain_text.txt");
	TDfa dfa2 = reader.Read(input);
	input.close();
	return 0;
}

template<typename TFsm>
void write_dot(TFsm fsm, string filename)
{
	FsmGraphVizWriter<TFsm> writer;
	ofstream s(filename);
	if(!s.is_open()) throw exception("El archivo no pudo ser abierto");
	writer.Write(fsm, s);
	s.close();
}

template<typename TFsm>
TFsm read_text(string filename)
{
	FsmPlainTextReader<TFsm> reader;
	ifstream fsm_input(filename);
	if(!fsm_input.is_open()) throw exception("El archivo no pudo ser abierto");
	auto fsm = reader.Read(fsm_input);
	fsm_input.close();
	return fsm;
}

int test8()
{
	cout << "Genera multiples NFA con diferentes numeros de estados y simbolos, los minimiza, los guarda y escribe un reporte." << endl;

	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	typedef Dfa<TState, TSymbol> TDfa;
	typedef Nfa<TState, TSymbol> TNfa;
	typedef MinimizationHopcroft<TDfa> TMinimizer;

	mt19937 rgen;
	ofstream report("report_test8.txt");
	report << "nfa_nQ,dfa_nQ,min_nQ,alpha,time_ns" << endl;
	
	for(int states=4; states<100; states*=2)
	{
		for(int symbols=2; symbols<20; symbols*=2)
		{	
			for(int redundancy=0; redundancy<20; redundancy++)
			{
				auto filename1 = format("nfa\\nfa_n%1%_a%2%_r%3%") % states % symbols % redundancy;
				auto filename = filename1.str();
						
				cout << "Generando " << filename << "... " << endl;
				NfaGenerator<TNfa, mt19937> nfagen;
				TNfa nfa = nfagen.Generate(states, symbols, 1, 1, 0.05f, rgen);

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
			
				auto fmt = format("%1%, %2%, %3%, %4%, %5%") % states % dfa.GetStates() % part.size() % symbols % timer.elapsed().wall;
				report << fmt.str() << endl;

				auto dfa2 = mini.Synthetize(dfa, part, conv_table);

				//write_dot(nfa, filename + ".dot");
				//write_dot(dfa, filename + "_dfa.dot");
				//write_dot(dfa2, filename + "_mini.dot");
			}
		}
	}
	return 0;
}

int test9()
{
	cout << "Prueba la determinizacion y la minimizacion Brzozowski" << endl;

	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	typedef Dfa<TState, TSymbol> TDfa;
	typedef Nfa<TState, TSymbol> TNfa;
	NfaGenerator<TNfa, mt19937> nfagen;
	boost::timer::cpu_timer timer;	

	mt19937 rgen;	
	int states = 50;
	int symbols = 2;
	float density = 0.005f;
	TNfa nfa = nfagen.Generate(states, symbols, 1, 1, density, rgen);

	cout << "Generado NFA con " << nfa.GetStates() << " estados, " << nfa.GetAlphabetLength() << " simbolos, d=" << density << endl;
	write_dot(nfa, "nfa\\t9_nfa_org.dot");	

	Determinization<TNfa, TNfa> determb;
	auto nfab = determb.Determinize(nfa);
	cout << "Determinizado con " << nfab.GetStates() << " estados, " << nfab.GetAlphabetLength() << " simbolos" << endl;
	write_dot(nfab, "nfa\\t9_dfa.dot");	
	
	MinimizationBrzozowski<TNfa> minb;	
	timer.start();
	auto nfa_minb = minb.Minimize(nfab);	
	timer.stop();
	cout << "Minimizado Brzozowski con " << nfa_minb.GetStates() << " estados, " << nfa_minb.GetAlphabetLength() << " simbolos" << endl;
	cout << "Minimizacion tomo " << timer.format(3) << endl;
	write_dot(nfa_minb, "nfa\\t9_dfa_min_b.dot");

	Determinization<TDfa, TNfa> determh;
	auto dfa = determh.Determinize(nfa);
	cout << "Determinizado con " << dfa.GetStates() << " estados, " << dfa.GetAlphabetLength() << " simbolos" << endl;
	write_dot(dfa, "nfa\\t9_dfa.dot");	

	MinimizationHopcroft<TDfa> minh;
	MinimizationHopcroft<TDfa>::TPartitionVector hpartitions;
	minh.ShowConfiguration = false;
		
	timer.start();
	minh.Minimize(dfa, hpartitions);	
	timer.stop();
	cout << "Minimizado Hopcroft con " << hpartitions.size() << " estados, " << dfa.GetAlphabetLength() << " simbolos" << endl;
	cout << "Minimizacion tomo " << timer.format(5) << endl;
	//write_dot(dfa_minh, "nfa\\t9_dfa_min_h.dot");

	return 0;
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
	};
	
	return 0;
}
