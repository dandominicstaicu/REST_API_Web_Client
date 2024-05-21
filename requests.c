// Copyright PCom Lab 9

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

char *compute_delete_request(char *host, char *url,
                             char *query_params, char **cookies, int cookies_count, int type)
{
    char *message = calloc(BUFLEN, sizeof(char));
    if (!message) {
        fprintf(stderr, "Memory allocation failed at %s:%d\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    char *line = calloc(LINELEN, sizeof(char));
    if (!line) {
        fprintf(stderr, "Memory allocation failed at %s:%d\n", __FILE__, __LINE__);
        free(message);
        exit(EXIT_FAILURE);
    }

    // Step 1: write the method name, URL, request params (if any) and protocol type
    if (query_params) {
        snprintf(line, LINELEN, "DELETE %s?%s HTTP/1.1", url, query_params);
    } else {
        snprintf(line, LINELEN, "DELETE %s HTTP/1.1", url);
    }
    compute_message(message, line);

    // Step 2: add the host
    snprintf(line, LINELEN, "Host: %s", host);
    compute_message(message, line);

    // Step 3 (optional): add headers and/or cookies, according to the protocol format
    if (cookies && cookies_count > 0) {
        snprintf(line, LINELEN, type == 0 ? "Cookie: " : "Authorization: Bearer ");

        for (int i = 0; i < cookies_count; ++i) {
            strncat(line, cookies[i], LINELEN - strlen(line) - 1);

            if (i < cookies_count - 1) {
                strncat(line, ";", LINELEN - strlen(line) - 1);
            }
        }
        compute_message(message, line);
    }

    // Step 4: add final new line
    compute_message(message, "");

    free(line);
    return message;
}

char *compute_get_request(char *host, char *url, char *query_params,
                          char **cookies, int cookies_count, int type)
{
    char *message = calloc(BUFLEN, sizeof(char));
    if (!message) {
        fprintf(stderr, "Memory allocation failed at %s:%d\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    char *line = calloc(LINELEN, sizeof(char));
    if (!line) {
        fprintf(stderr, "Memory allocation failed at %s:%d\n", __FILE__, __LINE__);
        free(message);
        exit(EXIT_FAILURE);
    }

    // Step 1: write the method name, URL, request params (if any) and protocol type
    if (query_params) {
        snprintf(line, LINELEN, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        snprintf(line, LINELEN, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // Step 2: add the host
    snprintf(line, LINELEN, "Host: %s", host);
    compute_message(message, line);

    // Step 3 (optional): add headers and/or cookies, according to the protocol format
    if (cookies && cookies_count > 0) {
        snprintf(line, LINELEN, type == 0 ? "Cookie: " : "Authorization: Bearer ");

        for (int i = 0; i < cookies_count; ++i) {
            strncat(line, cookies[i], LINELEN - strlen(line) - 1);

            if (i < cookies_count - 1) {
                strncat(line, ";", LINELEN - strlen(line) - 1);
            }
        }

        compute_message(message, line);
    }

    // Step 4: add final new line
    compute_message(message, "");

    free(line);
    return message;
}

char *compute_post_request(char *host, char *url, char *content_type, char **body_data,
                           int body_data_fields_count, char **cookies, int cookies_count, int type)
{
    char *message = calloc(BUFLEN, sizeof(char));
    if (!message) {
        fprintf(stderr, "Memory allocation failed at %s:%d\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    char *line = calloc(LINELEN, sizeof(char));
    if (!line) {
        fprintf(stderr, "Memory allocation failed at %s:%d\n", __FILE__, __LINE__);
        free(message);
        exit(EXIT_FAILURE);
    }

    char *body_data_buffer = calloc(BUFLEN, sizeof(char));
    if (!body_data_buffer) {
        fprintf(stderr, "Memory allocation failed at %s:%d\n", __FILE__, __LINE__);
        free(message);
        free(line);
        exit(EXIT_FAILURE);
    }

    // Step 1: write the method name, URL and protocol type
    snprintf(line, LINELEN, "POST %s HTTP/1.1", url);
    compute_message(message, line);

    // Step 2: add the host
    snprintf(line, LINELEN, "Host: %s", host);
    compute_message(message, line);

    // Step 3: add necessary headers (Content-Type and Content-Length are mandatory)
    for (int i = 0; i < body_data_fields_count; i++) {
        strncat(body_data_buffer, body_data[i], BUFLEN - strlen(body_data_buffer) - 1);

        if (i < body_data_fields_count - 1) {
            strncat(body_data_buffer, "&", BUFLEN - strlen(body_data_buffer) - 1);
        }
    }

    snprintf(line, LINELEN, "Content-Type: %s", content_type);
    compute_message(message, line);

    snprintf(line, LINELEN, "Content-Length: %zu", strlen(body_data_buffer));
    compute_message(message, line);

    // Step 4 (optional): add cookies
    if (cookies && cookies_count > 0) {
        snprintf(line, LINELEN, type == 0 ? "Cookie: " : "Authorization: Bearer ");

        for (int i = 0; i < cookies_count; i++) {
            strncat(line, cookies[i], LINELEN - strlen(line) - 1);

            if (i < cookies_count - 1) {
                strncat(line, ";", LINELEN - strlen(line) - 1);
            }
        }

        compute_message(message, line);
    }

    // Step 5: add new line at end of header
    compute_message(message, "");

    // Step 6: add the actual payload data
    strncat(message, body_data_buffer, BUFLEN - strlen(message) - 1);

    free(line);
    free(body_data_buffer);

    return message;
}

