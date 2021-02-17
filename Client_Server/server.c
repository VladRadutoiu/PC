#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "helpers.h"
#include <math.h>
typedef struct client
{
	char id[20];	//id-ul clientului
 	int fd;	//file descriptor asociat clientului
 	int SF; // daca are SF activat pentru vreun topic
 	int no_sf; //cate topicuri au SF activat
 	int disconnected; // daca are SF activat dar s-a deconectat
 	struct client* next;
 	struct client* prev;
}*Client;

typedef struct topic {
	char name[51];	//denumirea topicului
	int no_client;
	Client subs;	//lista de abonati
	struct topic* next;
	struct topic* prev;
}*Topic;
typedef struct storeforw{
	char buffer[1600];
	char id_client[20];
	char* ip;
	int port;
	char topic[50];
	int value;
	float value2;
	char value3[1600];
	int type;
	struct storeforw* next;
	struct storeforw* prev;
}*Store;//structura folosita pentru a stoca mesajele pentru clientii cu SF activat dar deconectati

Store initStore(){
	Store s = malloc(sizeof(struct storeforw));
	s->next = NULL;
	s->prev = NULL;
	memset(s->buffer,0,1600);
	return s;
}

int no_topics = 1;
int total_clients = 1;
Store addS(Store s,char* message,char* id_client,char* topic,int value){//functiile addS sunt pentru a adauga mesajele in structura care le retine pentru clientii deconectati dar cu SF 1
	if(s == NULL){
		Store new = initStore();
		strcpy(new->id_client,id_client);
		strcpy(new->topic,topic);
		new->type = 0;
		new->value = value;
		return new;
	}
	else{
		Store tmp = s;
		while(tmp->next != NULL)
			tmp = tmp->next;
		tmp->next = initStore();
		tmp->next->prev = tmp;
		strcpy(tmp->next->topic,topic);
		tmp->next->value = value;
		strcpy(tmp->next->id_client,id_client);
		tmp->next->type = 0;

		return s;

	}
}
Store addS2(Store s,char* message,char* id_client,char* topic,float value){
	if(s == NULL){
		Store new = initStore();
		strcpy(new->id_client,id_client);
		strcpy(new->topic,topic);
		new->value2 = value;
		new->type = 2;
		return new;
	}
	else{
		Store tmp = s;
		while(tmp->next != NULL)
			tmp = tmp->next;
		tmp->next = initStore();
		tmp->next->prev = tmp;
		strcpy(tmp->next->topic,topic);
		tmp->next->value2 = value;
		tmp->next->type = 2;
		strcpy(tmp->next->id_client,id_client);
		return s;

	}
}
Store addS3(Store s,char* message,char* id_client,char* topic,char* value){
	if(s == NULL){
		Store new = initStore();
		strcpy(new->id_client,id_client);
		strcpy(new->topic,topic);
		strcpy(new->value3,value);
		new->type = 3;
		return new;
	}
	else{
		Store tmp = s;
		while(tmp->next != NULL)
			tmp = tmp->next;
		tmp->next = initStore();
		tmp->next->prev = tmp;
		strcpy(tmp->next->topic,topic);
		strcpy(tmp->next->value3,value);
		strcpy(tmp->next->id_client,id_client);
		tmp->next->type = 3;
		return s;

	}
}
Store send_stored_msg(Store s, char* id_client,int fd,char* ip, int port){//functie care trimite mesajele stocate pentru clientii care se reconecteaza
	Store tmp = s;
	int ret;
	if( s!= NULL && s->next == NULL && s->prev == NULL){
				if(strcmp(s->id_client,id_client) == 0 ){
					memset(s->buffer, 0 , 1600);
					if(s->type == 0)
					sprintf(s->buffer, "ip -%s  port-%d  topic-%s  valoare Int-%d", ip, port, s->topic, s->value);
					else if(s->type ==2)
					sprintf(s->buffer, "ip -%s  port-%d  topic-%s  valoare Float-%0.8g", ip, port, s->topic, s->value2);
					else if(s->type == 3)
					sprintf(s->buffer, "ip -%s  port-%d  topic-%s  valoare String-%s", ip, port, s->topic, s->value3);
					ret = send(fd, s->buffer, 1600, 0);
					s = NULL;
					return s;
				}
		}

	while(tmp != NULL){
		if(strcmp(tmp->id_client,id_client) == 0 ){
			if(tmp->prev == NULL)
			s = s->next;
			Store aux = tmp->next;
				if(tmp->prev != NULL)
					tmp->prev->next = tmp->next;
					tmp->prev = NULL;
				if(tmp->next != NULL)
					tmp->next->prev = tmp->prev;
					tmp->next = NULL;
					memset(tmp->buffer, 0 , 1600);
				if(tmp->type == 0)
					sprintf(tmp->buffer, "ip -%s  port-%d  topic-%s  valoare Int-%d", ip, port, tmp->topic, tmp->value);
				else if(tmp->type ==2)
					sprintf(tmp->buffer, "ip -%s  port-%d  topic-%s  valoare Float-%0.8g", ip, port, tmp->topic, tmp->value2);
				else if(tmp->type == 3)
					sprintf(tmp->buffer, "ip -%s  port-%d  topic-%s  valoare String-%s", ip, port, tmp->topic, tmp->value3);
					ret = send(fd, tmp->buffer, 1600, 0);
					tmp = NULL;
					tmp = aux;
				

		}else
		tmp = tmp->next;
	}
	DIE(ret < 0, "forward message from server to tcp client");
	
	return s;
}

