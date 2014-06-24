// Septiembre 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include <vector>
#include <list>
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
	typedef std::tuple<TState, TState> TStatePair;
	typedef std::tuple<TState, TState, TSymbol> TSplitter;
	typedef std::vector<TState> TStateToPartition;
	typedef std::vector<std::list<TState>> TPartitionSet;
	typedef std::vector<std::vector<TSplitter>> TDelta;

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
			for (auto i = P.begin(); i != P.end(); i++) i->clear();
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
		return (q*q - q) / 2 + p;
	}

	std::tuple<TState, TState> GetPairFromIndex(TPairIndex index) const
	{
		TState q = static_cast<TState>(sqrt((1 + 8 * index) / 4.0f) + 0.5f);
		TState p = static_cast<TState>(index - (q*q - q) / 2);
		assert(p < q);
		assert(index == GetPairIndex(p, q));

		return make_tuple(p, q);
	}

	std::tuple<TState, TState> NormalizedPair(TState p, TState q) const
	{
		using namespace std;
		return make_pair(min(p, q), max(p, q));
	}

	TState Merge(NumericPartition& part, TState p, TState q) const
	{
		using namespace std;
		TState cp = part.Find(p);
		TState cq = part.Find(q);
		TState t = cp < cq ? cp : cq;
		TState s = cp < cq ? cq : cp;
		auto& pt = part.GetPartition(t);
		auto& ps = part.GetPartition(s);
		for (auto i = ps.begin(); i != ps.end(); i++)
		{
			part.state_to_partition[*i] = t;
		}
		pt.splice(pt.begin(), ps);
		if (ShowConfiguration) cout << "merged blocks for states " << p << " & " << q << endl;
		return t;
	}

	// Try splits all block partition using the splitter indicated by partition index and letter.
	// If split is performed on specific block, it modifies two iterators in order to be able to continue
	// iteration on next item inside original partition
	void Split(const TDfa& dfa, NumericPartition& part, TState splitter_partition_idx, TSymbol splitter_letter, typename std::vector<std::list<TState>>::iterator cur_part, typename std::list<TState>::iterator& i_p, typename std::list<TState>::iterator& i_q, bool* advanced) const
	{
		using namespace std;

		// calcula d_inverse para el conjunto de estados y letra indicado
		BitSet<TState> pred_states(dfa.GetStates());
		BitSet<TState> block_split(dfa.GetStates());
		auto& splitter_part = part.GetPartition(splitter_partition_idx);
		for (auto i = splitter_part.begin(); i != splitter_part.end(); i++)
		{
			TState st = *i;
			const auto& pred_local = dfa.GetPredecessors(st, splitter_letter);
			pred_states.UnionWith(pred_local);
		}
		if (ShowConfiguration)
		{
			cout << "split " << to_string(splitter_part) << "|" << static_cast<size_t>(splitter_letter);
			cout << " <- " << to_string(pred_states) << endl;
		}

		auto old_part_begin = part.P.begin();
		auto old_part_end = next(old_part_begin, part.new_index);
		// we ignore new partitions (new ones created inside this loop)
		for (auto j = pred_states.GetIterator(); !j.IsEnd(); j.MoveNext())
		{
			TState st = j.GetCurrent();
			TState block = part.Find(st);
			if (block_split.TestAndAdd(block)) continue;

			auto& old_part = part.P[block];

			assert(old_part.size() > 0);
			if (old_part.size() < 2) continue;

			auto& new_part = part.P[part.new_index];

			for (auto j = old_part.begin(); j != old_part.end();)
			{
				auto k = j++;
				// el bloque al menos contiene una vez un miembro de pred_states
				if (!pred_states.Contains(*k))
				{
					// Q necesita estar por delante de P					
					if (part.P.begin() + block == cur_part)
					{
						if (k == i_p)
						{
							i_p = j;
							if (i_p != cur_part->end()) i_q = next(i_p, 1);
							*advanced = true;
						}
						else if (k == i_q)
						{
							i_q = j;
							*advanced = true;
						}
						assert(i_p != k);
						assert(i_q != k);
					}
					part.state_to_partition[*k] = part.new_index;
					new_part.splice(new_part.end(), old_part, k);

					// p -> q  === !p || q
					assert(!(i_p == i_q) || (i_p == cur_part->end() && i_q == cur_part->end()));
					assert(!(i_p != i_q) || (distance(i_p, i_q) > 0));
				}
			}

			assert(old_part.size() > 0);

			// If we are here, split was done
			// new partition index must increment
			if (new_part.size() > 0)	part.new_index++;
		}
	}

	// False
	// True, return eq
	bool AreEquivalent(TState p, TState q,
		const TDfa& dfa,
		const NumericPartition& pi, const NumericPartition& ro,
		std::vector<TStatePair>& expl,
		BitSet<TPairIndex>& states,
		TDelta& delta,
		TStatePair* out_pair)
	{
		assert(p < q);
		using namespace std;

		states.Add(GetPairIndex(p, q));
				
		expl.clear();
		expl.push_back(make_tuple(p, q));
		
		while (!expl.empty()) 
		{
			TState p1, q1;
			tie(p1, q1) = expl.back();
			expl.pop_back();
			
			for (TSymbol a = 0; a < dfa.GetAlphabetLength(); a++)
			{
				if (ShowConfiguration) cout << "Test equiv ((" << p << ", " << q << "), " << a << ")";

				TState p2 = dfa.GetSuccessor(p1, a); TState q2 = dfa.GetSuccessor(q1, a);

				if (ShowConfiguration) cout << " => (" << p2 << ", " << q2 << ")";

				tie(p2, q2) = NormalizedPair(p2, q2);

				TState ro_p = ro.Find(p2); TState ro_q = ro.Find(q2);

				if (ShowConfiguration) cout << " rho blks: " << ro_p << ", " << ro_q << endl;

				if (ro_p == ro_q) continue;

				auto p2_q2 = GetPairIndex(p2, q2);
				states.Add(p2_q2);

				auto& col = delta[p2_q2];
				bool found = false;
				for (auto r : col)
				{
					found = get<0>(r) == p1 && get<1>(r) == q1;
					if (found) break;
				}
				if (!found) col.push_back(make_tuple(p1, q1, a));

				if (pi.Find(p2) != pi.Find(q2)) 
				{
					*out_pair = make_tuple(p2, q2);
					return false;
				}
				if (ro.Find(p2) != ro.Find(q2))
				{
					expl.push_back(make_tuple(p2, q2)); // ojo, muchas veces?
				}
			}
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
		for (auto i : part)
		{
			if (cnt++ > 0) str += ", ";
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
		int cont = 0;
		for (auto i = c.GetIterator(); !i.IsEnd(); i.MoveNext())
		{
			if (cont++ > 0) str.append(", ");
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
		for (TState cnt = 0; cnt<P.GetSize(); cnt++)
		{
			if (cnt > 0) str.append(", ");
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
		NumericPartition ro;
		ro.Clear(states);

		// Inicializa funcion inversa para obtener la particion a la que pertenece un estado
		const int finalsBlock = 1;
		const int nonFinalsBlock = 0;
		assert(finalsBlock != nonFinalsBlock);
		auto it_f = back_inserter(part.P[finalsBlock]), it_nf = back_inserter(part.P[nonFinalsBlock]);
		for (TState st = 0; st < states; st++)
		{
			auto f = dfa.IsFinal(st);
			if (f) *it_f++ = st;	else *it_nf++ = st;
			part.state_to_partition[st] = f ? finalsBlock : nonFinalsBlock;

			ro.state_to_partition[st] = st;
			ro.P[st].push_back(st);
		}
		// partitions count, are all finals?
		part.new_index = 0;
		ro.new_index = states;
		if (!part.P[1].empty()) part.new_index++;
		if (!part.P[0].empty()) part.new_index++;
		else {
			swap(part.P[0], part.P[1]);
			replace(part.state_to_partition.begin(), part.state_to_partition.end(), 1, 0);
		}

		// El numero de pares de estados sin repetir es el
		// numero de componentes en una matriz triangular
		// El area de un triangulo
		BitSet<TPairIndex> tocheck((states*states - states) / 2);
		vector<TStatePair> expl;
		vector<TStatePair> todolist;
		expl.reserve((states*states - states) / 2);		
		todolist.reserve((states*states - states) / 2);
		TDelta delta((states*states - states) / 2, vector<TSplitter>());

		for (auto cur_part = part.P.begin(); cur_part != next(part.P.begin(), part.new_index); cur_part++)
		{
			assert(cur_part->size() > 0);
			if (cur_part->size() < 2) continue;

			auto i_p = cur_part->begin();
			auto i_q = next(i_p, 1);
			while (i_p != cur_part->end() && i_q != cur_part->end())
			{
				assert(distance(i_p, i_q) > 0);

				TState p, q;
				tie(p, q) = NormalizedPair(*i_p, *i_q);

				assert(p < q);
				assert(dfa.IsFinal(p) == dfa.IsFinal(q));
				assert(part.Find(p) == part.Find(q));
				
				bool splitAdvancedIterators = false;
				
				if (ShowConfiguration)
				{
					cout << "pair: " << *i_p << ", " << *i_q << endl;
					cout << "Pi=" << to_string(part) << endl;
					cout << "Rho=" << to_string(ro) << endl;
				}
								
				todolist.clear();
				tocheck.Clear();
				for (auto& v : delta) v.clear();

				TStatePair neq_pair;
				bool isEquiv = AreEquivalent(p, q, dfa, part, ro, expl, tocheck, delta, &neq_pair);
				if (!isEquiv)
				{
					todolist.push_back(neq_pair);
					while (!todolist.empty())
					{
						TState p1, q1; tie(p1, q1) = todolist.back();
						todolist.pop_back();
						auto p1_q1_idx = GetPairIndex(p1, q1);
						tocheck.Add(p1_q1_idx);
						for (TSplitter s : delta[p1_q1_idx])
						{
							TState p2, q2; TSymbol a;
							tie(p2, q2, a) = s;
							TPairIndex p2_q2_idx = GetPairIndex(p2, q2);
							if (!tocheck.Contains(p2_q2_idx)) continue;
							if (part.Find(p2) != part.Find(q2))
							{
								TState b_p2 = part.Find(p2); TState b_q2 = part.Find(q2);
								TState min_part = part.GetPartition(b_p2).size() < part.GetPartition(b_q2).size() ? b_p2 : b_q2;
								Split(dfa, part, min_part, a, cur_part, i_p, i_q, &splitAdvancedIterators);
							}
							todolist.push_back(make_tuple(p2, q2));
						}
					}
				}
				for (auto it = tocheck.GetIterator(); !it.IsEnd(); it.MoveNext())
				{
					// merge equivalent states
					TState p1, q1; tie(p1, q1) = GetPairFromIndex(it.GetCurrent());
					TState b_p1 = ro.Find(p1); TState b_q1 = ro.Find(q1);
					Merge(ro, b_p1, b_q1);
					if (ShowConfiguration) cout << "Rho=" << to_string(ro) << endl;
				}
				// advance logic
				if (!splitAdvancedIterators) i_q++;
				if (i_q == cur_part->end()) // advance then is in the end?
				{
					i_q = ++i_p; // advance p
					if (i_p != cur_part->end()) // p is safe?
					{
						i_q++; // set q to next one
					}
				}
			}
		}
		if (ShowConfiguration)
		{
			cout << "Finished " << part.GetSize() << " states of " << dfa.GetStates() << endl;
		}
	}

	TDfa BuildDfa(const TDfa& dfa, const NumericPartition& partitions)
	{
		TDfa ndfa(dfa.GetAlphabetLength(), partitions.GetSize());
		int pidx = 0;
		for (const auto& p : partitions.P)
		{
			for (TState s : p)
			{
				for (TSymbol sym = 0; sym < dfa.GetAlphabetLength(); sym++)
				{
					TState tgt = dfa.GetSuccessor(s, sym);
					TState ptgt = partitions.state_to_partition[tgt];
					ndfa.SetTransition(pidx, sym, ptgt);
				}
				if (dfa.IsInitial(s)) ndfa.SetInitial(pidx);
				if (dfa.IsFinal(s)) ndfa.SetFinal(pidx);
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
