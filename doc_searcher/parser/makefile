FLAG=-std=c++11 -I /opt/homebrew/Cellar/boost/1.76.0 -lboost_filesystem -lboost_system 

parser:parser.cpp
	g++ $^ -o $@ ${FLAG}

.PHONY:clean
clean:
	rm -f parser