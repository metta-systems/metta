#include "parser.h"
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/CommandLine.h>

using namespace llvm;

static cl::opt<std::string>
inputFilename(cl::Positional, cl::desc("<input .if file>"), cl::init("-"));

static cl::list<std::string>
includeDirectories("I", cl::desc("Include path"), cl::value_desc("directory"), cl::ZeroOrMore);

class Meddler
{
    llvm::SourceMgr sm;
    parser_t parser;
    std::vector<std::string> include_dirs;

public:
    Meddler() : sm(), parser(sm) {}

    void set_include_dirs(std::vector<std::string> dirs)
    {
        include_dirs = dirs;
        sm.setIncludeDirs(include_dirs);
    }

    /* TODO: create corresponding parser and add to queue */
    bool add_source(std::string file)
    {
        unsigned bufn = sm.AddIncludeFile(file, llvm::SMLoc());
        if (bufn == ~0U)
            return false;
        parser.init(sm.getMemoryBuffer(bufn));
        return true;
    }

    int parse()
    {
        return parser.run();
    }
};

int main(int argc, char** argv)
{
    Meddler m;
    cl::ParseCommandLineOptions(argc, argv, "Meddler - Metta IDL parser.\n");

    m.set_include_dirs(includeDirectories);

    if (!m.add_source(inputFilename))
    {
        std::cerr << "Could not open input file " << inputFilename << std::endl;
        return -1;
    }

    return m.parse();
}
