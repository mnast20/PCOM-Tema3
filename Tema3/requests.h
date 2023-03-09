#ifndef _REQUESTS_
#define _REQUESTS_

void get_cookies(char *cookies, char** cookies_array, int *cookies_len);

// computes and returns a GET request string (query_params
// and cookies can be set to NULL if not needed)
char *compute_get_request(char *host, char *url, char *query_params,
                            char *session_cookie, char *jwt, int jwtAccess);

// computes and returns a DELETE request string (query_params
// and cookies can be set to NULL if not needed)
char *compute_delete_request(char *host, char *url, char *query_params,
                            char *session_cookie, char *jwt, int jwtAccess);

// computes and returns a POST request string (cookies can be NULL if not needed)
char *compute_post_request(char *host, char *url, char* content_type, char *payload,
                            char *session_cookie, char *jwt, int jwtAccess, int body_data_size);

#endif
