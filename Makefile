count : count.o trie_tree.o blockqueue.o
	g++ count.o trie_tree.o blockqueue.o -o count -pthread -g
count.o : count.cpp
	g++ -c count.cpp -g
#mutips_count : trie_tree.o mutips_count.o
#	g++ trie_tree.o mutips_count.o -o mutips_count -pthread -g
#mutips_count.o : mutips_count.cpp blockqueue.o
#	g++ -c blockqueue.o mutips_count.cpp -g
blockqueue.o : blockqueue.h blockqueue.cpp
	g++ -c blockqueue.h blockqueue.cpp -g
trie_tree.o : trie_tree.h trie_tree.cpp
	g++ -c trie_tree.h trie_tree.cpp -g
clean :
	rm *.o *.gch
