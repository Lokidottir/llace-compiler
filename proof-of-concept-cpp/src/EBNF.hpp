#ifndef EBNF_MAIN_HPP
#define EBNF_MAIN_HPP
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <regex>
#include <pcrecpp.h>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include "RegexHelpers.hpp"
#include "generic-btree.hpp"
#include "EvalEBNF.hpp"

#ifndef PARSE_TYPE_DEFAULTS
#define PARSE_TYPE_DEFAULTS
typedef uintmax_t uint_type;
typedef double prec_type;
#endif

std::string loadIntoString(const std::string& filename) {
    std::fstream file_stream(filename.c_str(), std::ios::in);
    if (file_stream.is_open()) {
        std::string file_string;
        std::string temp_str;
        while (std::getline(file_stream, temp_str, '\n')) {
            file_string += temp_str + '\n';
        }
        file_stream.close();
        return file_string;
    }
    else {
        std::cerr << "(File loading) Was not able to load file: " << filename << std::endl;
        file_stream.close();
        return "";
    }
}

#define EBNF_ERROUT std::cerr << "(EBNF interpreter) Error: "
#define EBNF_OUT std::cout << "(EBNF interpreter) "

class EBNF {
    private:

        std::string loaded_grammar;

        bool fetchRules(const std::string& content) {
            /*
                Clears the current id/rule set and loads all the identifiers it can find
                into
            */
            this->id_rule_map = std::map<std::string, std::string>();
            this->string_table = std::vector<std::string>();
            std::vector<std::string> identifiers;
            /*
                Move all strings to the string table, replacing them with "@str<#>" with #
                as the index.
            */
            std::string stripped_content(content);
            std::stringstream ss;;
            auto str_matches = RegexHelper::getListOfMatches(EBNF_REGEX_TERMSTR,stripped_content);
            ss << "str@<" << (str_matches.size() - 1) << ">";
            std::string str_id_temp = ss.str();
            for (uint_type iter = str_matches.size() - 1; iter >= 0 && iter < str_matches.size(); iter--) {
                this->string_table.insert(this->string_table.begin(),str_matches[iter].first.substr(1,str_matches[iter].first.size() - 2));
                ss.str("");
                ss.clear();
                ss << "str@<" << iter << ">";
                str_id_temp = ss.str();
                stripped_content.replace(str_matches[iter].second, str_matches[iter].first.size(), str_id_temp);
            }
            /*
                Strip content of comments before processing.
            */
            stripped_content = RegexHelper::strip(EBNF_REGEX_COMMENT,stripped_content);
            /*
                Find each match for a rule delcaration.
            */
            auto matches = RegexHelper::getListOfMatches(EBNF_REGEX_IDDECLR,stripped_content);
            /*
                Split each rule declaration into the identifier and the rule and load into
                the id/rule map.
            */
            for (uint_type i = 0; i < matches.size(); i++) {
                this->id_rule_map[RegexHelper::firstMatch(EBNF_REGEX_ID, matches[i].first)] = RegexHelper::firstMatch(EBNF_REGEX_RULE, matches[i].first);
            }
            return true;
        }

        void evaluateRules() {
            /*
                Evaluate rules into regexes, todo
            */
            if (this->id_rule_map.size() > 0) {
                EBNF_OUT << "beginning evaluation of rules..." << std::endl;
                for (auto& elem : this->id_rule_map) {
                    this->regex_map[elem.first] = EvalEBNF::evaluate(elem.first,this->id_rule_map,this->string_table);
                }
            }
            else {
                EBNF_ERROUT << "there are no rules to evaluate." << std::endl;
            }
        }

    public:

        EvalEBNF::Ruleset id_rule_map;
        std::map<std::string,EvalEBNF::EvaluatedRule> regex_map;
        std::vector<std::string> string_table;

        EBNF() : id_rule_map(), regex_map() {
        }

        EBNF(const EBNF& copy) : EBNF() {
            this->id_rule_map = copy.id_rule_map;
            this->regex_map = copy.regex_map;
            this->loaded_grammar = copy.loaded_grammar;
            this->string_table = copy.string_table;
        }

