count : trie_tree.o count.o blockqueue.o
	g++ trie_tree.o count.o blockqueue.o -o count -pthread -g
count.o : count.cpp blockqueue.o
	g++ -c blockqueue.o count.cpp -g
blockqueue.o : blockqueue.h blockqueue.cpp
	g++ -c blockqueue.h blockqueue.cpp
trie_tree.o : trie_tree.h trie_tree.cpp
	g++ -c trie_tree.h trie_tree.cpp -g
clean :
	rm *.o *.gch
