#ifndef EVAL_EBNF_HPP
#define EVAL_EBNF_HPP
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

#define EBNF_REGEX_COMMENT EBNF_REGEX_BETWEEN("\\(\\*","\\*\\)")
#define EBNF_REGEX_ID "(([a-zA-Z0-9]|_)([a-zA-Z0-9]|_| )*([a-zA-Z0-9])+)(?=(\\s*)(\\=)((.|\\s)*)(;))"
#define EBNF_REGEX_RULE "(?<=\\=)([^;]*)(?=;)"
#define EBNF_REGEX_IDDECLR "(([a-zA-Z0-9_]([a-zA-Z0-9_]|\\s)*)\\=[^;]*;)"
#define EBNF_REGEX_ID_LONE "(([a-zA-Z0-9]|_)([a-zA-Z0-9]|_| )*([a-zA-Z0-9])+)"
#define EBNF_REGEX_TERMSTR (genRegexBetweenStrings("\"","\"") + "|" + genRegexBetweenStrings("'","'"))

enum Type {
	EBNF_TYPE_GROUP,
	EBNF_TYPE_SPECIAL,
	EBNF_TYPE_REPEAT,
	EBNF_TYPE_TERMINAL,
	EBNF_TYPE_OPTION,
	EBNF_TYPE_RULE,
	EBNF_TYPE_IDENTIFIER,
	EBNF_TYPE_ALTERNATION,
	EBNF_TYPE_NOTYPE
};

#define EBNF_EVAL_OUT std::cout << "(EBNF Evaluation) "
#define EBNF_EVAL_ERROUT std::cerr << "(EBNF Evaluation) Error: "

namespace EvalEBNF {
	
	typedef std::map<std::string,std::string> Ruleset;
	
	
	std::string stripComments(const std::string& content) {
		/*
			Strips all comments not contained within a string from an entire
			EBNF grammar.
			 
			Not currently working entirely, would evaluate the centre of "(*" (* *) "*)"
			as not a comment.
		*/
		auto comment_matches = RegexHelper::getListOfMatches(EBNF_REGEX_COMMENT, content);
		auto string_mask = RegexHelper::matchMask(EBNF_REGEX_TERMSTR, content);
		std::string wrk_content = content;
		for (uint_type iter = comment_matches.size() - 1; iter >= 0 && iter < comment_matches.size(); iter--) {
			bool contained_by_string = (string_mask[comment_matches[iter].second]
									|| string_mask[comment_matches[iter].second + 1]
									|| string_mask[comment_matches[iter].second + comment_matches[iter].first.size() - 1]
									|| string_mask[comment_matches[iter].second + comment_matches[iter].first.size() - 2]);
			/*
				Remove match from the comment array if any part of the containing strings
				match as a string. 
			*/
			
			if (!contained_by_string) {
				/*
					Comment isn't within string, erase.
				*/
				wrk_content.erase(comment_matches[iter].second, comment_matches[iter].first.size());
			}
		}
		return wrk_content;
	}
	
	std::vector<std::string> splitSeperators(const std::string& between, const std::string segment) {
		auto results = RegexHelper::splitBetweenCharRaw(between,segment);
		for (uint_type iter = 0; iter < results.size(); iter++) {
			/*
				Stitch together any matches that occured between containers.
			*/
			for (auto& elem : EBNF_S_ALL) {
				std::string regex = genRegexBetweenStrings(elem.first,elem.second,true);
			}
		}
	}
	
	int segmentType(const std::string& segment) {
		std::string wrk_segment = RegexHelper::firstMatch("(\\S[\\s\\S]*[\\S])|\\S",segment);
		if (RegexHelper::firstMatch(genRegexBetweenStrings("str@<",">", true),wrk_segment) == wrk_segment) {
			return EBNF_TYPE_TERMINAL;
		}
		else if (RegexHelper::firstMatch(genRegexBetweenStrings("?","?"),wrk_segment) == wrk_segment) {
			return EBNF_TYPE_SPECIAL;
		}
		else if (!RegexHelper::firstMatch(",",wrk_segment).empty()) {
			return EBNF_TYPE_RULE;
		}
		else if (!RegexHelper::firstMatch("|",wrk_segment).empty()) {
			return EBNF_TYPE_ALTERNATION;
		}
		else return EBNF_TYPE_NOTYPE;
	}
	
