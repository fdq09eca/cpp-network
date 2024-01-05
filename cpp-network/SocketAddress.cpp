//
//  SocketAddress.cpp
//  cpp-network-programming
//
//  Created by ChrisLam on 29/12/2023.
//

#include "SocketAddress.hpp"

void SocketAddress::myGetHostByNameIPv4(std::vector<SocketAddress> &out_results, const char *hostname){
    addrinfo hint;
    memset(&hint, 0, sizeof(hint));
    out_results.clear();
    
    hint.ai_family = AF_INET;
    
    addrinfo* res = nullptr;
    int r = ::getaddrinfo(hostname, nullptr, &hint, &res);
    
    if (r < 0) {
        MyError("myGetHostByNameIPv4");
    }
    
    
    for (addrinfo* r = res; r != nullptr ; r = r->ai_next) {
        out_results.emplace_back(*r->ai_addr);
    }
    
    freeaddrinfo(res);
}

SocketAddress::SocketAddress() { memset(&addr, 0, sizeof(addr)); }

SocketAddress::SocketAddress(sockaddr addr_ ): addr(addr_) { }

SocketAddress::SocketAddress(const char* hostname, uint16_t port) { setIPv4(hostname); setPort(port); }

void SocketAddress::setIPv4(const char *hostname){
    
        std::vector<SocketAddress> results;
        myGetHostByNameIPv4(results, hostname);
        if (results.size() == 0) {
            throw MyError("setIPv4 (by hostname)");
        }
        *this = results[0];
    
}

void SocketAddress::setPort(uint16_t port){
    uint16_t* p = reinterpret_cast<uint16_t*>(addr.sa_data); // network port in byte
    *p = htons(port); // from host byte order to network byte order
}

uint16_t SocketAddress::getPort() {
    uint16_t* p = reinterpret_cast<uint16_t*>(addr.sa_data); // network port in byte
    return ntohs(*p); // from network byte order to host byte order
}

void SocketAddress::setIPv4(uint8_t a, uint8_t b, uint8_t c, uint8_t d){
    addr.sa_family = AF_INET;
    addr.sa_data[2] = a;
    addr.sa_data[3] = b;
    addr.sa_data[4] = c;
    addr.sa_data[5] = d;
};

void SocketAddress::print(){
    if (addr.sa_family == AF_INET) {
        auto* p = reinterpret_cast<const uint8_t*>(addr.sa_data);
        printf("IPv4 %u.%u.%u.%u : %u\n", p[2], p[3], p[4], p[5], getPort());
    }
}

