FLAG=-std=c++11 -I ../../install/cppjieba-master/include -lpthread 


main:main.cpp 
	g++ $^ -o $@ $(FLAG)

.PHONY:clean
clean:
	rm -rf main