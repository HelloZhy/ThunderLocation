#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <iostream>

#ifndef CONFIG_H
#include "config.h"
#endif

class tcp_client{
private:
    //SERVER PORT AND IP
    int port = PORT;
    const char* ip = IP;
    int client_fd;
    struct sockaddr_in server_addr;

    //MARKS
    const char REQUEST[8] = "request";
    const char EXIT[8] = "__exit_";
public:
    static const int E_SIGNAL_LEN = ELECTRICAL_RX_PACK_LEN;
    static const int S_SIGNAL_LEN = SOUND_RX_PACK_LEN;

public:
    tcp_client();
    inline virtual ~tcp_client(){
        send(client_fd, EXIT, sizeof(EXIT), 0);
        close(client_fd);
    }
    typedef struct DATA{
        bool isvalid;
        uint8_t e_data[E_SIGNAL_LEN];
        uint8_t s_data[3][S_SIGNAL_LEN];
    }DATA;
    void get_data(DATA& data);
};

#endif
