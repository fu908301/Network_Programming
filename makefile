all : compile
compile :
	g++ -o server1 step1_server.cpp
	g++ -o client1 step1_client.cpp
clean : 
	rm server1 client1