	std::string evaluateSegment(const std::string& given_segment, 
								const Ruleset& ruleset,
								const std::vector<std::string>& string_table,
								Stack<std::string>& id_stack,
								Stack<std::string>& depends_stack) {
		std::string segment = RegexHelper::firstMatch("((\\S+(\\s|\\S)+\\S+)|\\S)",given_segment);
		std::string regex = "(";
		std::string match;
		std::string temp;
		int temp_num = 0;
		std::vector<std::string> rules;
		std::cout << "segment type ID " << segmentType(segment) << std::endl;
		uint_type segment_type = segmentType(segment);
		if (segment_type == EBNF_TYPE_GROUP) {
			/*
				Get the non-inclusive match and pass it down to be further evaluated.
			*/
			match = RegexHelper::firstMatch(genRegexBetweenStrings("(",")",false),segment);
			regex += (std::string("(") + evaluateSegment(match,ruleset,string_table,id_stack,depends_stack) + ")");
		}
		else if (segment_type == EBNF_TYPE_SPECIAL) {
			/*
				Evaluate special as inline regular expression.
			*/
			/*
				Find the Regex embedded in the special with Perl notation:
				eg:
					? /[a-z]+/ ?
				will evaluate to [a-z]+.
			*/
			match = RegexHelper::firstMatch(genRegexBetweenStrings("/","/",true),segment);
			temp = RegexHelper::firstMatch(pcrecpp::RE::QuoteMeta("(?R)"),match);
			if (!temp.empty()) {
				/*
					There is an instance of self-recusion within the inlined regex, this needs
					to be processed as a named group that recurses itself and amended to the 
					beginning of the regular expression.
				*/
				uint_type id_num = 0;
				std::stringstream generated_id;
				do {
					generated_id.str(std::string());
					generated_id.clear();
					generated_id << id_stack[0] << "_" << id_num;
					id_num--;
				} while(id_stack.contains(generated_id.str()) && id_num != 0);
				/*
					A unique ID has been generated and must be pushed to the stack to prevent 
					a duplicate.
				*/
				id_stack.push(generated_id.str());
				/*
					Replace all instances of "(?R)" with \g'generated_id'
				*/
				std::string replace_with = "\\g'" + generated_id.str() + "'";
				pcrecpp::RE(pcrecpp::RE::QuoteMeta("(?R)")).GlobalReplace(replace_with.c_str(),&match);
				regex = std::string("((?P<") + generated_id.str() + ">(" + match + ")){0})" + regex;
			}
			else {
				/*
					Regex contains no instance of "(?R)" self-recursion, and needs no further
					processing.
				*/
				regex += match.substr(1,match.size() - 2);
			}
		}
		else if (segment_type == EBNF_TYPE_TERMINAL) {
			try {
				temp_num = std::stoi(RegexHelper::strip("[^0-9]+",segment));
			}
			catch (std::invalid_argument& err) {
				std::cout << "Could not load number from: " << segment << std::endl;
			}
			std::cout << "Hit terminal of " << segment << " as: " << temp_num << std::endl;
			regex += pcrecpp::RE::QuoteMeta(string_table[temp_num]);
		}
		else if (segment_type == EBNF_TYPE_OPTION) {
			match = RegexHelper::firstMatch(genRegexBetweenStrings("[","]",false),segment);
			regex += evaluateSegment(match,ruleset,string_table,id_stack,depends_stack);
		}
		else if (segment_type == EBNF_TYPE_RULE) {
			rules = RegexHelper::splitBetweenChar(",",segment);
			for (uint_type iter = 0; iter < rules.size(); iter++) {
				regex += evaluateSegment(rules[iter],ruleset,string_table,id_stack,depends_stack);
			}
		}
		else if (segment_type == EBNF_TYPE_ALTERNATION) {
			rules = RegexHelper::splitBetweenChar("|",segment);
			for (uint_type iter = 0; iter < rules.size(); iter++) {
				regex += evaluateSegment(match,ruleset,string_table,id_stack,depends_stack);
				if (iter < rules.size() - 1) regex += "|";
			}
		}
		else if (segment_type == EBNF_TYPE_IDENTIFIER) {
			temp = RegexHelper::firstMatch(EBNF_REGEX_ID_LONE,segment);
			depends_stack.push(temp);
			regex += ("\\g'" + temp + "'");
		}
		else {
			EBNF_EVAL_OUT << "rule segment \"" << segment << "\" has no known evaluation type" << std::endl;
		}
		regex += ")";
		return regex;
	}
	
