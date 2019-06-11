#include "tcp_client.h"

tcp_client::tcp_client(){
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(client_fd < 0){
        printf("create a socket [error: %d]\n", client_fd);
        //will be closed in ~client_fd()
        return;
    }else
        printf("create a socket [ok]\n");
    
    bzero((void*)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);
    bzero((void*)&(server_addr.sin_zero), sizeof(server_addr.sin_zero));

    if(connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("connect to server [error]\n");
        //will be closed in ~client_fd()
        return;
    }else
        printf("connect to server [ok]\n");
}

void tcp_client::get_data(tcp_client::DATA& data){
    if(send(client_fd, REQUEST, sizeof(REQUEST), 0) < 0){
        data.isvalid = false;
        printf("send REQUEST [error]\n");
        return;
    }
    ssize_t len = 0, _len = 0;
    int counter = 0;
    uint8_t* p = (uint8_t*)&data;
    long int rest = 2048;
    while(len < sizeof(data)){
        _len = recv(client_fd, p + len, rest, 0);
        if(_len > 0){
            len += _len;
            counter++;
        }else if(_len < 0){
            std::cout << "recv error: " << _len << std::endl;
        }else break;
        //std::cout << '[' << counter << "] recv length: " << _len << " rest:" << (long int)(sizeof(data) - len) << std::endl;
        if((long int)(sizeof(data) - len) < 2048 && (long int)(sizeof(data) - len) > 0)
            rest = (long int)(sizeof(data) - len);
    }
    std::cout << "load complete" << std::endl;
}
