#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <regex>
#include <pcrecpp.h>
#include <sstream>
#include "RegexHelpers.hpp"
#include "generic-btree.hpp"

#ifndef PARSE_TYPE_DEFAULTS
#define PARSE_TYPE_DEFAULTS
typedef uintmax_t uint_type;
typedef double prec_type;
#endif

/*
	EBNF contaiment strings
*/

const std::vector<std::pair<std::string, std::string> >ebnf_cont_str = {
	std::pair<std::string, std::string>("(*","*)"), //Comment
	std::pair<std::string, std::string>("\"","\""), //Quote (double)
	std::pair<std::string, std::string>("'","'"),   //Quote (single)
	std::pair<std::string, std::string>("(",")"),   //Group
	std::pair<std::string, std::string>("[","]"),   //Option
	std::pair<std::string, std::string>("{","}"),   //Repetition
	std::pair<std::string, std::string>("?","?")    //Special
};

//Macros
#define EBNF_S_COMMENT ebnf_cont_str[0]
#define EBNF_S_SINGLEQ ebnf_cont_str[1]
#define EBNF_S_DOUBLEQ ebnf_cont_str[2]
#define EBNF_S_GROUP   ebnf_cont_str[3]
#define EBNF_S_OPTION  ebnf_cont_str[4]
#define EBNF_S_REPEAT  ebnf_cont_str[5]
#define EBNF_S_SPECIAL ebnf_cont_str[6]
#define EBNF_S_ALL ebnf_cont_str

#define EBNF_REGEX_BETWEEN(lhs,rhs) std::string("((")+lhs+")(?<="+lhs+")(([^"+lhs+rhs+"]|(?R))*)(?="+rhs+")("+rhs+"))"
#define EBNF_REGEX_BETWEEN_SINGLE_CHARS(lhs,rhs) std::string("((")+lhs+")([^"+rhs+"]*)("+rhs+"))"

#define EBNF_REGEX_COMMENT EBNF_REGEX_BETWEEN("\\(\\*","\\*\\)")
#define EBNF_REGEX_ID "(([a-zA-Z0-9]|_)([a-zA-Z0-9]|_| )+([a-zA-Z0-9]))(?=(\\s*)(\\=)((.|\\s)*)(;))"
#define EBNF_REGEX_RULE "(?<=\\=)([^;]*)(?=;)"
#define EBNF_REGEX_IDDECLR "(([a-zA-Z0-9_]([a-zA-Z0-9_]|\\s)+)\\=[^;]*;)"
#define EBNF_REGEX_TERMSTR "(((\")(([^\"]|(\\\\\"))*)(\"))|((')([^']*)(')))"

class EBNFElement {
	private:
		std::string identifier_;
		std::string content_;
		uint_type line_;
		uint_type col_;
	public:
		
		EBNFElement() : identifier_(), content_(), line_(0), col_(0) {	
		}
		
		EBNFElement(const EBNFElement& copy) :  EBNFElement() {
			this->identifier_ = copy.identifier_;
			this->content_ = copy.content_;
			this->line_ = copy.line_;
			this->col_ = copy.col_;
		}
		
		EBNFElement(EBNFElement&& move) {
			std::swap(this->identifier_, move.identifier_);
			std::swap(this->content_, move.content_);
			std::swap(this->line_, move.line_);
			std::swap(this->col_, move.col_);
		}
		
		~EBNFElement() {	
		}
		
		EBNFElement(const std::string& identifier_, const std::string& content_, const uint_type line_, const uint_type col_) {
			this->identifier_ = identifier_;
			this->content_ = content_;
			this->line_ = line_;
			this->col_ = col_;
		}
		
		EBNFElement(const std::string& identifier_, const std::string& content_, const std::pair<uint_type,uint_type>& position) : EBNFElement(identifier_, content_, std::get<0>(position), std::get<1>(position)) {
		}
		
		uint_type line() {
			return this->line_;
		}
		
		uint_type col() {
			return this->col_;
		}
		
		uint_type size() {
			return this->content_.size();
		}
		
		std::string content() {
			return this->content_;
		}
		
		std::string identifier() {
			return this->identifier_;
		}
};

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

class EBNFTree {
	private:
	
		std::string loaded_grammar;
		
		class DependancyStack {
			private:
				struct DPNode {
					DPNode* next;
					std::string ID;
				};
			public:
				
		};
		
