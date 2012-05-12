//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#ifdef USTL
#include "../runtime/mustl/uvector.h"
using namespace ustl;
#define NAME "uSTL"
#endif
#ifdef STL
#include <vector>
using namespace std;
#define NAME "STL"
#endif
#include <time.h>
#include <stdio.h>

int main()
{
    vector<int> vec1;
    const clock_t start = clock();
//    vec1.reserve(10000000);
    for (int i = 0; i < 10000000; i++)
	vec1.push_back(i);
    clock_t end = clock();
    end += (start == end);
    printf(NAME" time taken %.3g sec\n", (double)(end - start) / CLOCKS_PER_SEC);

    return 0;
}
