#include "stdcrap.h"
#include "MersenneTwister.h"

static MTRand library_randomness;

extern "C" int rand()
{
//     return 4; // chosen randomly
    return library_randomness.randInt();
}
