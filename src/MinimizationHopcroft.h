// June 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include <stdint.h>
#include <assert.h>
#include <list>
#include <string>
#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <queue>
#include "Dfa.h"

/// Hopcroft's DFA Minimization Algorithm.
template<class TState, class TSymbol, class TToken = uint64_t>
class MinimizationHopcroft
{	
public:

	typedef BitSet<TState, TToken> TSet;
	typedef Dfa<TState, TSymbol, TToken> TDfa;
	typedef std::pair<TSet, TSymbol> TSplitter;	
	typedef std::unordered_set<TSet, typename TSet::hash> TSplitterSet;
	typedef std::list<TSet> TPartition;

private:

	/// Extract one member of partition.
	/// O(1)
	TSet ExtractOne(TSplitterSet& lset)
	{
		TSet r = *lset.begin();
		lset.erase(lset.begin());
		return r;
	}
		
	/// Util to get a printable string representing the Set
	std::string SetToString(const TSet& set)
	{
		int cnt = 0;
		std::string str;
		str.append("{");
		set.ForEachMember( [&cnt, &str] (TState i)
		{
			if(cnt++ > 0) str += ", ";			
			str.append(std::to_string((uint64_t)i));
			return true;
		});
		str.append("}");
		return str;
	}

	/// Util to get a printable string representing the partition
	std::string PartitionToString(const TPartition& p)
	{
		std::string str;
		int cnt = 0;
		str.append("{");
		std::for_each(p.cbegin(), p.cend(), [&](const TSet& x)
		{
			if(cnt++ > 0) str.append(", ");			
			str.append(SetToString(x));			
		});
		str.append("}");
		return str;
	}

	std::string SplitterToString(const TSplitter& c)
	{
		std::string str;		
		str.append("(");
		str.append(SetToString(c.first));
		str.append(", ");
		str.append(std::to_string((uint64_t)c.second));
		str.append(")");
		return str;
	}

	std::string SplitterSetToString(const TSplitterSet& s)
	{
		std::string str;
		int cnt = 0;
		str.append("{");
		for(auto c : s)
		{
			if(cnt++ > 0) str.append(", ");
			str.append(SetToString(c));
		}
		str.append("}");
		return str;
	}

	/// Split operation, Refine B' using the Splitter (B, a) resulting B1 and B2.
	/// <math>(B1, B2) = B' \ (B,a)</math>
	void Split(const TDfa& dfa, const TSet& Bc, const TSet& B, TSymbol a, TSet& B1, TSet& B2)
	{
		// En 'B1' los estados de 'Bc' que llegan a 'B' consumiendo 'a'
		// O(N) -> N estados de B
		B1.Clear();		
		B.ForEachMember([&, a](TState q)
		{
			// O(1)
			const TSet& pred = dfa.GetPredecessor(q, a);
			// O(1)
			B1.UnionWith(pred);
			return true;
		});
		B2 = B1;
		B1.IntersectWith(Bc);
		B2.Complement();
		B2.IntersectWith(Bc);
	}

	TSet Inverse(const TDfa& dfa, const TSet& B, TSymbol a)
	{
		TSet outp(dfa.GetStates());
		// O(Card(B))
		B.ForEachMember([&, a](TState q)
		{
			// O(1)
			const TSet& pred = dfa.GetPredecessor(q, a);
			// O(1)
			outp.UnionWith(pred);
			return true;
		});
		return outp;
	}

public:

	/// Controls the debugging info output
	bool ShowConfiguration;

	MinimizationHopcroft()
		:ShowConfiguration(true)
	{
	}

	void Minimize2(const TDfa& dfa)
	{
		using namespace std;
		typedef TState TStateSize;

		// L = { min(P) }
		unsigned final_states_count = dfa.Final.Count();
		unsigned non_final_states_count = dfa.GetStates() - final_states_count;

		// Un automata sin estado final?
		assert(final_states_count > 0);
				
		// P = { F, ~F }, maxima cantidad de particiones es una por estado
		vector<pair<TStateSize,TSet>> P(dfa.GetStates());
		
		auto final_it = P.push_front(dfa.Final);

		TSet t = dfa.Final;
		t.Complement();
		auto non_final_it = P.push_front(t);

		// conjunto de espera
		queue<pair<TStateSize, TSymbol>> wait_splitters;
		for(TSymbol c=0; c<dfa.GetAlphabethLength(); c++)
		{
			if(final_states_count < non_final_states_count) 
			{
				wait_splitters.emplace(final_it, c);
			} else {
				wait_splitters.emplace(non_final_it, c);
			}
		}
		
		TSet partitions_to_split(dfa.GetStates());
		vector<TStateSize> state_to_partition(dfa.GetStates());
		
		// El peor caso de W es cuando cada division 
		// añada un elemento por cada estado
		while (!wait_splitters.empty())
		{
			auto splitter = wait_splitters.pop();

			partitions_to_split.Clear();
			TSet predecessors = Inverse(dfa, *splitter.first, splitter.second);
			TSet comp_predecesors = predecessors;
			comp_predecesors.Complement();
			predecessors.ForEachMember([&](TState i)
			{
				auto partition_index = state_to_partition[i];
				if(partitions_to_split.Contains(partition_index)) return true;
				TSet split = partitions[partition_index];
				auto partition_size = split.Count();
				split.IntersectWith(predecessors);
				auto split_size = split.Count();
				partitions_to_split.Add(partition_index);
				
				if(split_size == partition_size) return true;
				
				partitions[partition_index].IntersectWith(comp_predecessors);
				partitions.push_back(split);

				// Update state to partition info
				split.ForEachMember([&](TState s)
				{
					state_to_partition[s] = partitions.size() - 1;
					return true;
				});

				for(TSymbol s=0; s<dfa.GetAlphabethLength(); s++)
				{
					if(split_size < partition_size - split_size) 
					{
						wait_splitters.emplace(split, s);
					} else {
						wait_splitters.emplace(partitions[partition_index], s);
					}
				}
				
				return true;
			});
		}		
	}

