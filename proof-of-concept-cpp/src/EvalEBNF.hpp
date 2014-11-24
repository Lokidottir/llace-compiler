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
#include "EBNFTypeDeduction.hpp"


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


#define EBNF_EVAL_OUT std::cout << "(EBNF Evaluation) "
#define EBNF_EVAL_ERROUT std::cerr << "(EBNF Evaluation) Error: "
#define EBNF_EVAL_WARNOUT std::cout << "(EBNF Evaluation) Warning: "

namespace EvalEBNF {

    typedef std::map<std::string,std::string> Ruleset;


    std::string stripComments(const std::string& content) {
        /*
            Strips all comments not contained within a string from an entire
            EBNF grammar.
        */
        return RegexHelper::strip(genRegexBetweenStrings("(*","*)"),content);
    }

    std::vector<std::string> splitSeperators(const std::string& between, const std::string segment) {
        auto results = RegexHelper::splitBetweenCharRaw(between,segment);
        std::vector<std::string> stiched;
        if (results.size() == 0) return stiched;
        stiched.push_back(results[0].first);
        if (results.size() > 1) for (uint_type iter = 1; iter < results.size(); iter++) {
            /*
                Stitch together any matches that occured between containers.
            */
            bool collided = false;
            for (auto& elem : EBNF_S_ALL) {
                std::string regex = genRegexBetweenStrings(elem.first,elem.second,true);
                auto between_matches = RegexHelper::getListOfMatches(regex,segment);
                if (between_matches.size() == 0) continue;
                for (uint_type iter_coll = 0; iter_coll < between.size() && !collided; iter_coll++) {
                    collided = (results[iter].second >= between_matches[iter_coll].second
                             && results[iter].second + results[iter].first.size() - 1
                             <= between_matches[iter_coll].second + between_matches[iter_coll].first.size() - 1);
                }
            }
            if (!collided) {
                stiched.push_back(results[iter].first);
            }
            else {
                EBNF_EVAL_OUT << "End is currently: " << stiched[stiched.size() - 1] << ", stiching: " << results[iter].first << std::endl;
                stiched[stiched.size() - 1] += results[iter].first;
            }
        }
        return stiched;
    }

    std::string evaluateSegment(const std::string& given_segment,
                                const Ruleset& ruleset,
                                const std::vector<std::string>& string_table,
                                Stack<std::string>& id_stack,
                                Stack<std::string>& depends_stack) {
        std::string segment = RegexHelper::firstMatch("((\\S+(\\s|\\S)*\\S+)|\\S)",given_segment);
        std::string regex = "(";
        std::string match;
        std::string temp;
        int temp_num = 0;
        std::vector<std::string> rules;
        EBNF_EVAL_OUT << "segment type for: " << segment << " is " << typeStr(segment) << std::endl;
        if (segment.size() == 0) return "";
        switch(type(segment)) {
            case types::alternation:
                rules = splitSeperators("|",segment);
                for (uint_type iter = 0; iter < rules.size(); iter++) {
                    regex += evaluateSegment(rules[iter],ruleset,string_table,id_stack,depends_stack);
                    if (iter < rules.size() - 1) regex += "|";
                }
                break;
            case types::group:
                regex += "(";
                regex += evaluateSegment(segment.substr(1,segment.size() - 2),ruleset,string_table,id_stack,depends_stack);
                regex += ")";
                break;
            case types::option:
                regex += "(?:";
                regex += evaluateSegment(segment.substr(1,segment.size() - 2),ruleset,string_table,id_stack,depends_stack);
                regex += ")?";
                break;
            case types::repeat:
                regex += "(";
                regex += evaluateSegment(segment.substr(1,segment.size() - 2),ruleset,string_table,id_stack,depends_stack);
                regex += ")+";
                break;
            case types::concatination:
                rules = splitSeperators(",",segment);
                for (uint_type iter = 0; iter < rules.size(); iter++) {
                    regex += evaluateSegment(rules[iter],ruleset,string_table,id_stack,depends_stack);
                }
                break;
            case types::terminal:
                try {
                    temp_num = std::stoi(RegexHelper::strip("([^0-9]+)",segment));
                }
                catch (std::invalid_argument& err) {
                    EBNF_EVAL_ERROUT << "Could not load number from: " << segment << std::endl;
                }
                regex += pcrecpp::RE::QuoteMeta(string_table[temp_num]);
                break;
            case types::special:
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
                    } while (id_stack.contains(generated_id.str()) && id_num != 0);
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
                break;
            case types::identifier:
                temp = RegexHelper::firstMatch(EBNF_REGEX_ID_LONE,segment);
                if (!depends_stack.contains(temp)) depends_stack.push(temp);
                else EBNF_EVAL_WARNOUT << "Circular dependancy on type \"" << temp << "\"" << std::endl;
                regex += ("\\g'" + temp + "'");
                break;
            case types::negation:
                break;
            case types::setrepeat:
                temp_num = std::stoi(lastMatch("([0-9]+)",segment));

                break;
            default:
                EBNF_EVAL_ERROUT << "rule segment \"" << segment << "\" has no known evaluation type." << std::endl;
                exit(-1);
                break;
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
                      const Stack<std::string> dependencies) : EvaluatedRule() {
            this->rule_id = rule_id;
            this->original = original;
            this->regex = regex;
            this->dependencies = std::vector<std::string>(dependencies);
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
            depends_stack.push(this->rule_id);
            std::string regex = ("(" + this->assembleNocall(rules,depends_stack) + "\\g'" + this->rule_id + "')");
            return regex;
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
                    exit(0);
                    return "";
                }
            }
            assembled += this->regex;
            return assembled;
        }
    };

    EvaluatedRule evaluate(const std::string& rule_id, const Ruleset& ruleset, const std::vector<std::string>& string_table) {
        /*
            Note: declaring a named expression as ((?P<name>(group)){0}) essentially works
            as a function declaration, able to be called later with \g'name' ect.
        */
        /*
            Declare regex header.
        */
        std::string regex = "((?P<" + rule_id + ">(";
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
        try {
            regex += evaluateSegment(ruleset.at(rule_id),ruleset,string_table,id_stack,depends_stack) + ")){0})";
        }
        catch (const std::out_of_range& oor) {
            EBNF_EVAL_ERROUT << "fetching ID " << rule_id << " from ruleset threw out of range error: " << oor.what() << std::endl;
            return EvaluatedRule();
        }
        EvaluatedRule rule;
        /*
            Manually copy data to the rule.
        */
        rule.rule_id = rule_id;
        rule.original = ruleset.at(rule_id);
        rule.regex = regex;
        rule.dependencies = depends_stack;
        return rule;
    }
};
#endif
