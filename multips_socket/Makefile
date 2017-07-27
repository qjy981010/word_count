mutips_count : trie_tree.o multips_count_socket.o
	g++ multips_count_socket.o trie_tree.o -o multips_count -pthread -g
mutips_count_socket.o : multips_count_socket.cpp
	g++ -c multips_count_socket.cpp -g
trie_tree.o : trie_tree.h trie_tree.cpp
	g++ -c trie_tree.h trie_tree.cpp -g
clean :
	rm *.o *.gch
