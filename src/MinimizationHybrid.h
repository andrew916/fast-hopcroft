// Septiembre 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include <vector>
#include <algorithm>
#include "Set.h"

// Incremental Almeida et al. Minimization Algorithm.
template<typename TDfa>
class MinimizationHybrid
{
public:
	typedef TDfa TDfa;
	typedef typename TDfa::TState TState;
	typedef typename TDfa::TSymbol TSymbol;
	typedef uint64_t TPairIndex;
	typedef std::tuple<TState,TState,TSymbol> TSplitter;
	typedef std::vector<TState> TStateToPartition;
	typedef std::vector<std::pair<TState,TState>> TPartitionSet;

	class NumericPartition
	{
	public:
		std::vector<TState> Pcontent;
		TStateToPartition state_to_partition;
		TPartitionSet P;
		TState new_index;

		TState GetSize() const { return new_index; }

		void Clear(TState states)
		{
			Pcontent.resize(states);
			state_to_partition.resize(states);
			P.resize(states);
			new_index = 0;
		}

		TState Find(TState st) const
		{
			return state_to_partition[st];
		}
	};

private:

	TPairIndex GetPairIndex(TState p, TState q) const
	{
		assert(p < q);
		return (q*q-q)/2+p;
	}

	std::tuple<TState,TState> GetPairFromIndex(TPairIndex index) const
	{
		TState q = static_cast<TState>(sqrt((1+8*index)/4.0f) + 0.5f);
		TState p = static_cast<TState>(index - (q*q-q)/2);
		assert(p < q);
		assert(index == GetPairIndex(p,q));

		return make_tuple(p, q);
	}

	std::tuple<TState,TState> NormalizedPair(TState p, TState q) const
	{
		return make_pair(std::min(p,q), std::max(p,q));
	}

	void Split(const TDfa& dfa, NumericPartition& part, TState splitter_partition, TSymbol splitter_letter) const
	{
		using namespace std;

		// calcula d_inverse para el conjunto de estados y letra indicado
		BitSet<TState> pred_states(dfa.GetStates());
		auto begin = next(part.Pcontent.begin(), part.P[splitter_partition].first);
		auto end = next(begin, part.P[splitter_partition].second);
		if(ShowConfiguration) cout << "(min){";
		for(auto i=begin; i!=end; i++)
		{
			TState st = *i;
			if(ShowConfiguration)
			{
				if(i!=begin) cout << ", ";
				cout << static_cast<size_t>(st);
			}
			const auto& pred_local = dfa.GetPredecessors(st, splitter_letter);
			pred_states.UnionWith(pred_local);
		}
		if(ShowConfiguration) 
		{
			cout << "} / " << static_cast<size_t>(splitter_letter) << " <- ";
			cout << "{";
			int cont = 0;
			for(auto bi=pred_states.GetIterator(); !bi.IsEnd(); bi.MoveNext())
			{
				if(cont++ > 0) cout << ", ";
				cout << static_cast<size_t>(bi.GetCurrent());
			}
			cout << "}" << endl;
		}		

		auto part_desc_begin = part.P.begin();
		auto part_desc_end = next(part_desc_begin, part.new_index);
		// we ignore new partitions (new ones created inside this loop)
		for(auto i=part_desc_begin; i!=part_desc_end; i++)
		{
			auto& partition_desc = *i;
			TState partition_size = partition_desc.second;
			assert(partition_size > 0);
			if(partition_size < 2) continue;
			const auto part_begin_const = next(part.Pcontent.begin(), partition_desc.first);
			auto part_begin = part_begin_const;
			auto part_end = next(part_begin, partition_size - 1);
			do
			{
				TState state = *part_begin;
				if(pred_states.Contains(state))
				{
					part_begin++;
				} else {
					iter_swap(part_begin, part_end);
					part.state_to_partition[state] = part.new_index;
					part_end--;
				}
			} while(part_begin != part_end);
			// last element remains without class
			{
				TState state = *part_begin;
				if(pred_states.Contains(state))
				{
					part_begin++;
				} else {
					part.state_to_partition[state] = part.new_index;
				}
			}

			// old partition new size
			TState split_size = static_cast<TState>(distance(part_begin_const, part_begin));

			// continue if was not division
			if(split_size == partition_size) continue;

			// new parition size
			TState split_complement_size = partition_size - split_size;

			// restore partition indices
			if(split_size == 0)
			{
				TState cur = static_cast<TState>(distance(part_desc_begin, i));
				for(part_begin=part_begin_const; part_begin != part_end; part_begin++)
				{
					part.state_to_partition[*part_begin] = cur;
				}
				continue;
			}

			partition_desc.second = split_size;

			// confirm descriptor for new partition
			auto new_partition_desc_it = next(part.P.begin(), part.new_index);
			new_partition_desc_it->first = partition_desc.first + split_size;
			new_partition_desc_it->second = split_complement_size;

			// If we are here, split was done
			// new partition index must increment
			part.new_index++;
		}
	}

