// Septiembre 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include <vector>
#include <algorithm>
#include "Set.h"

// Incremental Almeida et al. Minimization Algorithm.
template<typename _TDfa>
class MinimizationHybrid
{
public:
	typedef _TDfa TDfa;
	typedef typename TDfa::TState TState;
	typedef typename TDfa::TSymbol TSymbol;
	typedef uint64_t TPairIndex;
	typedef std::tuple<TState,TState,TSymbol> TSplitter;
	typedef std::vector<TState> TStateToPartition;
	typedef std::vector<std::list<TState>> TPartitionSet;

	// TODO: More partition details isolated from algoritmhs
	// in order to not expose internal members
	class NumericPartition
	{
	public:
		TStateToPartition state_to_partition;
		TState new_index;
		TPartitionSet P;		

		TState GetSize() const { return new_index; }

		void Clear(TState states)
		{
			state_to_partition.resize(states);
			P.resize(states);
			for(auto i=P.begin(); i!=P.end(); i++) i->clear();
			new_index = 0;
		}

		TState Find(TState st) const
		{
			return state_to_partition[st];
		}

		std::list<TState>& GetPartition(TState st)
		{
			return P[st];
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

	// Try splits all partition using the splitter indicated by partition index and letter.
	// If split is performed on specific partition, it modifies two iterators in order to be able to continue
	// iteration on next item inside original partition
	void Split(const TDfa& dfa, NumericPartition& part, TState splitter_partition_idx, TSymbol splitter_letter, typename std::vector<std::list<TState>>::iterator cur_part, typename std::list<TState>::iterator& i_p, typename std::list<TState>::iterator& i_q) const
	{
		using namespace std;

		// calcula d_inverse para el conjunto de estados y letra indicado
		BitSet<TState> pred_states(dfa.GetStates());
		BitSet<TState> block_split(dfa.GetStates());
		auto& splitter_part = part.GetPartition(splitter_partition_idx);
		for(auto i=splitter_part.begin(); i!=splitter_part.end(); i++)
		{
			TState st = *i;
			const auto& pred_local = dfa.GetPredecessors(st, splitter_letter);
			pred_states.UnionWith(pred_local);
		}
		if(ShowConfiguration) 
		{
			cout << "split " << to_string(splitter_part) << "|" << static_cast<size_t>(splitter_letter);
			cout << " <- " << to_string(pred_states) << endl;
		}

		auto old_part_begin = part.P.begin();
		auto old_part_end = next(old_part_begin, part.new_index);
		// we ignore new partitions (new ones created inside this loop)
		for(auto j=pred_states.GetIterator(); !j.IsEnd(); j.MoveNext())
		{
			TState st = j.GetCurrent();
			TState block = part.Find(st);
			if(block_split.TestAndAdd(block)) continue;

			auto& old_part = part.P[block];

			assert(old_part.size() > 0);
			if(old_part.size() < 2) continue;

			auto& new_part = part.P[part.new_index];

			for(auto j=old_part.begin(); j!=old_part.end(); )
			{
				auto k = j++;
				// el bloque al menos contiene una vez un miembro de pred_states
				if(!pred_states.Contains(*k))
				{
					// Q necesita estar por delante de P					
					if(part.P.begin()+block == cur_part)
					{
						if(k == i_p)
						{
							i_p = j;
							if(i_p != cur_part->end()) i_q = next(i_p, 1);
						}
						else if(k == i_q)
						{
							i_q = j;
							if(i_q == cur_part->end()) 
							{
								i_p++;
								if(i_p == k) i_p = j; // end
								else {
									i_q = next(i_p, 1);
									if(i_q == k){
										i_q = j; // end
										i_p = j; // end
									}
								}
							}
						}
						assert(i_p != k);
						assert(i_q != k);
					}
					part.state_to_partition[*k] = part.new_index;
					new_part.splice(new_part.end(), old_part, k);

					// p -> q  === !p || q
					assert( !(i_p == i_q) || (i_p == cur_part->end() && i_q == cur_part->end()) );
					assert( !(i_p != i_q) || (distance(i_p, i_q) > 0) );
				}
			}
			
			assert(old_part.size() > 0);
			
			// If we are here, split was done
			// new partition index must increment
			if(new_part.size() > 0)	part.new_index++;
		}
	}

	bool EquivP(TState p, TState q, const TDfa& dfa, const NumericPartition& part, BitSet<TPairIndex>& path, BitSet<TPairIndex>& equiv, std::vector<TSplitter>& splitter_stack)
	{
		using namespace std;
		if(ShowConfiguration) 
		{
			cout << "Test equiv (" << static_cast<size_t>(p) << ", " << static_cast<size_t>(q) << ")" << endl;
		}
		if(part.Find(p) != part.Find(q)) return false;
		TPairIndex root_pair = GetPairIndex(p,q);
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
			if(!EquivP(sp,sq, dfa, part, path, equiv, splitter_stack))
			{
				return false;
			}
			splitter_stack.pop_back();
			path.Remove(pair);
		}

		return true;
	}

public:

