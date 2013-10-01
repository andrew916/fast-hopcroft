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
	typedef typename TDfa::TSet TSet;
	typedef std::list<TState> TPartitionItem;
	typedef std::vector<TPartitionItem> TPartition;
	typedef std::vector<TState> TStateToPartition;

private:


public:

	std::string to_string(const TPartition& P) const
	{
		using namespace std;
		string str;
		str.append("{");
		for(TState i=0; i<P.size() && P[i].size()>0; i++)
		{
			if(i > 0) str += ", ";
			TPartitionItem pi = P[i];
			str.append(to_string(pi));
		}
		str.append("}");
		return str;
	}

	std::string to_string(const TPartitionItem& p) const
	{
		using namespace std;
		string str;
		str.append("{");
		int cnt = 0;
		for(auto i=p.begin(); i!=p.end(); i++)
		{
			if(cnt++ > 0) str.append(", ");
			size_t s = static_cast<size_t>(*i);
			str.append(std::to_string(s));
		}
		str.append("}");
		return str;
	}

	class NumericPartition
	{
	public:
		std::vector<TState> state_to_partition;
		std::vector<std::list<TState>> P;
		TState new_index;

		TState GetSize() const { return new_index; }
		void Clear(TState ns)
		{
			P.resize(ns);
			state_to_partition.resize(ns);
			for(auto& i : P) i.clear();
			new_index = 0;
		}
	};

	std::string to_string(const NumericPartition& p) const
	{
		return to_string(p.P);
	}

	std::string to_string(const TSet& s) const
	{
		using namespace std;
		string str;
		str.append("{");
		int cont=0;
		for(auto i=s.GetIterator(); !i.IsEnd(); i.MoveNext())
		{
			if(cont++ > 0) str.append(", ");
			size_t st = static_cast<size_t>(i.GetCurrent());
			str.append(std::to_string(st));
		}
		str.append("}");
		return str;
	}

	/// Controls the debugging info output
	bool ShowConfiguration;

	MinimizationHopcroft()
		:ShowConfiguration(true)
	{
	}

	TDfa BuildDfa(const TDfa& dfa, const NumericPartition& partitions)
	{
		TDfa ndfa(dfa.GetAlphabetLength(), partitions.GetSize());
		for(TState i=0; i<partitions.GetSize(); i++)
		{
			auto& pi = partitions.P[i];
			assert(pi.size() > 0);
			for(auto is=pi.begin(); is!=pi.end(); is++)
			{
				const auto s = *is;
				for(TSymbol sym=0; sym<dfa.GetAlphabetLength(); sym++)
				{
					TState tgt = dfa.GetSuccessor(s, sym);
					TState ptgt = partitions.state_to_partition[tgt];
					ndfa.SetTransition(i, sym, ptgt);
				}
				if(dfa.IsInitial(s)) ndfa.SetInitial(i);
				if(dfa.IsFinal(s)) ndfa.SetFinal(i);
			}
		}
		return ndfa;
	}

	void Minimize(const TDfa& dfa, NumericPartition& np)
	{
		using namespace std;

		TState final_states_count = dfa.GetFinals().Count();
		TState non_final_states_count = dfa.GetStates() - final_states_count;

		// Un automata sin estado final?
		assert(final_states_count > 0);

		// Maximo puede exisitir una particion por cada estado, por ello reservamos de esta forma
		np.Clear(dfa.GetStates());

		// Inicializa funcion inversa para obtener la particion a la que pertenece un estado		

		auto it_f = back_inserter(np.P[0]);
		auto it_nf = back_inserter(np.P[1]);
		for(TState st=0; st<dfa.GetStates(); st++)
		{
			if(dfa.IsFinal(st)) *it_f++ = st;
			else *it_nf++ = st;
			np.state_to_partition[st] = dfa.IsFinal(st) ? 0 : 1;
		}

		// partitions count
		if(np.P[1].empty()) { np.new_index=1; return; }
		np.new_index = 2;

		if(ShowConfiguration)
		{
			cout << "Initial P=" << to_string(np) << endl;
		}

		TState min_initial_partition_index = final_states_count < non_final_states_count ? 0 : 1;

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

			// current splitter partition
			const auto& splitter_partition = np.P[splitter_set.GetCurrent()];

			if(ShowConfiguration)
			{
				cout << "Spliter=" << to_string(splitter_partition) << endl;
			}

			// Per symbol loop
			for(TSymbol splitter_letter=0; splitter_letter<dfa.GetAlphabetLength(); splitter_letter++)
			{								
				predecessors.Clear();
				bool was_split = false;

				// recorre elementos de la particion
				for(auto pii=splitter_partition.begin(); pii!=splitter_partition.end(); pii++)
				{
					const TState state = *pii;
					predecessors.UnionWith(dfa.GetPredecessors(state, splitter_letter));
				}

				if(ShowConfiguration)
				{
					cout << "delta_inv (" << static_cast<size_t>(splitter_letter) << ") = " << to_string(predecessors) << endl;
				}

				partitions_to_split.Clear();

				// let a=splitter_letter, B belongs P
				// O(card(a^{-1}.B))				
				for(auto ss=predecessors.GetIterator(); !ss.IsEnd(); ss.MoveNext())
				{
					// state ss belongs to partition indicated with partition_index
					TState partition_index = np.state_to_partition[ss.GetCurrent()];

					// Is this partition already processed?
					if(partitions_to_split.TestAndAdd(partition_index)) continue;

					auto& old_part = np.P[partition_index];

					// Imposible to divide a single state partition
					if(old_part.size() == 1) continue;

					auto& new_part = np.P[np.new_index];

					for(auto k=old_part.begin(); k!=old_part.end();)
					{
						// al menos una vez estara k en predecessors
						// al menos una vez, no se hara el interior de este if
						auto l = k++;
						if(!predecessors.Contains(*l))
						{
							np.state_to_partition[*l] = np.new_index;
							new_part.splice(new_part.end(), old_part, l);
						}
					}

					assert(old_part.size() > 0);

					// si no hubo nada que dividir
					if(new_part.size() == 0) continue;
					was_split = true;
					
					if(ShowConfiguration)
					{
						cout << "pred state=" << static_cast<size_t>(ss.GetCurrent()) << " in partition " << static_cast<size_t>(partition_index) << endl;
					}

					if(wait_set_membership.Contains(partition_index)) 
					{
						wait_set_membership.Add(np.new_index);						
					} 
					else 
					{
						auto add_index = old_part.size() < new_part.size() ? partition_index : np.new_index;
						wait_set_membership.Add(add_index);
						// wait_set_letter[add_index] = 0; // is already zero
					}

					// If we are here, split was done
					// new partition index must increment
					np.new_index++;
				}
				if(ShowConfiguration && was_split)
				{
					cout << "P=" << to_string(np) << endl;
					cout << "Waiting=" << to_string(wait_set_membership) << endl;
				}
			}
			// remove current
			wait_set_membership.Remove(splitter_set.GetCurrent());
		}		
		if(ShowConfiguration)
		{
			cout << "Final P=" << to_string(np) << endl;
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
