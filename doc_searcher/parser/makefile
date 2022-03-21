FLAG=-std=c++11 -I ../../install/cppjieba-master/include -lboost_filesystem -lboost_system -ljsoncpp

libparser.so:parser.cpp parser.hpp
	g++ parser.cpp -o $@ -shared -fPIC $(FLAG)

.PHONY:clean
clean:
	rm -rf libparser.so