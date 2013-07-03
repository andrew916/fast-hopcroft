// June 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include <stdint.h>
#include <assert.h>
#include <list>
#include <string>
#include <algorithm>
#include <iostream>

/// Hopcroft's DFA Minimization Algorithm.
template<class TState, class TSymbol, class TToken = uint64_t>
class MinimizationHopcroft
{	
public:
	typedef Dfa<TState, TSymbol, TToken> TDfa;
	typedef BitSet<TState, TToken> TSet;
	typedef std::list<TSet> TPartition;

private:

	/// Extract one member of partition.
	/// O(1)
	TSet ExtractOne(TPartition& partition)
	{
		auto r = partition.front();
		partition.pop_front();
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
			str.append(std::to_string(i));
			return true;
		});
		str.append("}");
		return str;
	}

	/// Util to get a printable string representing the partition
	std::string PartitionToString(const TPartition& p)
	{
		std::string str;
		int cnt = 0;
		str.append("{");
		std::for_each(p.cbegin(), p.cend(), [&](const TSet& x)
		{
			if(cnt++ > 0) str.append(", ");			
			str.append(SetToString(x));			
		});
		str.append("}");
		return str;
	}

	/// Split operation, Refine B' using the Splitter (B, a) resulting B1 and B2.
	/// <math>(B1, B2) = B' \ (B,a)</math>
	void Split(const TDfa& dfa, TSet& Bc, TSet& B, TSymbol a, TSet& B1, TSet& B2, int* countB1, int* countB2)
	{
		*countB1 = 0;
		*countB2 = 0;
		// En 'B1' los estados de 'Bc' que llegan a 'B' consumiendo 'a'
		// O(N)
		Bc.ForEachMember([&, a](TState q)
		{
			// O(1)
			auto suc = dfa.GetSucessor(q, a);
			// O(1)
			if(B.Contains(suc))
			{
				// O(1)
				B1.Add(q);
				(*countB1)++;
			} else {
				// O(1)
				B2.Add(q);
				(*countB2)++;
			}
			return true;
		});
	}

public:

	/// Controls the debugging info output
	bool ShowConfiguration;

	MinimizationHopcroft()
		:ShowConfiguration(true)
	{
	}
	
	/// Entry point to minimize any DFA
	void Minimize(const TDfa& dfa) 
	{
		// P = { F, ~F }
		TPartition P;
		P.push_front(dfa.Final);
		auto t = dfa.Final;
		t.Complement();
		P.push_front(t); 

		// L = { min(P) }
		TPartition L;
		L.push_front(dfa.Final);

		// split sets
		TSet B1(dfa.GetMaxStates()), B2(dfa.GetMaxStates());
		int countB1, countB2;

		// O(1) empty test
		while(!L.empty())
		{
			auto S = ExtractOne(L); // extraction O(1)			
			for(TSymbol a = 0; a < dfa.GetAlphabethLength(); a++)
			{
				if(ShowConfiguration) 
				{
					auto strS = SetToString(S);
					auto stra = std::to_string((uint64_t)a);
					std::cout << "Splitter = (S:" << strS << ", a:" << stra << ")" << std::endl;
				}
				auto Biter = P.begin();
				auto Bend = P.end();
				while(Biter != Bend)
				{
					auto& B = *Biter;
					if(ShowConfiguration)
					{	
						auto strB = SetToString(B);
						std::cout << "-> B:" << strB << std::endl;
					}
					
					B1.Clear(); // O(Nmax) -> constant respect to N
					B2.Clear(); // O(Nmax) -> constant respect to N
					Split(dfa, B, S, a, B1, B2, &countB1, &countB2); // O(N) -> N elements within B
																				
					// was there split?
					if(countB1 != 0 && countB2 != 0)
					{
						// care, it destroys B, do not reference B beyond this point
						Biter = P.erase(Biter); // O(1)
						P.push_front(B1); // O(1)
						P.push_front(B2); // O(1)
						if(countB1 < countB2)
						{
							L.push_back(B1); // O(1)
						} 
						else 
						{
							L.push_back(B2); // O(1)
						}
						if(ShowConfiguration) 
						{
							auto strB1 = SetToString(B1);
							auto strB2 = SetToString(B2);
							auto strL = PartitionToString(L);
							auto strP = PartitionToString(P);

							std::cout 
								<< "+ Splits" << std::endl
								<< " B1:" << strB1 << " B2:" << strB2 << std::endl 
								<< " L:" << strL << " P:" << strP << std::endl;
						}
					} 
					else 
					{
						Biter++;
					}										
				}
			}
		}
	}
};
