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

	class NumericPartition
	{
	public:
		std::vector<TState> Pcontent;
		TStateToPartition state_to_partition;
		TPartitionSet P;
		TState new_index;

		TState GetSize() const { return new_index; }
	};

private:

	TPairIndex GetPairIndex(TState states, TState p, TState q) const
	{
		return states*p + q;
	}

	std::tuple<TState,TState> GetPairFromIndex(TState states, TPairIndex index) const
	{
		return make_tuple
			(
			static_cast<TState>(index / states), 
			static_cast<TState>(index % states)
			);
	}

	std::tuple<TState,TState> NormalizedPair(TState p, TState q) const
	{
		return make_pair(std::min(p,q), std::max(p,q));
	}

	void Split(const TDfa& dfa, NumericPartition& part, TSymbol a) const
	{
		using namespace std;
		TPartitionSet::iterator min_part = part.P.begin();
		for(auto i=next(min_part); i!=part.P.end())
		{
			if(i->second < min_part->second) 
			{
				min_part = i;
			}
		}
		BitSet<TState> pred_states(part.GetSize());
		TState begin = part.Pcontent.begin() + min_part->first;
		TState end = begin + min_part->second;
		for(auto i=begin; i!=end; i++)
		{
			TState st = *i;
			const auto& pred_local = pred_states.GetSuccessors(st, a);
			pred_states.UnionWith(pred_local);
		}
		for(auto i=pred_states.GetIterator(); !i.IsEnd(); i.MoveNext())
		{

		}
	}

	bool EquivP(TState p, TState q, const TDfa& dfa, const NumericPartition& part, BitSet<TPairIndex>& neq, BitSet<TPairIndex>& equiv, BitSet<TPairIndex>& path, vector<TSymbol>& sym_stack)
	{
		using namespace std;
		if(ShowConfiguration) {
			cout << "Test equiv (" << static_cast<size_t>(p) << ", " << static_cast<size_t>(q) << ")" << endl;
		}
		TState states = dfa.GetStates();
		TPairIndex root_pair = GetPairIndex(states, p,q);
		if(neq.Contains(root_pair)) return false;
		if(path.Contains(root_pair)) return true;
		path.Add(root_pair);
		for(TSymbol a=0; a<dfa.GetAlphabetLength(); a++)
		{
			TState sp = dfa.GetSuccessor(p,a);
			TState sq = dfa.GetSuccessor(q,a);
			if(part.Find(sp) == part.Find(sq)) continue;
			if(sp > sq) std::swap(sp, sq);
			TPairIndex pair = GetPairIndex(states, sp,sq);
			if(!equiv.Contains(pair))
			{
				equiv.Add(pair);
				sym_stack.push_back(a);
				if(!EquivP(sp,sq, dfa, part, neq, equiv, path))
				{
					return false;
				}
				path.Remove(pair);
				sym_stack.pop_back();
			}
		}
		equiv.Add(root_pair);
		return true;
	}

public:

	bool ShowConfiguration;

	MinimizationHybrid() : ShowConfiguration(false)
	{
	}

	void Minimize(const TDfa& dfa, NumericPartition& part)
	{
		using namespace std;
		TState states = dfa.GetStates();
		part.Clear(states);
		BitSet<TPairIndex> neq(states*states);

		for(TState q=1; q<states; q++)
		{
			bool fq = dfa.IsFinal(q);
			for(TState p=0; p<q; p++)
			{
				bool fp = dfa.IsFinal(p);
				if((fp && !fq) || (!fp && fq))
				{
					neq.Add(GetPairIndex(states, p,q));
					if(ShowConfiguration) {
						cout << "neq add (" << static_cast<size_t>(p) << ", " << static_cast<size_t>(q) << ") -> " << static_cast<size_t>(GetPairIndex(states, p,q)) << endl;
					}
				}
			}
		}

		BitSet<TPairIndex> equiv(states*states);
		BitSet<TPairIndex> path(states*states);
		vector<TSymbol> sym_stack;

		for(TState current_part_idx=0; current_part_idx!=part.GetSize(); current_part_idx++)
		{
			auto& current_part_desc = part.P[current_part_idx];
			for(TState i_p=0; i_p<current_part_desc.second; i_p++)
			{		
				for(TState i_q=i_p+1; i_q<current_part_desc.second; i_q++)
				{
					TState p, q;
					p = part.P[i_p];
					q = part.P[i_q];
					tie(p,q) = NormalizedPair(p,q);

					assert(p != q);

					if(ShowConfiguration) {
						cout << "test (" << static_cast<size_t>(p) << ", " << static_cast<size_t>(q) << ") -> " << static_cast<size_t>(GetPairIndex(states, p,q)) << endl;
					}
					if(neq.Contains(GetPairIndex(states, p,q))) continue;
					if(part.Find(p) == part.Find(q)) continue;
					equiv.Clear();
					path.Clear();
					sym_stack.clear();

					const bool isEquiv = EquivP(p, q, dfa, part, neq, equiv, path, sym_stack);
					auto it = isEquiv ? equiv.GetIterator() : path.GetIterator();
					if(ShowConfiguration) {
						cout << (isEquiv ? "YES" : "NO") << endl;
					}
					if(!isEquiv) {
						for(; !it.IsEnd(); it.MoveNext())
						{
							TPairIndex idx = it.GetCurrent();
							TState p_prime, q_prime;
							tie(p_prime, q_prime) = GetPairFromIndex(states, idx);
							assert(p_prime < q_prime);
							if(ShowConfiguration) {
								cout << "neq (" << static_cast<size_t>(p_prime) << ", " << static_cast<size_t>(q_prime) << ")" << endl;
							}
							neq.Add(idx); // p_prime, q_prime					
						}
					}
					else 
					{
						while(!sym_stack.empty())
						{
							TSymbol a = sym_stack.pop_back();
							Split(part, a);
						}
					}
				}
			}
			if(ShowConfiguration)
			{
				cout << "Finished " << part.GetSize() << " states of " << dfa.GetStates() << endl;
			}
		}

		TDfa BuildDfa(const TDfa& dfa, NumericPartition& seq)
		{
			seq.Compact();
			TDfa dfa_min(dfa.GetAlphabetLength(), seq.GetSize());
			for(auto k=dfa.GetInitials().GetIterator(); !k.IsEnd(); k.MoveNext())
			{
				TState initial_state = k.GetCurrent();
				initial_state = seq.Find(initial_state);
				dfa_min.SetInitial(initial_state);
			}
			for(auto k=dfa.GetFinals().GetIterator(); !k.IsEnd(); k.MoveNext())
			{
				TState final_state = k.GetCurrent();
				final_state = seq.Find(final_state);
				dfa_min.SetFinal(final_state);
			}

			for(auto i : seq.GetStore())
			{
				if(i.empty()) continue;
				TState src = *i.begin();
				TState src_c = seq.Find(src);
				for(TSymbol a=0; a<dfa.GetAlphabetLength(); a++)
				{
					TState tgt = dfa.GetSuccessor(src, a);
					TState tgt_c = seq.Find(tgt);
					dfa_min.SetTransition(src_c, a, tgt_c);
				}			
			}
			return dfa_min;
		}

		TDfa Minimize(const TDfa& dfa)
		{
			NumericPartition part;
			Minimize(dfa, part);
			TDfa dfa_min = BuildDfa(dfa, part);
			return dfa_min;
		}
	};