Client init_client(){
	Client c = malloc(sizeof(struct client));
	c->next = NULL;
	c->prev = NULL;
	c->SF = 0;
	c->no_sf =0;
	c->disconnected = 0;
	return c;
}
Client insert_client(Client c,char* id_c,int newsockfd){
	Client tmp = c;
	
	while(tmp->next!= NULL){
		tmp = tmp->next;
	}
		tmp->next = init_client();
		tmp->next->prev = tmp;
		tmp= tmp->next;
		strcpy(tmp->id, id_c);
		tmp->fd = newsockfd;
		return c;
}
Client remove_client(Client c,char* id_c,int newsockfd){
	Client tmp = c;
	while(tmp!=NULL){
		if(strcmp(tmp->id,id_c) == 0)
			break;
		tmp = tmp->next;
	}
	if(tmp == NULL)
	return c;
	if(tmp->prev != NULL)
	tmp->prev->next = tmp->next;
	if(tmp->next != NULL){
		tmp->next->prev = tmp->prev;
	}
	if(tmp != NULL)
	tmp = NULL;
return c;
}
Topic init_topic(){
	Topic t = malloc(sizeof(struct topic));
	t->next = NULL;
	t->prev = NULL;
	return t;
}
void insert_topic(Topic t,char* name){
	Topic tmp = t;
	
	while(tmp->next != NULL){
		tmp = tmp->next;
	}
	Topic new = init_topic();
	memcpy(new->name,name,strlen(name));
	tmp->next = new;
	new->prev = tmp;
	
}

