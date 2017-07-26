count : count.o trie_tree.o blockqueue.o
	g++ count.o trie_tree.o blockqueue.o -o count -pthread -g
count.o : count.cpp
	g++ -c count.cpp -g
blockqueue.o : blockqueue.h blockqueue.cpp
	g++ -c blockqueue.h blockqueue.cpp -g
trie_tree.o : trie_tree.h trie_tree.cpp
	g++ -c trie_tree.h trie_tree.cpp -g
clean :
	rm *.o *.gch
