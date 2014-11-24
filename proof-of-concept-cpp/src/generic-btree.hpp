#ifndef GENERIC_TOPOLOGY_HPP
#define GENERIC_TOPOLOGY_HPP
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <utility>

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
            if (this->data.size() > 0)return this->data[0];
            else return *this;
        }
        
        Trie<T>& rightmost() {
            if (this->data.size() > 0) return this->data[this->data.size() - 1];
            else return *this;
        }
        
        size_t size() const {
            size_t total_size = 1;
            for (size_t i = 0; i < this->data.size(); i++) {
                total_size += this->data[i].size();
            }
            return total_size;
        }
        
        bool isterminal() {
            return this->data.size() == 0;
        }
        
        Trie<T>& operator= (const Trie<T>& copy) {
            this->self = copy.self;
            this->data = copy.data;
            return *this;
        }
};

template<class T>
class Stack {
    private:
        struct Node {
            
            Node* node;
            T data;
            
            Node() : node(nullptr), data(T()) {
            }
            
            Node(const Node& copy) : Node(){
                this->data = copy.data;
                if (copy.node != nullptr) this->node = new Node(*copy.node);
            }
            
            Node(Node&& move) : Node() {
                std::swap(this->data,move.data);
                std::swap(this->node,move.node);
            }
            
            Node(const T& data) : Node() {
                this->data = data;
            }
            
            ~Node() {
                this->clean();
            }
            
            void clean() {
                if (this->node != nullptr) this->node->clean();
                delete this->node;
                this->node = nullptr;
            }
            
        };
        
        Node* stk;
        size_t logical_size;
        
        Node* topNode() {
            Node* ptr = this->stk;
            if (ptr != nullptr) while(ptr->node != nullptr) {
                ptr = ptr->node;
            }
            return ptr;
        }
        
    public:
        
        Stack() {
            this->stk = nullptr;
            this->logical_size = 0;
        }
        
        Stack(const Stack<T>& copy) : Stack() {
            this->stk = new Node(*copy.stk);
            this->logical_size = copy.logical_size;
        }
        
        Stack(Stack<T>&& move) : Stack() {
            this->stk = move.stk;
            move.stk = nullptr;
            this->logical_size = move.logical_size;
        }
        
        Stack(const std::vector<T>& copyvec) : Stack() {
            for (uint_type i = 0; i < copyvec.size(); i++) this->push(copyvec[i]);
        }
        
        ~Stack() {
            delete this->stk;
        }
        
        Stack<T>& operator=(const Stack<T>& copy) {
            this->stk = new Node(*copy.stk);
            this->logical_size = copy.logical_size;
            return *this;
        }
        
        T& operator[] (size_t index) const {
            size_t wrk_index = 0;
            Node* ptr = this->stk;
            if (ptr != nullptr) while (wrk_index < index && ptr->node != nullptr) {
                ptr = ptr->node;
            }
            return ptr->data;
        }
        
        T pop() {
            T val;
            Node* ptr = this->stk;
            if (ptr == nullptr) {
                return T();
            }
            else if (ptr->node == nullptr) {
                std::swap(val,ptr->data);
                
            }
            else {
                while (ptr->node->node != nullptr) {
                    ptr = ptr->node;
                }
                std::swap(val,ptr->node->data);
                delete ptr->node;
                ptr->node = nullptr;
            }
            if (this->logical_size > 0) this->logical_size--;
            return val;
        }
        
        void push(const T& data) {
            if (this->stk != nullptr) this->topNode()->node = new Node(data);
            else this->stk = new Node(data);
            this->logical_size++;
        }
        
        bool contains(const T& val) const {
            Node* ptr = this->stk;
            if (ptr != nullptr) while (ptr->node != nullptr) {
                if (ptr->data == val) return true;
                ptr = ptr->node;
            }
            return false;
        }
        
        operator std::vector<T> () const {
            std::vector<T> vec(this->size());
            for (size_t i = 0; i < this->size(); i++) {
                vec[i] = (*this)[i];
            }
            return vec;
        }
        
        size_t size() const {
            return this->logical_size;
        }
        
        T top() const {
            Node* ptr = this->topNode();
            if (ptr != nullptr) return ptr->data;
            else return T();
        }
};
#endif
