/*简单的可以重整的哈希表*/
#include "hashtable.h"

using namespace std;

int Hashtable::hash_func(char* key) {
	int b     = 378551;
	int a     = 63689;
	unsigned long hash = 0;
	for(char* i = key; *i != '\0'; i++) {
		hash = hash * a + *i;
		a    = a * b;
	}
	return hash % bucket_num;
}

void Hashtable::insert(char* key) {
	if (!*key) return;
	int index = hash_func(key);
	node_ptr h; // head of list
	std::lock_guard<std::mutex> lock(mymutex);
	h = buckets[index];
	if (!h) {
		++word_num;
		// std::lock_guard<std::mutex> lock(mymutex);
		buckets[index] = new Hash_node(key, 1);
		if (word_num > bucket_num) resize();
		return;
	}
	for (int i = 1; ; h = h->next, i++) {
		if (!strcmp(h->key, key)) {
			++h->val;
			return;
		}
		// std::lock_guard<std::mutex> lock(mymutex);
		if (!h->next) {
			++word_num;
			node_ptr newone = new Hash_node(key, 1);
			h->next = newone;
			if (word_num > bucket_num) resize();
			return;
		}
	}
}

void Hashtable::resize() {
	unsigned oldsize = bucket_num;
	bucket_num = size_list[++size_index];
	vector<node_ptr> tmp(bucket_num, (node_ptr) 0); // 新的vector
	for (unsigned bucket = 0; bucket < oldsize; ++bucket) {
		node_ptr first = buckets[bucket];
		while (first) { //重整vector时，每次将原节点插入新vector对应的bucket的最前端，避免了对bucket的遍历
			int new_bucket = hash_func(first->key);
			buckets[bucket] = first->next;
			first->next = tmp[new_bucket];
			tmp[new_bucket] = first;
			first = buckets[bucket];
		}
	}
	buckets.swap(tmp); // 交换新旧vector
}

unsigned Hashtable::size() {
	return word_num;
}

unsigned Hashtable::bucketsNum() {
	return bucket_num;
}

void Hashtable::moveToVector(std::vector<node_ptr>& target_vector, int begin, int end) {
	for (int i = begin; i < end; ++i) {
		while (buckets[i]) {
			{
				std::lock_guard<std::mutex> lock(mymutex);
				target_vector.push_back(buckets[i]);
			}
			buckets[i] = buckets[i]->next;
		}
	}
}
