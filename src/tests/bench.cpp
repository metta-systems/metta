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