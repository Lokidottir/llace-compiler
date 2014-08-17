#include <fstream>
#include <sstream>
#include <string>
#include <vector>

template<class T>
class BNode {
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

class BasicParseTree {
	public:
		BNode<std::string> top;
	
};
