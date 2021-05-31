FLAGS := -std=c++11

main: scanner.o parser.o main.o ast.o
	g++ $(FLAGS) scanner.o parser.o ast.o main.o -o main

scanner.o: parser.y scanner.l 
	bison --defines=parser.y.h -o parser.y.cpp parser.y
	flex -o scanner.l.cpp scanner.l
	g++ $(FLAGS) -c scanner.l.cpp -o scanner.o

parser.o:
	g++ $(FLAGS) -c parser.y.cpp -o parser.o

main.o: main.cpp
	g++ $(FLAGS) -c main.cpp -o main.o

ast.o: AST.cpp
	g++ $(FLAGS) -c AST.cpp -o ast.o

.PHONY: clean

clean:
	rm -f *.o
	rm -f scanner.l.*
	rm -f parser.y.*
	rm -f main