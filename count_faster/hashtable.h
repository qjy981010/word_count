#ifndef HASHTABLE_H_
#define HASHTABLE_H_

#include <vector>
#include <mutex>
#include <iostream>
#include <string.h>

extern std::mutex mymutex;

class Hashtable
{
public:
	struct Hash_node
	{
		Hash_node(char* _key, int _val): val(_val) { strcpy(key, _key); }
		Hash_node* next = nullptr;
		char key[128];
		int val;
	};
	typedef Hash_node* node_ptr;
	void insert(char* key);
	unsigned size();
	unsigned bucketsNum();
	void moveToVector(std::vector<node_ptr>& target_vector, int begin, int end);
private:
	unsigned bucket_num = 389; // number of buckets
	unsigned word_num = 0;
	std::vector<node_ptr> buckets = std::vector<node_ptr>(bucket_num, (node_ptr)nullptr);
	int hash_func(char* key);
	char size_index = -1;
	void resize();
	const unsigned long size_list[18] = {769, 1543, 3079, 6151, 12289, 24593, 49157, 98317, 196613, 393241, 786433,1572869, 3145739, 6291469, 12582917, 25165843, 50331653, 100663319};
};

#endif
