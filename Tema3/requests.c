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
                            char *session_cookie, char *jwt, int jwtAccess)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // add the host to the message
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // add JWT token to the message
    if (jwtAccess) {
        memset(line, 0, LINELEN);
        sprintf(line, "Authorization: Bearer %s", jwt);
        compute_message(message, line);
    }

    // add the session cookie to the message
    if (session_cookie != NULL) {
        memset(line, 0, LINELEN);
        strcat(line, "Cookie: ");

        strcat(line, session_cookie);
        compute_message(message, line);
    }

    // add final new line
    compute_message(message, "");

    free(line);
    return message;
}

char *compute_delete_request(char *host, char *url, char *query_params,
                            char *session_cookie, char *jwt, int jwtAccess)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // add the host to the message
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // add JWT token to the message
    if (jwtAccess) {
        memset(line, 0, LINELEN);
        sprintf(line, "Authorization: Bearer %s", jwt);
        compute_message(message, line);
    }

    // add the session cookie to the message
    if (session_cookie != NULL) {
        memset(line, 0, LINELEN);
        strcat(line, "Cookie: ");

        strcat(line, session_cookie);
        compute_message(message, line);
    }

    // add final new line
    compute_message(message, "");

    free(line);
    return message;
}

char *compute_post_request(char *host, char *url, char* content_type,
                            char *payload, char *session_cookie, char *jwt,
                            int jwtAccess, int body_data_size)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // write the method name, URL and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    // add the host to the message
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // add the content type to the message
    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    // add JWT token to the message
    if (jwtAccess) {
        memset(line, 0, LINELEN);
        sprintf(line, "Authorization: Bearer %s", jwt);
        compute_message(message, line);
    }

    // add session_cookie to the message
    if (session_cookie != NULL) {
        memset(line, 0, LINELEN);
        strcat(line, "Cookie: ");

        strcat(line, session_cookie);
        compute_message(message, line);
    }

    // add the content length to the message
    sprintf(line, "Content-Length: %d", body_data_size);
    compute_message(message, line);

    // add newline and then payload to the message
    compute_message(message, "");
    memset(line, 0, LINELEN);
    sprintf(line, "%s", payload);

    // add final new line
    compute_message(message, line);

    free(line);
    return message;
}
