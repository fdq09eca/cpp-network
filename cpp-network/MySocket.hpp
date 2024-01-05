//
//  MySocket.hpp
//  cpp-network-programming
//
//  Created by ChrisLam on 26/12/2023.
//

#ifndef MySocket_hpp
#define MySocket_hpp
#include <sys/socket.h>
#include <sys/ioctl.h>

#include "MyUtil.hpp"
#include "SocketAddress.hpp"

#if _WIN32
    
#else
    typedef int SOCKET;
    const SOCKET INVALID_SOCKET = -1;
#endif

class MySocket : public NonCopyable {
    
private:
    SOCKET _sock = INVALID_SOCKET;

public:
    MySocket() = default;
    MySocket(SOCKET s): _sock(s) { }
    
    MySocket(MySocket&& r) {
        *this = std::move(r);
    }
    
    void operator=(MySocket&& r) {
        
        MY_ASSERT(r._sock != _sock);
        
        close();
        _sock = r._sock;
        r._sock = INVALID_SOCKET;
    }
    
    bool operator==(const MySocket& rhs) {
        return rhs._sock == _sock;
    }
    
    ~MySocket() { close(); }
    SOCKET sock() { return _sock; }
//    operator SOCKET()  { return sock(); } // implicit coversion
    
    void createUDP();
    void createTCP();
    
    void bind(SocketAddress& sa);
    void sendto(SocketAddress& sa, const char*, size_t n);
    void sendto_c_str(SocketAddress& sa, const char* msg);
    
    void send(const char*, size_t n);
    void send_c_str(const char* msg);
    
    
    ssize_t recv(uint8_t* out_recvBuff, size_t n) {
        if (!out_recvBuff) {
            throw MyError("out_recvBuff is null");
        }
        
        ssize_t r = ::recv(_sock, out_recvBuff, n, 0);
        
        if (r < 0) {
            throw MyError("recv");
        }
        
        return r;
    }
    
    ssize_t recvfrom(SocketAddress& out_sa, std::vector<uint8_t>& out_recvBuff, size_t n);
    ssize_t recv(std::vector<uint8_t>& out_recvBuff, size_t n);
    
    

    
    void connect(SocketAddress& sa);
    void connectIPv4(const char * hostname, uint16_t port);
    
    void accept(MySocket& client);
    void listen(int backlog);
    
    size_t nByteToRead();
    
    void close();
    
    

};

#endif /* MySocket_hpp */
