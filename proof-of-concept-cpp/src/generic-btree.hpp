#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#ifndef PARSE_TYPE_DEFAULTS
#define PARSE_TYPE_DEFAULTS
typedef uintmax_t uint_type;
typedef double prec_type;
#endif

template<class T>
class BNode {
	protected:
		T* self;
		BNode<T>* left;
		BNode<T>* right;
	public:
		BNode() {
			this->self = nullptr;
			this->left = nullptr;
			this->right = nullptr;
		}
		
		BNode(T value) : BNode() {
			this->self = new T(value);
		}
		
		BNode(const BNode<T>& tocopy) : BNode() {
			this->self = new T(*tocopy.value);
			if (tocopy.left != nullptr) {
				this->left = new BNode<T>(*tocopy.left);
			}
			if (tocopy.right != nullptr) {
				this->right = new BNode<T>(*tocopy.right);
			}
		}
		
		BNode(BNode<T>&& tomove) {
			this->self = tomove.self;
			tomove.self = nullptr;
			this->left = tomove.left;
			tomove.left = nullptr;
			this->right = tomove.right;
			tomove.right = nullptr;
		}
		
		~BNode() {
			delete this->self;
			this->self = nullptr;
			delete this->left;
			this->left = nullptr;
			delete this->right;
			this->right = nullptr;
		}
		
		size_t size() const {
			if (this->self != nullptr) {
				size_t total = 1;
				if (this->left != nullptr) {
					total += this->left->size();
				}
				if (this->right != nullptr) {
					total += this->right->size();
				}
				return total;
			}
			else {
				return 0;
			}
		}
};

struct EBNFTree {
	public:
	
		/*
			EBNF contaiment strings
		*/
		const std::vector<std::vector<std::string> > ebnf_cont_str = {
				//Comment
				{"(*","*)"},
				//Quote
				//dq
				{"\"", "\""},
				//sq
				{"'", "'"},
				//Group
				{"(", ")"},
				//Option
				{"[","]"},
				//Repetition
				{"{","}"},
				//Special
				{"?","?"}};
			
			//Macros
			#define EBNF_S_COMMENT ebnf_cont_str[0]
			#define EBNF_S_SINGLEQ ebnf_cont_str[1]
			#define EBNF_S_DOUBLEQ ebnf_cont_str[2]
			#define EBNF_S_GROUP   ebnf_cont_str[3]
			#define EBNF_S_OPTION  ebnf_cont_str[4]
			#define EBNF_S_REPEAT  ebnf_cont_str[5]
			#define EBNF_S_SPECIAL ebnf_cont_str[6]
			#define EBNF_S_ALL ebnf_cont_str
			
		/*
			Static function, checks if a character within a string is contained
			between two other specified characters.
			
			Example: isContainedBy("Hello world", 4,{"l","l"}) would return false
			as the first instance of the containers are ajacent to eachother at
			indexes 2 & 3. If the containment strings were {"ll","w"} then the
			function would return true however as index 4 is between an instace
			of the containers.
		*/
		static bool isContainedBy(const std::string& content, uint_type index, const std::vector<std::string>& between) {
			/*
				First check that the string has content, the index is within
				bounds and the vector containing the 
			*/
			if (content.size() == 0 || index == 0 || index > content.size() || between.size() < 2) return false;
			/*
				Scan each instance of the two container strings, checking if the
				index is between any of them.
			*/
			for (uint_type f_index = 0; f_index < content.size() && f_index != std::string::npos; f_index = content.find(between[1],f_index + between[0].size()) + between[0].size()) {
				if (index > f_index + between[0].size() && index < content.find(between[1],f_index + between[0].size())) return true;
			}
			return false;
		}
		
		/*
			Static function, checks if a string is Extended Backus-Naur Form.
			Has to be hardcoded, as a basis is needed for the program. Returns
			true if the string fits the rules, false otherwise.
		*/
		static bool isEBNF(const std::string& content) {
			static auto getRule = [](const std::string& str) -> std::string {
				//Extracts a substring from the content that contains
				return "";
			};
			//First check the string has content.
			if (content.size() == 0) return false;
			return false;
		}
		BNode<std::string> top;
		std::string rules;
		
		
};
