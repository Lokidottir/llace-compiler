#ifndef BUILD_SYNTAX_TREE_HPP
#define BUILD_SYNTAX_TREE_HPP
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <pcrecpp.h>
#include <tuple>
#include "generic-btree.hpp"
#include "EBNF.hpp"

#define PARSE_OUT std::cout << "(Parsing) "
#define PARSE_ERROUT std::cerr << "(Parsing) Error: "

namespace syntree {
    
    struct SyntaxElement {
        uint_type index;
        std::string identifier;
        std::string content;
        
        SyntaxElement() : index(0), identifier(), content() {
        }

        SyntaxElement(const SyntaxElement& copy) : SyntaxElement() {
            this->index = copy.index;
            this->identifier = copy.identifier;
            this->content = copy.content;
        }
        
        SyntaxElement(SyntaxElement&& move) : SyntaxElement() {
            std::swap(this->index,move.index);
            std::swap(this->identifier,move.identifier);
            std::swap(this->content,move.content);
        }
        
        SyntaxElement(uint_type index, const std::string& identifier, const std::string& content) : SyntaxElement() {
            this->index = index;
            this->identifier = identifier;
            this->content = content;
        }
        
        ~SyntaxElement() {
        }
        
        bool operator == (const SyntaxElement& compare) const {
            return (this->index == compare.index && this->identifier == compare.identifier && this->content == compare.content);
        }
        
        bool operator != (const SyntaxElement& compare) const {
            return !((*this) == compare);
        }
        
        SyntaxElement& operator= (const SyntaxElement& copy) {
            this->index = copy.index;
            this->identifier = copy.identifier;
            this->content = copy.content;
            return *this;
        }
    };
    
    std::vector<SyntaxElement> largestMatches(const EBNF& grammar,const std::string& content, const SyntaxElement& previous) {
        /*
            Function that returns a vector of the largest, first occuring matches
            for further processing.
        */
        
        std::vector<SyntaxElement> matches;
        std::vector<std::pair<std::string,std::vector<std::pair<std::string,uint_type> > > > all_matches;
        uint_type index = 0;
        /*
            Find all matches for all regular expressions.
        */
        for (auto& elem : grammar.regex_map) {
            PARSE_OUT << "Finding matches for: " << elem.second.rule_id << std::endl;
            auto regex_matches = RegexHelper::getListOfMatches(elem.second.assemble(grammar.regex_map),content);
            all_matches.push_back(std::pair<std::string,std::vector<std::pair<std::string,uint_type> > >(elem.second.rule_id,regex_matches));
        }
        /*
            Find the  largest match by checking all matches.
        */
        #ifndef EBNF_GIVE_UP_EASILY
        while (index < content.size()) {
        #endif
            bool are_matches = true;
            while(index < content.size() && are_matches) {
                std::pair<std::string,uint_type> largest("",0);
                std::string type_string;
                are_matches = false;
                for (uint_type iter_all = 0; iter_all < all_matches.size(); iter_all++) {
                    for (uint_type iter = 0; iter < all_matches[iter_all].second.size(); iter++) {
                        if (all_matches[iter_all].second[iter].second == index 
                         && all_matches[iter_all].second[iter].first.size() > largest.first.size()
                         && all_matches[iter_all].second[iter].first != content) {
                             type_string = all_matches[iter_all].first;
                             largest = all_matches[iter_all].second[iter];
                             are_matches |= true;
                        }
                    }
                }
                if (are_matches) {
                    index = largest.first.size() + largest.second;
                    matches.push_back(SyntaxElement(largest.second + previous.index, type_string, largest.first));
                }
            }
        #ifndef EBNF_GIVE_UP_EASILY
            index++;
        }
        #endif
        return matches;
    }
    
    Trie<SyntaxElement> recurseParse(const EBNF& grammar, const SyntaxElement& previous) {
        Trie<SyntaxElement> tree(previous);
        auto large_matches = largestMatches(grammar,previous.content,previous);
        for (uint_type iter = 0; iter < large_matches.size(); iter++) {
            tree.data.push_back(recurseParse(grammar,large_matches[iter]));
        }
        return tree;
    }
    
    Trie<SyntaxElement> buildTree(const EBNF& grammar, const std::string& source) {
        return recurseParse(grammar,SyntaxElement(0,"__syntax_tree_whole__",source));
    }
    
    void treeSummary(const Trie<SyntaxElement>& tree, uint_type depth = 0) {
        std::string ws_str;
        for (uint_type i = 0; i < depth; i++) ws_str += "\t";
        std::cout << ws_str << "depth:" << depth << " type:" << tree.self.identifier << " index:" << tree.self.index <<  " content:" << std::endl;
        std::cout << ws_str << "\"" << tree.self.content << "\"" << std::endl;
        for (uint_type iter = 0; iter < tree.data.size(); iter++) {
            treeSummary(tree.data[iter],depth + 1);
        }
    }
    
};

#endif
