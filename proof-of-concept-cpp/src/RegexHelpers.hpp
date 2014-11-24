#ifndef REGEX_HELPERS_HPP
#define REGEX_HELPERS_HPP
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


#define EBNF_REGEX_COMMENT EBNF_REGEX_BETWEEN("\\(\\*","\\*\\)")
#define EBNF_REGEX_ID "(([a-zA-Z0-9]|_)([a-zA-Z0-9]|_| )*([a-zA-Z0-9])+)(?=(\\s*)(\\=)((.|\\s)*)(;))"
#define EBNF_REGEX_RULE "(?<=\\=)([^;]*)(?=;)"
#define EBNF_REGEX_IDDECLR "(([a-zA-Z0-9_]([a-zA-Z0-9_]|\\s)*)\\=[^;]*;)"
#define EBNF_REGEX_ID_LONE "(([a-zA-Z0-9]|_)([a-zA-Z0-9]|_| )*([a-zA-Z0-9])+)"
#define EBNF_REGEX_TERMSTR (genRegexBetweenStrings("\"","\"") + "|" + genRegexBetweenStrings("'","'"))

#define EBNF_REGEX_BETWEEN(lhs,rhs) genRegexBetweenStrings(lhs,rhs)
#define EBNF_REGEX_BETWEEN_SAME_CHARS(lhs,rhs) genRegexBetweenStrings(lhs,rhs)

//std::string genRegexBetweenStrings(const std::pair<std::string,std::string>& pair, bool inclusive = true) {
    //return genRegexBetweenStrings(std::string(std::get<0>(pair)),std::string(std::get<1>(pair)),inclusive);
//}

std::string genRegexBetweenStrings(const std::string& lhs, const std::string& rhs, bool inclusive = true) {
    std::string wrk_lhs = pcrecpp::RE::QuoteMeta(lhs);
    std::string wrk_rhs = pcrecpp::RE::QuoteMeta(rhs);
    if (lhs == rhs) {
        if (lhs.size() == 1) {
            /*
                Generate a regex that will match between two identical characters.
            */
            if (inclusive) return "((" + wrk_lhs + ")([^" + wrk_rhs + "]*)(" + wrk_rhs + "))";
            else {
                return "((?<=" + wrk_lhs + ")([^" + wrk_rhs + "]*)(?=" + wrk_rhs + "))";
            }
        }
        else {
            /*
                Generate a regex that will match between two identical strings.
            */
            return std::string();
        }
    }
    else {
        /*
            Generate a regex that will match between two non-identical strings
        */
        std::string regex = "(";
        if (inclusive) regex += "(" + wrk_lhs + ")";
        regex += "(?<=" + wrk_lhs + ")((([^" + wrk_lhs + wrk_rhs + "])|(?R))*)(?=" + wrk_rhs + ")";
        if (inclusive) regex += "(" + wrk_rhs + ")";
        regex += ")";
        return regex;
    }
}

std::string genNotBetweenStrings(const std::string& lhs, const std::string& rhs) {
    std::string regex;
    std::string wrk_lhs = pcrecpp::RE::QuoteMeta(lhs);
    std::string wrk_rhs = pcrecpp::RE::QuoteMeta(rhs);
    if (lhs == rhs) { 
        if (lhs.size() == 1) {
            regex = (std::string("([^") + lhs + "](?=(?:[^" + lhs + "]*" + lhs + "[^" + lhs + "]*" + lhs + ")*[^" + lhs +"]*\\Z))+");
        }
        
    }
    else {
        //Todo, if it's possible
    }
    return regex;
}

#ifndef PARSE_TYPE_DEFAULTS
#define PARSE_TYPE_DEFAULTS
typedef uintmax_t uint_type;
typedef double prec_type;
#endif

template<class T1, class T2>
bool oneOf(const T1& item, const T2& container, const uint_type size) {
    for (uint_type i = 0; i < size; i++) {
        if (container[i] == item) return true;
    }
    return false;
}

template<class T1, class T2>
bool oneOf(const T1& item, const T2& container) {
    return oneOf<T1,T2>(item,container,container.size());
}

namespace RegexHelper {
    std::vector<std::pair<std::string,uint_type> > getListOfMatches(const pcrecpp::RE& regex, const std::string& content) {
        /*
            Static function, returns a vector of pairs that contain a string and
            an unsigned integer. The string part of the pair is the matched    string,
            while the unsigned integer is the position within the orginal content string.
        */
        std::vector<std::pair<std::string, uint_type> > matches;
        if (regex.PartialMatch(content)) {
            pcrecpp::StringPiece wrk_content(content);
            std::string matched_text;
            uint_type cursor = 0;
            while (regex.FindAndConsume(&wrk_content, &matched_text)) {
                cursor = content.find(matched_text, cursor);
                matches.push_back(std::pair<std::string,uint_type>(matched_text,cursor));
                cursor++;
            }
            return matches;
        }
        else {
            /*
                There are no possible matches, return the empty vector.
            */
            return matches;
        }
    }
    