	public:
		
		uint_type idDependancies() {
			
		}
		
		std::map<std::string, std::string> id_rule_map;
		std::map<std::string, pcrecpp::RE> regex_map;
		
		EBNFTree() : id_rule_map(), regex_map() {
		}
		
		EBNFTree(const EBNFTree& copy) : EBNFTree() {
			this->id_rule_map = copy.id_rule_map;
			this->regex_map = copy.regex_map;
			this->loaded_grammar = copy.loaded_grammar;
		}
		
		EBNFTree(EBNFTree&& move) : EBNFTree() {
			std::swap(this->id_rule_map, move.id_rule_map);
			std::swap(this->regex_map, move.regex_map);
			std::swap(this->loaded_grammar, move.loaded_grammar);
		}
		
		const static uint_type flag_file = 0b0;
		const static uint_type flag_string = 0b1;
		
		EBNFTree(const std::string& content, uint_type stringtype_flag = flag_string) : EBNFTree() {
			if ((stringtype_flag & flag_string) != 0) {
				/*
					The string provided is a grammar in string form
				*/
				this->load(content);
			}
			else {
				/*
					The string provided is a file location, load the
					string from the file then process.
				*/
				this->load(loadIntoString(content));
			}
		}
		
		bool fetchRules(const std::string& content) {
			/*
				Clears the current id/rule set and loads all the identifiers it can find
				into 
			*/
			this->id_rule_map = std::map<std::string, std::string>();
			std::vector<std::string> identifiers;
			/*
				Strip content of comments before processing.
			*/
			std::string stripped_content(content);
			RegexHelper::strip(EBNF_REGEX_COMMENT,stripped_content);
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
			return false;
		}
		
		void evaluateRules() {
			if (this->id_rule_map.size() > 0) {
				
			}
			else {
				EBNF_ERROUT << "there are no rules to evaluate." << std::endl;
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
			return false;
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
		
		void append(const EBNFTree& tree) {
			this->loaded_grammar += tree.loaded_grammar;
			this->reload();
		}
		
		void append(const std::string& content) {
			this->loaded_grammar += content;
			this->reload();
		}
		
		EBNFTree& operator+= (const EBNFTree& tree) {
			this->append(tree);
			return *this;
		}
		
		EBNFTree& operator+= (const std::string& grammar) {
			this->append(grammar);
			return *this;
		}
		
		EBNFTree operator+ (const EBNFTree& tree) const {
			EBNFTree tree_new(*this);
			tree_new += tree;
			return tree_new;
		}
		
		EBNFTree operator+ (const std::string& grammar) const {
			EBNFTree tree_new(*this);
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
		
		std::string& referenceToGrammar() {
			//Added for reasons of manual editing. Poor method name to dissuade.
			return this->loaded_grammar;
		}
		
		std::string briefRules() {
			std::stringstream briefing;
			for (auto& elem : this->id_rule_map) {
				briefing << "\tid: " << elem.first << "\trule: " << elem.second << std::endl;
			}
			return briefing.str();
		}
		
		static void testProgram() {
			std::string str1 = "something (* This (* is a *) (* comment *) *) {}(**) s { \"A string! abcd.\" { recursive D: } }";
			std::cout << "for string \"" << str1 << "\" the areas that count as contained by comments are:" << std::endl;
			std::cout << str1 << std::endl;
			for (uint_type i = 0; i < str1.size(); i++) {
				std::cout << RegexHelper::isContainedBy(str1,i,EBNF_S_COMMENT);
			}
			std::cout << std::endl;
			RegexHelper::briefOnMatches(EBNF_REGEX_BETWEEN(pcrecpp::RE::QuoteMeta("(*"),pcrecpp::RE::QuoteMeta("*)")), str1);
			std::cout << "areas contained by {}:" << std::endl;
			std::cout << str1 << std::endl;
			auto matches_mask = RegexHelper::matchMask(EBNF_REGEX_BETWEEN("\\{","\\}"),str1);
			for (uint_type i = 0; i < matches_mask.size(); i++) {
				std::cout << matches_mask[i];
			}
			std::cout << std::endl;
			RegexHelper::briefOnMatches(EBNF_REGEX_BETWEEN(pcrecpp::RE::QuoteMeta("{"),pcrecpp::RE::QuoteMeta("}")), str1);
		}
};
