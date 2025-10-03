/*!------------------------------------------------------------------------------------------------
	File: Rand.h
	Project: NocturnalStudio

	This file contains ...

	Primary Author:
		 Name (%)
		-Brief

	Secondary Author:
		 Name (%)
		-Brief

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#ifndef RAND_H
#define RAND_H
#include <cstdlib>  // For rand() and srand()
#include <ctime>    // For time()

bool seeded = false;
template <typename T>
T Random() {
	static_assert(std::is_arithmetic_v<T>, "T must be an arithmetic type.");
	if (!seeded)
	{
		srand(static_cast<unsigned int>(time(NULL)));
		seeded = true;
	}
	return static_cast<T>(rand()) / static_cast<T>(RAND_MAX);
}

template <typename T>
T Random_Range( T min,  T max) {
	static_assert(std::is_arithmetic_v<T>, "T must be an arithmetic type.");
	if (!seeded)
	{
		srand(static_cast<unsigned int>(time(NULL)));
		seeded = true;
	}
	
	return  min + (Random<T>() * (max - min));
}



#endif
