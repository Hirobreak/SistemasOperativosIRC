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
  int member;
  int status;
} ;

struct channel{
	int id_canal;
	int num_usuarios_canal;
	string nombre_canal;
//	string password_canal;
	usuario *usuarios_canal;
};

pthread_mutex_t buff, full, empty;
int sender = -1;
int indice_general = 0;
int bufsize = 512;
char buffer[512];
int msize = 1024;
char mensaje_server[1024];
bool isExit = false;
usuario usuarios[25];
channel canales[25];
time_t start = time(0);
char builtTime[32];
char startTime[32];
char localTime[32];
void *connection_handler(void *);
void *server_handler(void *socket_desc);
void parse_command(char *textInput, int socket);
void show_info(int socket);
void show_time(int socket);
void join_channel(int socket);
void part_channel(int socket);
void builtDateTime();
void startDateTime();
void localDateTime();
void change_nickname(int socket);
void send_privmsg(int socket);
void show_motd(int socket);
void show_version(int socket);
void end_session(int socket);
template <typename T,unsigned S>
inline unsigned arraysize(const T (&v)[S]) { return S; }
int search_nickname(char *name);
int search_channel(char *channel_name);
int search_member(channel ,char *);

int main(){
	int client, server;
	int portNum = 9000;

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

	send(sock, "Connected...\n", strlen("Connected...\n"), 0);
	send(sock, "For info about this server type /INFO\n", strlen("For info about this server type /INFO\n"), 0);
	usuarios[sock].status = 1;

	while (usuarios[sock].status == 1){
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
			}else{
				//memset(buffer, 0, bufsize);
			}
		}
		pthread_mutex_unlock(&buff);
		pthread_mutex_unlock(&full);
		
	}
    


    //CLOSING CONNECTION
	if (usuarios[sock].nombre.empty()){
		cout << "Anonimo " << sock << " left the room..." << endl;
	}else{
		cout << usuarios[sock].nombre << " left the room..." << endl;
	}

	send(sock, "Disconnected... :(\n", strlen("Disconnected... :(\n"), 0);
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
	}else if(strcmp(strtok(textContent, " "), "/PRIVMSG") == 0){
		send_privmsg(socket);
	}else if(strcmp(textParsing,"/TIME") == 0){
		show_time(socket);
	}else if(strcmp(strtok(textContent, " "), "/JOIN") == 0){
		join_channel(socket);
	}else if(strcmp(textParsing,"/PART") == 0){
		part_channel(socket);
	}else if(strcmp(textParsing,"/MOTD") == 0){
		show_motd(socket);
	}else if(strcmp(textParsing,"/VERSION") == 0){
		show_version(socket);
	}else if(strcmp(textParsing,"/QUIT") == 0){
		end_session(socket);
	}

}

void part_channel(int socket){
	char textMessage[512];
	int indice=-1;
	char *word;
	if(usuarios[socket].member != 1){//Si el usuario no esta en ningun canal
		strcpy(textMessage, "SERVER: You are not member of a channel\n");
		send(socket, textMessage, strlen(textMessage),0);
	}else{
		int pos = 0;
		usuarios[socket].member = 0;
		strcpy(word,usuarios[socket].nombre.c_str());
		do{
			indice = search_member(canales[pos],word);
			pos++;
		}while(indice == -1);
		channel canal_deseado = canales[pos-1];//Obtengo el canal que contiene al usuario
		delete &canal_deseado.usuarios_canal[indice];//Elimino el puntero
		canal_deseado.num_usuarios_canal = canal_deseado.num_usuarios_canal-1;//Disminuyo la cantidad de usuarios
		cout<< canal_deseado.num_usuarios_canal << "\n";
		if(canal_deseado.num_usuarios_canal == 0){//Si ya no hay usuarios en el canal
			delete &canales[pos-1];//Elimino el puntero y el canal deja de existir
		}
		strcpy(textMessage, "SERVER: You has left the channel\n");
		send(socket, textMessage, strlen(textMessage),0);
	}
}


