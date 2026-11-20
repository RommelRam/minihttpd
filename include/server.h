#ifndef SERVER_H
#define SERVER_H



int passivesock(const char *service, const char *transport, int qlen);
void daemonize_process(void);
void run_server_loop(int server_fd, const char *www_root);


#endif