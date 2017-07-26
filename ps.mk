mutips_count : trie_tree.o blockqueue.o multips_count.o
	g++ -o multips_count multips_count.o trie_tree.o blockqueue.o -pthread -g
mutips_count.o : multips_count.cpp
	g++ -c multips_count.cpp -g
blockqueue.o : blockqueue.h blockqueue.cpp
	g++ -c blockqueue.h blockqueue.cpp -g
trie_tree.o : trie_tree.h trie_tree.cpp
	g++ -c trie_tree.h trie_tree.cpp -g
clean :
	rm *.o *.gch