void join_channel(int socket){
	int chan;
	char *mensaje = (char*)calloc(strlen(buffer)+1, sizeof(char));
	char textMessage[512];
	char *entrada;
	string pass = "";
	strcpy(mensaje, buffer);
	entrada = strtok(mensaje, " \r");
	entrada = strtok(NULL, " \r");
	if(usuarios[socket].member > 0){
		strcpy(textMessage, "SERVER: You are already in a channel\n");
		send(socket, textMessage, strlen(textMessage),0);
	}else{
		if(sizeof(entrada)>0){
			chan = search_channel(entrada);//Me devuelve el indice para obtener el elemento desde el arreglo
			if(chan > 0){//si existe el canal
				channel can1 = canales[chan];
				if(can1.num_usuarios_canal > 30){
					/**if(strcmp(can1.password_canal, "") != 0){//requiere contraseña
						cout<< "Enter the password: ";
						char contrasena = cin.get();
						while(contrasena != 32){ //32 es codigo ASCII del Espacio
							pass.push_back(contrasena);
							cout << '*';
							contrasena=cin.get();
						}
						if(pass == can1.password_canal){
							can1.usuarios_canal[can1.num_usuarios_canal] = usuarios[socket];
							can1.num_usuarios_canal++;
							cout<< "SERVER: Connected to channel\n";
							strcpy(textMessage, "SERVER: Connected to channel\n");
							send(chan, textMessage, strlen(textMessage),0);
						}else{
							cout<< "SERVER: Incorrect password\n";
							strcpy(textMessage, "SERVER: Incorrect password\n");
							send(socket, textMessage, strlen(textMessage), 0);
						}
					}*/
					strcpy(textMessage, "SERVER: The channel is full\n");
					send(socket, textMessage, strlen(textMessage),0);
				}else{
					can1.usuarios_canal[can1.num_usuarios_canal] = usuarios[socket];
					usuarios[socket].member = can1.id_canal;
					can1.num_usuarios_canal=can1.num_usuarios_canal+1;
					if(can1.num_usuarios_canal>0){
						cout<< "Agregueeee\n";
						cout<< can1.num_usuarios_canal << "\n";

					}else{
						cout<< "no hay nadie\n";
					}
					strcpy(textMessage, "SERVER: Connected to channel\n");
					send(chan, textMessage, strlen(textMessage),0);
				}	
			}else{//si no existe el canal
				channel canal;
				cout << "canal creado\n";
				canal.nombre_canal = entrada;
				canal.num_usuarios_canal = 0;
				cout << "asignando memoria a la lista de usuarios\n";
				canal.usuarios_canal = new usuario[30];
				//Cada que se crea se asigna en la posicion 0
				canal.usuarios_canal[canal.num_usuarios_canal] = usuarios[socket];
				cout << "Se ingresa el canal a la lista total de canales\n";
				if (arraysize(canales)){
					canal.id_canal=1;
				}else{
					canal.id_canal=arraysize(canales) + 1;
				}
				canales[indice_general]= canal;//Crea el canal
				indice_general++; 
				usuarios[socket].member = 1;
				canal.num_usuarios_canal=canal.num_usuarios_canal+1;
				
				if(canal.num_usuarios_canal>0){
					cout<< "Agregueeee\n";
					cout<< canal.num_usuarios_canal << "\n";
				}else{
					cout<< "no hay nadie\n";
				}
				
				strcpy(textMessage, "SERVER: The channel has been created\n");
				send(socket, textMessage, strlen(textMessage),0);
			}
		}
	}
}

void end_session(int socket){
	usuarios[socket].status = -1;
}

void show_motd(int socket){
	char textMessage[512];
	string msgString;
	memset(textMessage, 0, bufsize);
	strcpy(textMessage, "\tRemember OS is at the top 5 of the most important signatures of computer science! Have a nice day :)\n");
	send(socket, textMessage, strlen(textMessage), 0);
}

void show_version(int socket){
	char textMessage[512];
	string msgString;
	memset(textMessage, 0, bufsize);
	strcpy(textMessage, "\tCurrent Server Version: 1.0\n");
	send(socket, textMessage, strlen(textMessage), 0);
}

