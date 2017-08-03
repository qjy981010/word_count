count : count_faster.o hashtable.o
	g++ count_faster.o hashtable.o -o count -pthread -g
count_faster.o : count_faster.cpp
	g++ -c count_faster.cpp -g
hashtable.o : hashtable.h hashtable.cpp
	g++ -c hashtable.cpp hashtable.h -g
clean :
	rm *.o
