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
	typedef BitSet<TState, TToken> TSet;
	typedef Dfa<TState, TSymbol, TToken> TDfa;
	typedef std::pair<TStateSize, TSymbol> TSplitter;	
	typedef std::queue<TSplitter> TSplitterSet;
	typedef std::pair<TStateSize,TSet> TPartition;
	typedef std::vector<TPartition> TPartitionSet;

private:

	/// Extract one member of partition.
	/// O(1)
	TSet ExtractOne(TSplitterSet& lset)
	{
		auto i = lset.begin();
		auto j = rand() % lset.size();
		while(j != 0) { j--; i++; }
		TSet r = *i;
		lset.erase(i);
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

	std::string to_string(const TPartitionSet& p)
	{
		std::string str;
		int cnt = 0;
		str.append("{");
		std::for_each(p.cbegin(), p.cend(), [&](const TPartition& x)
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
		
		unsigned final_states_count = dfa.Final.Count();
		unsigned non_final_states_count = dfa.GetStates() - final_states_count;

		// Un automata sin estado final?
		assert(final_states_count > 0);

		// cantidad de estados en la particion y particion
		TPartitionSet P;
		P.reserve(dfa.GetStates());

		P.emplace_back(final_states_count, dfa.Final);

		TSet t = dfa.Final;
		t.Complement();		
		P.emplace_back(non_final_states_count, t);

		// conjunto de espera
		TSplitterSet wait_splitters;
		TSet wait_splitters_membership(dfa.GetStates()*dfa.GetAlphabethLength());
		for(TSymbol c=0; c<dfa.GetAlphabethLength(); c++)
		{
			if(final_states_count < non_final_states_count) 
			{
				wait_splitters.emplace(0, c); // final states
				wait_splitters_membership.Add(0*dfa.GetAlphabethLength()+c);
			} else {
				wait_splitters.emplace(1, c); // Non-final states
				wait_splitters_membership.Add(1*dfa.GetAlphabethLength()+c);
			}
		}
		
		TSet partitions_to_split(dfa.GetStates());
		// Inicializa funcion inversa para obtener la particion a la que pertenece un estado
		vector<TStateSize> state_to_partition(dfa.GetStates());		
		for(TState st=0; st<dfa.GetStates(); st++)
		{
			state_to_partition[st] = dfa.IsFinal(st) ? 0 : 1;
		}
				
		// El peor caso de W es cuando cada division 
		// añada un elemento por cada estado
		while (!wait_splitters.empty())
		{
			TSplitter splitter = wait_splitters.front();
			wait_splitters.pop();
			wait_splitters_membership.Remove(splitter.first*dfa.GetAlphabethLength()+splitter.second);

			if(ShowConfiguration)
			{
				cout << "Spliter=" << to_string(splitter, P) << endl;
				cout << "P=" << to_string(P) << endl;
			}
						
			TSet predecessors = Inverse(dfa, P[splitter.first].second, splitter.second);
			TSet comp_predecessors = predecessors;
			comp_predecessors.Complement();
			partitions_to_split.Clear();

			predecessors.ForEachMember([&](TState i)
			{
				TStateSize partition_index = state_to_partition[i];				
				
				if(partitions_to_split.Contains(partition_index)) return true;
				partitions_to_split.Add(partition_index);
								
				TStateSize partition_size = P[partition_index].first;
				if(partition_size == 1) return true;

				if(ShowConfiguration)
				{
					cout << "state=" << (uint64_t)i << " in partition " << partition_index << endl;
				}

				TSet& partition = P[partition_index].second;
				
				TSet backup = partition;

				partition.IntersectWith(predecessors); // make split
				TStateSize split_size = partition.Count();

				if(split_size == partition_size) return true;
				P[partition_index].first = split_size; // update count (lookup)

				backup.IntersectWith(comp_predecessors);
				
				// indice del complemento en el conjunto de particiones
				TStateSize new_index = (TStateSize)P.size();
				TStateSize split_complement_size = partition_size - split_size;

				// Update state to partition info
				backup.ForEachMember([&](TState s)
				{
					state_to_partition[s] = new_index;
					return true;
				});
				P.emplace_back(split_complement_size, backup);
				
				for(TSymbol s=0; s<dfa.GetAlphabethLength(); s++)
				{
					TStateSize index_to_add;
					if(wait_splitters_membership.Contains(partition_index*dfa.GetAlphabethLength()+s))
					{
						index_to_add = new_index;
					} else {
						index_to_add = split_size < split_complement_size ? partition_index : new_index;
					}
					wait_splitters.emplace(index_to_add, s);
					wait_splitters_membership.Add(index_to_add*dfa.GetAlphabethLength()+s);					
				}
				
				return true;
			});
		}	
		cout << "Finished, " << P.size() << " states of " << dfa.GetStates() << endl;
		if(ShowConfiguration)
		{
			cout << "Final P=" << to_string(P) << endl;
		}
	}
};
