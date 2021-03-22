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


// computes and returns a GET request string
char *compute_get_request(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count, char *token)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *buffer = calloc(LINELEN, sizeof(char));

    // write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }
    compute_message(message, line);

    // add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // add Authorization header
    if(token != NULL) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    // add cookies, according to the protocol format
    if (cookies != NULL) {
        memset(line, 0, LINELEN);

        // make line with cookies
        strcat(line, "Cookie: ");
        for(int i = 0; i < cookies_count - 1; i++) {
            sprintf(buffer, "%s; ", cookies[i]);
            strcat(line, buffer);
        }
        strcat(line, cookies[cookies_count - 1]);

        compute_message(message, line);
    }

    // add final new line
    compute_message(message, "");

    // free allocated variables and return message
    free(buffer);
    free(line);
    return message;
}


// computes and returns a DELETE request string
char *compute_delete_request(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count, char *token)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *buffer = calloc(LINELEN, sizeof(char));

    // write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }
    compute_message(message, line);

    // add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // add Authorization header
    if(token != NULL) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    // add cookies, according to the protocol format
    if (cookies != NULL) {
        memset(line, 0, LINELEN);

        // make line with cookies
        strcat(line, "Cookie: ");
        for(int i = 0; i < cookies_count - 1; i++) {
            sprintf(buffer, "%s; ", cookies[i]);
            strcat(line, buffer);
        }
        strcat(line, cookies[cookies_count - 1]);

        compute_message(message, line);
    }

    // add final new line
    compute_message(message, "");

    // free alocated variables and return message
    free(buffer);
    free(line);
    return message;
}

// computes and returns a POST request string 
char *compute_post_request(char *host, char *url, char* content_type, char *body_data,
                        char **cookies, int cookies_count, char *token)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *buffer = calloc(LINELEN, sizeof(char));
    int i;

    // write the method name, URL and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    // add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // add Authorization header
    if(token != NULL){
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    // add Content-Type and Content-Length headers
    sprintf(line , "Content-Type: %s", content_type);
    compute_message(message, line);


    sprintf(line, "Content-Length: %ld", strlen(body_data));
    compute_message(message, line);

    // add cookies
    if (cookies != NULL) {
        memset(line, 0, LINELEN);

        // make line with cookies
        for (i = 0; i < cookies_count - 1; i++) {
            sprintf(buffer, "Cookie: %s;", cookies[i]);
            strcat(line, buffer);
        } 
        sprintf(buffer, "Cookie: %s", cookies[cookies_count - 1]);
        strcat(line, buffer);

        compute_message(message, line);
    }

    
    // add new line at end of header
    compute_message(message, "");

    // add the actual payload data
    compute_message(message, body_data);

    // free allocated variables and return message
    free(buffer);
    free(line);
    return message;
}
