//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "parser.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/CommandLine.h>

using namespace llvm;
using namespace std;

static cl::opt<string>
inputFilename(cl::Positional, cl::desc("<input .if file>"), cl::init("-"));

static cl::list<string>
includeDirectories("I", cl::Prefix, cl::desc("Include path"), cl::value_desc("directory"), cl::ZeroOrMore);

static cl::opt<bool>
verbose("v", cl::desc("Increase verbosity level."), cl::ZeroOrMore);

static cl::opt<string>
outputDirectory("o", cl::Prefix, cl::desc("Output path"), cl::value_desc("directory"), cl::init("."));

class Meddler
{
    llvm::SourceMgr sm;
    parser_t parser;
    vector<string> include_dirs;

public:
    Meddler(bool verbose) : sm(), parser(sm, verbose) {}

    void set_include_dirs(vector<string> dirs)
    {
        include_dirs = dirs;
        sm.setIncludeDirs(include_dirs);
    }

    /* TODO: create corresponding parser and add to queue */
    bool add_source(string file)
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

    bool emit(const string& output_dir)
    {
        ostringstream impl_h, interface_h, interface_cpp, filename;
        parser.parse_tree->emit_impl_h(impl_h);
        parser.parse_tree->emit_interface_h(interface_h);
        parser.parse_tree->emit_interface_cpp(interface_cpp);

        filename << output_dir << "/" << parser.parse_tree->name() << "_impl.h";
        ofstream of(filename.str().c_str(), ios::out|ios::trunc);
        of << impl_h.str();
        of.close();

        filename.str("");
        filename << output_dir << "/" << parser.parse_tree->name() << "_interface.h";
        of.open(filename.str().c_str(), ios::out|ios::trunc);
        of << interface_h.str();
        of.close();

        filename.str("");
        filename << output_dir << "/" << parser.parse_tree->name() << "_interface.cpp";
        of.open(filename.str().c_str(), ios::out|ios::trunc);
        of << interface_cpp.str();
        of.close();

        return true;
    }
};

int main(int argc, char** argv)
{
    cl::ParseCommandLineOptions(argc, argv, "Meddler - Metta IDL parser.\n");
    Meddler m(verbose);

    m.set_include_dirs(includeDirectories);

    if (!m.add_source(inputFilename))
    {
        cerr << "Could not open input file " << inputFilename << endl;
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

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