void show_info(int socket){
	char textMessage[512];
	string msgString;
	memset(textMessage, 0, bufsize);

	strcpy(textMessage, "\t\t\tSERVER INFO\nServer version: 1.0\nServer compiled: ");
	strcat(textMessage, builtTime);
	strcat(textMessage, "\nServer started: ");
	strcat(textMessage, startTime);
	strcat(textMessage, "\nOther commands: /JOIN /LIST /MOTD /NAMES /NICK /PART /PRIVMSG /QUIT /SETNAME /TIME /USER /USERS /VERSION\n");
	send(socket, textMessage, strlen(textMessage), 0);
}

void show_time(int socket){
	char textMessage[512];
	string msgString;
	memset(textMessage, 0, bufsize);
	localDateTime();

	strcat(textMessage, "Server local time: ");
	strcat(textMessage, localTime);
	strcat(textMessage, "\n");
	send(socket, textMessage, strlen(textMessage), 0);
}

void localDateTime(){
	time_t local = time(0);
	struct  tm actual;
	char buff[80];
	actual = *localtime(&local);
	strftime(buff, sizeof(buff), "%Y-%m-%d.%X", &actual);
    strcpy(localTime, buff);
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
	char* word;
	strcpy(mensaje, buffer);
	cout << "que hay en el buffer " << buffer << endl;
	word = strtok(mensaje, " \r");
	word = strtok(NULL, " \r");
	if (sizeof(word)>0){//FALTA VERIFICAR SI YA SE ESTA USANDO EL NICK
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
			if(*buffer == '/'){
				parse_command(buffer, sender);
			}else{
				for( int i = 0; i < 25; i++){
					memset(mensaje_server, 0, msize);
					if(usuarios[sender].nombre.empty()){
						strcat(mensaje_server, "Anonimo");
					}else{
						strcat(mensaje_server, usuarios[sender].nombre.c_str());
					}
					strcat(mensaje_server, ": ");
					strcat(mensaje_server, buffer);

					if(i != sender && usuarios[i].status == 1 && usuarios[i].member==usuarios[sender].member){
					//strcpy(mensaje_server, "Habla Flaco!!! ");
						send(i, mensaje_server, msize, 0);
					}
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

void send_privmsg(int socket){
	int iter, dest;
	char* mensaje = (char*)calloc(strlen(buffer)+1, sizeof(char));
	char* word;
	char* msgContent;
	char textMessage[512];
	string msgString;
	memset(textMessage, 0, bufsize);
	
	strcpy(mensaje, buffer);
	word = strtok(mensaje, " \r");
	word = strtok(NULL, " \r");
	if (sizeof(word)>0){ //FALTA VERIFICAR SI EXISTE DESTINATARIO
		dest = search_nickname(word);
		if (dest>0){
			msgContent = strtok(NULL, "\n");
			if (sizeof(msgContent)>0){
				strcpy(textMessage, usuarios[socket].nombre.c_str());
				strcat(textMessage, "(PRIVATE): ");
				strcat(textMessage, msgContent);
				strcat(textMessage, "\n");
				send(dest, textMessage, strlen(textMessage), 0);
			}else{
				strcpy(textMessage, "SERVER: No message to be sent\n");
				send(socket, textMessage, strlen(textMessage), 0);
			}
		}else{
			strcpy(textMessage, "SERVER: The nickname ");
			strcat(textMessage, word);
			strcat(textMessage, " doesn't exist\n");
			send(socket, textMessage, strlen(textMessage), 0);
		}
	}else{
		strcpy(textMessage, "SERVER: You must insert a nickname\n");
		send(socket, textMessage, strlen(textMessage), 0);
	}
}

int search_nickname(char *name){
	int iter;
	for (iter=0; iter < arraysize(usuarios); iter++){
		if (strcmp(usuarios[iter].nombre.c_str(), name)==0){
			return iter;
		}
	}
	return -1;
}

int search_channel(char *channel_name){
	int indice;
	for (indice=0; indice < arraysize(canales); indice++){
		if (strcmp(canales[indice].nombre_canal.c_str(), channel_name)==0){
			return indice;
		}
	}
	return -1;
}


int search_member(channel canal,char *member_name){
	int val;
	for (val=0; val < sizeof(canal.usuarios_canal); val++){
		if (strcmp(canal.usuarios_canal[val].nombre.c_str(), member_name)==0){
			return val;
		}
	}
	return -1;
}