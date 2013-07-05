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
	typedef TState TStateSize;
	typedef Dfa<TState, TSymbol, TToken> TDfa;
	typedef typename TDfa::TSet TSet;
	typedef std::pair<TStateSize, TSymbol> TSplitter;	
	typedef std::queue<TSplitter> TSplitterSet;
	typedef std::pair<TStateSize,TSet> TPartition;
	typedef std::vector<TPartition> TPartitionSet;

private:

	// TSet
	std::string to_string(const TSet& set)
	{
		using namespace std;
		int cnt = 0;
		string str;
		str.append("{");
		set.ForEachMember( [&] (TState i)
		{
			if(cnt++ > 0) str += ", ";
			str.append(std::to_string((uint64_t)i));
			return true;
		});
		str.append("}");
		return str;
	}

	std::string to_string(const TPartitionSet& p, TStateSize size)
	{
		std::string str;
		int cnt = 0;
		str.append("{");
		std::for_each(p.cbegin(), p.cbegin()+size, [&](const TPartition& x)
		{
			if(cnt++ > 0) str.append(", ");
			str.append(to_string(x.second));
		});
		str.append("}");
		return str;
	}

	std::string to_string(const TSplitter& splitter, const TPartitionSet& P)
	{
		using namespace std;
		string str;
		str.append("(");
		str.append(to_string(P[splitter.first].second));
		str.append(", ");
		str.append(std::to_string((uint64_t)splitter.second));
		str.append(")");
		return str;
	}

	void Inverse(const TDfa& dfa, const TSet& B, TSymbol a, TSet& outp)
	{
		// O(Card(B))
		B.ForEachMember([&, a](TState q) throw()
		{
			// O(1)
			const TSet& pred = dfa.GetPredecessor(q, a);
			
			// O(N)  - N es la cantidad de estados de dfa
			outp.UnionWith(pred);

			// O(Card(pred))
			/*
			pred.ForEachMember([&](TState q) throw()
			{
				outp.Add(q);
				return true;
			});
			*/
			return true;
		});
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

		unsigned final_states_count = dfa.Final.Count();
		unsigned non_final_states_count = dfa.GetStates() - final_states_count;

		// Un automata sin estado final?
		assert(final_states_count > 0);

		// cantidad de estados en la particion y particion
		// Maximo puede exisitir una particion por cada estado, por ello reservamos de esta forma
		TPartitionSet P(dfa.GetStates(), TPartition(0, dfa.GetStates()));

		P[0].first = final_states_count;
		P[0].second = dfa.Final;

		TSet t = dfa.Final;
		t.Complement();		
		P[1].first = non_final_states_count;
		P[1].second = t;

		TStateSize new_index = 2;

		TStateSize min_initial_partition_index = final_states_count < non_final_states_count ? 0 : 1;

		// conjunto de espera
		TSplitterSet wait_splitters;
		TSet wait_splitters_membership(dfa.GetStates()*dfa.GetAlphabethLength());
		for(TSymbol c=0; c<dfa.GetAlphabethLength(); c++)
		{
			wait_splitters.emplace(min_initial_partition_index, c); // final states
			wait_splitters_membership.Add(min_initial_partition_index*dfa.GetAlphabethLength()+c);
		}

		TSet partitions_to_split(dfa.GetStates());
		// Inicializa funcion inversa para obtener la particion a la que pertenece un estado
		vector<TStateSize> state_to_partition(dfa.GetStates());		
		for(TState st=0; st<dfa.GetStates(); st++)
		{
			state_to_partition[st] = dfa.IsFinal(st) ? 0 : 1;
		}

		// conjunto de predecesores
		TSet predecessors(dfa.GetStates());

		// El peor caso de W es cuando cada division 
		// añada un elemento por cada estado (y por cada letra, claro esta)
		while (!wait_splitters.empty())
		{
			assert(new_index < dfa.GetStates());

			TSplitter splitter = wait_splitters.front();
			wait_splitters.pop();
			wait_splitters_membership.Remove(splitter.first*dfa.GetAlphabethLength()+splitter.second);

			if(ShowConfiguration)
			{
				cout << "P=" << to_string(P, new_index) << endl;
				cout << "Spliter=" << to_string(splitter, P) << endl;				
			}

			predecessors.Clear();
			Inverse(dfa, P[splitter.first].second, splitter.second, predecessors);
			partitions_to_split.Clear();

			// let a=splitter.second, B=P[splitter.first].second
			// O(card(a^{-1}.B))
			predecessors.ForEachMember([&](TState i) throw()
			{
				TStateSize partition_index = state_to_partition[i];				

				if(partitions_to_split.Contains(partition_index)) return true;
				partitions_to_split.Add(partition_index);

				TStateSize partition_size = P[partition_index].first;
				if(partition_size == 1) return true;

				if(ShowConfiguration)
				{
					cout << "pred state=" << (uint64_t)i << " in partition " << partition_index << endl;
				}

				TSet& partition = P[partition_index].second;
				TSet& backup = P[new_index].second;

				backup = partition;

				TStateSize split_size = partition_size;
				
				// intersect O(card(B))
				partition.ForEachMember([&](TState t) throw()
				{
					if(!predecessors.Contains(t)) 
					{	
						partition.Remove(t); 
						split_size--; 
					}
					else 
					{
						backup.Remove(t);
					}
					return true;
				});

				if(split_size == partition_size) return true;
				
				// tamaño de conjunto intersectado con el complemento
				TStateSize split_complement_size = partition_size - split_size;

				P[partition_index].first = split_size; // update count (lookup)
				P[new_index].first = split_complement_size;
				
				// Update state to partition info
				backup.ForEachMember([&](TState s) throw()
				{
					state_to_partition[s] = new_index;
					return true;
				});
				
				for(TSymbol s=0; s<dfa.GetAlphabethLength(); s++)
				{
					TStateSize index_to_add;
					if(wait_splitters_membership.Contains(partition_index*dfa.GetAlphabethLength()+s))
					{
						index_to_add = new_index;
					}
					else
					{
						index_to_add = split_size < split_complement_size ? partition_index : new_index;
					}
					wait_splitters.emplace(index_to_add, s);
					wait_splitters_membership.Add(index_to_add*dfa.GetAlphabethLength()+s);					
				}

				// si ha llegado hasta aqui, hizo division, el indice para la nueva particion debe elevarse
				new_index++;
				return true;
			});
		}
		cout << "Finished, " << new_index << " states of " << dfa.GetStates() << endl;
		if(ShowConfiguration)
		{
			cout << "Final P=" << to_string(P, new_index) << endl;
		}
	}
};
