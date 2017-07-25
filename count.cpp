#include "trie_tree.h"
#include "blockqueue.h"
#include <thread>
#include <algorithm>

ThreadQueue myqueue;
Trie_tree mytree;
char breakout = '$';
std::mutex mymutex;
std::vector< std::pair<int, char*> > word_list;
std::vector< std::pair<int, char*> > temp;
typedef std::vector<std::pair<int, char*>>::iterator vec_it;

void read_by_word(char* filename) {
	if (FILE *fp = fopen(filename, "r")) {
		char* str;
		int i;
		for (;;) {
			str = new char[128];
			fscanf(fp, "%s", str);
			myqueue.push(str);
			if (feof(fp)) break;
		}
		str = &breakout;
		myqueue.push(str);
		myqueue.push(str);
		myqueue.justgo();
		fclose(fp);
	}
}

void move_words_to_tree() {
	for (;;) {
		char* str = new char[128];
		// if (myqueue.empty() && breakout) return;
		myqueue.pop(&str);
		if (*str == '$') return;
		mytree.insert(str);
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
	// for (vec_it i = begin; i < mid; ++i) std::cout << (*i).first << '\n';
	// std::cout << "################################################################" << '\n';
	// for (vec_it i = mid; i < end; ++i) std::cout << (*i).first << '\n';
	// std::cout << "################################################################" << '\n';
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
	char filename[] = "text.txt";
	
	std::thread move_words_to_tree0{move_words_to_tree};
	std::thread move_words_to_tree1{move_words_to_tree};
	read_by_word(filename);
	
	move_words_to_tree0.join();
	move_words_to_tree1.join();
	word_list.reserve(mytree.size());
	
	std::thread move_words_to_vector0{move_words_to_vector, 0, 9};
	std::thread move_words_to_vector1{move_words_to_vector, 9, 18};
	move_words_to_vector(18, 26);
	
	move_words_to_vector0.join();
	move_words_to_vector1.join();
	
	int n = word_list.size(), bound[3] = {n >> 2, n >> 1, 3 * (n >> 2)};

	std::thread sort0{mysort, word_list.begin(), word_list.begin() + bound[0]};
	std::thread sort1{mysort, word_list.begin() + bound[0], word_list.begin() + bound[1]};
	std::thread sort2{mysort, word_list.begin() + bound[1], word_list.begin() + bound[2]};
	std::sort(word_list.begin() + bound[2], word_list.end(), comp);

	sort0.join();
	sort1.join();
	sort2.join();

	// for (int i = 0; i != n; ++i) {
	// 	printf("%s: %d\n", word_list[i].second, word_list[i].first);
	// }
	// printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");

	temp.reserve(n);
	std::thread merge0{merge, word_list.begin(), word_list.begin()+bound[0], word_list.begin()+bound[1], temp.begin()};
	std::thread merge1{merge, word_list.begin()+bound[1], word_list.begin()+bound[2], word_list.end(), temp.begin()+bound[1]};

	merge0.join();
	merge1.join();

	merge(temp.begin(), temp.begin()+bound[1], temp.begin()+n, word_list.begin());

	for (int i = 0; i != n; ++i) {
		printf("%s: %d\n", word_list[i].second, word_list[i].first);
	}
	return 0;
}
