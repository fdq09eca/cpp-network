//
//  MySocket.cpp
//  cpp-network-programming
//
//  Created by ChrisLam on 26/12/2023.
//

#include "MySocket.hpp"

void MySocket::createUDP(){
    close();
    _sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (_sock == INVALID_SOCKET) {
        throw MyError("createUDP");
    }
}


void MySocket::createTCP(){
    close();
    _sock = socket(AF_INET, SOCK_STREAM, 0);
    if (_sock == INVALID_SOCKET) {
        throw MyError("createTCP");
    }
}

size_t MySocket::nByteToRead(){
        int n = 0;
        int r = ::ioctl(_sock, FIONREAD, &n);
        if (r < 0) {
            throw MyError("nBtyeToRead");
        }
        return static_cast<size_t>(n);
}

void MySocket::sendto(SocketAddress& sa, const char* msg, size_t dataSize){
    if (dataSize > INT_MAX) {
        throw MyError("data size too big to send.\n");
    }
    ssize_t r = ::sendto(_sock, msg, dataSize, 0, &sa.addr, sizeof(sa.addr));
    if (r < 0) {
        throw MyError("send");
    }
}

void MySocket::send(const char * msg, size_t dataSize) {
    if (dataSize > INT_MAX) {
        throw MyError("data size too big to send.\n");
    }
    ssize_t r = ::send(_sock, msg, dataSize, 0);
    if (r < 0) {
        throw MyError("send");
    }
}

void MySocket::send_c_str(const char * c_str){
    return send(c_str, strlen(c_str));
}

void MySocket::sendto_c_str(SocketAddress &sa, const char * c_str){
    return sendto(sa, c_str, strlen(c_str));
}

ssize_t MySocket::recvfrom(SocketAddress& out_sa, std::vector<uint8_t>& recvBuff, size_t byteToRecv) {
    
    socklen_t addr_len = sizeof(out_sa.addr);
    recvBuff.clear();
    recvBuff.resize(byteToRecv);
    auto r = ::recvfrom(_sock, recvBuff.data(), byteToRecv, 0, &out_sa.addr, &addr_len);
    recvBuff.resize(r);
    
    if (r < 0) {
        throw MyError("recvfrom");
    }
    
    return r;
}


ssize_t MySocket::recv(std::vector<uint8_t> &recvBuff, size_t byteToRecv) {
    recvBuff.clear();
    recvBuff.resize(byteToRecv);
    
    auto r = recv(recvBuff.data(), byteToRecv);
    recvBuff.resize(r);
    
    if (r < 0) {
        throw MyError("recv");
    }
    
    return r;
}

void MySocket::bind(SocketAddress &sa){
    int r = ::bind(_sock, &sa.addr, sizeof(sa.addr));
    if (0 != r) {
        throw MyError("bind");
    } else {
        printf("bind to: "); sa.print(); printf("\n");
    }
}

void MySocket::listen(int backlog)  {
    int r = ::listen(_sock, backlog);
    if (r < 0) {
        throw MyError("listen");
    }
    printf("listen...\n");
};

void MySocket::connect(SocketAddress &sa){
    
    int r = ::connect(_sock, &sa.addr, sizeof(sa.addr));
    
    if (r < 0) {
        throw MyError("connect");
    }
    printf("socket [%p] connected to ", this); sa.print(); printf("\n");
    
}

void MySocket::connectIPv4(const char *hostname, uint16_t port) {
    
        SocketAddress sa;
        sa.setIPv4(hostname);
        sa.setPort(port);
        connect(sa);
    
}

void MySocket::accept(MySocket& out_client){
    int c = ::accept(_sock, nullptr, nullptr);
    if (c < 0) {
        throw MyError("accept");
    }
    out_client._sock = c;
    printf("accepted.\n");
}


void MySocket::close(){
    if (_sock != INVALID_SOCKET) {
        ::close(_sock);
        _sock = INVALID_SOCKET;
    }
}
