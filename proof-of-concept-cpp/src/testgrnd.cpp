#include <string>
#include <iostream>
#include "EBNF.hpp"
#include "RegexHelpers.hpp"
#include "EvalEBNF.hpp"
#include "EBNFTypeDeduction.hpp"

int main(int argc, char** args) {
    for (int i = 0; i < argc; i++) {
        std::cout << "Arg: " << args[i] << " is type: " << EvalEBNF::typeStr(args[i]) << std::endl;
    }
    return 0;
}
