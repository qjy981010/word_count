#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <assert.h>
#include <thread>
#include <algorithm>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>

#define CHILDTHREADNUM 3

#include "trie_tree.h"

Trie_tree mytree;
std::mutex mymutex;
std::vector< std::pair<int, char*> > word_list;
typedef std::vector<std::pair<int, char*>>::iterator vec_it;
char blockover[] = "$$"; // break out flag
Pair* breakout = new Pair(0, blockover); // break out flag

void move_words_to_tree(int sock) {
	char* str = new char[128];
	for (;;) {
		read(sock, str, 128);
		if (*str == '$' && *(str+1) == '$') break;
		mytree.insert(str);
		printf("%s\n", str);
	}
	delete str;
}

size_t getFilesize(const char* filename) {
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}

void readfile(const char* path)
{
	if (fork() == 0) {
		/*fork1: count*/
		int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
		char sun_path[108] = "/tmp/test_socket";
		struct sockaddr_un servaddr;
		servaddr.sun_family = AF_UNIX;
		strcpy(servaddr.sun_path, sun_path);
		//sleep(1);
		while (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
			printf("connect failed\n");;
		//	exit(1);
		}
		move_words_to_tree(sockfd);
		exit(0);
	}
	else {
		if (fork() == 0) {
			/*fork2: count*/
			int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
			char sun_path[108] = "/tmp/test_socket";
			struct sockaddr_un servaddr;
			servaddr.sun_family = AF_UNIX;
			strcpy(servaddr.sun_path, sun_path);
			//sleep(1);
			while (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
				printf("connect failed\n");;
			//	exit(1);
			}
			move_words_to_tree(sockfd);
			exit(0);
		}
		else {
			/*main: read and sort*/
			size_t filesize = getFilesize(path);
			int fd = open(path, O_RDONLY, 0);
			assert(fd != -1);
			char* file = (char*)mmap(NULL, filesize, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0); // memory map
			char content[filesize+1];
			strcpy(content, file);
			munmap(file, filesize);
			char* word = content, *it = content;

			int sockfd = socket(AF_UNIX, SOCK_STREAM, 0), cnt = 0;
			char sun_path[108] = "/tmp/test_socket";
			struct sockaddr_un servaddr;
			servaddr.sun_family = AF_UNIX;
			strcpy(servaddr.sun_path, sun_path);

			assert(bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != -1);
			assert(listen(sockfd, 2) != -1);
			int conns[2];
			conns[0] = accept(sockfd, (struct sockaddr*)NULL, NULL);
			conns[1] = accept(sockfd, (struct sockaddr*)NULL, NULL);

			for (; *it != '\0'; ++it) {
				if (*it == ' ' || *it == '\t' || *it == '\n') {
					*(it++) = '\0';
					write(conns[cnt&1], word, 128);
					/*write*/
					word = it;
					++cnt;
				}
			}
			write(conns[1], word, 128);
			write(conns[0], blockover, 128);
			write(conns[1], blockover, 128);

			while(wait(NULL) > 0);
			remove("/tmp/test_socket");
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
	return 0;
}

/*
Note:
debug tricks: std::cout << strerror(errno);
*/