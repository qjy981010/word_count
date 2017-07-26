#include "trie_tree.h"
#include "blockqueue.h"
#include <thread>
#include <algorithm>
#include <dirent.h>

#define CHILDTHREADNUM 3

ThreadQueue myqueue;
Trie_tree mytree;
std::mutex mymutex;
std::vector< std::pair<int, char*> > word_list;
std::vector< std::pair<int, char*> > temp;
typedef std::vector< std::pair<int, char*> 	>::iterator vec_it;
std::thread threads[CHILDTHREADNUM];

void move_words_to_tree() {
	char* str;
	for (;;) {
		myqueue.pop(&str);
		if (*str == '$' && *(str+1) == '$') return;
		mytree.insert(str);
		delete str;
	}
}

void read_by_word(const char* path) { // the main thread read the file, and two child thread count
	if (DIR* dir = opendir(path)) { // if path is a dir, read the files recursively
		char* filename, num = strlen(path);
		dirent* file;
		while(file = readdir(dir)) {
			if (file->d_name[0] == '.') continue; // ignore . and ..
			filename = new char[256];
			strcpy(filename, path);
			filename[num] = '/';
			filename[num+1] = '\0';
			strcat(filename, file->d_name);
		    read_by_word(filename);
		}
		closedir(dir);
	}
	else if (FILE *fp = fopen(path, "r")) { // if path is a file
		char* str, blockover[] = "$$";
		int i;
		for (i = 0; i < CHILDTHREADNUM; ++i) {
			threads[i] = std::thread{move_words_to_tree};
		}
		//std::thread move_words_to_tree0{move_words_to_tree};
		//std::thread move_words_to_tree1{move_words_to_tree};
		while (!feof(fp)) {
			str = new char[128];
			fscanf(fp, "%s", str); // read word by word, iostream may waste the memory, so i only included cstdio
			myqueue.push(str);                // memory map may be faster, it's used in the mutips_count.cpp
		}
		fclose(fp);
		str = blockover;
		for (i = 0; i < CHILDTHREADNUM; ++i) myqueue.push(str);
		myqueue.justgo();
		for (i = 0; i < CHILDTHREADNUM; ++i) threads[i].join();
		// move_words_to_tree0.join();
		// move_words_to_tree1.join();
	}
}

void move_words_to_vector(char begin, char end) {
	mytree.moveToVector(word_list, begin, end);
}

bool comp(std::pair<int, char*> a, std::pair<int, char*> b) {
	return a.first > b.first;
}

void mysort(vec_it begin, vec_it end) {
	std::sort(begin, end, comp);
}

void merge(vec_it begin, vec_it mid, vec_it end, vec_it result) {
	vec_it beginfrommid = mid;
	for (; begin < mid && beginfrommid < end; ++result) {
		if (*begin > *beginfrommid) {
			*result = *begin;
			++begin;
		}
		else {
			*result = *beginfrommid;
			++beginfrommid;
		}
	}
	if (beginfrommid == end) {
		for (; begin < mid; ++begin, ++result) {
			*result = *begin;
		}
	}
	else {
		for (; beginfrommid < end; ++beginfrommid, ++result) {
			*result = *beginfrommid;
		}
	}
}

int main(int argc, char const *argv[]) {
	// char filename[] = "txt";

	// read and count
	if (argc == 1) {
		printf("usage: %s <filename>\n", argv[0]);
		return 1;
	}
	read_by_word(argv[1]);
	int n = mytree.size();
	word_list.reserve(n);

	// move words to vector
	int i, gap = 26 / (CHILDTHREADNUM + 1);
	for (i = 0; i < CHILDTHREADNUM; ++i) {
		threads[i] = std::thread{move_words_to_vector, gap * i, gap * (i + 1)};
	}
	move_words_to_vector(gap * i, 26);
	for (i = 0; i < CHILDTHREADNUM; ++i) threads[i].join();

	// divide and sort, use four threads to sort
	int bound[3] = {n >> 2, n >> 1, 3 * (n >> 2)};
	std::thread sort0{mysort, word_list.begin(), word_list.begin() + bound[0]};
	std::thread sort1{mysort, word_list.begin() + bound[0], word_list.begin() + bound[1]};
	std::thread sort2{mysort, word_list.begin() + bound[1], word_list.begin() + bound[2]};
	std::sort(word_list.begin() + bound[2], word_list.end(), comp);
	sort0.join();
	sort1.join();
	sort2.join();

	// merge sort
	temp.reserve(n);
	std::thread merge0{merge, word_list.begin(), word_list.begin()+bound[0], word_list.begin()+bound[1], temp.begin()};
	std::thread merge1{merge, word_list.begin()+bound[1], word_list.begin()+bound[2], word_list.end(), temp.begin()+bound[1]};
	merge0.join();
	merge1.join();
	merge(temp.begin(), temp.begin()+bound[1], temp.begin()+n, word_list.begin());

	// print and delete
	for (int i = 0; i != n; ++i) {
		printf("%s: %d\n", word_list[i].second, word_list[i].first);
		delete word_list[i].second;
	}
	return 0;
}
