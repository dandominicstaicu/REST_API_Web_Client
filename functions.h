#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "requests.h"
#include "helpers.h"
#include "buffer.h"
#include "parson.h"


#define NMAX 100
#define LEN 50
#define IP "34.246.184.49"
#define PORT 8080
#define REGISTER_PATH "/api/v1/tema/auth/register"
#define CONTENT_TYPE "application/json"
#define LOGIN_PATH "/api/v1/tema/auth/login"
#define GET_PATH "/api/v1/tema/library/books"
#define LOGOUT_PATH "/api/v1/tema/auth/logout"
#define ENTER_LIBRARY_PATH "/api/v1/tema/library/access"
#define ADD_BOOK_PATH GET_PATH


char *duplicate(const char *src);
int is_number(const char *str);

char* generate_user_info(char *user, char *password);
void populate_user_json_object(JSON_Object *obj, char *user, char *password);
char* serialize_json_to_string(JSON_Value *value);
void register_user(int sockfd);

char *send_login_request(int sockfd, char *info);
void handle_login_response(const char *response, char **to_ret, int *success);
void prompt_for_credentials(char *user, char *passwd);
char* process_login(int sockfd, char *user, char *passwd);
char* login(int sockfd, char *cookie);

char *build_url(const char *base_path, const char *id_str);
void send_get_request(int sockfd, char *url, char *token);
char* create_get_message(char *url, char *token);
void transmit_message(int sockfd, char *message);
char* fetch_response(int sockfd);
void handle_get_response(const char *response);
void get_book(int sockfd, char *token);

char *build_get_books_request(char *token);
void send_request(int sockfd, char *message);
void handle_books_response(const char *response);

int validate_token(char *token);
void prompt_for_id(char *id_str);
int check_id_is_number(char *id_str);
void process_book_request(int sockfd, char *id_str, char *token);
void get_books(int sockfd, char *token);

char *build_enter_library_request(char *cookie);
void send_enter_library_request(int sockfd, char *message);
char *parse_enter_library_response(const char *response);
char *enter_library(int sockfd, char *cookie);

void read_book_info(JSON_Object *obj);
char *build_add_book_request(JSON_Value *val, char *token);
void send_add_book_request(int sockfd, char *message);
void handle_add_book_response(char *response);
void add_book(int sockfd, char *token);

char *build_delete_book_url(const char *id_str);
char *build_delete_book_request(char *url, char *token);
void send_delete_book_request(int sockfd, char *message);
void handle_delete_book_response(char *response);
void delete_book(int sockfd, char *token);

char *build_logout_request(char *cookie);
void send_logout_request(int sockfd, char *message);
void handle_logout_response(char *response);
void logout(int sockfd, char *cookie);

void exit_client(int sockfd);

#endif