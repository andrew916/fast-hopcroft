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
template<typename TState, typename TSymbol, typename TToken = uint64_t>
class MinimizationHopcroft
{	
public:	
	typedef TState TStateSize;
	typedef Dfa<TState, TSymbol, TToken> TDfa;
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
			str.append(std::to_string((uint64_t)Pcontent[j]));
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

	/// Controls the debugging info output
	bool ShowConfiguration;

	MinimizationHopcroft()
		:ShowConfiguration(true)
	{
	}

	TDfa Synthetize(const TDfa& dfa, const TPartitionVector& partitions, const TStateToPartition& state_to_partition)
	{
		TDfa ndfa(dfa.GetAlphabetLength(), (unsigned int)partitions.size());
		int pidx = 0;
		for(auto p : partitions)
		{
			for(auto s : p)
			{
				for(TSymbol sym=0; sym<dfa.GetAlphabetLength(); sym++)
				{
					TState tgt = dfa.GetSucessor(s, sym);
					TState ptgt = state_to_partition[tgt];
					ndfa.SetTransition(pidx, sym, ptgt);
				}
				if(dfa.IsInitial(s)) ndfa.SetInitial(pidx);
				if(dfa.IsFinal(s)) ndfa.SetFinal(pidx);
			}
			pidx++;
		}
		return ndfa;
	}		

	TSet ComputeReachable(const TDfa& dfa)
	{
		using namespace std;		
		TSet reachable = dfa.Initial;
		TSet new_states = reachable;
		TSet temp(dfa.GetStates());
		do
		{
			temp.reset();
			for(auto q=new_states.find_first(); q!=new_states.npos; q=new_states.find_next(q))
			{
				for(TSymbol c=0; c<dfa.GetAlphabetLength(); c++)
				{
					TState qd = dfa.GetSucessor((TState)q, c);					
					temp.set(qd);
				}
			}
			new_states = temp - reachable;
			reachable |= new_states;
		} while(!new_states.none());
		return reachable;
	}

	TDfa CleanUnreachable(const TDfa& dfa)
	{		
		TSet reach = ComputeReachable(dfa);		
		TDfa ndfa(dfa.GetAlphabetLength(), (unsigned)reach.count());
		vector<TState> table(dfa.GetStates());
		for(TState q=0, qn=0; q<dfa.GetStates(); q++)
		{
			if(!reach.test(q)) continue;
			table[q] = qn++;
		}
		for(TState q=0; q<dfa.GetStates(); q++)
		{
			if(!reach.test(q)) continue;
			TState qn = table[q];
			ndfa.SetInitial(qn, dfa.IsInitial(q));
			ndfa.SetFinal(qn, dfa.IsFinal(q));
			for(TSymbol c=0; c<dfa.GetAlphabetLength(); c++)
			{
				TState qd = dfa.GetSucessor(q, c);
				ndfa.SetTransition(qn, c, table[qd]);
			}
		}
		return ndfa;
	}

