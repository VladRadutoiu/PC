#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "helpers.h"

void usage(char *file)
{
	fprintf(stderr, "Usage: %s client id server_address server_port\n", file);
	exit(0);
}

int main(int argc, char *argv[])
{
	int sockfd, n, ret;
	struct sockaddr_in serv_addr;
	char buffer[BUFLEN];
	fd_set read;	
	fd_set tmp;		
	int fdmax;	
	
	FD_ZERO(&tmp);
	FD_ZERO(&read);

	if (argc < 4) {
		usage(argv[0]);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");
	FD_SET(sockfd, &read);
	FD_SET(STDIN_FILENO, &read);
	if(sockfd > STDIN_FILENO){
	fdmax = sockfd;
	}else fdmax = STDIN_FILENO;

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[3]));
	ret = inet_aton(argv[2], &serv_addr.sin_addr);
	DIE(ret == 0, "inet_aton");

	ret = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "connect");

	ret = send(sockfd, argv[1], strlen(argv[1]), 0);	//trimit id ul catre server
	DIE(ret < 0, "id");


	while (1) {
  		// se citeste de la tastatura
  		tmp = read;

		select(fdmax + 1, &tmp, NULL, NULL, NULL);

		for(int i = 0; i<=fdmax; i++){

		if(FD_ISSET(i, &tmp)) {

			if(i == STDIN_FILENO){
				//citesc tast
			memset(buffer, 0, BUFLEN);
			fgets(buffer, BUFLEN, stdin);

		if (strncmp(buffer, "exit", 4) == 0) {
			exit(0);
		} else{
						char command[100], topic[50];
						char aux[20];
						strcpy(aux,buffer);
						int SF = parsare(aux, command, topic);
						if(SF < 0 || SF > 1){
							printf("SF trebuie sa fie 0 sau 1\n");
							break;
						}
						//verific daca comanda este valida
						if(strcmp(command, "unsubscribe") != 0 && strcmp(command, "subscribe") != 0){
							printf("Comanda invalida\n");
							break;
						}
						// trimit mesaj catre server
						n = send(sockfd, buffer, strlen(buffer), 0);
						DIE(n < 0, "send");
				}
	
	}else if(i == sockfd) {//citesc de laserver
					memset(buffer, 0, 1600);
            		n = recv(sockfd, buffer, 1600, 0);
      
            		if(n == 0) {
              			return -1;
            		} else {
					if(strlen(buffer) !=0)
						printf("%s\n", buffer);
					}

	}

}

}
	}
	close(sockfd);

	return 0;
}
