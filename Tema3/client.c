#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "parson/parson.h"
#include "helpers.h"
#include "requests.h"

#define MAX_LEN 2000

#define HOST "34.241.4.235"
#define PORT 8080

int main(int argc, char *argv[])
{
    int sockfd;
    int logged_in_client = 0, library_access = 0;
    char* session_cookie;
    char* jwt;

    session_cookie = (char *)calloc(MAX_LEN, sizeof(char));

    jwt = (char *)calloc(MAX_LEN, sizeof(char));

    printf("Please choose one of these commands: ");
    printf("register, login, enter_library, get_books, ");
    printf("get_book, add_book, delete_book, exit.\n\n");

    while (1) {
        char command[1000];
        fgets(command, 1000, stdin);

        if (!strcmp(command, "register\n")) {
            char username[1000], password[1000];

            // open connection to server
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            char *access_route = "/api/v1/tema/auth/register";
            char *content_type = "application/json";

            printf("username = ");
            fgets(username, 1000, stdin);
            username[strlen(username) - 1] = '\0';

            printf("password = ");
            fgets(password, 1000, stdin);
            password[strlen(password) - 1] = '\0';

            if (logged_in_client) {
                printf("User is already logged in. If ");
                printf("you want to register another user, log out first.\n\n");
                continue;
            }
            
            // create a JSON object containing the username and password fields
            JSON_Value *value = json_value_init_object();
            JSON_Object *object = json_value_get_object(value);
            json_object_set_string(object, "username", username);
            json_object_set_string(object, "password", password);
            char *payload = json_serialize_to_string_pretty(value);
            int size = strlen(payload);

            // create the POST message and send it to the server
            char *message = compute_post_request(HOST, access_route,
                    content_type, payload, NULL, jwt, library_access, size);
            send_to_server(sockfd, message);

            char *response = receive_from_server(sockfd);
            free(message);

            if (strstr(response, "Too many requests")) {
                printf("Too many requests have been made to the server. ");
                printf("Please try again later.\n\n");

                json_free_serialized_string(payload);
                json_value_free(value);
                free(response);
                close(sockfd);
                continue;
            }
            
            if (strstr(response, "error")) {
                printf("Username already in use.\n\n");
            } else {
                printf("Successful registration.\n\n");
            }

            json_free_serialized_string(payload);
            json_value_free(value);
            free(response);
            // close socket
            close(sockfd);
        } else if (!strcmp(command, "login\n")) {
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            char *access_route = "/api/v1/tema/auth/login";
            char *content_type = "application/json";

            char username[1000], password[1000];

            printf("username = ");
            fgets(username, 1000, stdin);
            username[strlen(username) - 1] = '\0';

            printf("password = ");
            fgets(password, 1000, stdin);
            password[strlen(password) - 1] = '\0';

            if (logged_in_client) {
                printf("User is already logged in. ");
                printf("If you want to switch users, log out first.\n\n");
                continue;
            }

            // create a JSON object containing the username and password fields
            JSON_Value *value = json_value_init_object();
            JSON_Object *object = json_value_get_object(value);
            json_object_set_string(object, "username", username);
            json_object_set_string(object, "password", password);
            char *payload = json_serialize_to_string_pretty(value);
            int size = strlen(payload);

            // create the POST message and send it to the server
            char *message = compute_post_request(HOST, access_route,
                    content_type, payload, NULL, jwt, library_access, size);
            send_to_server(sockfd, message);

            // get server's response
            char *response = receive_from_server(sockfd);
            free(message);

            if (strstr(response, "Too many requests")) {
                printf("Too many requests have been made to the server. ");
                printf("Please try again later.\n\n");

                json_free_serialized_string(payload);
                json_value_free(value);
                free(response);
                close(sockfd);
                continue;
            }
            
            if (strstr(response, "error")) {
                printf("Credentials don't match.\n\n");
            } else {
                logged_in_client = 1;
                char response_copy[strlen(response)];
                strncpy(response_copy, response, strlen(response));
                // get beginning of session cookie
                char *beginning_cookie = strstr(response_copy, "connect.sid");

                if (!beginning_cookie) {
                    json_free_serialized_string(payload);
                    json_value_free(value);
                    close(sockfd);

                    printf("A problem occurred in getting the session cookie.\n\n");
                    continue;
                }

                // get end of session cookie
                char *end_cookie = strstr(beginning_cookie, ";");
                int nr_letters = end_cookie - beginning_cookie;
                *end_cookie = '\0';

                // extract session cookie
                memset(session_cookie, 0, MAX_LEN);
                strncpy(session_cookie, beginning_cookie, nr_letters);
                printf("Successful authentification.\n\n");
            }

            json_free_serialized_string(payload);
            json_value_free(value);
            free(response);
            // close socket
            close(sockfd);
        } else if (!strcmp(command, "enter_library\n")) {
            if (!logged_in_client) {
                printf("User is not logged in.\n\n");
                continue;
            }

            // open connection to server
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            char *access_route = "/api/v1/tema/library/access";

            // create the GET message and send it to the server
            char* message = compute_get_request(HOST, access_route, NULL,
                                    session_cookie, jwt, library_access);
            send_to_server(sockfd, message);

            // get server's response
            char* response = receive_from_server(sockfd);
            free(message);

            if (strstr(response, "Too many requests")) {
                printf("Too many requests have been made to the server. ");
                printf("Please try again later.\n\n");

                free(response);
                close(sockfd);
                continue;
            }

            if (strstr(response, "error")) {
                printf("There was an error in accessing the library.\n\n");
            } else {
                library_access = 1;

                char *JSON_message = strstr(response, "{");

                if (!JSON_message) {
                    free(response);
                    close(sockfd);

                    printf("A problem occurred in getting the JWT token.\n\n");
                    continue;
                }

                JSON_Value *value = json_parse_string(JSON_message);

                // get JWT value in JSON format
                memset(jwt, 0, MAX_LEN);
                char *token = json_serialize_to_string(value);

                // create copy
                char token_copy[strlen(token)];
                strncpy(token_copy, token, strlen(token));
                // go to the end of the JWT value
                char *end_token = strstr(token_copy, "\"}");
                *end_token = '\0';

                // get the beginning of the JWT value
                char *beginning_token = strstr(token_copy, ":\"");
                beginning_token = beginning_token + 2;

                // extract the JWT value and keep it
                memcpy(jwt, beginning_token, strlen(beginning_token));

                printf("User now has access to library.\n\n");

                json_free_serialized_string(token);
                json_value_free(value);
            }

            // close socket
            free(response);
            close(sockfd);
        } else if (!strcmp(command, "get_books\n")) {
            if (!logged_in_client) {
                printf("User is not logged in.\n\n");
                continue;
            }

            if (!library_access) {
                printf("User could not access the library.\n\n");
                continue;
            }

            // open connection to server
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            char *access_route = "/api/v1/tema/library/books";

            // create the GET message and send it to the server
            char* message = compute_get_request(HOST, access_route, NULL,
                                    session_cookie, jwt, library_access);
            send_to_server(sockfd, message);

            // get server's response
            char *response = receive_from_server(sockfd);
            free(message);

            if (strstr(response, "Too many requests")) {
                printf("Too many requests have been made to the server. ");
                printf("Please try again later.\n\n");

                free(response);
                close(sockfd);
                continue;
            }

            if (strstr(response, "error")) {
                printf("There was an error in accessing the library's books.\n\n");
            } else {
                char *JSON_message = strstr(response, "[");

                if (!JSON_message) {
                    free(response);
                    close(sockfd);
                    continue;
                }

                // get library in JSON format
                JSON_Value *value = json_parse_string(JSON_message);
                char *books = json_serialize_to_string_pretty(value);
                printf("%s\n\n", books);

                json_free_serialized_string(books);
                json_value_free(value);
            }

            free(response);
            // close socket
            close(sockfd);
        } else if (!strcmp(command, "get_book\n")) {
            char id[1000];

            printf("id = ");
            fgets(id, 1000, stdin);
            id[strlen(id) - 1] = '\0';

            if (!logged_in_client) {
                printf("User is not logged in.\n\n");
                continue;
            }

            if (!library_access) {
                printf("User could not access the library.\n\n");
                continue;
            }

            // open connection to server
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            char *access_route = calloc(MAX_LEN, sizeof(char));

            // get access route based on ID
            snprintf(access_route, strlen(id) +
                    strlen("/api/v1/tema/library/books/") + 1,
                        "/api/v1/tema/library/books/%s", id);

            // create the GET message and send it to the server
            char* message = compute_get_request(HOST, access_route, NULL,
                                    session_cookie, jwt, library_access);
            send_to_server(sockfd, message);

            // get server's response
            char *response = receive_from_server(sockfd);
            free(message);

            if (strstr(response, "Too many requests")) {
                printf("Too many requests have been made to the server. ");
                printf("Please try again later.\n\n");

                free(access_route);
                free(response);
                close(sockfd);
                continue;
            }

            if (strstr(response, "error")) {
                printf("There was an error in accessing the requested book.\n\n");
            } else {
                // get book in JSON format
                char *JSON_message = strstr(response, "[{");
                JSON_Value *value = json_parse_string(JSON_message);
                char *book = json_serialize_to_string_pretty(value);
                printf("%s\n\n", book);

                json_free_serialized_string(book);
                json_value_free(value);
            }

            free(access_route);
            free(response);
            // close socket
            close(sockfd);
        } else if (!strcmp(command, "add_book\n")) {
            char title[1000], author[1000], genre[1000];
            char publisher[1000], page_count[1000];

            printf("title = ");
            fgets(title, 1000, stdin);
            title[strlen(title) - 1] = '\0';

            printf("author = ");
            fgets(author, 1000, stdin);
            author[strlen(author) - 1] = '\0';

            printf("genre = ");
            fgets(genre, 1000, stdin);
            genre[strlen(genre) - 1] = '\0';

            printf("publisher = ");
            fgets(publisher, 1000, stdin);
            publisher[strlen(publisher) - 1] = '\0';

            printf("page_count = ");
            fgets(page_count, 1000, stdin);
            page_count[strlen(page_count) - 1] = '\0';

            if (!logged_in_client) {
                printf("User is not logged in.\n\n");
                continue;
            }

            if (!library_access) {
                printf("User could not access the library.\n\n");
                continue;
            }

            // open connection to server
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            char *access_route = "/api/v1/tema/library/books";
            char* content_type = "application/json";

            // create a JSON object containing the title, author, genre,
            // page count and publisher fields
            JSON_Value *value = json_value_init_object();
            JSON_Object *object = json_value_get_object(value);
            json_object_set_string(object, "title", title);
            json_object_set_string(object, "author", author);
            json_object_set_string(object, "genre", genre);
            json_object_set_string(object, "publisher", publisher);
            json_object_set_number(object, "page_count", atoi(page_count));
            char *payload = json_serialize_to_string_pretty(value);
            int size = strlen(payload);

            // create the POST message and send it to the server
            char *message = compute_post_request(HOST, access_route,
            content_type, payload, session_cookie, jwt, library_access, size);
            send_to_server(sockfd, message);

            // get server's response
            char *response = receive_from_server(sockfd);
            free(message);

            if (strstr(response, "Too many requests")) {
                printf("Too many requests have been made to the server. ");
                printf("Please try again later.\n\n");

                json_free_serialized_string(payload);
                json_value_free(value);
                free(response);
                close(sockfd);
                continue;
            }

            if (strstr(response, "error")) {
                printf("There was an error in adding the requested book.\n\n");
            } else {
                printf("Book was successfully added to the database.\n\n");
            }

            json_free_serialized_string(payload);
            json_value_free(value);
            free(response);
            // close socket
            close(sockfd);
        } else if (!strcmp(command, "delete_book\n")) {
            char id[1000];

            printf("id = ");
            fgets(id, 1000, stdin);
            id[strlen(id) - 1] = '\0';

            if (!logged_in_client) {
                printf("User is not logged in.\n\n");
                continue;
            }

            if (!library_access) {
                printf("User could not access the library.\n\n");
                continue;
            }

            // open connection to server
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            char *access_route = calloc(MAX_LEN, sizeof(char));

            // get access route based on ID
            snprintf(access_route, strlen(id) +
                    strlen("/api/v1/tema/library/books/") + 1,
                        "/api/v1/tema/library/books/%s", id);

            // create the DELETE message and send it to the server
            char* message = compute_delete_request(HOST, access_route, NULL,
                                        session_cookie, jwt, library_access);
            send_to_server(sockfd, message);

            // get server's response
            char *response = receive_from_server(sockfd);
            free(message);

            if (strstr(response, "Too many requests")) {
                printf("Too many requests have been made to the server. ");
                printf("Please try again later.\n\n");

                free(access_route);
                free(response);
                close(sockfd);
                continue;
            }

            if (strstr(response, "error")) {
                printf("There was an error in deleting the requested book.\n\n");
            } else {
                printf("Book was successfully deleted from the database.\n\n");
            }

            free(access_route);
            free(response);
            // close socket
            close(sockfd);
        } else if (!strcmp(command, "logout\n")) {
            if (!logged_in_client) {
                printf("There is no user logged in.\n\n");
                continue;
            }

            // open connection to server
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            char *access_route = "/api/v1/tema/auth/logout";

            // create the GET message and send it to the server
            char* message = compute_get_request(HOST, access_route, NULL,
                                    session_cookie, jwt, library_access);
            send_to_server(sockfd, message);

            // get server's response
            char *response = receive_from_server(sockfd);
            free(message);

            if (strstr(response, "Too many requests")) {
                printf("Too many requests have been made to the server. ");
                printf("Please try again later.\n\n");

                free(response);
                close(sockfd);
                continue;
            }

            if (strstr(response, "error")) {
                printf("There was an error in disconnecting user.\n\n");
            } else {
                logged_in_client = 0;
                library_access = 0;
                printf("User was successfully disconnected.\n\n");
            }

            free(response);
            // close socket
            close(sockfd);
        } else if (!strcmp(command, "exit\n")) {
            break;
        } else {
            printf("Unavailable command. Please choose between ");
            printf("register, login, enter_library, get_books, ");
            printf("get_book, add_book, delete_book, exit.\n\n");
        }
    }

    free(jwt);
    free(session_cookie);

    return 0;
}
