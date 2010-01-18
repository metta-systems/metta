#include "raiifile.h"
#include "../tools/parsedwarf/leb128.h"

using namespace raii_wrapper;

int main(int, char**)
{
    file f("slebtest.txt", fstream::in);
    size_t fsize = f.size();
    char* buffer = new char [fsize];
    f.read(buffer, fsize);

    address_t start = reinterpret_cast<address_t>(buffer);
    size_t offset = 0;
    sleb128_t sleb;

    while (offset < fsize)
    {
        sleb.decode(start, offset);
    }
}
