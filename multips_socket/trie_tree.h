#ifndef TRIE_TREE_H_
#define TRIE_TREE_H_

#include <vector>
#include <utility>
#include <mutex>
#include <string.h>
#include <unistd.h>

extern std::mutex mymutex;

struct Pair {
	Pair(int a, char* b):first(a){
		strcpy(second, b);
	}
	int first;
	char second[128];
};

class Trie_tree
{
public:
	unsigned size() const;
	void insert(char* element, int num = 1);
	void moveToVector(std::vector< std::pair<int, char*> >& v, char begin, char end);
	void moveToPipe(int pfd);

private:
	struct Node_base // the node of the tree
	{
		typedef Node_base* Base_ptr;
		Base_ptr children[28] = {nullptr};
		int count = 0; // the number of this word, root->count is the number of words;
	};
	typedef Node_base* Base_ptr;

	Base_ptr root = new Node_base;

public:
	void moveRecursively(Base_ptr h, std::vector< std::pair<int, char*> >& v, char* old_str, char depth);
	void moveRecursively(Base_ptr h, int pfd, char* old_str, char depth);
};

#endif