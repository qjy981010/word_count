#ifndef TRIE_TREE_H_
#define TRIE_TREE_H_

#include <vector>
#include <utility>
#include <mutex>

extern std::mutex mymutex;

class Trie_tree
{
public:
	unsigned size() const;
	void insert(char* element); // 插入节点
	void moveToVector(std::vector< std::pair<int, char*> >& v, char begin, char end);

private:
	struct Node_base // 声明节点类
	{
		typedef Node_base* Base_ptr;
		Base_ptr children[28] = {nullptr};
		int count = 0;
	};
	typedef Node_base* Base_ptr;

	Base_ptr root = new Node_base;

public:
	void moveRecursively(Base_ptr h, std::vector< std::pair<int, char*> >& v, char* old_str, char depth);
};

#endif