// July 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

// Workaround to allow use intrinsics
#ifdef BOOST_DYNAMIC_BITSET_DYNAMIC_BITSET_HPP
#error "Include this file first to allow BOOST_DYNAMIC_BITSET_DONT_USE_FRIENDS"
#endif
#define BOOST_DYNAMIC_BITSET_DONT_USE_FRIENDS
#include <boost/dynamic_bitset.hpp>

#include <stdint.h>

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