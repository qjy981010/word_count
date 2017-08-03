#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <thread>
#include <algorithm>
#include <fstream>
#include <dirent.h>

#include "hashtable.h"

#define CHILD_THREAD_NUM 1

std::mutex mymutex;
std::vector<Hashtable::node_ptr> word_list;
std::vector<Hashtable::node_ptr> temp;
typedef std::vector<Hashtable::node_ptr>::iterator vec_it;
std::thread threads[CHILD_THREAD_NUM];
Hashtable mytable;
int filenum = 0;

void count(char* begin, char* end) {
	char word[128], *it = begin, *it_of_word = word;
	while (it < end) {
		if (*it == ' ' || *it == '\n' || *it == '\t') {
			*it_of_word = '\0';
			mytable.insert(word);
			++it;
			while (*it == ' ' || *it == '\n' || *it == '\t') ++it;
			it_of_word = word;
		}
		else {
			*(it_of_word++) = *(it++);
		}
	}
	*it_of_word = '\0';
	mytable.insert(word);
}

size_t getFileSize(const char* filename) {
    struct stat st;
    stat(filename, &st);
    printf("%d\n", st.st_mode & S_IEXEC);
    if (st.st_mode & S_IEXEC) return 0;
    return st.st_size;
}

void readfile(const char* path) {
	filenum++;
	try {
		if (DIR* dir = opendir(path)) { // if path is a dir, read the files recursively
			char* filename, num = strlen(path);
			dirent* file;
			while(file = readdir(dir)) {
				if (file->d_name[0] == '.' || file->d_type != DT_REG) continue; // ignore hiden file
				filename = new char[256];
				strcpy(filename, path);
				if (filename[num-1] != '/') {
					filename[num] = '/';
					filename[num+1] = '\0';
				}
				strcat(filename, file->d_name);
			    readfile(filename);
			    delete[] filename;
			}
			closedir(dir);
		}
		else {
			size_t filesize = getFileSize(path);
			if (!filesize) return;
			int fd = open(path, O_RDONLY, 0), gap = filesize / (CHILD_THREAD_NUM + 1), i = 0;
			if (fd < 0) throw std::ifstream::failure("Failed to open the file!\n");
			char* file = (char*)mmap(NULL, filesize, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0); // memory map
			char* bound[CHILD_THREAD_NUM + 2];
			for (; i < CHILD_THREAD_NUM + 1; ++i) bound[i] = file + i * gap;
			bound[CHILD_THREAD_NUM + 1] = file + filesize;
			for (i = 1; i < CHILD_THREAD_NUM + 1; ++i) {
				while (*bound[i] != ' ' && *bound[i] != '\n' && *bound[i] != '\t' && *bound[i] != '\0') ++bound[i];
				threads[i-1] = std::thread{count, bound[i-1], bound[i]};
			}
			count(bound[CHILD_THREAD_NUM], bound[CHILD_THREAD_NUM+1]);
			for (i = 0; i < CHILD_THREAD_NUM; ++i) threads[i].join();
		}
	}
	catch(std::ifstream::failure e) {
		std::cerr << "Failed to open/read the file!\n";
		exit(1);	
	}
}

void move_words_to_vector(int begin, int end) {
	mytable.moveToVector(word_list, begin, end);
}

bool comp(Hashtable::node_ptr a, Hashtable::node_ptr b) {
	return a->val > b->val;
}

void mysort(vec_it begin, vec_it end) {
	std::sort(begin, end, comp);
}

void merge(vec_it begin, vec_it mid, vec_it end, vec_it result) {
	vec_it begin_from_mid = mid;
	for (; begin < mid && begin_from_mid < end; ++result) {
		if ((*begin)->val > (*begin_from_mid)->val) {
			*result = *begin;
			++begin;
		}
		else {
			*result = *begin_from_mid;
			++begin_from_mid;
		}
	}
	if (begin_from_mid == end) {
		for (; begin < mid; ++begin, ++result) {
			*result = *begin;
		}
	}
	else {
		for (; begin_from_mid < end; ++begin_from_mid, ++result) {
			*result = *begin_from_mid;
		}
	}
}

int main(int argc, char const *argv[])
{
	if (argc != 2) {
		std::cerr << "usage: " << argv[0] << " <filename>\n";
		return 1;
	}
	readfile(argv[1]);

	int word_num = mytable.size();
	word_list.reserve(word_num);

	// move words to vector
	int i, gap = mytable.bucketsNum() / (CHILD_THREAD_NUM + 1);
	for (i = 0; i < CHILD_THREAD_NUM; ++i) {
		threads[i] = std::thread{move_words_to_vector, gap * i, gap * (i + 1)};
	}
	move_words_to_vector(gap * CHILD_THREAD_NUM, mytable.bucketsNum());
	for (i = 0; i < CHILD_THREAD_NUM; ++i) threads[i].join();

	// divide and sort, use four threads to sort
	int bound[3] = {word_num >> 2, word_num >> 1, 3 * (word_num >> 2)};
	std::thread sort0{mysort, word_list.begin(), word_list.begin() + bound[0]};
	std::thread sort1{mysort, word_list.begin() + bound[0], word_list.begin() + bound[1]};
	std::thread sort2{mysort, word_list.begin() + bound[1], word_list.begin() + bound[2]};
	std::sort(word_list.begin() + bound[2], word_list.end(), comp);
	sort0.join();
	sort1.join();
	sort2.join();

	// merge sort
	temp.reserve(word_num);
	std::thread merge0{merge, word_list.begin(), word_list.begin()+bound[0], word_list.begin()+bound[1], temp.begin()};
	merge(word_list.begin()+bound[1], word_list.begin()+bound[2], word_list.end(), temp.begin()+bound[1]);
	merge0.join();
	// std::cout << "merge over\n";
	merge(temp.begin(), temp.begin()+bound[1], temp.begin()+word_num, word_list.begin());
	// std::sort(word_list.begin(), word_list.end());

	// print and delete
	for (int i = 0; i != word_num; ++i) {
		printf("%s: %d\n", word_list[i]->key, word_list[i]->val);
	}
	return 0;
}