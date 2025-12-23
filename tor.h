#ifndef TOR_H
#define TOR_H

int tor_connect(void);
int tor_handshake(int sock);
int tor_request(int sock, char *address, int port);

#endif