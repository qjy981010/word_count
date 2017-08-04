count : count_fastest.o hashtable.o
	g++ count_fastest.o hashtable.o -o count -pthread -g
count_fastest.o : count.cpp
	g++ -c count.cpp -g -o count_fastest.o
hashtable.o : hashtable.h hashtable.cpp
	g++ -c hashtable.cpp hashtable.h -g
clean :
	rm *.o