    std::string nthMatch(const pcrecpp::RE& regex, const std::string& content, uint_type match_number = 0) {
        /*
            0-indexed nth match for a regex in string. if that match doesn't exist then
            an empty string is returned.
        */
        if (regex.PartialMatch(content)) {
            pcrecpp::StringPiece wrk_content(content);
            std::string matched_text;
            uint_type i = 0; //The current match
            while (regex.FindAndConsume(&wrk_content,&matched_text)) {
                if (i == match_number) return matched_text;
                i++;
            }
            /*
                nth match was not found.
            */
        }
        return std::string();
    }
    
    std::string firstMatch(const pcrecpp::RE& regex, const std::string& content) {
        /*
            Returns the first match of a regex, or an empty string if there are no matches.
        */
        return nthMatch(regex,content,0);
    }
    
    std::string lastMatch(const pcrecpp::RE& regex, const std::string& content) {
        /*
            Returns the last match of a regex, or an empty string if there are no matches.
        */
        if (regex.PartialMatch(content)) {
            pcrecpp::StringPiece wrk_content(content);
            std::string matched_text;
            std::string last_matched_text;
            do {
                last_matched_text = matched_text;
            } while(regex.FindAndConsume(&wrk_content, &matched_text));
            return last_matched_text;
        }
        return std::string();
    }
    
    void briefOnMatches(const std::string& regtxt, const std::string& content) {
        pcrecpp::RE reg(regtxt);
        std::cout << "Matches for regex " << regtxt << ":" << std::endl;
        auto matches = RegexHelper::getListOfMatches(reg,content);
        for (uint_type i = 0; i < matches.size(); i++) {
            std::cout << "\t@" << std::get<1>(matches[i]) << "\tstr: " << std::get<0>(matches[i]) << std::endl; 
        }
    }
    
    bool isContainedBy(const std::string& content, uint_type index, const std::pair<std::string,std::string>& between) {
        /*
            Static function, checks if a character within a string is contained
            between two other specified strings.
            
            Example: isContainedBy("Hello world", 4,{"l","l"}) would return false
            as the first instance of the containers are ajacent to eachother at
            indexes 2 & 3. If the containment strings were {"ll","w"} then the
            function would return true however as index 4 is between an instace
            of the containers.
            
            Being between the containers INCLUDES being part of the container
            strings.
        */
        /*
            This function, now implemented with PCRE, is costly. Avoid, unless you
            don't really mind. Text parsing is kind of slow no matter what really.
        */
        pcrecpp::RE reg_p = EBNF_REGEX_BETWEEN(std::get<0>(between),std::get<1>(between));
        std::vector<std::pair<std::string,uint_type> > matches = getListOfMatches(reg_p,content);
        if (matches.size() > 0) {
            //for (uint_type i = 0; i < matches.size(); i++) std::cout << "@ " << std::get<1>(matches[i]) << " str: " << std::get<0>(matches[i]) << std::endl;
            //exit(0);
            for (uint_type i = 0; i < matches.size(); i++) {
                /*
                    Check if the index is between any instance.
                */
                if (std::get<1>(matches[i]) <= index && index < std::get<1>(matches[i]) + std::get<0>(matches[i]).size()) return true;
                //if (index == std::get<1>(matches[i])) return true;
            }
            /*
                Control reches here before finding an instance, therefore no instance
                contains the index
            */
            return false;
        }
        else {
            /*
                There are no matches for the containment strings, so it is impossible
                for the index to be within any instance.
            */
            return false;
        }
    }
    
    std::vector<bool> matchMask(const pcrecpp::RE& regex, const std::string& content) {
        /*
            Returns a vector of bools the same size as the content string, where each index
            in the vector represents the content string's matched
        */
        std::vector<bool> mask(content.size(), false);
        auto matches = getListOfMatches(regex,content);
        for (uint_type i = 0; i < matches.size(); i++) {
            for (uint_type str_index = 0; str_index < matches[i].first.size(); str_index++) {
                mask[str_index + matches[i].second] = true;
            }
        }
        return mask;
    }
    
    uint_type skipThrough(const std::string& content, uint_type index, const std::pair<std::string,std::string>& between) {
        /*
            Static function, moves the index integer to an index outside of
            the container strings if it is between an instance. Returns the
            next index outside of the container strings.
        */
        if (isContainedBy(content,index,between)) {
            return skipThrough(content,content.find(std::get<1>(between), index) + std::get<1>(between).size(),between);
        }
        else {
            return index;
        }
    }
    
    std::string strip(const std::string& regex, const std::string& content) {
        auto matches = getListOfMatches(regex,content);
        std::string wrk_content(content);
        for (uint_type iter = matches.size() - 1; iter >= 0 &&  iter < matches.size(); iter--) {
            wrk_content.erase(matches[iter].second,matches[iter].first.size());
        }
        return wrk_content;
    }
    
    std::vector<std::pair<std::string,uint_type > > splitBetweenCharRaw(const std::string& between, const std::string& content) {
        std::string regex = "([^";
        regex += pcrecpp::RE::QuoteMeta(between);
        regex += "]+)";
        return getListOfMatches(regex,content);
    }
    
    std::vector<std::string> splitBetweenChar(const std::string& between, const std::string& content) {
        std::vector<std::string> result;
        auto matches = splitBetweenCharRaw(between,content);
        for (uint_type iter = 0; iter < matches.size(); iter++) {
            result.push_back(matches[iter].first);
        }
        return result;
    }

};
#endif
