// June 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include "dynamic_bitset.h"
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
template<typename TDfa>
class MinimizationHopcroft
{	
public:	
	typedef TDfa TDfa;
	typedef typename TDfa::TState TState;
	typedef typename TDfa::TSymbol TSymbol;
	typedef typename TState TStateSize;	
	typedef typename TDfa::TSet TSet;
	typedef std::pair<TStateSize,TStateSize> TPartition;
	typedef std::vector<TPartition> TPartitionSet;

	typedef std::vector<std::vector<TState>> TPartitionVector;
	typedef std::vector<TStateSize> TStateToPartition;
	
private:

	std::string to_string(const TPartition& P, const std::vector<TState>& Pcontent)
	{
		using namespace std;
		string str;
		str.append("{");
		for(TStateSize j=P.first, cnt=0; cnt<P.second; j++, cnt++)
		{
			if(cnt > 0) str += ", ";
			str.append(std::to_string((size_t)Pcontent[j]));
		}
		str.append("}");
		return str;
	}

	std::string to_string(const TPartitionSet& p, TStateSize size, const std::vector<TStateSize>& Pcontent)
	{
		using namespace std;
		string str;
		int cnt = 0;
		str.append("{");
		for_each(p.cbegin(), p.cbegin()+size, [&](const TPartition& x)
		{
			if(cnt++ > 0) str.append(", ");
			str.append(to_string(x, Pcontent));
		});
		str.append("}");
		return str;
	}

public:

	class NumericPartition
	{
	public:
		std::vector<TState> Pcontent;
		TStateToPartition state_to_partition;
		TPartitionSet P;
		TState new_index;

		TState GetSize() const { return new_index; }
	};

	/// Controls the debugging info output
	bool ShowConfiguration;

	MinimizationHopcroft()
		:ShowConfiguration(true)
	{
	}

	TDfa BuildDfa(const TDfa& dfa, const NumericPartition& partitions)
	{
		TDfa ndfa(dfa.GetAlphabetLength(), partitions.GetSize());
		int pidx = 0;
		for(auto p : partitions.P)
		{
			auto begin = partitions.Pcontent.begin()+p.first;
			auto end = begin + p.second;
			if(begin == end) continue;
			for(auto is=begin; is!=end; is++)
			{
				auto s = *is;
				for(TSymbol sym=0; sym<dfa.GetAlphabetLength(); sym++)
				{
					TState tgt = dfa.GetSuccessor(s, sym);
					TState ptgt = partitions.state_to_partition[tgt];
					ndfa.SetTransition(pidx, sym, ptgt);
				}
				if(dfa.IsInitial(s)) ndfa.SetInitial(pidx);
				if(dfa.IsFinal(s)) ndfa.SetFinal(pidx);
			}
			pidx++;
		}
		return ndfa;
	}