	bool EquivP(TState p, TState q, const TDfa& dfa, const NumericPartition& part, const BitSet<TPairIndex>& neq, BitSet<TPairIndex>& path, BitSet<TPairIndex>& equiv, std::vector<TSplitter>& splitter_stack)
	{
		using namespace std;
		if(ShowConfiguration) 
		{
			cout << "Test equiv (" << static_cast<size_t>(p) << ", " << static_cast<size_t>(q) << ")" << endl;
		}
		if(part.Find(p) != part.Find(q)) return false;
		TPairIndex root_pair = GetPairIndex(p,q);
		if(neq.Contains(root_pair)) return false;
		if(path.TestAndAdd(root_pair)) return true;		
		equiv.Add(root_pair);
		for(TSymbol a=0; a<dfa.GetAlphabetLength(); a++)
		{
			TState sp = dfa.GetSuccessor(p,a);
			TState sq = dfa.GetSuccessor(q,a);
			if(sp > sq) std::swap(sp, sq);
			if(sp == sq) continue;
			TPairIndex pair = GetPairIndex(sp,sq);
			if(equiv.Contains(pair)) continue;
			splitter_stack.push_back(make_tuple(sp, sq, a));
			if(!EquivP(sp,sq, dfa, part, neq, path, equiv, splitter_stack))
			{
				return false;
			}
			splitter_stack.pop_back();
			path.Remove(pair);
		}
		
		return true;
	}

public:

	std::string to_string(const typename TPartitionSet::value_type& P, const std::vector<TState>& Pcontent)
	{
		using namespace std;
		string str;
		str.append("{");
		for(TState j=P.first, cnt=0; cnt<P.second; j++, cnt++)
		{
			if(cnt > 0) str += ", ";
			str.append(std::to_string((size_t)Pcontent[j]));
		}
		str.append("}");
		return str;
	}

	std::string to_string(const TPartitionSet& p, TState size, const std::vector<TState>& Pcontent)
	{
		using namespace std;
		string str;
		int cnt = 0;
		str.append("{");
		for_each(p.cbegin(), p.cbegin()+size, [&](const typename TPartitionSet::value_type& x)
		{
			if(cnt++ > 0) str.append(", ");
			str.append(to_string(x, Pcontent));
		});
		str.append("}");
		return str;
	}

	bool ShowConfiguration;

	MinimizationHybrid() : ShowConfiguration(false)
	{
	}

	void Minimize(const TDfa& dfa, NumericPartition& part)
	{
		using namespace std;
		TState final_states_count = dfa.GetFinals().Count();
		TState non_final_states_count = dfa.GetStates() - final_states_count;
		TState states = dfa.GetStates();
		part.Clear(states);

		part.P[0].first = 0;
		part.P[0].second = final_states_count;

		part.P[1].first = final_states_count;
		part.P[1].second = non_final_states_count;

		// Inicializa funcion inversa para obtener la particion a la que pertenece un estado
		auto it_f = part.Pcontent.begin();
		auto it_nf = part.Pcontent.rbegin();
		for(TState st=0; st<dfa.GetStates(); st++)
		{
			if(dfa.IsFinal(st)) *it_f++ = st;
			else *it_nf++ = st;
			part.state_to_partition[st] = dfa.IsFinal(st) ? 0 : 1;
		}
		// partitions count
		part.new_index = 2;

		BitSet<TPairIndex> neq((states*states-states)/2);
		BitSet<TPairIndex> path((states*states-states)/2);
		BitSet<TPairIndex> equiv((states*states-states)/2);
		vector<TSplitter> splitter_stack;

		for(TState current_part_idx=0; current_part_idx!=part.GetSize(); current_part_idx++)
		{
			auto& current_part_desc = part.P[current_part_idx];
			if(current_part_desc.second < 2) continue;
			for(TState i_p=0; i_p<current_part_desc.second; i_p++)
			{		
				for(TState i_q=i_p+1; i_q<current_part_desc.second; i_q++)
				{
					TState p, q;
					p = part.Pcontent[i_p + current_part_desc.first];
					q = part.Pcontent[i_q + current_part_desc.first];
					tie(p,q) = NormalizedPair(p,q);

					assert(p < q);
					assert(dfa.IsFinal(p) == dfa.IsFinal(q));
					assert(part.Find(p) == part.Find(q));

					if(neq.Contains(GetPairIndex(p,q)))
					{
						continue;
					}

					path.Clear();
					equiv.Clear();
					splitter_stack.clear();

					bool isEquiv = EquivP(p, q, dfa, part, neq, path, equiv, splitter_stack);
					if(ShowConfiguration)
					{
						cout << (isEquiv ? "YES" : "NO") << endl;
					}
					if(!isEquiv) 
					{
						neq.UnionWith(path);
						while(!splitter_stack.empty())
						{
							TSplitter s = splitter_stack.back();							
							TState s1 = get<0>(s);
							TState s2 = get<1>(s);
							TSymbol a = get<2>(s);

							s1 = part.Find(s1);
							s2 = part.Find(s2);

							TState min_part;
							if(part.P[s1].second < part.P[s2].second)
							{
								min_part = s1;
							} else {
								min_part = s2;
							}

							Split(dfa, part, min_part, a);
							splitter_stack.pop_back();
							if(ShowConfiguration)
							{
								cout << "splitter a=" << static_cast<size_t>(a) << ", " << to_string(part.P, part.GetSize(), part.Pcontent) << endl;
							}
						}
					}
				}
			}
		}
		if(ShowConfiguration)
		{
			cout << "Finished " << part.GetSize() << " states of " << dfa.GetStates() << endl;
		}
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

	TDfa Minimize(const TDfa& dfa)
	{
		NumericPartition part;
		Minimize(dfa, part);
		TDfa dfa_min = BuildDfa(dfa, part);
		return dfa_min;
	}
};
