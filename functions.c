#include "functions.h"

char *duplicate(const char *src) {
    // Check if the source string is NULL
    if (src == NULL) {
        return NULL;  // If so, return NULL as there's nothing to duplicate
    }

    // Allocate memory for the duplicate string, including space for the null terminator
    char *dup = malloc(strlen(src) + 1); // +1 to account for the null terminator

    // Check if memory allocation was successful
    if (dup == NULL) {
        // Handle memory allocation failure by printing an error message
        fprintf(stderr, "Memory allocation failed in duplicate\n");
        return NULL;  // Return NULL to indicate failure
    }

    // Copy the source string to the newly allocated memory
    strcpy(dup, src);

    // Return the pointer to the newly duplicated string
    return dup;
}


int is_number(const char *str)
{
    // Iterate through each character in the string
    while (*str) {
        // Check if the current character is not a digit
        if (!isdigit(*str)) {
            return 0; // Return 0 (false) if a non-digit character is found
        }
        str++; // Move to the next character in the string
    }
    return 1; // Return 1 (true) if all characters are digits
}


char* generate_user_info(char *user, char *password) {
    // Initialize a new JSON object
    JSON_Value *value = json_value_init_object();
    JSON_Object *obj = json_value_get_object(value);

    // Populate the JSON object with user credentials
    populate_user_json_object(obj, user, password);

    // Serialize the JSON object to a formatted string
    char *info = serialize_json_to_string(value);

    // Free the JSON value and all associated memory
    json_value_free(value);

    // Return the serialized JSON string
    return info;
}

// Helper function to populate a JSON object with user credentials
void populate_user_json_object(JSON_Object *obj, char *user, char *password) {
    // Set the "username" field in the JSON object
    json_object_set_string(obj, "username", user);

    // Set the "password" field in the JSON object
    json_object_set_string(obj, "password", password);
}

// Helper function to serialize JSON value to a formatted string
char* serialize_json_to_string(JSON_Value *value) {
    return json_serialize_to_string_pretty(value);
}

void register_user(int sockfd)
{
    char user[NMAX], passwd[NMAX];

    // Prompt the user for their username
    printf("username=");
    scanf("%s", user);

    // Prompt the user for their password
    printf("password=");
    scanf("%s", passwd);

    // Generate a JSON string containing the user information
    char *info = generate_user_info(user, passwd);

    // Create a POST request message with the user info JSON string
    char *message = compute_post_request((char *)IP, (char *)REGISTER_PATH, (char *)CONTENT_TYPE, &info, 1, NULL, 0, 0);
    // Free the JSON string allocated by generate_user_info
    json_free_serialized_string(info);

    // Send the POST request to the server
    send_to_server(sockfd, message);
    // Free the message string allocated by compute_post_request
    free(message);

    // Receive the server's response
    char *response = receive_from_server(sockfd);
    // Tokenize the response to get the first line (usually the status line)
    char *token = strtok(response, "\n");

    // Print the first line of the server's response
    printf("%s\n", token);

    // Free the response string allocated by receive_from_server
    free(response);
}

char *send_login_request(int sockfd, char *info) {
    // Create a POST request message with the login info JSON string
    char *message = compute_post_request((char *)IP, (char *)LOGIN_PATH, (char *)CONTENT_TYPE, &info, 1, NULL, 0, 0);

    // Send the POST request to the server
    send_to_server(sockfd, message);

    // Free the message string allocated by compute_post_request
    free(message);

    // Receive and return the server's response
    return receive_from_server(sockfd);
}

void handle_login_response(const char *response, char **output, int *is_successful) {
    // Create a duplicate of the response to avoid modifying the original
    char *response_copy = duplicate(response);

    // Extract the first line from the response and print it
    char *line_one = strtok(response_copy, "\n");
    printf("%s\n", line_one);
    free(response_copy); // Free the duplicated response

    // Search for the "connect" string in the response
    char *connect_start = strstr(response, "connect");
    if (connect_start == NULL) {
        // If "connect" is not found, handle the error message
        char *json_start = strchr(response, '{');
        char *json_end = strchr(response, '}');

        if (json_start != NULL && json_end != NULL) {
            json_end[1] = '\0'; // Null-terminate the JSON string

            // Duplicate the JSON part of the response
            char *error_json = duplicate(json_start);
            // Parse the JSON string
            JSON_Value *parsed_json = json_parse_string(error_json);
            // Get the JSON object from the parsed JSON value
            JSON_Object *json_object = json_value_get_object(parsed_json);

            // Print the error message extracted from the JSON object
            printf("Error: %s\n", json_object_get_string(json_object, "error"));

            // Free the duplicated JSON string and the parsed JSON value
            free(error_json);
            json_value_free(parsed_json);
        }

        // Set the success flag to 0 (indicating failure)
        *is_successful = 0;
        return;
    }

    // Extract the part of the response following "connect"
    char *tmp_token = strtok(connect_start, ";");
    // Duplicate the extracted part and store it in the output parameter
    *output = duplicate(tmp_token);
    // Set the success flag to 1 (indicating success)
    *is_successful = 1;
}


