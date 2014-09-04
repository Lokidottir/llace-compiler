#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <regex>

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

#define EBNF_REGEX_COMMENT "(\\(\\*)(?<=\\(\\*)((|\\v|.)*)(?=\\*\\))(\\*\\))"
#define EBNF_REGEX_IDDECLR "([[:alnum:]]|_)(([[:alnum:]]|_)*)(\\s*)(?=(=)((.|\\n)*)(;))"
#define EBNF_REGEX_TERMSTR "((\")([^\"]*)(\"))|((')([^']*)('))"


struct EBNFTree {
	public:
		static void testProgram();
		/*
			EBNF contaiment strings
		*/
		
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
			uint_type index = 0;
			do {
				for (uint_type i = 0; i < notBeWithin.size(); i++) {
					index = skipThrough(content,index,notBeWithin[i]);
				}
				
			} while (false);
			
			return true;
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

void EBNFTree::testProgram() {
	std::string str1 = "(*This is a comment*) abcd";
	std::cout << "for string \"" << str1 << "\" the areas that count as contained by comments are:" << std::endl;
	std::cout << str1 << std::endl;
	for (uint_type i = 0; i < str1.size(); i++) {
		std::cout << EBNFTree::isContainedBy(str1,i,EBNF_S_COMMENT);
	}
	std::cout << std::endl;
}
