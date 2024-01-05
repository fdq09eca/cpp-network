//
//  main.cpp
//  cpp-network-programming
//
//  Created by ChrisLam on 26/12/2023.
//

//#include "MySocket.hpp"
#include "MyServer.hpp"

void testTCP(bool isServer) {
    if (isServer) {
        // server
        MySocket listenSock;
        listenSock.createTCP();
        SocketAddress sa;
        sa.setIPv4(0,0,0,0);
        sa.setPort(8000);
        listenSock.bind(sa);
        listenSock.listen(64);
        
        MySocket client;
        listenSock.accept(client);
        
        std::vector<uint8_t> buff;
        
        for(;;){
            size_t n = client.nByteToRead();
            if (n == 0) {
                my_sleep(1);
                puts("sleep..\n");
                continue;
            }
            puts("recv...\n");
            auto r = client.recv(buff, n);
            buff.push_back(0);
            for (size_t i = 0; i < buff.size() - 1; i++){
                printf("buff[%zu]: %u\n", i, buff[i]);
            }
            printf("[recv] %s (%lu/%lu)\n", buff.data(), r, n);
        }
    } else {
        // client
        MySocket s;
        s.createTCP();
        SocketAddress sa;
        sa.setIPv4(127,0,0,1); // localhost
        sa.setPort(5000);
        s.connect(sa);
        s.send("1234", 4);
        printf("send ok\n");
    }
}

void testUDP(bool isServer){
    if (isServer) {
        // server
        MySocket s;
        s.createUDP();
        SocketAddress sa;
        sa.setIPv4(0,0,0,0);
        sa.setPort(5000);
        s.bind(sa);
        std::vector<uint8_t> buff;
        
        for(;;){
            size_t n = s.nByteToRead();
            if (n == 0) {
                my_sleep(1);
                puts("sleep..\n");
                continue;
            }
            puts("recv...\n");
            auto r = s.recv(buff, n);
            buff.push_back(0);
            
            printf("[recv (%lu/%lu)] %s \n", r, n, buff.data());
        }
    } else {
        // client
        MySocket s;
        s.createUDP();
        SocketAddress sa;
        sa.setIPv4(127,0,0,1); // localhost
        sa.setPort(5000);
        s.sendto(sa, "1234", 4);
        printf("send ok\n");
    }
}

void test_lookup(){
    MySocket s;
    s.createTCP();
    s.connectIPv4("www.gov.hk", 80);
    s.send_c_str("GET /index.html HTTP/1.1\r\n"
                        "Host: www.gov.hk\r\n"
                        "\r\n");
    
    std::vector<uint8_t> buff;
    
    for(;;) {
        size_t n = s.nByteToRead();
        if (n == 0) {
            my_sleep(0);
            continue;
        }
        puts("recv...\n");
        auto r = s.recv(buff, n);
        buff.push_back(0);
        printf("[recv (%lu/%lu)] %s", r, n, buff.data());
    }
    
}

void test_server(const char* send_c_str_msg = nullptr, const char* expect_result = nullptr, size_t n_iter = 3){
    puts("test_server_connection");
    MySocket s;
    s.createTCP();
    s.connectIPv4("localhost", 2525);
    std::vector<uint8_t> recvBuff;
    
    if (send_c_str_msg) {
        s.send_c_str(send_c_str_msg);
    }
    
    // recv loop
    for(int i = 0; i < n_iter; i++) {
        size_t n = s.nByteToRead();
        if (n == 0) {
            my_sleep(1);
            printf(".");
            continue;
        }
//        puts("recv...\n");
        auto r = s.recv(recvBuff, n);
        recvBuff.push_back(0);
        printf("[recv (%lu/%lu)] %s", r, n, recvBuff.data());
        break;
    }
    
    if (expect_result) {
        std::string actual_result;
        char* p = (char*) recvBuff.data();
        actual_result.assign(p, p + recvBuff.size());
        if (strcmp(actual_result.data(), expect_result) == 0) {
            printf("[OK] %s == %s\n", actual_result.data(), expect_result);
        } else {
            printf("[FAIL] %s != %s\n", actual_result.data(), expect_result);
        }
    }
    
}

int main(int argc, const char * argv[]) {
    if (argc > 1 && 0 == strcmp(argv[1], "--server")) {
            MyServer server;
            server.run(2525);
            
    } else {

//         test_server(nullptr, "220 smtp.example.com ESMTP Postfix\r\n"); // OK
         
//        test_server("token1 123\r\n", nullptr); // OK
//        test_server("    token1 123\r\n", nullptr); // OK
//        test_server("token1 123\r\ntoken2 312\r\n", nullptr); // OK
//        test_server("    token1 123\r\n   token2 312\r\n", nullptr);
        
        
        
//        printf("Client!\n");
    }
    

    
    
    
    std::cout << "\n== program ends here ==\n";
    return 0;
}