	void Minimize(const TDfa& dfa, TPartitionVector& out_partitions = TPartitionVector(), TStateToPartition& state_to_partition = TStateToPartition())
	{
		using namespace std;

		TStateSize final_states_count = (TStateSize)dfa.Final.count();
		TStateSize non_final_states_count = (TStateSize)dfa.GetStates() - final_states_count;

		// Un automata sin estado final?
		assert(final_states_count > 0);

		// cantidad de estados en la particion y particion
		// Maximo puede exisitir una particion por cada estado, por ello reservamos de esta forma
		vector<TState> Pcontent(dfa.GetStates());
		// vector de parejas que indican en el vector de contenido cada particion
		// Cada pareja contiene el indice donde inicia una particion y su longitud
		TPartitionSet P(dfa.GetStates());

		P[0].first = 0;
		P[0].second = final_states_count;

		P[1].first = final_states_count;
		P[1].second = non_final_states_count;

		// Inicializa funcion inversa para obtener la particion a la que pertenece un estado
		state_to_partition.clear();
		state_to_partition.resize(dfa.GetStates());

		auto it_f = Pcontent.begin();
		auto it_nf = Pcontent.rbegin();
		for(TState st=0; st<dfa.GetStates(); st++)
		{
			if(dfa.IsFinal(st)) *it_f++ = st;
			else *it_nf++ = st;
			state_to_partition[st] = dfa.IsFinal(st) ? 0 : 1;
		}

		// partitions count
		TStateSize new_index = 2;

		TStateSize min_initial_partition_index = final_states_count < non_final_states_count ? 0 : 1;

		// set containing the next partitions to be processed
		TSet wait_set_membership(dfa.GetStates());
		wait_set_membership.set(min_initial_partition_index);

		// set containing the already processed partitions
		TSet partitions_to_split(dfa.GetStates());

		// conjunto de predecesores
		TSet predecessors(dfa.GetStates());

		// worst case is when WaitSet has one entry per state
		for(auto splitter_set=wait_set_membership.find_first(); splitter_set!=TSet::npos; splitter_set=wait_set_membership.find_first())
		{
			assert(new_index <= dfa.GetStates());

			// remove current
			wait_set_membership.reset(splitter_set);

			// current splitter partition
			const auto& splitter_partition = P[splitter_set];

			if(ShowConfiguration)
			{				
				cout << "Spliter=" << to_string(splitter_partition, Pcontent) << endl;
			}

			// Per symbol loop
			for(TSymbol splitter_letter=0; splitter_letter<dfa.GetAlphabetLength(); splitter_letter++)
			{								
				predecessors.reset();

				// recorre elementos de la particion
				auto partition_it_begin = Pcontent.begin() + splitter_partition.first;
				auto partition_it_end = partition_it_begin + splitter_partition.second;
				for(; partition_it_begin != partition_it_end; partition_it_begin++)
				{
					TState state = *partition_it_begin;
					predecessors |= dfa.GetPredecessors(state, splitter_letter);
				}

				partitions_to_split.reset();

				// let a=splitter_letter, B belongs P
				// O(card(a^{-1}.B))				
				for(auto ss=predecessors.find_first(); ss!=TSet::npos; ss=predecessors.find_next(ss))
				{
					// state ss belongs to partition indicated with partition_index
					TStateSize partition_index = state_to_partition[ss];

					// Is this partition already processed?
					if(partitions_to_split.test(partition_index)) continue;
					partitions_to_split.set(partition_index);

					TPartition& partition_desc = P[partition_index];

					// Imposible to divide a single state partition
					const TStateSize partition_size = partition_desc.second;
					if(partition_size == 1) continue;

					// Partition start point
					const auto partition_it_original_begin = Pcontent.begin() + partition_desc.first;

					partition_it_begin = partition_it_original_begin;					
					partition_it_end = partition_it_begin + partition_desc.second - 1;
					// iterates trying split
					do
					{
						TState state = *partition_it_begin;
						if(predecessors.test(state))
						{
							partition_it_begin++;
						}
						else 
						{
							iter_swap(partition_it_begin, partition_it_end);
							state_to_partition[state] = new_index;
							partition_it_end--;
						}
					} while(partition_it_begin != partition_it_end);					
					// last element remains without class, assign it
					{ 
						TState state = *partition_it_begin;
						if(predecessors.test(state))
						{
							partition_it_begin++;							
						}
						else 
						{							
							state_to_partition[state] = new_index;						
						}
					}

					// old partition new size
					TStateSize split_size = (TStateSize)(partition_it_begin - partition_it_original_begin);

					// continue if was not division
					if(split_size == partition_size) continue;

					if(ShowConfiguration)
					{
						cout << "symbol=" << (uint64_t)splitter_letter << endl;
						cout << "pred state=" << (uint64_t)ss << " in partition " << partition_index << endl;						
					}	

					// new parition size
					TStateSize split_complement_size = partition_size - split_size;

					partition_desc.second = split_size;

					// confirm descriptor for new partition
					P[new_index].first = partition_desc.first + split_size;
					P[new_index].second = split_complement_size;

					if(wait_set_membership.test(partition_index)) 
					{
						wait_set_membership.set(new_index);
					} 
					else 
					{
						auto add_index = split_size < split_complement_size ? partition_index : new_index;
						wait_set_membership.set(add_index);
					}

					// If we are here, split was done
					// new partition index must increment
					new_index++;

					if(ShowConfiguration)
					{
						cout << "P=" << to_string(P, new_index, Pcontent) << endl;
					}		
				}
			}
		}
		cout << "Finished, " << new_index << " states of " << dfa.GetStates() << endl;
		if(ShowConfiguration)
		{
			cout << "Final P=" << to_string(P, new_index, Pcontent) << endl;
		}

		// synth new
		out_partitions.clear();
		out_partitions.resize(new_index);
		auto p = P.begin();
		for(auto it1=out_partitions.begin(); it1 != out_partitions.end(); it1++, p++)
		{
			for(auto it2=Pcontent.begin()+p->first; it2 != Pcontent.begin()+p->first+p->second; it2++)
			{
				it1->push_back(*it2);
			}
		}		
	}
};