char* login(int sockfd, char *cookie) {
    if (cookie) {
        printf("User is already logged in!\n");
        return cookie;
    }

    char user[NMAX];
    char passwd[NMAX];

    // Prompt for username and password
    prompt_for_credentials(user, passwd);

    // Process the login with the provided credentials
    return process_login(sockfd, user, passwd);
}

void prompt_for_credentials(char *user, char *passwd) {
    // Prompt for username and password
    printf("username=");
    scanf("%s", user);

    printf("password=");
    scanf("%s", passwd);
}

char* process_login(int sockfd, char *user, char *passwd) {
    char *info = generate_user_info(user, passwd);
    char *response = send_login_request(sockfd, info);

    // Free the JSON string allocated by generate_user_info
    json_free_serialized_string(info);

    char *to_ret = NULL;
    int success = 0;

    // Handle the server's login response
    handle_login_response(response, &to_ret, &success);

    // Free the response string allocated by receive_from_server
    free(response);

    // Return NULL if login was not successful, otherwise return the session cookie
    return success ? to_ret : NULL;
}

char *build_url(const char *base_path, const char *id_str) {
    // Calculate the required length for the URL string
    int path_length = strlen(base_path);
    int id_length = strlen(id_str);
    int total_length = path_length + id_length + 2; // 1 for '/' and 1 for '\0'

    // Allocate memory for the URL string
    char *full_url = (char *)malloc(total_length * sizeof(char));
    if (full_url == NULL) {
        // Handle memory allocation failure
        fprintf(stderr, "Memory allocation failed at %s:%d\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    // Initialize the allocated memory to ensure it's empty
    memset(full_url, 0, total_length * sizeof(char));

    // Construct the URL by concatenating base_path, '/', and id_str
    snprintf(full_url, total_length, "%s/%s", base_path, id_str);

    // Return the constructed URL string
    return full_url;
}



// Main function for sending a GET request
void send_get_request(int sockfd, char *url, char *token) {
    // Create a GET request message with the provided URL and token
    char *message = create_get_message(url, token);

    // Send the GET request to the server
    transmit_message(sockfd, message);

    // Free the message string allocated by create_get_message
    free(message);

    // Receive the server's response
    char *response = fetch_response(sockfd);

    // Handle the received response
    handle_get_response(response);

    // Free the response string allocated by fetch_response
    free(response);
}

char* create_get_message(char *url, char *token) {
    // create a GET request message
    return compute_get_request((char *)IP, url, NULL, &token, 1, 1);
}

void transmit_message(int sockfd, char *message) {
    // transmit the GET request message to the server
    send_to_server(sockfd, message);
}

char* fetch_response(int sockfd) {
    // fetch the server's response
    return receive_from_server(sockfd);
}

void handle_get_response(const char *response)
{
    // Find the start of the JSON object in the response
    char *start = strchr(response, '{');
    // Find the end of the JSON object in the response
    char *stop = strchr(response, '}');

    // Check if both the start and stop of the JSON object were found
    if (start && stop) {
        // Null-terminate the JSON string
        stop[1] = '\0';
        // Print the JSON part of the response
        printf("%s\n", start);
    } else {
        // Print an error message if the response format is invalid
        printf("Invalid response format\n");
    }
}

int validate_token(char *token) {
    if (!token) {
        printf("Cannot get the book - invalid or missing token\n");
        return 0;
    }
    return 1;
}

void prompt_for_id(char *id_str) {
    printf("id=");
    scanf("%s", id_str);
}

int check_id_is_number(char *id_str) {
    if (is_number(id_str) == 0) {
        printf("ID is not a number, please try again!\n");
        return 0;
    }
    return 1;
}

void process_book_request(int sockfd, char *id_str, char *token) {
    char *url = build_url(GET_PATH, id_str);
    send_get_request(sockfd, url, token);
    free(url);
}

void get_book(int sockfd, char *token) {
    if (!validate_token(token)) {
        return;
    }

    char id_str[NMAX];
    prompt_for_id(id_str);

    if (!check_id_is_number(id_str)) {
        return;
    }

    process_book_request(sockfd, id_str, token);
}

char *build_get_books_request(char *token)
{
    // Create a GET request message with the provided token
    return compute_get_request((char *)IP, GET_PATH, NULL, &token, 1, 1);
}

void send_request(int sockfd, char *message)
{
    // Send the request message to the server
    send_to_server(sockfd, message);

    // Receive the server's response
    char *response = receive_from_server(sockfd);

    // Handle the received response
    handle_books_response(response);

    // Free the response string allocated by receive_from_server
    free(response);
}

void handle_books_response(const char *response)
{
    // Find the start of the JSON array in the response
    char *start = strchr(response, '[');
    // Find the end of the JSON array in the response
    char *end = strchr(response, ']');

    // Check if both the start and end of the JSON array were found
    if (start && end) {
        // Null-terminate the JSON array string
        end[1] = '\0';
        // Print the JSON array part of the response
        printf("%s\n", start);
    } else {
        // Print an error message if the response format is invalid
        printf("Invalid response format\n");
    }
}

void get_books(int sockfd, char *token)
{
    // Check if the token is valid
    if (!token) {
        printf("Cannot get the list of books - invalid or missing token\n");
        return; // Return if the token is invalid or missing
    }

    // Build the GET request message using the provided token
    char *message = build_get_books_request(token);

    // Send the request to the server and handle the response
    send_request(sockfd, message);

    // Free the message string allocated by build_get_books_request
    free(message);
}

char *build_enter_library_request(char *cookie)
{
    // Create a GET request message with the provided cookie    
    return compute_get_request((char *)IP, ENTER_LIBRARY_PATH, NULL, &cookie, 1, 0);
}

void send_enter_library_request(int sockfd, char *message)
{
    // Send the request message to the server
    send_to_server(sockfd, message);
}

char *parse_enter_library_response(const char *response)
{
    // Create a duplicate of the response to avoid modifying the original
    char *copy = duplicate(response);
    // Extract and print the first line from the response
    char *tk = strtok(copy, "\n");
    printf("%s\n", tk);
    // Free the duplicated response
    free(copy);

    // Find the start of the JSON object in the response
    char *start = strchr(response, '{');
    // Find the end of the JSON object in the response
    char *stop = strchr(response, '}');

    // Check if both the start and end of the JSON object were found
    if (start && stop) {
        // Null-terminate the JSON string
        stop[1] = '\0';
        // Create a duplicate of the JSON string
        char *json_string = duplicate(start);

        // Parse the JSON string
        JSON_Value *result = json_parse_string(json_string);
        // Get the JSON object from the parsed JSON value
        JSON_Object *obj = json_value_get_object(result);

        // Extract the "token" field from the JSON object
        char *token = duplicate(json_object_get_string(obj, "token"));

        // Free the parsed JSON value and the duplicated JSON string
        json_value_free(result);
        free(json_string);

        // Return the extracted token
        return token;
    } else {
        // Print an error message if the response format is invalid
        printf("Invalid response format\n");
        return NULL; // Return NULL to indicate failure
    }
}

char *enter_library(int sockfd, char *cookie)
{
    // Build the enter library request message
    char *message = build_enter_library_request(cookie);

    // Send the enter library request to the server
    send_enter_library_request(sockfd, message);

    // Free the message string allocated by build_enter_library_request
    free(message);

    // Receive the server's response
    char *response = receive_from_server(sockfd);

    // Parse the response to extract the access token
    char *token = parse_enter_library_response(response);

    // Free the response string allocated by receive_from_server
    free(response);

    // Return the extracted token
    return token;
}

void read_book_info(JSON_Object *obj)
{
    char buff[NMAX];
    // Fields and prompts for book information
    const char *fields[] = {"title", "author", "genre", "publisher", "page_count"};
    const char *prompts[] = {"title=", "author=", "genre=", "publisher=", "page_count="};
    int num_fields = sizeof(fields) / sizeof(fields[0]);

    // Loop through each field to get input from the user
    for (int i = 0; i < num_fields; i++) {
        // Prompt the user for the current field
        printf("%s", prompts[i]);
        // Read user input into buff
        fgets(buff, NMAX, stdin);
        // Remove the newline character if present
        if (buff[strlen(buff) - 1] == '\n')
            buff[strlen(buff) - 1] = '\0';

        // If the current field is "page_count", validate and set it as a number
        if (strcmp(fields[i], "page_count") == 0) {
            if (is_number(buff) == 0) {
                printf("Invalid number of pages! Aborting request.\n");
                exit(1);
            }
            // Convert the input to an integer and set it in the JSON object
            json_object_set_number(obj, fields[i], (double)atoi(buff));
        } else {
            // Set the input as a string in the JSON object for other fields
            json_object_set_string(obj, fields[i], buff);
        }
    }
}

char *build_add_book_request(JSON_Value *val, char *token)
{
    // Serialize the JSON value to a pretty string
    char *serialized_info = json_serialize_to_string_pretty(val);
    char *request_message;

    // Prepare headers array with the token if available
    char *headers[] = { token ? token : NULL };
    int header_count = token ? 1 : 0;

    // Create a POST request message with the serialized JSON and headers
    request_message = compute_post_request(
        (char *)IP,
        (char *)ADD_BOOK_PATH,
        (char *)CONTENT_TYPE,
        &serialized_info,
        1,
        headers,
        header_count,
        1
    );

    // Free the serialized JSON string
    json_free_serialized_string(serialized_info);

    // Return the constructed POST request message
    return request_message;
}


void send_add_book_request(int sockfd, char *message)
{
    // Send the request message to the server
    send_to_server(sockfd, message);

    // Receive the server's response
    char *response = receive_from_server(sockfd);

    // Handle the received response
    handle_add_book_response(response);

    // Free the response string allocated by receive_from_server
    free(response);
}

void handle_add_book_response(char *response)
{
    // Extract and print the first line from the response
    char *tk = strtok(response, "\n");
    printf("%s\n", tk);
}

void add_book(int sockfd, char *token)
{
    // Initialize a new JSON object for the book information
    JSON_Value *val = json_value_init_object();
    JSON_Object *obj = json_value_get_object(val);

    // Read book information from the user and populate the JSON object
    read_book_info(obj);

    // Build the add book request message
    char *message = build_add_book_request(val, token);

    // Send the add book request to the server
    send_add_book_request(sockfd, message);

    // Free the message string allocated by build_add_book_request
    free(message);

    // Free the JSON value allocated for the book information
    json_value_free(val);
}

char *build_delete_book_url(const char *id_str)
{
    // Calculate the total length required for the URL string
    int base_len = strlen(GET_PATH);
    int id_len = strlen(id_str);
    int total_len = base_len + id_len + 2; // 1 for '/' and 1 for null terminator

    // Allocate memory for the URL string
    char *url = malloc(total_len * sizeof(char));
    if (url == NULL) {
        // Handle memory allocation failure
        fprintf(stderr, "Memory allocation failed at %s:%d\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    // Construct the URL string
    snprintf(url, total_len, "%s/%s", GET_PATH, id_str);

    // Return the constructed URL string
    return url;
}


char *build_delete_book_request(char *url, char *token)
{
    // Create a DELETE request message with the provided URL and token
    return compute_delete_request((char *)IP, url, NULL, &token, 1, 1);
}

void send_delete_book_request(int sockfd, char *message)
{
    // Send the request message to the server
    send_to_server(sockfd, message);

    // Receive the server's response
    char *response = receive_from_server(sockfd);

    // Handle the received response
    handle_delete_book_response(response);

    // Free the response string allocated by receive_from_server
    free(response);
}

void handle_delete_book_response(char *response)
{
    // Extract the first line from the response
    char *tk = strtok(response, "\n");
    // Print the first line of the response
    printf("%s\n", tk);
}

/**
 * A function that performs a DELETE request in order to delete a book from the
 * server.
 * Parameters: the socket file descriptor, the cookie (to check if the user is
 * logged in) and the token (to check if the user accessed the library).
 */
void delete_book(int sockfd, char *token)
{
    char id_str[NMAX];

    // Prompt the user for the book ID
    printf("id=");
    scanf("%s", id_str);

    // Check if the provided ID is a number
    if (is_number(id_str) == 0) {
        printf("ID is not a number, please try again!\n");
        return; // Return if the ID is not a number
    }

    // Build the URL for the delete book request using the provided ID
    char *url = build_delete_book_url(id_str);
    // Build the delete book request message
    char *message = build_delete_book_request(url, token);

    // Send the delete book request to the server
    send_delete_book_request(sockfd, message);

    // Free the URL and message strings allocated by build_delete_book_url and build_delete_book_request
    free(url);
    free(message);
}

char *build_logout_request(char *cookie)
{
    // Create a GET request message with the provided cookie
    return compute_get_request((char *)IP, LOGOUT_PATH, NULL, &cookie, 1, 0);
}

void send_logout_request(int sockfd, char *message)
{
    // Send the request message to the server
    send_to_server(sockfd, message);

    // Receive the server's response
    char *response = receive_from_server(sockfd);

    // Handle the received response
    handle_logout_response(response);

    // Free the response string allocated by receive_from_server
    free(response);
}

void handle_logout_response(char *response)
{
    // Extract the first line from the response
    char *tk = strtok(response, "\n");
    // Print the first line of the response
    printf("%s\n", tk);
}

void logout(int sockfd, char *cookie)
{
    // Build the logout request message using the provided cookie
    char *message = build_logout_request(cookie);

    // Send the logout request to the server and handle the response
    send_logout_request(sockfd, message);

    // Free the message string allocated by build_logout_request
    free(message);
}

void exit_client(int sockfd)
{
    // Close the HTTP connection over TCP
    close_connection(sockfd);

    // Exit the program
    exit(0);
}