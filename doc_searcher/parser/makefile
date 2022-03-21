FLAG=-std=c++11 -I ../../install/cppjieba-master/include -D _GLIBCXX_USE_CXX11_ABI=0 -L /usr/lib64/ -lboost_system -lboost_filesystem -ljsoncpp

libparser.so:parser.cpp parser.hpp
	g++ parser.cpp -o $@ -shared -fPIC $(FLAG)

.PHONY:clean
clean:
	rm -rf libparser.so
