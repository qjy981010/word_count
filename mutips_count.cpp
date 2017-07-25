#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <thread>
#include <algorithm>

#include "trie_tree.h"
#include "blockqueue.h"

ThreadQueue myqueue;
Trie_tree mytree;
std::mutex mymutex;
std::vector< std::pair<int, char*> > word_list;
std::vector< std::pair<int, char*> > temp;
typedef std::vector<std::pair<int, char*>>::iterator vec_it;
Pair* breakout = new Pair(0, nullptr); // break out flag

void move_words_to_tree(int pfd) {
	char str[128];
	for (;;) {
		read(pfd, str, 128);
		if (*str == '$' && *(str+1) == '$') break;
		mytree.insert(str);
		printf("%s\n", str);
	}
}

void merge_tree(int pfd) { // merge the result of two child
	Pair* word = new Pair(0, nullptr);
	for (;;) {
		read(pfd, word, 16);
		if (!word->first) break;
		mytree.insert(word->second, word->first);
		printf("%s: %p\n", word->second, word);
	}	
}

size_t getFilesize(const char* filename) {
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}

void readfile(char* path)
{
	int pfds1[2]; // the pipe from parent to child
	int pfds2[2]; // the pipe from child to parent
	if (pipe(pfds1) == 0 && pipe(pfds2) == 0) {
		if (fork() == 0) {
			/*fork1: count*/
			move_words_to_tree(pfds1[0]);
			printf("fork1 task1 finished\n");
			mytree.moveToPipe(pfds2[1]);
			write(pfds2[1], breakout, 16); // tell the parent to break the dead loop
			printf("fork1 task2 finished\n");
			exit(0);
		}
		else {
			if (fork() == 0) {
				/*fork2: count*/
				move_words_to_tree(pfds1[0]);
				printf("fork2 task1 finished\n");
				mytree.moveToPipe(pfds2[1]);
				write(pfds2[1], breakout, 16); // tell the parent to break the dead loop
				write(pfds2[1], breakout, 16);
				printf("fork2 task2 finished\n");
				exit(0);
			}
			else {
				/*main: read and sort*/
				size_t filesize = getFilesize(path);
				int fd = open(path, O_RDONLY, 0), cnt = 0;
				assert(fd != -1);
				char* file = (char*)mmap(NULL, filesize, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0); // memory map
				char content[filesize+1];
				strcpy(content, file);
				munmap(file, filesize);
				char* word = content, *it = content, blockover[3] = "$$"; // break out flag
				for (; *it != '\0'; ++it) {
					if (*it == ' ' || *it == '\t' || *it == '\n') {
						*(it++) = '\0';
						write(pfds1[1], word, 128);
						word = it;
						++cnt;
					}
				}
				write(pfds1[1], word, 128);
				write(pfds1[1], blockover, 128); // tell the children to break the dead loop
				write(pfds1[1], blockover, 128);
				printf("main task1 finished\n");

				std::thread merge_tree0{merge_tree, pfds2[0]}; // start two to merge
				std::thread merge_tree1{merge_tree, pfds2[0]};
				merge_tree(pfds2[0]);
				merge_tree0.join();
				merge_tree1.join();
				printf("main task2 finished\n");

				while(wait(NULL) > 0);
			}
		}
	}
}

// next are the copy of count.cpp

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
	char filename[] = "test.txt";
	readfile(filename);

	// move words to vector
	word_list.reserve(mytree.size());

	std::thread move_words_to_vector0{move_words_to_vector, 0, 9};
	std::thread move_words_to_vector1{move_words_to_vector, 9, 18};
	move_words_to_vector(18, 26);

	move_words_to_vector0.join();
	move_words_to_vector1.join();

	// divide and sort
	int n = word_list.size(), bound[3] = {n >> 2, n >> 1, 3 * (n >> 2)};

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