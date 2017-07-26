#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <assert.h>
#include <thread>
#include <algorithm>

#include "trie_tree.h"

#define CHILDTHREADNUM 3

Trie_tree mytree;
std::mutex mymutex;
std::vector< std::pair<int, char*> > word_list;
typedef std::vector<std::pair<int, char*>>::iterator vec_it;
char blockover[] = "$$"; // break out flag
Pair* breakout = new Pair(0, blockover); // break out flag

void move_words_to_tree(int pfd) {
	char* str = new char[128];
	for (;;) {
		read(pfd, str, 128);
		if (*str == '$' && *(str+1) == '$') break;
		mytree.insert(str);
	}
	delete str;
}

void merge_tree(int pfd) { // merge the result of two child
	Pair* word = new Pair(0, blockover);
	for (;;) {
		read(pfd, word, 132);
		if (!word->first) break;
		mytree.insert(word->second, word->first);
	}
	delete word;
}

size_t getFilesize(const char* filename) {
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}

void readfile(const char* path)
{
	int pfds1[2]; // the pipe from parent to child
	int pfds2[2]; // the pipe from child to parent
	if (pipe(pfds1) == 0 && pipe(pfds2) == 0) {
		if (fork() == 0) {
			/*fork1: count*/
			move_words_to_tree(pfds1[0]);
			mytree.moveToPipe(pfds2[1]);
			for (int i = 0; i < (CHILDTHREADNUM >> 1) + 1; ++i) {
				write(pfds2[1], breakout, 132); // tell the parent to break the dead loop
			}
			exit(0);
		}
		else {
			if (fork() == 0) {
				/*fork2: count*/
				move_words_to_tree(pfds1[0]);
				mytree.moveToPipe(pfds2[1]);
				for (int i = 0; i < CHILDTHREADNUM - (CHILDTHREADNUM >> 1); ++i) {
					write(pfds2[1], breakout, 132); // tell the parent to break the dead loop
				}
				exit(0);
			}
			else {
				/*main: read and sort*/
				size_t filesize = getFilesize(path);
				int fd = open(path, O_RDONLY, 0), i;
				assert(fd != -1);
				char* file = (char*)mmap(NULL, filesize, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0); // memory map
				char content[filesize+1];
				strcpy(content, file);
				munmap(file, filesize);
				char* word = content, *it = content;
				for (; *it != '\0'; ++it) {
					if (*it == ' ' || *it == '\t' || *it == '\n') {
						*(it++) = '\0';
						write(pfds1[1], word, 128);
						word = it;
					}
				}
				write(pfds1[1], word, 128);
				write(pfds1[1], blockover, 128); // tell the children to break the dead loop
				write(pfds1[1], blockover, 128);
				std::thread threads[CHILDTHREADNUM];

				for (i = 0; i < CHILDTHREADNUM; ++i) {
					threads[i] = std::thread(merge_tree, pfds2[0]);
				}
				merge_tree(pfds2[0]);
				for (i = 0; i < CHILDTHREADNUM; ++i) threads[i].join();

				while(wait(NULL) > 0);
			}
		}
	}
}

// the following is almost the copy of count.cpp

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


int main(int argc, char const *argv[])
{
	// char filename[] = "text.txt";
	if (argc == 1) {
		printf("usage: %s <filename>\n", argv[0]);
		return 1;
	}
	readfile(argv[1]);
	
	int n = mytree.size();
	word_list.reserve(n);
	std::thread threads[CHILDTHREADNUM];
	std::vector< std::pair<int, char*> > temp;

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

/*
Note:
debug tricks: std::cout << strerror(errno);
*/