//
//  MyServer.hpp
//  cpp-network-programming
//
//  Created by ChrisLam on 29/12/2023.
//

#ifndef MyServer_hpp
#define MyServer_hpp

#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include "MyClient.hpp"

class MyServer : NonCopyable {
private:
    
    
public:
    std::vector<MyClient> clients;
    
    void run(uint16_t port){
        
        SocketAddress sa;
        
        sa.setIPv4(0, 0, 0, 0);
        sa.setPort(port);
        
        MySocket listenSock;
        listenSock.createTCP();
        listenSock.bind(sa);
        listenSock.listen(64);
        
        
        while (true) {
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(listenSock.sock(), &readfds);
            
            int max_fd = listenSock.sock();
            timeval tv;
            tv.tv_usec = 0;
            tv.tv_sec = 1;
            
            for (size_t i = 0; i < clients.size();) {
                auto& client = clients[i];
                if (client.isValid()) {
                    // add client to fd_reads
                    SOCKET c = client.sock();
                    FD_SET(c, &readfds);
                    if (c > max_fd) {
                        max_fd = c;
                    }
                    i++;
                } else {
                    
                    // remove invalid client
                    if (client == clients.back()) {
                        clients.resize(clients.size() - 1);
                        printf("[client %p] removed\n", &client);
                        continue;
                    }
                        
                    if (clients.size() > 1) {
                        std::swap(client, clients.back());
                        
                    }
                    clients.resize(clients.size() - 1);
                    printf("[client %p] removed, current clients size: %zu\n",
                           &client, clients.size());
                }
            }
            
            int ret = ::select(max_fd + 1, &readfds, nullptr, nullptr, &tv);
            
            if (ret <  0) { throw MyError("select"); }
            if (ret == 0) { continue; }
            
            
            // check incoming connection
            if (0 != FD_ISSET(listenSock.sock(), &readfds)) {
                MySocket s;
                listenSock.accept(s);
                auto& c = clients.emplace_back();
                c.setSock(std::move(s));
                c.onConnect();
            }
            
            // check every client for incoming msg
            for (auto& client : clients) {
                if (0 != FD_ISSET(client.sock(), &readfds)) {
                    client.onRecv();
                }
            }
        }
        printf("server stop running\n");
    } // run
    
};

#endif /* MyServer_hpp */
