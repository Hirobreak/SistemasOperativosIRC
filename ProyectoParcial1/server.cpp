#include<iostream>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<unistd.h>

using namespace std;

int main(){
	int client, server;
	int portNum = 9000;
	bool isExit = false;
	int bufsize = 512;
	char buffer[bufsize];

	struct sockaddr_in server_addr;
	socklen_t size;

	client = socket(AF_INET, SOCK_STREAM, 0);

	if (client < 0){
		cout << "Error trying to connect..." << endl;
		exit(1);
	}

	cout << "Server Socket Created" << endl;

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htons(INADDR_ANY);
	server_addr.sin_port = htons(portNum);

	if (bind(client, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
		cout << "Error binding socket..." << endl;
		exit(1);
	}

	size = sizeof(server_addr);


	cout << "Listening for clients" << endl;
	listen(client, 1);

	server = accept(client, (struct sockaddr*)&server_addr, &size);

	if (server < 0){
		cout << "Error accepting..." << endl;
		exit(1);
	}

	while (server > 0){
		memset(buffer, 0, bufsize);
		strcpy(buffer, "Hello, Hello! Server connected...\n");
		send(server, buffer, bufsize, 0);
		memset(buffer, 0, bufsize);

		cout << "Connected with client..." << endl;
		cout << "Enter # to end connection" << endl;

		cout << "Client: ";
		do {
			recv(server, buffer, bufsize, 0);
			cout << buffer << "";
			if (*buffer == '#'){
				*buffer = '*';
				isExit = true;
			}else{
				memset(buffer, 0, bufsize);
			}
		} while (*buffer != '*');
		memset(buffer, 0, bufsize);
		do {
			cout << "\nServer: ";
			do {
				cin >> buffer;
				strcat(buffer, "\n");
				send(server, buffer, bufsize, 0);
				if (*buffer == '#'){
					send(server, buffer, bufsize, 0);
					*buffer = '*';
					isExit = true;
				}else{
					memset(buffer, 0, bufsize);
				}
			} while(*buffer != '*');
			memset(buffer, 0, bufsize);
			cout << "Client: ";

			do{
				recv(server, buffer, bufsize, 0);
				cout << buffer << "";
				if (*buffer == '#'){
					*buffer = '*';
					isExit = true;
				}else{
					memset(buffer, 0, bufsize);
				}
			} while (*buffer != '*');

		} while (!isExit);

		cout << "Connection terminated..." << endl;
		cout << "Goodbye..." << endl;
		isExit = false;
		exit(1);
	}
	close(client);
	return 0;
}