	void Minimize(const TDfa& dfa, NumericPartition& np)
	{
		using namespace std;

		TStateSize final_states_count = (TStateSize)dfa.GetFinals().Count();
		TStateSize non_final_states_count = (TStateSize)dfa.GetStates() - final_states_count;

		// Un automata sin estado final?
		assert(final_states_count > 0);

		// cantidad de estados en la particion y particion
		// Maximo puede exisitir una particion por cada estado, por ello reservamos de esta forma
		np.Pcontent.resize(dfa.GetStates());
		
		// vector de parejas que indican en el vector de contenido cada particion
		// Cada pareja contiene el indice donde inicia una particion y su longitud
		np.P.resize(dfa.GetStates()*2);

		np.P[0].first = 0;
		np.P[0].second = final_states_count;

		np.P[1].first = final_states_count;
		np.P[1].second = non_final_states_count;

		// Inicializa funcion inversa para obtener la particion a la que pertenece un estado
		np.state_to_partition.resize(dfa.GetStates());

		auto it_f = np.Pcontent.begin();
		auto it_nf = np.Pcontent.rbegin();
		for(TState st=0; st<dfa.GetStates(); st++)
		{
			if(dfa.IsFinal(st)) *it_f++ = st;
			else *it_nf++ = st;
			np.state_to_partition[st] = dfa.IsFinal(st) ? 0 : 1;
		}

		// partitions count
		np.new_index = 2;

		TStateSize min_initial_partition_index = final_states_count < non_final_states_count ? 0 : 1;

		// set containing the next partitions to be processed
		TSet wait_set_membership(dfa.GetStates());
		wait_set_membership.Add(min_initial_partition_index);

		// set containing the already processed partitions
		TSet partitions_to_split(dfa.GetStates());

		// conjunto de predecesores
		TSet predecessors(dfa.GetStates());

		// worst case is when WaitSet has one entry per state
		for(auto splitter_set=wait_set_membership.GetIterator(); !splitter_set.IsEnd(); splitter_set=wait_set_membership.GetIterator())
		{
			assert(np.new_index <= dfa.GetStates());

			// remove current
			wait_set_membership.Remove(splitter_set.GetCurrent());

			// current splitter partition
			const auto& splitter_partition = np.P[splitter_set.GetCurrent()];

			if(ShowConfiguration)
			{				
				cout << "Spliter=" << to_string(splitter_partition, np.Pcontent) << endl;
			}

			// Per symbol loop
			for(TSymbol splitter_letter=0; splitter_letter<dfa.GetAlphabetLength(); splitter_letter++)
			{								
				predecessors.Clear();

				// recorre elementos de la particion
				auto partition_it_begin = np.Pcontent.begin() + splitter_partition.first;
				auto partition_it_end = partition_it_begin + splitter_partition.second;
				for(; partition_it_begin != partition_it_end; partition_it_begin++)
				{
					TState state = *partition_it_begin;
					predecessors.UnionWith(dfa.GetPredecessors(state, splitter_letter));
				}

				partitions_to_split.Clear();

				// let a=splitter_letter, B belongs P
				// O(card(a^{-1}.B))				
				for(auto ss=predecessors.GetIterator(); !ss.IsEnd(); ss.MoveNext())
				{
					// state ss belongs to partition indicated with partition_index
					TStateSize partition_index = np.state_to_partition[ss.GetCurrent()];

					// Is this partition already processed?
					if(partitions_to_split.TestAndAdd(partition_index)) continue;

					TPartition& partition_desc = np.P[partition_index];

					// Imposible to divide a single state partition
					const TStateSize partition_size = partition_desc.second;
					if(partition_size == 1) continue;

					// Partition start point
					const auto partition_it_original_begin = np.Pcontent.begin() + partition_desc.first;

					partition_it_begin = partition_it_original_begin;					
					partition_it_end = partition_it_begin + partition_desc.second - 1;
					// iterates trying split
					do
					{
						TState state = *partition_it_begin;
						if(predecessors.Contains(state))
						{
							partition_it_begin++;
						}
						else 
						{
							iter_swap(partition_it_begin, partition_it_end);
							np.state_to_partition[state] = np.new_index;
							partition_it_end--;
						}
					} while(partition_it_begin != partition_it_end);					
					// last element remains without class, assign it
					{ 
						TState state = *partition_it_begin;
						if(predecessors.Contains(state))
						{
							partition_it_begin++;							
						}
						else 
						{							
							np.state_to_partition[state] = np.new_index;						
						}
					}

					// old partition new size
					TStateSize split_size = (TStateSize)(partition_it_begin - partition_it_original_begin);

					// continue if was not division
					if(split_size == partition_size) continue;

					if(ShowConfiguration)
					{
						cout << "symbol=" << (size_t)splitter_letter << endl;
						cout << "pred state=" << (size_t)ss.GetCurrent() << " in partition " << partition_index << endl;
					}

					// new parition size
					TStateSize split_complement_size = partition_size - split_size;

					partition_desc.second = split_size;

					assert(np.new_index < np.P.size());

					// confirm descriptor for new partition
					np.P[np.new_index].first = partition_desc.first + split_size;
					np.P[np.new_index].second = split_complement_size;

					if(wait_set_membership.Contains(partition_index)) 
					{
						wait_set_membership.Add(np.new_index);
					} 
					else 
					{
						auto add_index = split_size < split_complement_size ? partition_index : np.new_index;
						wait_set_membership.Add(add_index);
					}

					// If we are here, split was done
					// new partition index must increment
					np.new_index++;

					if(ShowConfiguration)
					{
						cout << "P=" << to_string(np.P, np.new_index, np.Pcontent) << endl;
					}		
				}
			}
		}		
		if(ShowConfiguration)
		{
			cout << "Final P=" << to_string(np.P, np.new_index, np.Pcontent) << endl;
			cout << "Finished, " << np.new_index << " states of " << dfa.GetStates() << endl;
		}
	}

	TDfa Minimize(const TDfa& dfa)
	{
		NumericPartition p;
		Minimize(dfa, p);
		return BuildDfa(dfa, p);
	}
};