Store send_message_tcp(Topic vect,char* buff,char* ip, int port,Store s){//aici trimit mai departe mesajele udp pentru clientii tcp conectati sau le stochez pentru cei deconectati 
																		//cu SF 1
	char topicc[51],continut[1600],message[1600];
	memcpy(topicc,buff,50);
	int value_int;
	float sendm;
	int ok = 0,type,ret;
	Topic tmpt = vect;
	while(tmpt != NULL){
		if(strcmp(tmpt->name,topicc) == 0) //caut un topic 
			ok = 1;
		tmpt = tmpt->next;
	}
	
	if(ok == 0){//daca nu am gasit topicul il adaug
		insert_topic(vect,topicc);
	}

	type = buff[50];
	if(type == 0){//tipul e un int
		int sign = buff[51];
		uint32_t value;
		memcpy(&value, buff + 52, sizeof(uint32_t));
		value_int = ntohl(value);
		memset(message, 0 , 1600);
		if(sign == 1) 
		sprintf(message, "ip -%s  port-%d  topic-%s  valoare Int-%d", ip, port, topicc, -1*value_int);
		else
		sprintf(message, "ip -%s  port-%d  topic-%s  valoare Int-%d", ip, port, topicc, value_int);
	} if (type == 1){//tipul e short real
		uint16_t value;
		memset(&value, 0, sizeof(uint16_t));
		memcpy(&value, buff + 51, sizeof(uint16_t));
		 value_int = ntohl(value) / 100;

		memset(message, 0 , 1600);
		sprintf(message, "ip -%s  port-%d  topic-%s  valoare Short Real-%d", ip, port, topicc, value_int);
	}  
		 if (type == 3){//tipul e string
		memset(continut, 0, 1600);
		memset(message, 0 , 1600);
		char * copie = strdup(buff+51);
		memcpy(continut, copie, 1501);
		sprintf(message, "ip -%s  port-%d  topic-%s  valoare String-%s",ip,port,topicc,continut);
	}
		 if (type == 2){//tipul e float
		int sign = buff[51];
		 memset(message, 0 , 1600);
		uint32_t val;
		memset(&val, 0, sizeof(uint32_t));
		memcpy(&val, buff + 52, sizeof(uint32_t));
		uint8_t put;
		memset(&put, 0, sizeof(uint8_t));
		memcpy(&put, buff + 52 + sizeof(uint32_t), sizeof(uint8_t));
		int b = 10;
		for(int i = 0; i<put; i++){
			b= b*10;
		}
		 sendm = ntohl(val)/b;
		if(sign == 1) 
		sprintf(message, "ip -%s  port-%d  topic-%s  valoare Float-%0.8g", ip, port, topicc, -1 *sendm);
		else
			sprintf(message, "ip -%s  port-%d  topic-%s  valoare Float-%0.8g", ip, port, topicc, sendm);

	} 
		Topic tmp = vect;
		while(tmp!=NULL){
			if(strcmp(tmp->name,topicc) == 0)
				break;
			tmp = tmp->next;
		}
	
		Client tmpc = tmp->subs;
		while(tmpc != NULL){
			if(tmpc->disconnected == 0 && tmpc->fd != -1){//aici verifi daca il pot trimite
			ret = send(tmpc->fd, message, 1600, 0);
			DIE(ret < 0, "forward message from server to tcp client");
		}
			else if(tmpc->SF == 1){	//aici verific daca il stochez
					if(type == 1 || type == 0)
					s = addS(s,message,tmpc->id,topicc,value_int);
					else if(type == 2)
					s = addS2(s,message,tmpc->id,topicc,sendm);
					else if(type == 3)
					s = addS3(s,message,tmpc->id,topicc,continut);
			}
			tmpc = tmpc->next;
		}
		
	
	return s;
}
void set_sf(Client c,char* id_c,int newsockfd,int k){//functie de setat care clienti trebuie sa aiba mesajele stocate
	Client tmp = c;
	while(tmp!=NULL){
		if(strcmp(tmp->id,id_c) == 0)
			break;
		tmp = tmp->next;
	}
	if(tmp == NULL)
		return;
	
	tmp->disconnected = k;//daca disconnected e 1 ins ca trebuie sa ii stochez mesajele
	tmp->fd = newsockfd;//daca fd devine -1 ins ca clientul e deconectat
}



void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_port\n", file);
	exit(0);
}

