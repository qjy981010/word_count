#count : trie_tree.o count.o blockqueue.o
#	g++ trie_tree.o count.o blockqueue.o -o count -pthread -g
#count.o : count.cpp blockqueue.o
#	g++ -c blockqueue.o count.cpp -g
mutips_count : trie_tree.o mutips_count.o blockqueue.o
	g++ trie_tree.o mutips_count.o blockqueue.o -o mutips_count -pthread -g
mutips_count.o : mutips_count.cpp blockqueue.o
	g++ -c blockqueue.o mutips_count.cpp -g
blockqueue.o : blockqueue.h blockqueue.cpp
	g++ -c blockqueue.h blockqueue.cpp -g
trie_tree.o : trie_tree.h trie_tree.cpp
	g++ -c trie_tree.h trie_tree.cpp -g
clean :
	rm *.o *.gch
