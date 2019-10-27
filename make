dbms: dbms.o 
	g++ dbms.o -o dbms


dbms.o: dbms.cpp
	g++ -c dbms.cpp


clean:
	rm *.o 
