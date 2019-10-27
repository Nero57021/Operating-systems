dbms: dbms.o 
	g++ -std=c++11 dbms.o -o dbms -lpthread


dbms.o: dbms.cpp
	g++ -std=c++11 -c dbms.cpp


clean:
	rm *.o 
