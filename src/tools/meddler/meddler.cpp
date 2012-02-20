//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "parser.h"
#include "logger.h"
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
    bool verbose;
    vector<parser_t*> parser_stack;
    vector<string> include_dirs;

public:
    Meddler(bool verbose_) : sm(), verbose(verbose_) {}

    void set_include_dirs(vector<string> dirs)
    {
        include_dirs = dirs;
        sm.setIncludeDirs(include_dirs);
    }

    bool add_source(string file)
    {
        L(cout << "### Adding file " << file << endl);
        std::string full_path;
        unsigned bufn = sm.AddIncludeFile(file, llvm::SMLoc(), full_path);
        if (bufn == ~0U)
            return false;
        L(cout << "### Parsing file " << file << endl);
        parser_t* parser = new parser_t(sm, verbose);
        L(cout << "### Initing parser" << endl);
        parser->init(sm.getMemoryBuffer(bufn));
        L(cout << "### Adding parser to stack" << endl);
        parser_stack.push_back(parser);
        return true;
    }

    bool parse()
    {
        assert(parser_stack.size() > 0);

        bool res = true;
        do {
            L(cout << "### Running parse" << endl);
            res &= parser_stack.at(parser_stack.size()-1)->run();

            // Since parent interfaces can only "extend" current interface, we put them into parent interfaces list of current interface
            // after parsing and consult them during emit phase for matching types, exceptions and methods - they are considered LOCAL to this
            // interface.
            if (parser_stack.size() > 1) 
            {
                L(cout << "### Linking interface to parent" << endl);
                parser_stack.at(parser_stack.size()-2)->link_to_parent(parser_stack.at(parser_stack.size()-1));
            }
            if (res && (parser_stack.at(parser_stack.size()-1)->parent_interface() != ""))
            {
                L(cout << "### Adding another interface file" << endl);
                add_source(parser_stack.at(parser_stack.size()-1)->parent_interface() + ".if");
            }
            L(cout << "### Running another round" << endl);
        } while (res && (parser_stack.at(parser_stack.size()-1)->parent_interface() != ""));
        L(cout << "### Finished parsing!" << endl);
        return res;
    }

    bool emit(const string& output_dir)
    {
        ostringstream impl_h, interface_h, interface_cpp, filename;
        parser_t& parser = *parser_stack[0];

        L(cout << "### Emitting impl_h" << endl);
        parser.parse_tree->emit_impl_h(impl_h, "");
        L(cout << "### Emitting interface_h" << endl);
        parser.parse_tree->emit_interface_h(interface_h, "");
        L(cout << "### Emitting interface_cpp" << endl);
        parser.parse_tree->emit_interface_cpp(interface_cpp, "");

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
