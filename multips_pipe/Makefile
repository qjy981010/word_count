mutips_count : trie_tree.o multips_count.o
	g++ multips_count.o trie_tree.o -o multips_count -pthread -g
mutips_count.o : multips_count.cpp
	g++ -c multips_count.cpp -g
trie_tree.o : trie_tree.h trie_tree.cpp
	g++ -c trie_tree.h trie_tree.cpp -g
clean :
	rm *.o *.gch
