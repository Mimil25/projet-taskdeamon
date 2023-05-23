#ifndef LIBMESSAGE
#define LIBMESSAGE

/*
 * send a string by a file descriptor
 */
int send_string(int fd, char* str);

/*
 * recieve a string from a file descriptor
 */
char* recv_string(int fd);

/*
 * send a NULL terminated array of strings by a file descriptor
 */
int send_argv(int fd, char* argv[]);

/*
 * recieve a NULL terminated array of strings from a file descriptor
 */
char** recv_argv(int fd);

#endif // LIBMESSAGE
