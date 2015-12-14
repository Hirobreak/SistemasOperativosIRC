#include<iostream>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<unistd.h>
#include<string>
#include<algorithm>
#include<time.h>
#include<cassert>

using namespace std;

struct usuario {
  string nombre;
  int whispering;
  int channel;
} ;

int bufsize = 512;
char buffer[512];
bool isExit = false;
string names[10];
usuario usuarios[25];
time_t start = time(0);
char builtTime[32];
char startTime[32];
void *connection_handler(void *);
void parse_command(char *textInput, int socket);
void show_info(int socket);
void builtDateTime();
void startDateTime();
void change_nickname(int socket);

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
	builtDateTime();
	startDateTime();
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
    
    //char mensaje[512];
    string nombre;

    memset(buffer, 0, bufsize);
	strcpy(buffer, "Connected...\n");
	strcpy(buffer, "For info about this server type /INFO\n");
	send(sock, buffer, bufsize, 0);
	memset(buffer, 0, bufsize);

	do {
		recv(sock, buffer, bufsize, 0);

		//strcpy(mensaje, buffer);
		if (usuarios[sock].nombre.empty()){
			cout << "Anonimo " << sock << ": ";
		}else{
			cout << usuarios[sock].nombre << ": ";
		}
		cout << buffer << "";
		if (*buffer == '#'){
			*buffer = '*';
			isExit = true;
		}else if (*buffer == '/'){
			parse_command(buffer, sock);
		}else{
			memset(buffer, 0, bufsize);
		}
		
	} while (*buffer != '*');
    


    //CLOSING CONNECTION
	if (usuarios[sock].nombre.empty()){
		cout << "Anonimo " << sock << " left the room..." << endl;
	}else{
		cout << usuarios[sock].nombre << " left the room..." << endl;
	}

	memset(buffer, 0, bufsize);
	strcpy(buffer, "Disconnected... :(\n");
	send(sock, buffer, bufsize, 0);
	memset(buffer, 0, bufsize);
	close(sock);
    return 0;
} 


void parse_command(char *textInput, int socket){
	char textBuffer[512];
	char textParsing[512];
	char *textContent;
	memset(textBuffer, 0, bufsize);
	strcpy(textBuffer, textInput);

	int i;
	if (textInput == NULL)
		return;
	textContent = strtok(textBuffer, "\r\n");
	strcpy(textParsing, textContent);
	if (strcmp(textParsing,"/INFO")==0){
		show_info(socket);
	}else if(strcmp(strtok(textContent, " "), "/NICK") == 0){
		change_nickname(socket);
	}

}

void show_info(int socket){
	char textMessage[512];
	string msgString;
	memset(textMessage, 0, bufsize);

	strcpy(textMessage, "Server info:\nServer version: 1.0\nServer compiled: ");
	strcat(textMessage, builtTime);
	strcat(textMessage, "\nServer started: ");
	strcat(textMessage, startTime);
	strcat(textMessage, "\nOther commands: /JOIN /LIST /MOTD /NAMES /NICK /PART /PRIVMSG /QUIT /SETNAME /TIME /USER /USERS /VERSION\n");
	send(socket, textMessage, strlen(textMessage), 0);
}

void builtDateTime() {
	static const char *built = __DATE__" "__TIME__; 
    struct tm t;
  	const char *ret = strptime(built, "%b %d %Y %H:%M:%S", &t);
  	assert(ret);
    char buf[80];
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &t);
    strcpy(builtTime, buf);
}

void startDateTime() {
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&start);
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
    strcpy(startTime, buf);
}

void change_nickname(int socket){
	char* mensaje = (char*)calloc(strlen(buffer)+1, sizeof(char));
	char * word;
	strcpy(mensaje, buffer);
	word = strtok(mensaje, " \r");
	word = strtok(NULL, " \r");
	if (sizeof(word)>0){
		cout << usuarios[socket].nombre << " Changing Name To " << word << endl;
		usuarios[socket].nombre = string(word);
	}else{
		cout << "Name not changed \n" << endl;
	}
	free(mensaje);
}