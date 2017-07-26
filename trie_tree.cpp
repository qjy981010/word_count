#include "trie_tree.h"

unsigned Trie_tree::size() const {
	return root->count;
}

void Trie_tree::insert(char* key, int num) {
	char* p = key;
	for (; *key ; ++key) {
		if (*key <= 'z' && *key >= 'a' || *key <= 'Z' && *key >= 'A') break;
	}
	for (char* i = key + strlen(key) - 1; i >= key; --i) {
		if (*i <= 'z' && *i >= 'a' || *i <= 'Z' && *i >= 'A' || *i == '\'') break;
		else *i = '\0';
	}
	if (!*key) return;
	
	{
    	std::lock_guard<std::mutex> lock(mymutex); // lock
		Base_ptr node = root;
		while(*p) {
			if (*p <= 'z' && *p >= 'a') {
				if (!node->children[*p-'a']) node->children[*p-'a'] = new Node_base;
				node = node->children[*p-'a'];
			}
			else if (*p <= 'Z' && *p >= 'A') {
				if (!node->children[*p-'A']) node->children[*p-'A'] = new Node_base;
				node = node->children[*p-'A'];
			}
			else if (*p == '\'') {
				if (!node->children[26]) node->children[26] = new Node_base;
				node = node->children[26];
			}
			else if (*p == '-') {
				if (!node->children[27]) node->children[27] = new Node_base;
				node = node->children[27];
			}
			++p;
		}
		if (node->count) ++root->count;
		node->count += num;
	}
}

void Trie_tree::moveToVector(std::vector< std::pair<int, char*> >& v, char begin, char end) {
	for (int i = begin; i < end; ++i) {
		if (root->children[i]) {
			char* str = new char[128];
			str[0] = 'a' + i;
			str[1] = '\0';
			moveRecursively(root->children[i], v, str, 1);
		}
	}
}

void Trie_tree::moveRecursively(Base_ptr h, std::vector< std::pair<int, char*> >& v, char* old_str, char depth) {
	if (h->count) {
    	std::lock_guard<std::mutex> lock(mymutex); // lock
		v.push_back(std::make_pair(h->count, old_str));
	}
	for (int i = 0; i < 28; ++i) {
		if (h->children[i]) {
			char* str = new char [128];
			strcpy(str, old_str);
			if (i < 26) str[depth] = 'a' + i;
			else if (i == 26) str[depth] = '\'';
			else str[depth] = '-';
			str[depth+1] = '\0';
			moveRecursively(h->children[i], v, str, depth + 1);
		}
	}
	delete h;
}

void Trie_tree::moveToPipe(int pfd) {
	for (int i = 0; i < 26; ++i) {
		if (root->children[i]) {
			char* str = new char[128];
			str[0] = 'a' + i;
			str[1] = '\0';
			moveRecursively(root->children[i], pfd, str, 1);
		}
	}
}

void Trie_tree::moveRecursively(Base_ptr h, int pfd, char* old_str, char depth) {
	for (int i = 0; i < 28; ++i) {
		if (h->children[i]) {
			char* str = new char [128];
			strcpy(str, old_str);
			if (i < 26) str[depth] = 'a' + i;
			else if (i == 26) str[depth] = '\'';
			else str[depth] = '-';
			str[depth+1] = '\0';
			moveRecursively(h->children[i], pfd, str, depth + 1);
		}
	}
	if (h->count) {
		Pair* p = new Pair(h->count, old_str);
		write(pfd, p, 132);
	}
	else delete old_str;
	delete h;
}