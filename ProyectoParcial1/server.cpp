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
  string realname;
  string user;
} ;

struct channel{
	int id_canal;
	int num_usuarios_canal;
	string nombre_canal;
//	string password_canal;
	usuario usuarios_canal[25];
};

pthread_mutex_t buff, full, empty;
int sender = -1;
int bufsize = 512;
char buffer[512];
int msize = 1024;
int id_actual = 0;
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
void show_names(int socket);
void set_user(int socket);
void change_realname(int socket);
void show_users(int socket);
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
	send(sock, "Please specify your user and real name with /USER [user] [realname]\n", strlen("Please specify your user and real name with /USER [user] [realname]\n"), 0);
	usuarios[sock].status = 1;

	while (usuarios[sock].user.empty() || usuarios[sock].realname.empty()){
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
	usuarios[sock].nombre.clear();
	usuarios[sock].user.clear();
	usuarios[sock].realname.clear();
	usuarios[sock].status = -1;
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

	if(usuarios[socket].user.empty() || usuarios[socket].realname.empty()){
		cout << "Ya sabe como es uno 2" << endl;
		if (strcmp(strtok(textContent, " "),"/USER")==0){
			cout << "Ya sabe como es uno 3" << endl;
			set_user(socket);
		}else{
			send(socket, "Please set your user and real name\n", strlen("Please set your user and real name\n"), 0);
			return;
		}
	}

	if (strcmp(textParsing,"/INFO")==0){
		show_info(socket);
	}else if(strcmp(strtok(textContent, " "), "/NICK") == 0){
		change_nickname(socket);
	}else if(strcmp(strtok(textContent, " "), "/SETNAME") == 0){
		change_realname(socket);
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
	}else if(strcmp(strtok(textContent, " "),"/NAMES") == 0){
		show_names(socket);
	}else if(strcmp(textParsing,"/USERS") == 0){
		show_users(socket);
	}

}

