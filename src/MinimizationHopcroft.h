// June 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

// Workaround to allow use intrinsics
#ifdef BOOST_DYNAMIC_BITSET_DYNAMIC_BITSET_HPP
#error "Include this file first to allow BOOST_DYNAMIC_BITSET_DONT_USE_FRIENDS"
#endif
#define BOOST_DYNAMIC_BITSET_DONT_USE_FRIENDS
#include <boost/dynamic_bitset.hpp>

#include <stdint.h>
#include <assert.h>
#include <list>
#include <string>
#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <queue>
#include "Dfa.h"

template<typename Block, typename Allocator = std::allocator<Block>>
class dynamic_bitset : public boost::dynamic_bitset<Block, Allocator>
{
	typedef boost::dynamic_bitset<uint64_t, Allocator> __base;
public:
	size_type find_first() const { return __base::find_first(); }
	size_type find_next(size_type n) const { return __base::find_next(n);}
	bool test(size_type n) const { return __base::test(n); }
	dynamic_bitset& set(size_type n, bool val = true) { __base::set(n, val); return *this; }
	dynamic_bitset& reset(size_type n) { __base::reset(n); return *this; }

	dynamic_bitset(const dynamic_bitset& c) 
		: __base(c)
	{
	}	

	explicit dynamic_bitset(size_type num_bits, unsigned long value = 0, const Allocator& alloc = Allocator()) 
		: __base(num_bits, value, alloc)
	{
	}
};

#ifdef _MSC_VER  
// MSVC specific - intrinsics usage
// TODO: port to use GNU G++ intrinsics

template<typename Allocator>
class dynamic_bitset<uint64_t, Allocator> : public boost::dynamic_bitset<uint64_t, Allocator>
{
	typedef boost::dynamic_bitset<uint64_t, Allocator> __base;
public:
	dynamic_bitset(const dynamic_bitset& c) 
		: __base(c)
	{
	}

	explicit dynamic_bitset(size_type num_bits, unsigned long value = 0, const Allocator& alloc = Allocator()) 
		: __base(num_bits, value, alloc)
	{
	}

	bool test(size_type n) const
	{
		auto r = (const int64_t*)&m_bits[n/bits_per_block];
		auto idx = n%bits_per_block;
		return _bittest64(r, idx) != 0;
	}

	dynamic_bitset& set(size_type n, bool val = true)
	{
		auto r = (int64_t*)&m_bits[n/bits_per_block];
		auto idx = n%bits_per_block;
		if(val) _bittestandset64(r, idx);
		else _bittestandreset64(r, idx);
		return *this;
	}

	dynamic_bitset& reset(size_type n)
	{
		auto r = (int64_t*)&m_bits[n/bits_per_block];
		auto idx = n%bits_per_block;
		_bittestandreset64(r, idx);
		return *this;
	}

	dynamic_bitset& reset()
	{		
		__base::reset();
		return *this;
	}

	size_type find_first() const
	{		
		int c = 0;
		for(auto i=m_bits.begin(); i!=m_bits.end(); i++, c+=bits_per_block)
		{
			unsigned long l;
			if(_BitScanForward64(&l, *i) != 0) return l+c;
		}
		return npos;
	}

	size_type find_next(size_type t) const
	{
		const size_t offset_mask = bits_per_block-1;

		size_t s = t / bits_per_block;
		auto i = m_bits.begin()+s;

		size_t c = t & ~offset_mask;  // 111111110000
		size_t o = t & offset_mask; // 000000001111
		if(o == offset_mask) goto __other_block;				
		block_type ii = *i; 		
		block_type old_mask = ((block_type)(-1) << (o+1));
		ii = ii & old_mask;// removes previous bits
		//ii = _blsr_u64(ii); // avx2 version
		unsigned long l;
		if(_BitScanForward64(&l, ii) != 0) 
		{ 
			assert(l+c > t);
			return l+c; 
		}
__other_block:
		i++; c+=bits_per_block;
		for(; i!=m_bits.end(); i++, c+=bits_per_block)
		{		
			ii = *i;		
			if(_BitScanForward64(&l, ii) != 0)
			{
				assert(l+c > t);
				return l+c;		
			}
		}
		return npos;
	}

};

#endif//_MSC_VER  

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

	void Minimize2(const TDfa& dfa)
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
		vector<TStateSize> state_to_partition(dfa.GetStates());		

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
		dynamic_bitset<uint64_t> wait_set_membership(dfa.GetStates());
		wait_set_membership.set(min_initial_partition_index);

		// set containing the already processed partitions
		dynamic_bitset<uint64_t> partitions_to_split(dfa.GetStates());

		// conjunto de predecesores
		dynamic_bitset<uint64_t> predecessors(dfa.GetStates());

		// worst case is when WaitSet has one entry per state
		for(auto splitter_set=wait_set_membership.find_first(); splitter_set!=TSet::npos; splitter_set=wait_set_membership.find_first())
		{
			assert(new_index < dfa.GetStates());

			// remove current
			wait_set_membership.reset(splitter_set);

			// current splitter partition
			const auto& splitter_partition = P[splitter_set];

			if(ShowConfiguration)
			{				
				cout << "Spliter=" << to_string(splitter_partition, Pcontent) << endl;
			}

			// Per symbol loop
			for(TSymbol splitter_letter=0; splitter_letter<dfa.GetAlphabethLength(); splitter_letter++)
			{								
				predecessors.reset();

				// recorre elementos de la particion
				auto partition_it_begin = Pcontent.begin() + splitter_partition.first;
				auto partition_it_end = partition_it_begin + splitter_partition.second;
				for(; partition_it_begin != partition_it_end; partition_it_begin++)
				{
					TState state = *partition_it_begin;
					predecessors |= dfa.GetPredecessor(state, splitter_letter);
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
	}
};
