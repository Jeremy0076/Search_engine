FLAG=-std=c++11 -I ../../install/cppjieba-master/include -lpthread -lboost_filesystem -lboost_system -ljsoncpp -L ../parser -lparser -L ../searcher -lsearcher


main:main.cpp 
	g++ $^ -o $@ $(FLAG)

.PHONY:clean
clean:
	rm -rf main
