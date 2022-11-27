all: client serverC serverCS serverEE serverM

client:
	g++ -std=c++11 -o client client.cpp
 
serverC:
	g++ -std=c++11 -o serverC serverC.cpp

serverCS:
	g++ -std=c++11 -o serverCS serverCS.cpp

serverEE:
	g++ -std=c++11 -o serverEE serverEE.cpp

serverM:
	g++ -std=c++11 -o serverM serverM.cpp

t:
	g++ -std=c++11 -o test test.cpp

clean:
	rm -f client serverC serverCS serverEE serverM test