        EBNF(EBNF&& move) : EBNF() {
            std::swap(this->id_rule_map, move.id_rule_map);
            std::swap(this->regex_map, move.regex_map);
            std::swap(this->loaded_grammar, move.loaded_grammar);
            std::swap(this->string_table, move.string_table);
        }

        const static uint_type flag_file = 0b0;
        const static uint_type flag_string = 0b1;

        EBNF(const std::string& content, uint_type stringtype_flag = flag_string) : EBNF() {
            if ((stringtype_flag & flag_string) != 0) {
                /*
                    The string provided is a grammar in string form
                */
                this->load(content);
            }
            else {
                /*
                    The string provided is a file location, load the string from the file
                    then process.
                */
                this->load(loadIntoString(content));
            }
        }

        bool load(const std::string& content) {
            /*
                Member for loading a string representing an EBNF grammar into the
                object.
            */
            //First check the string has content.
            if (content.size() == 0) {
                EBNF_ERROUT << "string is empty, cannot read." << std::endl;
                return false;
            }
            this->loaded_grammar = content;
            //Find identifiers
            this->fetchRules(content);
            this->evaluateRules();
            return true;
        }

        void reload() {
            /*
                Reload the grammar
            */
            this->load(std::string(this->loaded_grammar));
        }

        /*
            Append functions, for increasing the size of the grammar.
        */

        void append(const EBNF& tree) {
            this->loaded_grammar += tree.loaded_grammar;
            this->reload();
        }

        void append(const std::string& content) {
            this->loaded_grammar += content;
            this->reload();
        }

        EBNF& operator+= (const EBNF& tree) {
            this->append(tree);
            return *this;
        }

        EBNF& operator+= (const std::string& grammar) {
            this->append(grammar);
            return *this;
        }

        EBNF operator+ (const EBNF& tree) const {
            EBNF tree_new(*this);
            tree_new += tree;
            return tree_new;
        }

        EBNF operator+ (const std::string& grammar) const {
            EBNF tree_new(*this);
            tree_new += grammar;
            return tree_new;
        }

        uint_type size() {
            /*
                Returns the number of rules.
            */
            return this->id_rule_map.size();
        }

        std::string grammar() {
            return this->loaded_grammar;
        }

        static void testProgram() {
            std::string str1 = "something (* This (* is a *) (* comment *) *) {}(**) s { \"A string! abcd.\" { recursive D: } }";
            std::cout << "for string \"" << str1 << "\" the areas that count as contained by comments are:" << std::endl;
            std::cout << str1 << std::endl;
            auto matches_mask = RegexHelper::matchMask(EBNF_REGEX_BETWEEN("(*","*)"),str1);
            for (uint_type i = 0; i < matches_mask.size(); i++) {
                std::cout << matches_mask[i];
            }
            std::cout << std::endl;
            RegexHelper::briefOnMatches(EBNF_REGEX_BETWEEN("(*","*)"), str1);
            std::cout << "areas contained by {}:" << std::endl;
            std::cout << str1 << std::endl;
            matches_mask = RegexHelper::matchMask(EBNF_REGEX_BETWEEN("{","}"),str1);
            for (uint_type i = 0; i < matches_mask.size(); i++) {
                std::cout << matches_mask[i];
            }
            std::cout << std::endl;
            RegexHelper::briefOnMatches(EBNF_REGEX_BETWEEN("{","}"), str1);
            std::cout << "terminal strings:" << std::endl;
            std::cout << str1 << std::endl;
            matches_mask = RegexHelper::matchMask(EBNF_REGEX_BETWEEN("\"","\""),str1);
            for (uint_type i = 0; i < matches_mask.size(); i++) {
                std::cout << matches_mask[i];
            }
            std::cout << std::endl;
            RegexHelper::briefOnMatches(EBNF_REGEX_BETWEEN("\"","\""),str1);
        }
};
#endif
