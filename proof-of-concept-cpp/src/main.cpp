#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <pcrecpp.h>
#include "EBNF.hpp"

std::string stripSubstr(const std::string& content, const std::string& toStrip) {
	std::string altered_content(content);
	for (unsigned int i = 0; i < content.size() && i != std::string::npos; i = content.find(toStrip,i)) {
		altered_content.erase(i, i + toStrip.size());
	}
	return altered_content;
}

int main(int argc, char** args) {
	std::cout << "Circuit language compiler." << std::endl;
	std::cout << "using pcre version: " << pcre_version() << std::endl;
	std::cout << "compiled at " << __TIME__ << " on " << __DATE__ << std::endl; 
	std::string ebnf_filename;
	for (int i = 0; i < argc - 1; i++) {
		if (strcmp(args[i], "-ebnf") == 0) {
			ebnf_filename = args[i + 1];
		}
	}
	bool runtest = false;
	for (int i = 0; i < argc && !runtest; i++) {
		if (strcmp(args[i],"-test") == 0) {
			runtest = true;
		}
	}
	if (ebnf_filename.size() > 0) {
		EBNFTree tree(ebnf_filename, EBNFTree::flag_file);
		std::cout << "Loaded EBNF file from source: " << ebnf_filename << std::endl;
		std::cout << "Grammar evaluated to:" << std::endl;
		for (auto& elem : tree.regex_map) {
			std::cout << "\tRule \"" << elem.first << "\" as:" << std::endl;
			std::cout << "\t\t" << elem.second.regex << std::endl;
			std::cout << "\t\t" << elem.second.assemble(tree.regex_map) << std::endl;
		}
	}
	if (runtest) {
		EBNFTree::testProgram();
	}
	return 0;
}
