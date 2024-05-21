#include <stdio.h>
#include "functions.h"

int parse_from_string(char *command)
{
    if (!strcmp(command, "register"))
        return 0;
    
    if (!strcmp(command, "login"))
        return 1;

    if (!strcmp(command, "get_book"))
        return 2;

    if (!strcmp(command, "get_books"))
        return 3;

    if (!strcmp(command, "enter_library"))
        return 4;

    if (!strcmp(command, "add_book"))
        return 5;
    
    if (!strcmp(command, "delete_book"))
        return 6;
    
    if (!strcmp(command, "logout"))
        return 7;
    
    if (!strcmp(command, "exit"))
        return 8;
    
    return 9;
}

int main(void)
{
    char command[NMAX];
    char *cookie = NULL, *token = NULL;

    while (fgets(command, NMAX, stdin)) {
        size_t len = strlen(command);
        if (len > 0 && command[len - 1] == '\n') {
            command[len - 1] = '\0';
        }

        // open a new connection - HTTP is stateless
        int sockfd = open_connection((char *)IP, PORT, AF_INET, SOCK_STREAM, 0);

        // each command type
        int command_type = parse_from_string(command);
        switch (command_type) {
            case 0:
                if (cookie) {
                    printf("User is already logged in!\n");
                } else {
                    register_user(sockfd);
                }
                break;

            case 1:
                if (!cookie) {
                    cookie = login(sockfd, cookie);
                } else {
                    free(cookie);
                    cookie = login(sockfd, cookie);
                }
                break;

            case 2:
                if (!cookie) {
                    printf("User not logged in!\n");
                } else {
                    get_book(sockfd, token);
                }
                break;

            case 3:
                if (!cookie) {
                    printf("User not logged in!\n");
                } else {
                    get_books(sockfd, token);
                }
                break;

            case 4:
                if (!cookie) {
                    printf("User not logged in!\n");
                } else {
                    char *tmp = enter_library(sockfd, cookie);
                    if (tmp) {
                        free(token);
                        token = tmp;
                    }
                }
                break;

            case 5:
                if (!cookie) {
                    printf("User not logged in!\n");
                } else {
                    add_book(sockfd, token);
                }
                break;

            case 6:
                if (!cookie) {
                    printf("User not logged in!\n");
                } else if (!token) {
                    printf("Invalid token!\n");
                } else {
                    delete_book(sockfd, token);
                }
                break;

            case 7:
                if (!cookie) {
                    printf("User already logged out!\n");
                } else {
                    logout(sockfd, cookie);
                    free(cookie);
                    free(token);
                    token = NULL;
                    cookie = NULL;
                }
                break;

            case 8:
                exit_client(sockfd);
                break;

            case 9:
                break;

            default:
                printf("Unknown command!\n");
                break;
        }

        // Close the connection on the TCP socket.
        close_connection(sockfd);
    }

    return 0;
}
