#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "tor.h"

// makes socket to tor socks5
int tor_connect(void) {
    int sock;
    struct sockaddr_in tor_addr;
    struct timeval tv;
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
            return -1;
    }
    
    // setup tor address
    tor_addr.sin_family = AF_INET;
    tor_addr.sin_port = htons(9050); // 9050 socks port is default
    tor_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    //btw if you have changed the default port consider putting it back to 9050 if you wanna run this.
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    
    
    if (connect(sock, (struct sockaddr*)&tor_addr, sizeof(tor_addr)) < 0) {
        close(sock); //connection
        return -1;
    }
    
    return sock;
}


int tor_handshake(int sock) {
    unsigned char hello[3];
    unsigned char reply[2];
    
    // socks5, 1 method, no auth
    hello[0] = 0x05;
    hello[1] = 0x01;
    hello[2] = 0x00;
    
    if (send(sock, hello, 3, 0) != 3) {
        return -1;
    }
    
    if (recv(sock, reply, 2, 0) != 2) {
        return -1;
    }
    
    // should be 05 00
    if (reply[0] != 0x05) {
        return -1;
    }
    if (reply[1] != 0x00) {
        return -1;
    }
    
    return 0;
}

// connection to onion
int tor_request(int sock, char *addr, int port) {
    char clean_addr[256];
    unsigned char req[263];
    unsigned char resp[10];
    int len, name_len, req_len;
    int n;
    const char *p = addr;
    
  // Skips http:// or https:// 
    if (strncmp(addr, "http://", 7) == 0) {
        p = addr + 7;
    } else if (strncmp(addr, "https://", 8) == 0) {
        p = addr + 8;
    }
    
    // copy to buffer
    strncpy(clean_addr, p, sizeof(clean_addr) - 1);
    clean_addr[sizeof(clean_addr) - 1] = 0;
    
        len = strlen(clean_addr);
    
    // for removing  www.  
    if (len >= 4 && strncmp(clean_addr, "www.", 4) == 0) {
        memmove(clean_addr, clean_addr + 4, len - 4 + 1);
        len -= 4;
    }
    
    // this is for removing trailing slash
    while (len > 0 && clean_addr[len - 1] == '/') {
        clean_addr[--len] = 0;
    }
    
    // checks if too short or too long
    if (len < 1 || len > 255) {
        return -1;
    }
    
    //socks5 request
    req[0] = 0x05; // socks5
    req[1] = 0x01; // connect
    req[2] = 0x00; // reserved
    req[3] = 0x03; // domain name
    
    name_len = len;
    req[4] = name_len; 
    
    // copies the onion address
    memcpy(req + 5, clean_addr, name_len);
    
    // port bytes
    req[5 + name_len] = (port >> 8) & 0xFF;
    req[6 + name_len] = port & 0xFF;
    
    req_len = 7 + name_len;
    
    // sends it
    if (send(sock, req, req_len, 0) != req_len) {
        return -1;
    }
    
    // get response
    n = recv(sock, resp, sizeof(resp), 0);
    if (n < 4) {
        return -1; // need at least 4 bytes
    }
    
    
    if (resp[0] != 0x05) {
        return -1;
    } //checks if okay
    if (resp[1] != 0x00) {
        return -1;
    }
    
    return 0;
}