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

#define EBNF_REGEX_BETWEEN(lhs,rhs) std::string("((")+lhs+")(?<="+lhs+")(([^"+lhs+rhs+"]|(?R))*)(?="+rhs+")("+rhs+"))"
#define EBNF_REGEX_BETWEEN_SINGLE_CHARS(lhs,rhs) std::string("((")+lhs+")([^"+rhs+"]*)("+rhs+"))"

#define EBNF_REGEX_COMMENT EBNF_REGEX_BETWEEN("\\(\\*","\\*\\)")
#define EBNF_REGEX_ID "(([a-zA-Z0-9]|_)([a-zA-Z0-9]|_| )+([a-zA-Z0-9]))(?=(\\s*)(\\=)((.|\\s)*)(;))"
#define EBNF_REGEX_RULE "(?<=\\=)([^;]*)(?=;)"
#define EBNF_REGEX_IDDECLR "(([a-zA-Z0-9_]([a-zA-Z0-9_]|\\s)+)\\=[^;]*;)"
#define EBNF_REGEX_TERMSTR "(((\")(([^\"]|(\\\\\"))*)(\"))|((')([^']*)(')))"

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
			an unsigned integer. The string part of the pair is the matched	string,
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
	
	std::string& strip(const pcrecpp::RE& regex, std::string& content) {
		pcrecpp::StringPiece wrk_content(content);
		std::string matched_text;
		if (regex.PartialMatch(content)) {
			while(regex.FindAndConsume(&wrk_content,&matched_text)) {
				content.erase(content.find(matched_text),matched_text.size());
			}
		}
		return content;
	}
};

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
		
	public:
	
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
				EBNF_ERROUT << "there are no regexes to evaluate." << std::endl;
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
			for (uint_type i = 0; i < str1.size(); i++) {
				std::cout << RegexHelper::isContainedBy(str1,i,EBNF_S_REPEAT);
			}
			std::cout << std::endl;
			RegexHelper::briefOnMatches(EBNF_REGEX_BETWEEN(pcrecpp::RE::QuoteMeta("{"),pcrecpp::RE::QuoteMeta("}")), str1);
		}
};
