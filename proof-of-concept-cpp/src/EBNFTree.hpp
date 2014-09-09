#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <regex>
#include <pcrecpp.h>
#include <sstream>

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

#define EBNF_REGEX_BETWEEN(lhs,rhs) std::string("(("+lhs+")(?<="+lhs+")(([^"+lhs+rhs+"]|(?R))*)(?="+rhs+")("+rhs+"))")
#define EBNF_REGEX_BETWEEN_SINGLE_CHARS(lhs,rhs) std::string("(("+lhs+")([^"+rhs+"]*)("+rhs+"))")

#define EBNF_REGEX_COMMENT "(\\(\\*)(?<=\\(\\*)((\\n|.)*)(?=\\*\\))(\\*\\))"
#define EBNF_REGEX_IDDECLR "(([a-zA-Z0-9]|_| )+)(\\s*)(?=(=)((.|\\n)*)(;))"
#define EBNF_REGEX_TERMSTR "((\")([^\"]*)(\"))|((')([^']*)('))"

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

struct EBNFTree {
	public:
		
		static std::vector<std::pair<std::string,uint_type> > getListOfMatches(pcrecpp::RE& reg, const std::string& content) {
			/*
				Static function, returns a vector of pairs that contain a string and
				an unsigned integer. The string part of the pair is the matched	string,
				while the unsigned integer is the position within the orginal content string.
			*/
			std::vector<std::pair<std::string, uint_type> > matches;
			if (reg.PartialMatch(content)) {
				pcrecpp::StringPiece wrk_content(content);
				std::string matched_text;
				uint_type cursor = 0;
				while (reg.FindAndConsume(&wrk_content, &matched_text)) {
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
		
		static void testProgram();
		static void briefOnMatches(const std::string&, const std::string&);
		
		static bool isContainedBy(const std::string& content, uint_type index, const std::pair<std::string,std::string>& between) {
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
			std::string lhs, rhs;
			lhs = pcrecpp::RE::QuoteMeta(std::get<0>(between));
			rhs = pcrecpp::RE::QuoteMeta(std::get<1>(between));
			pcrecpp::RE reg_p((lhs != rhs) ? EBNF_REGEX_BETWEEN(lhs,rhs) : EBNF_REGEX_BETWEEN_SINGLE_CHARS(lhs,rhs));
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
		
		static uint_type skipThrough(const std::string& content, uint_type index, const std::pair<std::string,std::string>& between) {
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
		
		bool loadIdentifiers(const std::string& content) {
			/*
				Member for loading identifiers into the object, returns true if loading
				was successful, false if there were any syntactic problems with 
			*/
			/*
				Valid identifiers will not be within a terminal string or comment and
				be immmediately preceded by either a space or a "=" also not commented
				or within a terminal string.
			*/
			static std::vector<std::pair<std::string,std::string> > notBeWithin = {EBNF_S_COMMENT, EBNF_S_DOUBLEQ, EBNF_S_SINGLEQ};
			std::vector<std::string> identifiers;
			return false;
		}
		
		bool loadEBNF(const std::string& content) {
			/*
				Member for loading a string representing an EBNF grammar into the
				object.
			*/
			//First check the string has content.
			if (content.size() == 0) {
				std::cerr << "(EBNF interpreter) Error: string is empty, cannot read." << std::endl;
				return false;
			}
			//Find identifiers
			this->loadIdentifiers("(**)");
			
			return false;
		}
		
		std::map<std::string, std::string> id_rule_map;
};

void EBNFTree::briefOnMatches(const std::string& regtxt, const std::string& content) {
	pcrecpp::RE reg(regtxt);
	std::cout << "Matches for regex " << regtxt << ":" << std::endl;
	auto matches = EBNFTree::getListOfMatches(reg,content);
	for (uint_type i = 0; i < matches.size(); i++) {
		std::cout << "\t@" << std::get<1>(matches[i]) << "\tstr:" << std::get<0>(matches[i]) << std::endl; 
	}
}

void EBNFTree::testProgram() {
	std::string str1 = "something (* This (* is a *) (* comment *) *) {}(**) s { \"A string! abcd.\" { recursive D: } }";
	std::cout << "for string \"" << str1 << "\" the areas that count as contained by comments are:" << std::endl;
	std::cout << str1 << std::endl;
	for (uint_type i = 0; i < str1.size(); i++) {
		std::cout << EBNFTree::isContainedBy(str1,i,EBNF_S_COMMENT);
	}
	std::cout << std::endl;
	EBNFTree::briefOnMatches(EBNF_REGEX_BETWEEN(pcrecpp::RE::QuoteMeta("(*"),pcrecpp::RE::QuoteMeta("*)")), str1);
	std::cout << "areas contained by {}:" << std::endl;
	std::cout << str1 << std::endl;
	for (uint_type i = 0; i < str1.size(); i++) {
		std::cout << EBNFTree::isContainedBy(str1,i,EBNF_S_REPEAT);
	}
	std::cout << std::endl;
	EBNFTree::briefOnMatches(EBNF_REGEX_BETWEEN(pcrecpp::RE::QuoteMeta("{"),pcrecpp::RE::QuoteMeta("}")), str1);
	
}
