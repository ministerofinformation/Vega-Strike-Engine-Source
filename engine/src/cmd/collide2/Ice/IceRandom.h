///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for random generators.
 *	\file		IceRandom.h
 *	\author		Pierre Terdiman
 *	\date		August, 9, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Updated by Stephen G. Tuggy 2021-07-03
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICERANDOM_H__
#define __ICERANDOM_H__

	ICECORE_API	void	SRand(uint32_t seed);
	 uint32_t	Rand();

	//! Returns a unit random floating-point value
	inline_ float UnitRandomFloat()	{ return float(Rand()) * ONE_OVER_RAND_MAX;	}

	//! Returns a random index so that 0<= index < max_index
	ICECORE_API	uint32_t GetRandomIndex(uint32_t max_index);

	class ICECORE_API BasicRandom
	{
		public:

		//! Constructor
		inline_				BasicRandom(uint32_t seed=0)	: mRnd(seed)	{}
		//! Destructor
		inline_				~BasicRandom()								{}

		inline_	void		SetSeed(uint32_t seed)		{ mRnd = seed;											}
		inline_	uint32_t		GetCurrentValue()	const	{ return mRnd;											}
		inline_	uint32_t		Randomize()					{ mRnd = mRnd * 2147001325 + 715136305; return mRnd;	}

		private:
				uint32_t		mRnd;
	};

#endif // __ICERANDOM_H__

