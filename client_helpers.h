/*  Tema3 Protocoale de Comunicatii
    Gherasie Stefania 323CB */

#ifndef _CLIENTHELPERS_
#define _CLIENTHELPERS_

#include <stdio.h>      /* printf, sprintf */
#include <stdbool.h>
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
#include "parson/parson.h"


// checks if server is busy and can't receive requests
bool check_response(char* response) {
    char* code = strstr(response, "429 Too Many Requests");
    if(code != NULL) {
        printf("[Error]: Too many requests, please try again later.\n\n");
        return false;
    }
    return true;
}

// check if the response from server contains an error
bool check_error(char* recv_json) {
    JSON_Value *value = json_parse_string(recv_json);
    JSON_Object *object = json_object(value);

    // check for errors
    if(json_object_has_value(object, "error") == 1) {
        const char* error = json_object_get_string(object, "error");

        // if the authorization header is missing, there is no access to the library
        if(strcmp(error, "Authorization header is missing!") == 0)
            printf("[Error]: Authentication required to perform this operation!\n\n");
        // for other error, I kept the error format fron the JSON
        else
            printf("[Error]: %s\n\n", error);
        json_value_free(value);
        return false;
    }

    json_value_free(value);
    // there is no error
    return true;
}


// check if string contains only digits
bool check_digits(char *s)
{
    while (*s) {
        if (isdigit(*s++) == 0) 
            return false;
    }
    return true;
}


// get the token required to prove I am logged in
char* get_login_token(char* cookie_str){
    // get the token
    char* token = strtok(cookie_str, "=");
    token = strtok(NULL, ";");
    char* login_token = malloc(strlen(token) * sizeof(char));
    memcpy(login_token, token, strlen(token)); 

    return login_token; 
}


// get the JWT token from the received JSON
char* get_jwt(char* recv_json) {
    JSON_Value *value = json_parse_string(recv_json);
    JSON_Object *object = json_object(value);

    // get the token
    const char* token = json_object_get_string(object, "token");
    char* access_token = malloc(strlen(token) * sizeof(char));
    memcpy(access_token, token, strlen(token));
    
    json_value_free(value);
    return access_token;
}


// compute JSON needed for register and login
char* compute_json_register_login() {
    char username[30], password[30];

    // get username
    printf("username=");
    fgets(username, 30, stdin);
    username[strlen(username) - 1] = 0;

    // get password
    printf("password=");
    fgets(password, 30, stdin);
    password[strlen(password) - 1] = 0;

    // compute JSON
    JSON_Value *value = json_value_init_object();
    JSON_Object *object = json_value_get_object(value);
    json_object_set_string(object, "username", username);
    json_object_set_string(object, "password", password);

    // parse it as a string
    char* json = json_serialize_to_string_pretty(value);
    json_value_free(value);
    return json;
}


// compute JSON needed for add_book
char* compute_json_add_book(){
    char title[30], author[30], genre[30], publisher[30], page_count[20];

    // get title
    printf("title=");
    fgets(title, 30, stdin);
    title[strlen(title) - 1] = 0;

    // get author
    printf("author=");
    fgets(author, 30, stdin);
    author[strlen(author) - 1] = 0;

    // get genre
    printf("genre=");
    fgets(genre, 30, stdin);
    genre[strlen(genre) - 1] = 0;

    // get publisher
    printf("publisher=");
    fgets(publisher,30, stdin);
    publisher[strlen(publisher) - 1] = 0;

    // get page_count
    printf("page_count=");
    fgets(page_count, 20, stdin);
    page_count[strlen(page_count) - 1] = 0;

    // check if page_count contains other characters than digits
    if(check_digits(page_count) == false) {
        printf("[Error]: You must enter <page_count=POSITIVE_NUMBER>! Try again!\n\n");
        return NULL;
    } 

    // compute JSON 
    JSON_Value *value = json_value_init_object();
    JSON_Object *object = json_value_get_object(value);
    char *json = NULL;
    json_object_set_string(object, "title", title);
    json_object_set_string(object, "author", author);
    json_object_set_string(object, "genre", genre);
    json_object_set_number(object, "page_count", atoi(page_count));
    json_object_set_string(object, "publisher", publisher);

    // parse it as a string
    json = json_serialize_to_string_pretty(value);
    json_value_free(value);
    return json;   
}


// Form the URL path needed for get_book ot delete_book
char* form_path() {
    char id[10];

    // get book ID
    printf("id=");
    fgets(id, 10, stdin);
    id[strlen(id) - 1] = 0;

    if(check_digits(id) == false){
        printf("[Error]: You must enter <id=POSITIVE_NUMBER>! Try again!\n\n");
        return NULL;
    }

    // form path
    char* path = malloc(100*sizeof(char));    
    memset(path, 0, strlen(path));
    strcpy(path,"/api/v1/tema/library/books/");
    strcat(path, id);

    return path;
}

// print the book list in a pretty format
void show_books_pretty(char* book_list) {
    JSON_Value *value;
    JSON_Array *objects;
    JSON_Object *object;
    size_t i;

    value = json_parse_string(book_list);
    if (json_value_get_type(value) != JSONArray) {
        return;
    }

    // obtain an array of JSONs containing book informations
    objects = json_value_get_array(value);

    if(json_array_get_count(objects) == 0) {
        printf("Library does not contain any books.\n\n");
        json_value_free(value);
        return;
    }

    // print the header
    printf("%-6s %s\n", ":Id:", ":Title:");
    // print ID and Title for each book
    for (i = 0; i < json_array_get_count(objects); i++) {
        object = json_array_get_object(objects, i);
        printf("%-6d %s\n", (int) (json_object_get_number(object, "id")),
                        json_object_get_string(object, "title"));
    }
    printf("\n");

    // free allocated space
    json_value_free(value);
}

// print the book informations in a pretty format
void show_book_info_pretty(char* book_info) {
    JSON_Value *value;  
    JSON_Object *object;

    // get a JSON object from the string
    value = json_parse_string(book_info);
    object = json_object(value);

    // print headers and informations
    printf("%-13s %s\n", ":Title:", json_object_get_string(object, "title"));
    printf("%-13s %s\n", ":Author:", json_object_get_string(object, "author"));
    printf("%-13s %s\n", ":Genre:", json_object_get_string(object, "genre"));
    printf("%-13s %d\n", ":Page count:", 
                            (int) json_object_get_number(object, "page_count"));
    printf("%-13s %s\n\n", ":Publisher:", json_object_get_string(object, "publisher"));


    // free allocated variable
    json_value_free(value);
}

#endif