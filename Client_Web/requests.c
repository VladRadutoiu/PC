#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count,char* token,char*id)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
   if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
    	if(id == NULL)
        sprintf(line, "GET %s HTTP/1.1", url);
    	else
    		 sprintf(line, "GET %s%s HTTP/1.1", url,id);
    }

    compute_message(message, line);
  sprintf(line, "HOST: %s", host);
    compute_message(message, line);
    if(token != NULL){
    	sprintf(line,"Authorization: Bearer %s",token)	;
    }
    compute_message(message, line);

    if (cookies != NULL) {
        sprintf(line, "Cookie: %s", cookies[0]);
        for (int i = 1 ; i < cookies_count ; i++) {
    	strcat(line, "; ");
        strcat(line, cookies[i]);
        }
        compute_message(message,line);
    }
    compute_message(message, "");
    return message;
}

char *compute_post_request(char *host, char *url, char* content_type, char **body_data,
                            int body_data_fields_count, char **cookies, int cookies_count,char*token)
{
   char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *body_data_buffer = calloc(LINELEN, sizeof(char));
    long int data_size = 0;
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    sprintf(line, "HOST: %s", host);
    compute_message(message, line);
     if(token != NULL){
    	sprintf(line,"Authorization: Bearer %s",token)	;
    }
    compute_message(message, line);
    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);


    int i = 0;
    for ( i = 0; i < body_data_fields_count - 1; i++){
        strcat(body_data_buffer, body_data[i]);
        strcat(body_data_buffer, "&");
        data_size += strlen(body_data[i])+ 1;
    }
    strcat(body_data_buffer, body_data[i]);
    data_size += strlen(body_data[i]);

    strcpy(line, "");
    sprintf(line, "Content-Length: %ld", data_size);
    compute_message(message, line);
  
    if (cookies != NULL) {

    }
   
    sprintf(line, "");
    compute_message(message, "");
  
    compute_message(message, body_data_buffer);

    free(line);
    return message;
}
char *compute_delete_request(char *host, char *url, char *query_params,
                            char* token,char*id)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
  if (query_params != NULL) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    } else {
    	if(id == NULL)
        sprintf(line, "DELETE %s HTTP/1.1", url);
    	else
    		 sprintf(line, "DELETE %s%s HTTP/1.1", url,id);
    }

    compute_message(message, line);
    sprintf(line, "HOST: %s", host);
    compute_message(message, line);
  if(token != NULL){
    	sprintf(line,"Authorization: Bearer %s",token)	;
    }
    compute_message(message, line);
  compute_message(message, "");
    return message;
}