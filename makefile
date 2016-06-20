all : compile
compile :
	g++ -o server1 step1_server.cpp
	g++ -o client1 step1_client.cpp
	g++ -o server2 step2_server.cpp
	g++ -o client2 step2_client.cpp
	g++ -o server3 step3_server.cpp
	g++ -o client3 step3_client.cpp
	g++ -o server4 step4_server.cpp
	g++ -o client4 step4_client.cpp
	g++ -o server5 step5_server.cpp
	g++ -o client5 step5_client.cpp
	g++ -o server6 step6_server.cpp
	g++ -o client6 step6_client.cpp
clean : 
	rm server1 client1 server2 client2 server3 client3 server4 client4 server5 client5 server6 client6