	/// Entry point to minimize any DFA
	void Minimize(const TDfa& dfa) 
	{
		// P = { F, ~F }
		TPartition P;
		P.push_front(dfa.Final);

		TSet t = dfa.Final;
		t.Complement();
		P.push_front(t); 

		// L = { min(P) }
		unsigned final_states_count = dfa.Final.Count();
		unsigned non_final_states_count = dfa.GetStates() - final_states_count;

		// Un automata sin estado final?
		assert(final_states_count > 0);

		TSplitterSet W(dfa.GetStates()*2);
		
		if(final_states_count < non_final_states_count) 
		{
			W.emplace(dfa.Final);
		} else {
			W.emplace(t);
		}
		
		TSet B1(dfa.GetStates()), B2(dfa.GetStates());
		TSet S(dfa.GetStates());
		// El peor caso de W es cuando cada division 
		// añada un elemento por cada estado
		while (!W.empty())
		{
			S = ExtractOne(W);
			if(ShowConfiguration) 
			{
				std::cout << "S=" << SetToString(S) << std::endl;
			}
			// Ciclo aportando siempre una contribucion lineal
			// en el numero de simbolos
			for(TSymbol b=0; b<dfa.GetAlphabethLength(); b++)
			{
				if(ShowConfiguration) 
				{
					std::cout << "b=" << (uint64_t)b << std::endl;
				}

				// pred = b^(-1).S 
				// estados que consumiendo b llegan a S
				// O(Card(S)) la cardinalidad de S crece con log2(N)
				// como se enuncia en  
				// "On the complexity of Hopcroft's state minimization algorithm, Berstel, Carton, 2004, pag. 3"
				// y "Minimization of Automata, Berstel, Boasson, Carton, Fagnot, 2011, pag. 12"				
				const TSet pred = Inverse(dfa, S, b);

				// Las particiones van aumentando con cada division
				// maximo al doble de Card(P) en un bloque completo de
				// este ciclo.
				for(auto B = P.begin(); B != P.end(); )
				{
					if(ShowConfiguration) 
					{
						std::cout << "Test B=" << SetToString(*B) << std::endl;
					}
					
					// termina de calcular el split en O(1)
					B1 = pred;
					B2 = pred;
					B1.IntersectWith(*B);
					B2.Complement();
					B2.IntersectWith(*B);

					// was there split?
					if(B1.IsEmpty() || B2.IsEmpty()) 
					{
						B++; // advance B
						continue;
					}

					if(ShowConfiguration)
					{
						std::cout << "B1=" << SetToString(B1) << std::endl;
						std::cout << "B2=" << SetToString(B2) << std::endl;
					}

					auto f = W.find(*B);
					if(f != W.end())
					{
						W.erase(f);
						W.emplace(B1);
						W.emplace(B2);
					} else {		
						auto B1count = B1.Count();
						auto B2count = B2.Count();
						// El añadido contendra maximo la mitad de los estados en B
						if(B1count < B2count) 
						{
							W.emplace(B1);
						} else {
							W.emplace(B2);
						}		
					}
				
					// Insertamos antes del iterador porque B1 y B2 no pueden dividirse mas
					// con este mismo splitter
					P.insert(B, B2);
					P.insert(B, B1);
					B = P.erase(B); // advance B

					if(ShowConfiguration)
					{
						std::cout << "P=" << PartitionToString(P) << std::endl;
						std::cout << "W=" << SplitterSetToString(W) << std::endl;
					}
				}
			}
			if(W.size() % 1000 == 0) std::cout << W.size() << " splitters remaining" << std::endl;
		}
		if(ShowConfiguration)
		{			
			std::cout << "FINALLY P=" << PartitionToString(P) << std::endl;
		}
	}
};
