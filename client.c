/*  Tema3 Protocoale de Comunicatii
    Gherasie Stefania 323CB */

#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include <ctype.h>
#include "helpers.h"
#include "requests.h"
#include "client_helpers.h"
#include "parson/parson.h"



int main(int argc, char *argv[])
{
    char *message;
    char *response;
    int sockfd;
    char command[30], buffer[200];
    char* login_token = NULL;
    char* access_token = NULL;
    char** cookies = NULL;

    while (1) {
        // read command from stdin
        fgets(command, 30, stdin);

        // "register" command
        if(strcmp(command, "register\n") == 0) {
            // compute JSON with credentials
            char* send_json = compute_json_register_login();
            if(send_json == NULL) 
                continue;

            // open connection, send POST request and get response
            sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
            message = compute_post_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", 
                            "/api/v1/tema/auth/register", "application/json", 
                            send_json, NULL, 0, NULL);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // extract received JSON and print message
            char* recv_json = basic_extract_json_response(response);
            if(check_error(recv_json) && check_response(response)) {
                printf("[Success]: You registered.\n\n");
            }

            // free variables and close connection
            free(message); free(response);
            close_connection(sockfd);
        }

        // "login" command
        else if(strcmp(command, "login\n") == 0) {
            // the user is already logged in
            if(login_token != NULL) {
                printf("[Error]: You are already logged in! %s\n\n",
                            "If you want to switch users, logout first!");
                continue;
            }

            // compute JSON with credentials
            char* send_json = compute_json_register_login();
            if(send_json == NULL) 
                continue;

            // open connection, send POST request and get response
            sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
            message = compute_post_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com",
                 "/api/v1/tema/auth/login", "application/json", send_json, NULL, 0, NULL);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
           
            // extract received JSON and print message
            char* recv_json = basic_extract_json_response(response);
            if(check_error(recv_json) && check_response(response)) {
                printf("[Success]: You logged in.\n\n");

                // get cookie line and remove "Set-Cookie:"
                char *cookie_str = strstr(response, "Set-Cookie: ");
                memmove(cookie_str, cookie_str + 12, strlen(cookie_str));
                // get cookies
                memset(buffer, 0, strlen(buffer));
                memcpy(buffer, cookie_str, strlen(cookie_str));
                char* cookie = strtok(buffer, ";");
                cookies = (char *[]){cookie};

                // get token needed for logging in
                login_token = get_login_token(cookie_str);
            }

            // free variables and close connection
            free(message); free(response);
            close_connection(sockfd);
        }

        // "enter_library" command
        else if(strcmp(command, "enter_library\n") == 0) {
            // open connection, send GET request and get response
            sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
            message = compute_get_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com",
                "/api/v1/tema/library/access", NULL, cookies, 1, login_token);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // extract received JSON and print message
            char* recv_json = basic_extract_json_response(response);
            if(check_error(recv_json)) {
                printf("[Success]: You gained access to library.\n\n");

                // get JWT token needed for proving authentification
                access_token = get_jwt(recv_json);
            }

            // free variables and close connection
            free(message); free(response);
            close_connection(sockfd);
        }

        // "get_books" command
        else if(strcmp(command, "get_books\n") == 0) {
            // open connection, send GET request and get response
            sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
            message = compute_get_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com",
                "/api/v1/tema/library/books", NULL, cookies, 1, access_token);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // extract received JSON and print message
            char* recv_json = basic_extract_json_response(response);
            if(check_error(recv_json)) {
                // get books
                char* book_list = get_books_list(response);

                // print books
                printf("[Success]: Granted access to library books.\n\n");
                show_books_pretty(book_list);
            }

            // free variables and close connection
            free(message); free(response);
            close_connection(sockfd);
        }

        // "get_book" command
        else if(strcmp(command, "get_book\n") == 0) {
            // compute the path needed to get book with given ID
            char* path = form_path();
            if(path == NULL)
                continue;

            // open connection, send GET request and get response
            sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
            message = compute_get_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com",
                            path, NULL, cookies, 1, access_token);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // extract received JSON and print message
            char* recv_json = basic_extract_json_response(response);
            if(check_error(recv_json)) {
                printf("[Success]: Granted access to book informations.\n\n");
                show_book_info_pretty(recv_json);
            }

            // free variables and close connection
            free(path);
            free(message); free(response);
            close_connection(sockfd);
        }

        // "add_book" command
        else if(strcmp(command, "add_book\n") == 0) {
            // compute JSON with given informations
            char* send_json = compute_json_add_book();
            if(send_json == NULL)
                continue;

            // open connection, send POST request and get response
            sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
            message = compute_post_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", 
                        "/api/v1/tema/library/books", "application/json", send_json, 
                        cookies, 1, access_token);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // extract received JSON and print message
            char* recv_json = basic_extract_json_response(response);
            if(check_error(recv_json) && check_response(response)) {
                printf("[Success]: Book was added to library.\n\n");
            }

            // free variables and close connection
            free(message); free(response);
            close_connection(sockfd);
        }

        // "delete_book" command
        else if(strcmp(command, "delete_book\n") == 0) {
            // compute the path needed to delete book with given ID
            char* path = form_path();
            if(path == NULL)
                continue;

            // open connection, send DELETE request and get response
            sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
            message = compute_delete_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", 
                path, NULL, cookies, 1, access_token);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // extract received JSON and print message
            char* recv_json = basic_extract_json_response(response);
            if(check_error(recv_json)) {
                printf("[Succes]: Book was deleted from library.\n\n");
            }

            // free variables and close connection
            free(path);
            free(message); free(response);
            close_connection(sockfd);
        }        

        // "logout" command
        else if(strcmp(command, "logout\n") == 0) {
            // open connection, send GET request and get response
            sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
            message = compute_get_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", 
                "/api/v1/tema/auth/logout", NULL, cookies, 1, login_token);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // extract received JSON and print message
            char* recv_json = basic_extract_json_response(response);
            if(check_error(recv_json)) {
                printf("[Success]: You logged out.\n\n");
            }

            // free variables and close connection
            free(login_token);  free(access_token); 
            free(message); free(response);
            login_token = NULL;   access_token = NULL;
            close_connection(sockfd);            
        }

        // "exit" command
        else if(strcmp(command, "exit\n") == 0) {
            printf("[Success]: Client closed connection.\n");
            // free variables and exit
            free(login_token);  free(access_token); 
            exit(0);
        }

        // invalid command
        else if(strcmp(command, "\n")) {
            printf("[Error]: Invalid command!\n\n");
        }
    }

    // free the allocated data
    free(access_token);
    free(login_token);

    return 0;
}