int main(int argc, char *argv[])//prima parte e din laborator
{
	
	int sockfd, newsockfd, portno, sockfd_udp;
	char buffer[BUFLEN], buffer_udp[BUFLEN];
	struct sockaddr_in serv_addr, cli_addr;
	Client clients = NULL;
	Topic topics = NULL;	
	int n, i, ret;
	Store s = NULL;
	socklen_t clilen;
	fd_set read_fds;	// multimea de citire folosita in select()
	fd_set tmp_fds;		// multime folosita pt a duplica read_fds
	int fdmax;			// valoare maxima fd din multimea read_fds

	if (argc < 2) {
		usage(argv[0]);
	}

	// se goleste multimea de descriptori de citire (read_fds) si multimea temporara (tmp_fds)
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");

	sockfd_udp = socket(AF_INET, SOCK_DGRAM, 0);
	DIE(sockfd < 0, "socket udp");

	portno = atoi(argv[1]);
	DIE(portno == 0, "atoi");

	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	ret = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
	DIE(ret < 0, "bind");

	ret = bind(sockfd_udp, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
	DIE(ret < 0, "bind");

	ret = listen(sockfd, MAX_CLIENTS);
	DIE(ret < 0, "listen");

	// se adauga noul file descriptor (socketul pe care se asculta conexiuni) in multimea read_fds
	FD_SET(sockfd, &read_fds);//tcp
	FD_SET(sockfd_udp, &read_fds);//udp
	FD_SET(STDIN_FILENO, &read_fds);//citire de la tastatura
	int aux;
	if(sockfd > sockfd_udp)
		aux = sockfd;
	else aux = sockfd_udp;

	if(aux > STDIN_FILENO)
		fdmax = aux;
	else fdmax = STDIN_FILENO;

	while (1) {
		tmp_fds = read_fds; 
		
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");

		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
				if (i == sockfd) {
					// a venit o cerere de conexiune pe socketul inactiv (cel cu listen),
					// pe care serverul o accepta
					clilen = sizeof(cli_addr);
					newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
					DIE(newsockfd < 0, "accept");

					memset(buffer, 0, BUFLEN);
					n = recv(newsockfd, buffer, sizeof(buffer),0);
					DIE(n < 0, "recv_id");

					char* id_client;
					id_client = strdup(buffer);

					// se adauga noul socket intors de accept() la multimea descriptorilor de citire
					FD_SET(newsockfd, &read_fds);
					if (newsockfd > fdmax) { 
						fdmax = newsockfd;
					}
					if(clients == NULL){
						clients = init_client();
						strcpy(clients->id,id_client);
						clients->fd = newsockfd;
					}
					else{
						Client tmp = clients;
						while(tmp != NULL){
							if(strcmp(tmp->id,id_client) == 0){//daca exista deja clientul ins ca are SF = 1 si va trebui sa ii returnez mesajele
								tmp->fd = newsockfd;//noul fd
								tmp->disconnected = 0;//s-a reconectat
								s = send_stored_msg(s, id_client,newsockfd, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
								break;
							}
							tmp = tmp->next;
						}
						if(tmp == NULL)
							insert_client(clients,id_client,newsockfd);//daca nu l-am gasit in multimea veche il creez	
							Topic tmpt = topics;
							while(tmpt != NULL){
								set_sf(tmpt->subs,id_client,newsockfd,0);//setez disconnected si fd
								tmpt = tmpt->next;
							}
						
					}
					printf("Noua conexiune de la %s, adr %s, port %d\n",
							id_client,inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
				
				}else if(i == sockfd_udp){//primesc mesaje de pe clientul udp
					memset(buffer_udp, 0, BUFLEN);
					recvfrom(i, buffer_udp, BUFLEN, 0, (struct sockaddr*) &serv_addr,0);
					if(topics == NULL){
						topics = init_topic();
						strcpy(topics->name,buffer_udp);
					}
					s = send_message_tcp(topics, buffer_udp, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port),s);
				}else if (i == STDIN_FILENO){
					memset(buffer, 0, BUFLEN);
					fgets(buffer, BUFLEN, stdin);
					if(strcmp(buffer, "exit\n") == 0) 
						exit(0);
				}
					else {
					// s-au primit date pe unul din socketii de client,
					// asa ca serverul trebuie sa le receptioneze
					memset(buffer, 0, BUFLEN);
					n = recv(i, buffer, sizeof(buffer), 0);
					DIE(n < 0, "recv");

					if (n == 0) {//se inchide conexiunea
						printf("Socket-ul client %d a inchis conexiunea\n", i);
						close(i);
						char id_c[20];
						int ok = 0;
						Client tmp = clients;
						if(clients->next == NULL){
								if(clients->fd == i){
									strcpy(id_c,clients->id);
								if(clients->no_sf >= 1){//daca clientul are SF activat nu il scot din multimea de clienti dar il consider deconectat
										clients->disconnected = 1;
										ok = 1;
									}
									else
								//free(clients);
								clients = NULL;
						}
					}else{
						while(tmp != NULL){
							if(tmp->fd == i)
								{	strcpy(id_c,tmp->id);
									if(tmp->no_sf >= 1){//daca clientul are SF activat nu il scot din multimea de clienti dar il consider deconectat
										tmp->disconnected = 1;
										ok = 1;
										break;
									}
									if(tmp->prev != NULL)
									tmp->prev->next = tmp->next;
								if(tmp->next != NULL)
								tmp->next->prev = tmp->prev;
								tmp = NULL;
								break;
							}
							tmp = tmp->next;
						}
					}
						
						Topic tmpt = topics;
						while(tmpt != NULL){
							set_sf(tmpt->subs,id_c,-1,ok);//setez fd la minus 1 si SF la 0 sau 1
							tmpt = tmpt->next;
						
					}

						// se scoate din multimea de citire socketul inchis 
						FD_CLR(i, &read_fds);
					} else {//aici incep sa iau mesajul
						char *start = buffer;
						char subscribe[100], topicc[51];
						int SF = parsare(start, subscribe, topicc);//aici parsez
						Client tmpc = clients;
					
						while(tmpc != NULL){
							if(tmpc->fd == i)//caut pana gasesc clientul
								break;
							tmpc = tmpc->next;
						}
						if(SF == 1)
						tmpc->no_sf++;
						
						printf("Client (%s) %s la (%s)\n", tmpc->id, subscribe, topicc);

						Topic tmp = topics;//caut in topicuri
						while(tmp != NULL){
							if(strcmp(tmp->name,topicc) == 0)
							break;
							tmp = tmp->next; 
						}
						if(tmp != NULL){	//daca topicul exista deja
							if(strcmp(subscribe,"subscribe") == 0)
								{
									if(tmp->subs == NULL){//daca nu are subscriberi ii creez
										tmp->subs = init_client();
										strcpy(tmp->subs->id,tmpc->id);
										tmp->subs->fd = tmpc->fd;
										tmp->subs->SF = SF;
									}else{
									Client c = tmp->subs;
									tmp->subs = insert_client(tmp->subs,tmpc->id,tmpc->fd);//inserez
									while(tmp->subs->next != NULL)
									{
										tmp->subs = tmp->subs->next;
									}
									tmp->subs->SF = SF;
									tmp->subs = c;
									
								}
							}
							else{//aici dau unsubscribe
								Client c = tmp->subs;
								if(tmp->subs->next == NULL && tmp->subs->prev == NULL)
									{
										if(tmp->subs->SF == 1)
										tmpc->no_sf--;
										tmp->subs = NULL;
								}
								else if(tmp->subs->prev == NULL){
									if(strcmp(tmp->subs->id,tmpc->id) == 0){
										if(tmp->subs->SF == 1)
											tmpc->no_sf--;
											tmp->subs = tmp->subs->next;
											tmp->subs->prev = NULL;
										}
									else{
									while(tmp->subs!=NULL){
									if(strcmp(tmp->subs->id,tmpc->id) == 0){
										if(tmp->subs->SF == 1)
											tmpc->no_sf--;
										break;
									}
									tmp->subs = tmp->subs->next;
								}
								if(tmp->subs != NULL){	
								if(tmp->subs->prev != NULL)
								tmp->subs->prev->next = tmp->subs->next;
								if(tmp->subs->next != NULL){
									tmp->subs->next->prev = tmp->subs->prev;
								}
									//free(tmp->subs);
									tmp->subs = NULL;
									tmp->subs = c;
							}
						}
						
								}

						
								
							}
						}else{//daca topicul nu exista
							if(strcmp(subscribe,"subscribe") == 0){
								if(topics == NULL)
								{
									topics = init_topic();
									memcpy(topics->name,topicc,strlen(topicc));
								}else
								insert_topic(topics,topicc);
								tmp = topics;
								while(tmp != NULL){
									if(strcmp(tmp->name,topicc) == 0)
										break;
									tmp = tmp->next;
								}

								
									if(tmp->subs == NULL){
										tmp->subs = init_client();
										strcpy(tmp->subs->id,tmpc->id);
										tmp->subs->fd = tmpc->fd;
										tmp->subs->SF = SF;
									}
									else
									{
									tmp->subs = insert_client(tmp->subs,tmpc->id,tmpc->fd);
									Client c = tmp->subs;
									while(c->next != NULL)
									{
										c = c->next;
									}
									c->SF = SF;
								}
							}else{
								printf("Topicul nu exista. Eroare\n");
							}
					}
				}
			}
		}
	}
}
	close(sockfd);

	return 0;
}