void part_channel(int socket){
	char textMessage[512];
	int indice=-1;
	char word[512];
	if(usuarios[socket].member == 0){//Si el usuario no esta en ningun canal

		strcpy(textMessage, "SERVER: You are not member of a channel\n");
		send(socket, textMessage, strlen(textMessage),0);

	}else{

		int pos = 0;
		cout << "Resetea la asignacion de miembro al usuario\n";
		usuarios[socket].member = 0;
		strcpy(word ,usuarios[socket].nombre.c_str());
		cout << "setea el nombre en una variable char * y es: " << word << "\n" ;
		while(indice == -1){
			cout << "Busca el indice del chateador\n";//SE CAE AQUI
			indice = search_member(canales[pos],word);
			cout << "Encontroooo y es:" << indice << "\n";
			pos++;
			cout << "Ya aumento su posicion\n";
		};

		cout << "Obteniendo el canal\n";
		channel canal_deseado = canales[pos-1];//Obtengo el canal que contiene al usuario
		
		delete &canal_deseado.usuarios_canal[indice];//seteo en 0
		cout << "Eliminando al usuario del canal\n";
		canal_deseado.num_usuarios_canal--;//Disminuyo la cantidad de usuarios
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
		cout << entrada << search_channel(entrada) << "PROBANDO SI EXISTE  " << endl;
		if(sizeof(entrada)>0){
			chan = search_channel(entrada);//Me devuelve el indice para obtener el elemento desde el arreglo
			if(chan > 0){//si existe el canal
				if(canales[chan].num_usuarios_canal > 24){//Si esta lleno
					strcpy(textMessage, "SERVER: The channel is full\n");
					send(socket, textMessage, strlen(textMessage),0);
				}else{//Si aun tiene espacio
					canales[chan].num_usuarios_canal++;
					canales[chan].usuarios_canal[canales[chan].num_usuarios_canal] = usuarios[socket];
					usuarios[socket].member = canales[chan].id_canal;
					if(canales[chan].num_usuarios_canal>0){
						cout<< "Agregueeee\n";
						cout<< canales[chan].num_usuarios_canal << "\n";

					}else{
						cout<< "no hay nadie\n";
					}
					strcpy(textMessage, "SERVER: Connected to channel\n");
					send(socket, textMessage, strlen(textMessage),0);
				}	
			}else{//si no existe el canal
				id_actual++;
				cout << "canal creado\n";
				canales[id_actual].nombre_canal = entrada;
				canales[id_actual].num_usuarios_canal = 0;
				cout << "asignando memoria a la lista de usuarios\n";
				canales[id_actual].usuarios_canal[0] = usuarios[socket];
				//Cada que se crea se asigna en la posicion 0
				cout << "Se ingresa el canal a la lista total de canales\n";
				canales[id_actual].id_canal=id_actual;
				usuarios[socket].member = id_actual;
				canales[id_actual].num_usuarios_canal++;
				
				if(canales[id_actual].num_usuarios_canal>0){
					cout<< "Agregueeee\n";
					cout<< canales[id_actual].num_usuarios_canal << "\n";
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
	char textMessage[512];
	int i;
	int taken = 0;
	memset(textMessage, 0, bufsize);
	
	cout << "que hay en el buffer " << buffer << endl;
	word = strtok(mensaje, " \r");
	word = strtok(NULL, " \r");
	if (sizeof(word)>0){
		for (i=0; i<arraysize(usuarios); i++){
			if (strcmp(word, usuarios[i].nombre.c_str())==0){
				strcpy(textMessage, "SERVER: El nickname ");
				strcat(textMessage, word);
				strcat(textMessage, " ya existe.\n");
				send(socket, textMessage, sizeof(textMessage),0);
				taken=1;
			}
		}
		if(!taken){
			cout << usuarios[socket].nombre << " Changing Name To " << word << endl;
			usuarios[socket].nombre = string(word);
		}
	}else{
		cout << "Name not changed \n" << endl;
	}
	free(mensaje);
}

void change_realname(int socket){
	char* mensaje = (char*)calloc(strlen(buffer)+1, sizeof(char));
	char* word;
	strcpy(mensaje, buffer);
	char textMessage[512];
	int i;
	int taken = 0;
	memset(textMessage, 0, bufsize);
	
	cout << "que hay en el buffer " << buffer << endl;
	word = strtok(mensaje, " \r");
	if(word != NULL){
		word = strtok(NULL, " \r");
	}else{
		return;
	}
	if(word == NULL){
		return;
	}
	if (sizeof(word)>0){
		for (i=0; i<arraysize(usuarios); i++){
			if (strcmp(word, usuarios[i].realname.c_str())==0 && i!=socket){
				strcpy(textMessage, "SERVER: El nombre real ");
				strcat(textMessage, word);
				strcat(textMessage, " ya existe.\n");
				send(socket, textMessage, sizeof(textMessage),0);
				taken=1;
			}
		}
		if(!taken){
			cout << usuarios[socket].nombre << " Changing RealName To " << word << endl;
			usuarios[socket].realname = string(word);
		}
	}else{
		cout << "RealName not changed \n" << endl;
	}
	free(mensaje);
}

void show_users(int socket){
	char textMessage[1024];
	int i;
	memset(textMessage, 0, 2*bufsize);
	strcpy(textMessage, "SERVER: Lista de usuarios\n");
	strcat(textMessage, "User\tRealN\tNick\tChannel\n");
	for (i=0; i<arraysize(usuarios); i++){
		if(!usuarios[i].user.empty() && !usuarios[i].realname.empty()){
			strcat(textMessage, usuarios[i].user.c_str());
			strcat(textMessage, "\t");
			strcat(textMessage, usuarios[i].realname.c_str());
			strcat(textMessage, "\t");
			if(usuarios[i].nombre.empty()){
				strcat(textMessage, "Anonimo");
			}else{
				strcat(textMessage, usuarios[i].nombre.c_str());
			}
			strcat(textMessage, "\t");
			if(usuarios[i].member == 0){
				strcat(textMessage, "Global");
			}else{
				strcat(textMessage, canales[usuarios[i].member].nombre_canal.c_str());
			}
			strcat(textMessage, "\n");
		}
	}
	send(socket, textMessage, sizeof(textMessage),0);
}

void set_user(int socket){
	char* mensaje = (char*)calloc(strlen(buffer)+1, sizeof(char));
	char* word;
	strcpy(mensaje, buffer);
	char textMessage[512];
	int i;
	int taken = 0;
	memset(textMessage, 0, bufsize);
	
	cout << "que hay en el buffer " << buffer << endl;
	word = strtok(mensaje, " \r");
	if(word != NULL){
		word = strtok(NULL, " \r");
	}else{
		send(socket, "Please set your user and real name\n", strlen("Please set your user and real name\n"), 0);
		return;
	}
	if(word == NULL){
		return;
	}
	if (sizeof(word)>0){
		for (i=0; i<arraysize(usuarios); i++){
			if (strcmp(word, usuarios[i].user.c_str())==0 && i!=socket){
				strcpy(textMessage, "SERVER: El Usuario ");
				strcat(textMessage, word);
				strcat(textMessage, " ya existe.\n");
				send(socket, textMessage, sizeof(textMessage),0);
				taken=1;
			}
		}
		if(!taken){
			cout << usuarios[socket].user << " Changing User To " << word << endl;
			usuarios[socket].user = string(word);
		}
	}else{
		cout << "Name not changed \n" << endl;
	}
	if(word != NULL){
		word = strtok(NULL, " \r");
	}else{
		send(socket, "Please set your user and real name\n", strlen("Please set your user and real name\n"), 0);
		return;
	}
	if(word == NULL){
		send(socket, "Please set your user and real name\n", strlen("Please set your user and real name\n"), 0);
		return;
	}
	if (sizeof(word)>0 && strcmp(word, "")!=0 && strcmp(word, " ")!=0 && strcmp(word, "\n")!=0){
		cout << usuarios[socket].realname << " Changing RealName To " << word << endl;
		usuarios[socket].realname = string(word);
	}else{
		send(socket, "Please set your user and real name\n", strlen("Please set your user and real name\n"), 0);
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
				if(usuarios[sender].user.empty() || usuarios[sender].realname.empty()){
					send(sender, "Please set your user and real name\n", strlen("Please set your user and real name\n"), 0);
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
	char* j = (char*)calloc(strlen(buffer)+1, sizeof(char));
	strcpy(j, buffer);
	for (indice=0; indice < arraysize(canales); indice++){
		cout << "DENTRO DEL SEARCH CHANNEL" << endl;
		cout << canales[indice].nombre_canal.c_str() << endl;
		cout << channel_name << endl;
		cout << strcmp(canales[indice].nombre_canal.c_str(), channel_name) << endl;
		if (strcmp(canales[indice].nombre_canal.c_str(), channel_name)==0){
			return indice;
		}
	}
	return -1;
}


int search_member(channel canal,char *member_name){
	int val;
	for (val=0; val < arraysize(canal.usuarios_canal); val++){
		if (strcmp(canal.usuarios_canal[val].nombre.c_str(), member_name)==0){
			return val;
		}
	}
	return -1;
}

void show_names(int socket){
	int indice, nombIter, counter;
	char* mensaje = (char*)calloc(strlen(buffer)+1, sizeof(char));
	char* word;
	char* nombresCanales;
	char textMessage[512];
	string msgString;
	memset(textMessage, 0, bufsize);
	strcpy(textMessage, "SERVER: Channels list:\n");
	strcpy(mensaje, buffer);
	counter = 0;
	cout << "VIENDO ESTO " << endl;
	word = strtok(mensaje, " \r\n,.-");
	word = strtok(NULL, " \r\n,.-");
	while (word != NULL){
		cout << word << endl;
		indice = search_channel(word);
		
		if(indice > 0){
			for(nombIter=0; nombIter < 25; nombIter++){
				if(canales[indice].usuarios_canal[nombIter].nombre != ""){
					strcat(textMessage, "Channel: ");
					strcat(textMessage, word);
					strcat(textMessage, ":@");
					strcat(textMessage, canales[indice].usuarios_canal[nombIter].nombre.c_str());
					strcat(textMessage, "\n");
					counter++;
				}
			}
		}
		word = strtok(NULL, " \r\n,.-");
	}
	if (counter == 0){
		for(indice = 0; indice < arraysize(canales); indice++){
			if(canales[indice].id_canal > 0){
				for(nombIter=0; nombIter < 25; nombIter++){
					if(canales[indice].usuarios_canal[nombIter].nombre != ""){
						strcat(textMessage, "Channel:");
						strcat(textMessage, canales[indice].nombre_canal.c_str());
						strcat(textMessage, " :@");
						strcat(textMessage, canales[indice].usuarios_canal[nombIter].nombre.c_str());
						strcat(textMessage, "\n");
					}
				}
			}
		}
	}
	cout << "VIENDO AQUELLO " << endl;
	send(socket, textMessage, sizeof(textMessage),0);
}