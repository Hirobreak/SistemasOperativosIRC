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
#include<pthread.h>
#include<sys/fcntl.h>

using namespace std;

struct usuario {
  string nombre;
  int whispering;
  int channel;
} ;

pthread_mutex_t buff, full, empty;
int sender = -1;
int bufsize = 512;
char buffer[512];
int msize = 1024;
char mensaje_server[1024];
bool isExit = false;
string names[10];
usuario usuarios[25];
time_t start = time(0);
char builtTime[32];
char startTime[32];
void *connection_handler(void *);
void *server_handler(void *socket_desc);
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
	pthread_create( &thread_id , NULL ,  server_handler, (void*) &server);

	while(client = accept(server, (struct sockaddr*)&client_addr, &size)){
		cout << "Connected with client..." << endl;
		fcntl(client, F_SETFL, O_NONBLOCK);
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

	close(server);
	return 0;
}

void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int length = 0;
    //char mensaje[512];
    string nombre;

    //memset(buffer, 0, bufsize);
	strcpy(buffer, "Connected...\n");
	strcpy(buffer, "For info about this server type /INFO\n");
	send(sock, buffer, bufsize, 0);
	//memset(buffer, 0, bufsize);

	do {
		pthread_mutex_lock(&empty);//SECCION CRITICA
		pthread_mutex_lock(&buff);
		memset(buffer, 0, bufsize);
		length = recv(sock, buffer, bufsize, 0);
		if(length < 1){
			sender = -1;
		}else{
			sender = sock;
			cout << "mensaje de " << sock << endl;
		}

		if(length >= 1){
			cout << "Im here " << sock << endl;
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
				//memset(buffer, 0, bufsize);
			}
		}
		pthread_mutex_unlock(&buff);
		pthread_mutex_unlock(&full);
		
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
	cout << "que hay en el buffer " << buffer << endl;
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

void *server_handler(void *server_desc){

 	while(true){
		pthread_mutex_lock(&full);
		pthread_mutex_lock(&buff);

		if(sender > 0){
			cout << "por aqui ando" << endl;
			for( int i = 0; i < 25; i++){
				memset(mensaje_server, 0, msize);
				if(usuarios[sender].nombre.empty()){
					strcat(mensaje_server, "Anonimo");
				}else{
					strcat(mensaje_server, usuarios[sender].nombre.c_str());
				}
				strcat(mensaje_server, ": ");
				strcat(mensaje_server, buffer);

				if(i != sender && !usuarios[i].nombre.empty()){
					//strcpy(mensaje_server, "Habla Flaco!!! ");
					send(i, mensaje_server, msize, 0);
				}
			}
			cout << "liberar locks" << endl;
			sender = -1;
		}
		
		pthread_mutex_unlock(&buff);
		pthread_mutex_unlock(&empty);
	}
    return 0;
} 