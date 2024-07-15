/**
 * @file udp.h
 *
 * Definition of udp client and server.
 *
 * @author dashen
 */

#ifndef UDP_H_
#define UDP_H_

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

class UdpSocket {
private:
    int _sockfd;
    struct sockaddr_in _server_addr, _client_addr;

public:
    UdpSocket(int port) ;

    ~UdpSocket() {
        close(_sockfd);
    }

    bool send(void *message , size_t msg_len, const char *client_ip, int port) ;

    bool receive(void *message , size_t msg_len) ;
};


#endif
