//
//  SocketAddress.hpp
//  cpp-network-programming
//
//  Created by ChrisLam on 29/12/2023.
//

#ifndef SocketAddress_hpp
#define SocketAddress_hpp

#include <stdio.h>
#include "MyUtil.hpp"
#include <netdb.h>

class SocketAddress {
private:
    void myGetHostByNameIPv4(std::vector<SocketAddress>& out_results, const char * hostname) ;

public:
    sockaddr addr;

    SocketAddress();
    
    SocketAddress(sockaddr addr_);
    
    SocketAddress(const char* hostname, uint16_t port);
    
    void setIPv4(const char* hostname);
    
    void setPort(uint16_t port);
    
    uint16_t getPort();
    
    void setIPv4(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
    
    void print();
    
};
#endif /* SocketAddress_hpp */
