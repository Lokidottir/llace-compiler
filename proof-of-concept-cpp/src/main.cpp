#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <pcrecpp.h>
#include "generic-btree.hpp"
#include "EBNFTree.hpp"

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
	std::string ebnf_filename;
	for (int i = 0; i < argc - 1; i++) {
		if (strcmp(args[i], "-ebnf") == 0) {
			ebnf_filename = stripSubstr(stripSubstr(args[i + 1], "\""),"'");
		}
	}
	EBNFTree::testProgram();
	return 0;
}
