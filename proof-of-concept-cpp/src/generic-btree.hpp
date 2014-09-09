#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <regex>

#ifndef PARSE_TYPE_DEFAULTS
#define PARSE_TYPE_DEFAULTS
typedef uintmax_t uint_type;
typedef double prec_type;
#endif

template<class T>
class BNode {
	public:
		T* self;
		BNode<T>* left;
		BNode<T>* right;
		
		BNode() {
			this->self = nullptr;
			this->left = nullptr;
			this->right = nullptr;
		}
		
		BNode(const T& value) : BNode() {
			this->self = new T(value);
		}
		
		BNode(const BNode<T>& copy) : BNode() {
			this->self = new T(*copy.value);
			if (copy.left != nullptr) {
				this->left = new BNode<T>(*copy.left);
			}
			if (copy.right != nullptr) {
				this->right = new BNode<T>(*copy.right);
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

template<class T>
class Trie {
	public:
		T self;
		std::vector<Trie<T> > data;
		
		Trie() : self(T()), data(std::vector<Trie<T> >()) {
		}
		
		Trie(const T& value) : Trie() {
			this->self = value;
		}
		
		Trie(const Trie<T>& copy) : Trie() {
			this->self = copy.self;
			this->data = copy.data;
		}
		
		Trie(Trie<T>&& move) : Trie() {
			std::swap(this->self,move.self);
			std::swap(this->data,move.data);
		}
		
		~Trie() {
		}
		
		Trie<T>& leftmost() {
			return this->data[0];
		}
		
		Trie<T>& rightmost() {
			return this->data[this->data.size() - 1];
		}
		
		size_t size() {
			size_t total_size = 1;
			for (size_t i = 0; i < this->data.size(); i++) {
				total_size += this->data[i].size();
			}
			return total_size;
		}
		
		bool isterminal() {
			return this->data.size() == 0;
		}
};
