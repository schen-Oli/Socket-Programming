all: client serverC serverCS serverEE serverM

client:
	g++ -o client client.cpp
 
serverC:
	g++ -o serverC serverC.cpp

serverCS:
	g++ -o serverCS serverCS.cpp

serverEE:
	g++ -o serverEE serverEE.cpp

serverM:
	g++ -o serverM serverM.cpp

t:
	g++ -o test test.cpp

clean:
	rm -f client serverC serverCS serverEE serverM test