	std::string to_string(const std::list<TState>& part) const
	{
		using namespace std;
		string str;
		str.append("{");
		TState cnt = 0;
		for(auto i : part)
		{
			if(cnt++ > 0) str += ", ";
			str.append(std::to_string(static_cast<size_t>(i)));
		}
		str.append("}");
		return str;
	}

	std::string to_string(const BitSet<TState>& c) const
	{
		using namespace std;
		string str;
		str.append("{");
		int cont=0;
		for(auto i=c.GetIterator(); !i.IsEnd(); i.MoveNext())
		{
			if(cont++ > 0) str.append(", ");
			auto s = static_cast<size_t>(i.GetCurrent());
			str.append(std::to_string(s));
		}
		str.append("}");
		return str;
	}

	

	std::string to_string(const NumericPartition& P) const
	{
		using namespace std;
		string str;
		TState cnt = 0;
		str.append("{");
		for(TState cnt=0; cnt<P.GetSize(); cnt++)
		{
			if(cnt > 0) str.append(", ");
			str.append(to_string(P.P[cnt]));
		}
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

		TState states = dfa.GetStates();
		part.Clear(states);

		// Inicializa funcion inversa para obtener la particion a la que pertenece un estado
		auto it_f = back_inserter(part.P[0]), it_nf = back_inserter(part.P[1]);
		for(TState st=0; st<dfa.GetStates(); st++)
		{
			auto f = dfa.IsFinal(st);
			if(f) *it_f++ = st;	else *it_nf++ = st;
			part.state_to_partition[st] = f ? 0 : 1;
		}
		// partitions count, are all finals?
		if(part.P[1].empty()) { part.new_index = 1; return; }
		part.new_index = 2;

		BitSet<TPairIndex> path((states*states-states)/2);
		BitSet<TPairIndex> equiv((states*states-states)/2);
		vector<TSplitter> splitter_stack;

		for(auto cur_part=part.P.begin(); cur_part!=next(part.P.begin(), part.new_index); cur_part++)
		{
			assert(cur_part->size() > 0);
			if(cur_part->size() < 2) continue;

			auto i_p = cur_part->begin();
			auto i_q = next(i_p, 1);
			while(i_p != cur_part->end() && i_q != cur_part->end())
			{
				if(ShowConfiguration)
				{
					cout << "pair: " << static_cast<size_t>(*i_p) << ", " << static_cast<size_t>(*i_q) << endl;
				}

				assert(distance(i_p, i_q) > 0);

				TState p, q;
				tie(p,q) = NormalizedPair(*i_p, *i_q);

				assert(p < q);
				assert(dfa.IsFinal(p) == dfa.IsFinal(q));
				assert(part.Find(p) == part.Find(q));				

				path.Clear();
				equiv.Clear();
				splitter_stack.clear();

				bool isEquiv = EquivP(p, q, dfa, part, path, equiv, splitter_stack);
				if(ShowConfiguration)
				{
					cout << (isEquiv ? "YES" : "NO") << endl;
				}
				if(isEquiv) 
				{					
					i_q++;
					if(i_q == cur_part->end()) 
					{
						i_p++;
						i_q = next(i_p, 1);
					}
					continue;
				}

				while(!splitter_stack.empty())
				{
					TSplitter s = splitter_stack.back();							
					TState s1 = get<0>(s);
					TState s2 = get<1>(s);
					TSymbol a = get<2>(s);

					s1 = part.Find(s1);
					s2 = part.Find(s2);

					TState min_part;
					if(part.P[s1].size() < part.P[s2].size())
					{
						min_part = s1;
					} else {
						min_part = s2;
					}

					Split(dfa, part, min_part, a, cur_part, i_p, i_q);
					splitter_stack.pop_back();
					if(ShowConfiguration)
					{
						cout << "P=" << to_string(part) << endl;
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
		for(const auto& p : partitions.P)
		{
			for(TState s : p)
			{
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