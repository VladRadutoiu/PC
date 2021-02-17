#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.c"
#include "parson.h"

int main(int argc, char *argv[])
{
    char *message;
    char *response;
    int no_usr = 0;
    char buffer[1600];
    char *token = NULL;
    int sockfd;
    int log = 0;
    char **cookies = NULL;
    int try = 0;

    while (1) {


        memset(buffer, 0, 1600);
        fgets(buffer, 1600, stdin);

        if (strncmp(buffer, "exit", 4) == 0) {
            exit(0);
            break;
        }
        else if (strncmp(buffer, "register", 8) == 0) {
            sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
            char username[100];
            char password[100];
            memset(username, 0, 100);
            memset(password, 0, 100);
            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object = json_value_get_object(root_value);
            char **serialized_string = malloc(1 * sizeof(char*));
            serialized_string[0] = NULL;
            printf("Username: ");
            scanf("%s", username);
            printf("Password: ");
            scanf("%s", password);
            json_object_set_string(root_object, "username", username);
            json_object_set_string(root_object, "password", password);
            serialized_string[0] = json_serialize_to_string_pretty(root_value);
            puts(serialized_string[0]);
            message = compute_post_request("3.8.116.10", "/api/v1/tema/auth/register", "application/json", serialized_string, 1, NULL, 0, NULL);
            printf("Request-ul: %s\n", message);
            send_to_server(sockfd, message);
            //primesc de la server mesaj
            response = receive_from_server(sockfd);
            printf("Response: %s\n", response);
            free(message);
            message = NULL;
            free(response);
            response = NULL;
            json_value_free(root_value);
            close_connection(sockfd);

        }
        else if (strncmp(buffer, "login", 5) == 0) {
            if (no_usr == 0) {
                no_usr++;
                sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
                char username[100];
                char password[100];
                memset(username, 0, 100);
                memset(password, 0, 100);
                JSON_Value *root_value = json_value_init_object();
                JSON_Object *root_object = json_value_get_object(root_value);
                char **serialized_string = malloc(1 * sizeof(char*));
                serialized_string[0] = NULL;
                printf("Username: ");
                scanf("%s", username);
                printf("Password: ");
                scanf("%s", password);
                json_object_set_string(root_object, "username", username);
                json_object_set_string(root_object, "password", password);
                serialized_string[0] = json_serialize_to_string_pretty(root_value);
                puts(serialized_string[0]);
                message = compute_post_request("3.8.116.10", "/api/v1/tema/auth/login", "application/json", serialized_string, 1, NULL, 0, NULL);
                printf("Request-ul: %s\n", message);
                send_to_server(sockfd, message);
                //primesc de la server mesaj
                response = receive_from_server(sockfd);
                cookies = malloc(sizeof(char*));
                cookies[0] = malloc(500 * sizeof(char));
                cookies[1] = malloc(10 * sizeof(char));
                cookies[2] = malloc(10 * sizeof(char));
                printf("Response: %s\n", response);
                char *cookie = strstr(response, "Set-Cookie: ");
                if (cookie == NULL) {
                    printf("nu exista contul\n");
                    no_usr--;
                    continue;
                }
                log = 1;
                char *actual = strtok(cookie, ";");
                strcpy(cookies[0], actual);
                cookies[0] = cookies[0] + 12;
                actual = strtok(NULL, ";");
                strcpy(cookies[1], actual);
                cookies[1] = cookies[1] + 1;
                actual = strtok(NULL, "\r\n");
                strcpy(cookies[2], actual);
                cookies[2] = cookies[2] + 1;
                free(message);
                message = NULL;
                free(response);
                response = NULL;
                json_value_free(root_value);
                close_connection(sockfd);
            } else printf("Cate un user odata\n");

        }
        else if (strncmp(buffer, "enter_library", 13) == 0) {
            if (log == 1) {
                sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
                message = compute_get_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", "/api/v1/tema/library/access", NULL, cookies, 1, NULL, NULL);
                //printf("%s", message);
                send_to_server(sockfd, message);
                //primesc de la server mesaj
                response = receive_from_server(sockfd);
                printf("%s\n", response );
                char* actual = strstr(response, "token");
                actual += 8;

                token = malloc((strlen(actual) - 1) * sizeof(char));
                strncpy(token, actual, strlen(actual) - 2);
                free(message);
                free(response);
                close_connection(sockfd);
            } else printf("Te rog sa te loghezi mai intai\n");


        }
        else if (strncmp(buffer, "get_books", 9) == 0) {
            if (log == 1 && token != NULL) {
                sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
                message = compute_get_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", "/api/v1/tema/library/books", NULL, NULL, 1, token, NULL);
                send_to_server(sockfd, message);

                response = receive_from_server(sockfd);

                printf("Response: %s\n", response);
                char* actual = strstr(response, "Error");
                if (actual != NULL) {
                    printf("Eroare te rog sa mai introduci odata enter_library\n");
                }
                actual = strstr(response, "Bad Request");
                if (actual != NULL) {
                    printf("Eroare te rog sa mai introduci odata enter_library\n");
                }
                free(message);
                free(response);
                close_connection(sockfd);
            } else if (log == 0)printf("Te rog sa te loghezi mai intai\n");
            else if (token == NULL)printf("Te rog sa intri in biblioteca\n");

        }
        else if (strncmp(buffer, "get_book", 8) == 0) {
            if (log == 1 && token != NULL) {
                sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
                char id[100];
                memset(id, 0, 100);
                printf("id: ");
                fgets(id, 100, stdin);
                char *pos;
                if ((pos = strchr(id, '\n')) != NULL)
                    * pos = '\0';
                message = compute_get_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", "/api/v1/tema/library/books/", NULL, cookies, 1, token, id);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);

                printf("Response: %s\n", response);
                char* actual = strstr(response, "Error");
                if (actual != NULL) {
                    printf("Eroare te rog sa mai introduci odata enter_library\n");
                }
                actual = strstr(response, "Bad Request");
                if (actual != NULL) {
                    printf("Eroare te rog sa mai introduci odata enter_library\n");
                }
                free(message);
                free(response);
                close_connection(sockfd);
            } else if (log == 0)printf("Te rog sa te loghezi mai intai\n");
            else if (token == NULL)printf("Te rog sa intri in biblioteca\n");
        }
        else if (strncmp(buffer, "add_book", 8) == 0) {
            if (log == 1 && token != NULL) {
                sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
                char title[100];
                char author[100];
                char genre[100];
                char page_count[100];
                char publisher[100];
                memset(title, 0, 100);
                memset(author, 0, 100);
                memset(genre, 0, 100);
                memset(page_count, 0, 100);
                memset(publisher, 0, 100);
                JSON_Value *root_value = json_value_init_object();
                JSON_Object *root_object = json_value_get_object(root_value);
                char **serialized_string = malloc(1 * sizeof(char*));
                serialized_string[0] = NULL;
                printf("title: ");
                fgets(title, 100, stdin);
                printf("author: ");
                fgets(author, 100, stdin);
                printf("genre: ");
                fgets(genre, 100, stdin);
                printf("page_count: ");
                fgets(page_count, 100, stdin);
                printf("publisher: ");
                fgets(publisher, 100, stdin);
                json_object_set_string(root_object, "title", title);
                json_object_set_string(root_object, "author", author);
                json_object_set_string(root_object, "genre", genre);
                json_object_set_string(root_object, "page_count", page_count);
                json_object_set_string(root_object, "publisher", publisher);
                serialized_string[0] = json_serialize_to_string_pretty(root_value);
                puts(serialized_string[0]);
                message = compute_post_request("3.8.116.10", "/api/v1/tema/library/books", "application/json", serialized_string, 1, NULL, 0, token);
                printf("Request-ul: %s\n", message);
                send_to_server(sockfd, message);
                //primesc de la server mesaj
                response = receive_from_server(sockfd);
                printf("Response: %s\n", response);
                char* actual = strstr(response, "Error");
                if (actual != NULL) {
                    printf("Eroare te rog sa mai introduci odata enter_library\n");
                }
                actual = strstr(response, "Bad Request");
                if (actual != NULL) {
                    printf("Eroare te rog sa mai introduci odata enter_library\n");
                }
                free(message);
                free(response);
                json_value_free(root_value);
                close_connection(sockfd);
            } else if (log == 0)printf("Te rog sa te loghezi mai intai\n");
            else if (token == NULL)printf("Te rog s aintri in biblioteca\n");
        }
        else if (strncmp(buffer, "delete_book", 11) == 0) {
            if (log == 1 && token != NULL) {
                sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
                char id[100];
                memset(id, 0, 100);
                printf("id: ");
                fgets(id, 100, stdin);
                char *pos;
                if ((pos = strchr(id, '\n')) != NULL)
                    * pos = '\0';
                message = compute_delete_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", "/api/v1/tema/library/books/", NULL, token, id);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                printf("Response: %s\n", response);
                char* actual = strstr(response, "Error");
                if (actual != NULL) {
                    printf("Eroare te rog sa mai introduci odata enter_library\n");
                }
                actual = strstr(response, "Bad Request");
                if (actual != NULL) {
                    printf("Eroare te rog sa mai introduci odata enter_library\n");
                }
                free(message);
                free(response);
                close_connection(sockfd);
            } else if (log == 0)printf("Te rog sa te loghezi mai intai\n");
            else if (token == NULL)printf("Te rog sa intri in biblioteca\n");

        }
        else if (strncmp(buffer, "logout", 6) == 0) {
            if (log == 1) {
                no_usr--;
                try = 1;
                sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
                message = compute_get_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", "/api/v1/tema/auth/logout", NULL, cookies, 1, NULL, NULL);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                printf("Response: %s\n", response);
                free(message);
                free(response);
                if (token != NULL)
                    free(token);
                token = NULL;
                if (cookies != NULL)
                    free(cookies);
                cookies = NULL;
                close_connection(sockfd);
                log = 0;
            } else printf("Te rog sa te loghezi mai intai\n");

        }



    }



    return 0;
}