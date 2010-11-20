#include "parser.h"
#include <llvm/Support/MemoryBuffer.h>

int main(int argc, char** argv)
{
    if (argc != 2)
        return -1;

    llvm::MemoryBuffer* buf = llvm::MemoryBuffer::getFile(argv[1]);
    if (!buf)
        return -1;

    parser_t parser(buf);
    return parser.run();
}