	struct EvaluatedRule {
		std::string rule_id;
		std::string original;
		std::string regex;
		std::vector<std::string> dependencies;
		
		EvaluatedRule() : rule_id(), original(), regex(), dependencies() {
		}
		
		EvaluatedRule(const EvaluatedRule& copy) : EvaluatedRule() {
			this->original = copy.original;
			this->regex = copy.regex;
			this->dependencies = copy.dependencies;
			this->rule_id = copy.rule_id;
		}
		
		EvaluatedRule(EvaluatedRule&& move) : EvaluatedRule() {
			std::swap(this->rule_id,move.rule_id);
			std::swap(this->original,move.original);
			std::swap(this->regex,move.regex);
			std::swap(this->dependencies,move.dependencies);
		}
		
		EvaluatedRule(const std::string rule_id, 
					  const std::string original, 
					  const std::string regex, 
					  const std::vector<std::string> dependencies) : EvaluatedRule() {
			this->rule_id = rule_id;
			this->original = original;
			this->regex = regex;
			this->dependencies = dependencies;
		}
		
		~EvaluatedRule() {
		}
		
		EvaluatedRule& operator= (const EvaluatedRule& copy) {
			this->rule_id = copy.rule_id;
			this->original = copy.original;
			this->regex = copy.regex;
			this->dependencies = copy.dependencies;
			return *this;
		}
		
		std::string assemble(const std::map<std::string,EvaluatedRule>& rules) const {
			Stack<std::string> depends_stack;
			std::string regex = this->assembleNocall(rules,depends_stack) + "\\g'" + this->rule_id + "'";
			std::string dependencies;
			for (uint_type i = depends_stack.size() - 1; i == depends_stack.size() - 1; i--) {
				dependencies += depends_stack.pop();
			}
			return dependencies + regex;
		}
		
		std::string assembleNocall(const std::map<std::string,EvaluatedRule>& rules, Stack<std::string>& depends_stack) const {
			std::string assembled;
			depends_stack.push(this->rule_id);
			for (uint_type i = 0; i < this->dependencies.size(); i++) {
				try {
					/*
						Add dependency only if it's not already in the stack
					*/
					if (!depends_stack.contains(dependencies[i])) assembled += rules.at(dependencies[i]).assembleNocall(rules,depends_stack);
				}
				catch (const std::out_of_range& oor) {
					EBNF_EVAL_ERROUT << "fetching rule with ID " << dependencies[i] << " for assembly threw out of range error with: " << oor.what() << std::endl;
					return "";
				}
			}
			assembled += this->regex;
			return assembled;
		}
	};
	
	EvaluatedRule evaluate(const std::string& rule_id, const Ruleset& ruleset, const std::vector<std::string>& string_table) {
		/*
			Evaluates a rule with no other rule dependencies.
			Note: declaring a named expression as ((?P<name>(group)){0}) essentially works
			as a function declaration, able to be called later with \g'name' ect.
		*/
		std::string regex = "((?P<" + rule_id + ">(";
		std::vector<std::string> segments;
		try {
			segments = RegexHelper::splitBetweenChar(",",ruleset.at(rule_id));
		}
		catch (const std::out_of_range& oor) {
			EBNF_EVAL_ERROUT << "fetching ID " << rule_id << " from ruleset threw out of range error: " << oor.what() << std::endl;
			return EvaluatedRule();
		}
		/*
			The ID stack just acts as a container for what IDs are currently declared. A stack
			seems like an illogical choice but it's the easiest declared type with a contained.
		*/
		Stack<std::string> depends_stack;
		Stack<std::string> id_stack;
		/*
			Push the given ID on to the stack.
		*/
		id_stack.push(rule_id);
		for (uint_type iter = 0; iter < segments.size(); iter++) regex += evaluateSegment(segments[iter],ruleset,string_table,id_stack,depends_stack); 
		regex += ")){0})";
		EvaluatedRule rule;
		/*
			Manually copy data to the rule.
		*/
		rule.rule_id = rule_id;
		rule.original = ruleset.at(rule_id);
		rule.regex = regex;
		for (uint_type i = 0; i < depends_stack.size(); i++) {
			rule.dependencies.push_back(depends_stack[i]);
		}
		return rule;
	}
};
#endif
