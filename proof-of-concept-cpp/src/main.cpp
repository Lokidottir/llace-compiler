#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <pcrecpp.h>
#define EBNF_GIVE_UP_EASILY
#include "EBNF.hpp"
#include "BuildSyntaxTree.hpp"

int main(int argc, char** args) {
    std::cout << "LLace compiler." << std::endl;
    std::cout << "using pcre version: " << pcre_version() << std::endl;
    std::cout << "compiled at " << __TIME__ << " on " << __DATE__ << std::endl;
    std::string ebnf_filename;
    std::string source_filename = "source_file_example.txt";
    for (int i = 0; i < argc - 1; i++) {
        if (strcmp(args[i], "-ebnf") == 0) {
            ebnf_filename = args[i + 1];
        }
        if (strcmp(args[i],"-src") == 0) {
            source_filename = args[i + 1];
        }
    }
    bool runtest = false;
    for (int i = 0; i < argc && !runtest; i++) {
        if (strcmp(args[i],"-test") == 0) {
            runtest = true;
        }
    }
    if (ebnf_filename.size() > 0) {
        EBNF ebnf(ebnf_filename, EBNF::flag_file);
        std::cout << "Loaded EBNF file from source: " << ebnf_filename << std::endl;
        std::cout << "Grammar evaluated to:" << std::endl;
        for (auto& elem : ebnf.regex_map) {
            std::cout << "\tRule \"" << elem.first << "\" as:" << std::endl;
            std::cout << "\t\t" << elem.second.regex << std::endl;
            //std::cout << "\tWith dependencies:" << std::endl;
            //std::cout << "\t\t" << elem.second.assemble(ebnf.regex_map) << std::endl;
        }
        std::cout << "Parsing file with generated Regexes..." << std::endl;
        auto trie = syntree::buildTree(ebnf,loadIntoString(source_filename));
        std::cout << "Parsing complete. Size of tree is: " << trie.size() << std::endl;
        syntree::treeSummary(trie);
    }
    if (runtest) {
        EBNF::testProgram();
    }
    return 0;
}
