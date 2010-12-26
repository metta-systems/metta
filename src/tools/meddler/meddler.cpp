#include "parser.h"
#include <sstream>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/CommandLine.h>

using namespace llvm;

static cl::opt<std::string>
inputFilename(cl::Positional, cl::desc("<input .if file>"), cl::init("-"));

static cl::list<std::string>
includeDirectories("I", cl::desc("Include path"), cl::value_desc("directory"), cl::ZeroOrMore);

static cl::opt<std::string>
outputDirectory("o", cl::desc("Output path"), cl::value_desc("directory"), cl::init("."));

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

    bool parse()
    {
        return parser.run();
    }

    bool emit(const std::string& /*output_dir*/)
    {
        std::ostringstream s;
        parser.parse_tree->emit_impl_h(s);
        std::cout << s.str();
        return true;
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

    if (m.parse())
    {
        m.emit(outputDirectory);
    }
    else
        return -1;

    return 0;
}
