

all: cache-sim 
	./cache-sim trace1.txt output.txt

cache-sim: test.cpp
	g++ test.cpp -o $@ -g


clean:
	rm -f cache-sim output.txt


debug: cache-sim
	gdb cache-sim