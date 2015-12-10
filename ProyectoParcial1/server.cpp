#include<iostream>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<unistd.h>
#include<string>

using namespace std;

int bufsize = 512;
char buffer[512];
bool isExit = false;
string names[10];
void *connection_handler(void *);

int main(){
	int client, server;
	int portNum = 9000;

	names[0]="Pedro";
	names[1]="Roberto";
	names[2]="Ray";
	names[3]="Bronza";
	names[4]="Omnilasher";
	names[5]="Skyline";
	names[6]="Hirobreak";
	names[7]="Pikachu";
	names[8]="Ayame";
	names[9]="Hiroshi";

	struct sockaddr_in server_addr, client_addr;
	socklen_t size;

	server = socket(AF_INET, SOCK_STREAM, 0);

	if (server < 0){
		cout << "Error trying to connect..." << endl;
		exit(1);
	}

	cout << "Server Socket Created" << endl;

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htons(INADDR_ANY);
	server_addr.sin_port = htons(portNum);

	if (bind(server, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
		cout << "Error binding socket..." << endl;
		exit(1);
	}

	size = sizeof(server_addr);


	cout << "Listening for clients" << endl;
	listen(server, 3);
	pthread_t thread_id;

	while(client = accept(server, (struct sockaddr*)&client_addr, &size)){
		cout << "Connected with client..." << endl;

		if( pthread_create( &thread_id , NULL ,  connection_handler , (void*) &client) < 0)
        {
            cout << "Couldn't create thread" << endl;
            return 1;
        }else{
        	cout << client << endl;
        }
	}

	if (server < 0){
		cout << "Error accepting..." << endl;
		exit(1);
	}

	/*while (server > 0){
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
	}*/
	close(server);
	return 0;
}

void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;

    memset(buffer, 0, bufsize);
	strcpy(buffer, "Connected...\n");
	send(sock, buffer, bufsize, 0);
	memset(buffer, 0, bufsize);

	do {
		recv(sock, buffer, bufsize, 0);
		cout << names[sock] << ": ";
		cout << buffer << "";
		if (*buffer == '#'){
			*buffer = '*';
			isExit = true;
		}else{
			memset(buffer, 0, bufsize);
		}
	} while (*buffer != '*');
    
	memset(buffer, 0, bufsize);
	strcpy(buffer, "Disconnected...\n");
	send(sock, buffer, bufsize, 0);
	memset(buffer, 0, bufsize);

    return 0;
} 
