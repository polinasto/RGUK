#include "task5.cpp"
#include <cstring>
#include <stdexcept>

class binary_heap : priority_queue {
private:
	struct node
	{
		int key;
		char* value;
		node*left,right;
		node(const char* v, const int k):key(k),left(nullptr),right(nullptr)
		{
			value=new char[strlen(v)+1];
			strcpy(value,v);
		}
		~node()
		{
			delete[] value;
			delete left;
			delete right;
			
		}
	};
	node * root=nullptr;
	
	char const * obtain_max(priority_queue *with)
	{
			if (!root)
			{
				throw std::invalid_argument("queue is empty!");
				return root->value;
			}
	}
public:
	binary_priority_queue() : priority_queue() {}

    binary_priority_queue(const char* value, int key) : priority_queue(value, key)
    {
        root = new node(value, key);
    }
	
	~binary_priority_queue() override
    {
        delete root;
    }
	static node* merge_nodes(node*first,node*second)
	{
		if (!first) return second;
		if (!second) return first;
		
		if (first->key < second->key)
		{
			std::swap(first,second);
		}
		first->right=merge_nodes(first->right,second);
		
		if(!first->left)
		{
			std::swap(first->left,first->right);
		}
		else if(first->left->key<first->right->key)
		{
			std::swap(first->left,first->right);
		}
		return first;
	}
	binary_priority_queue merge(priority_queue * with)
	{
		binary_priority_queue * other=dynamic_cast<binary_priority_queue*>(with);
		if (!other)
		{
			throw srd::invalid_argument("invalid heap type");
		}
		root=merge_nodes(root,other.root);
		other->root=nullptr;
		return this;
	}
	
	char const * obtain_max() override
	{
		if (!root)
		{
			throw std::invalid_argument("heap is empty");
		}
		return root->value;
	}
	
	char const*remove_max () override
	{
		if (!root)
		{
			throw std::invalid_argument("heap is empty");
		}
		char const* max_value=root->value;
		node* left_subtree=root->left;
		node* right_subtree=root->right;
		
		root->left=nullptr;
		root->right=nullptr;
		
		root=merge_nodes(left_subtree,right_subtree);
		return max_value;
	}
	
	
}