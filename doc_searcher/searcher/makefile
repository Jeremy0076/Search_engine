FLAG=-std=c++11 -I ../../install/cppjieba-master/include -lboost_filesystem -lboost_system -ljsoncpp

libsearcher.so:searcher.cpp searcher.hpp
	g++ searcher.cpp -o $@ -shared -fPIC $(FLAG)

.PHONY:clean
clean:
	rm -rf libsearcher.so
