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

void test_InitRecvLoop(MySocket& s, std::vector<uint8_t>& recvBuff, size_t sec = 3) {
    recvBuff.clear();
    for(int i = 0; i < sec; i++) {
        size_t n = s.nByteToRead();
        if (n == 0) {
            my_sleep(1);
            continue;
        }
        auto r = s.recv(recvBuff, n);
        MY_ASSERT(r >= 0);
        recvBuff.push_back(0);
        break;
    }
}

void test_result(const char* actual_result, const char* expect_result, const char* testname){
        if (strcmp(actual_result, expect_result) == 0) {
            printf("[OK] %s\n", testname);
        } else {
            printf("[FAIL] %s\nactual_result: [%s]\n expect_result: [%s]", testname, actual_result, expect_result);
        }
}


void test_SMTP_normal(){
    // arrange
    MySocket s;
    s.createTCP();
    s.connectIPv4("localhost", 2525);
    std::vector<uint8_t> recvBuff;
    
    test_InitRecvLoop(s, recvBuff);
    test_result((char*) recvBuff.data(), "220 smtp.example.com ESMTP Postfix\r\n", "test_SMTP::WELCOME_MSG");
    
    // EHLO
    s.send_c_str("EHLO \r\n");
    test_InitRecvLoop(s, recvBuff);
    test_result((char*) recvBuff.data(), "250 OK\r\n", "test_SMTP::EHLO");
    
    // MAIL
    s.send_c_str("MAIL FROM:<bob@example.org>\r\n");
    test_InitRecvLoop(s, recvBuff);
    test_result((char*) recvBuff.data(), "250 OK\r\n", "test_SMTP::MAIL");
    
    // RCPT
    s.send_c_str("RCPT TO:<alice@example.com>\r\n");
    test_InitRecvLoop(s, recvBuff);
    test_result((char*) recvBuff.data(), "250 OK\r\n", "test_SMTP::RCPT");
    
    // RCPT
    s.send_c_str("RCPT TO:<theboss@example.com>\r\n");
    test_InitRecvLoop(s, recvBuff);
    test_result((char*) recvBuff.data(), "250 OK\r\n", "test_SMTP::RCPT");
    
    // DATA
    s.send_c_str("DATA \r\n");
    test_InitRecvLoop(s, recvBuff);
    test_result((char*) recvBuff.data(), "354 End data with <CR><LF>.<CR><LF>\r\n", "test_SMTP::DATA");
    
    
    // DATA-CONTENT
    s.send_c_str("From: \"Bob Example\" <bob@example.org>\r\n");
    s.send_c_str("To: \"Alice Example\" <alice@example.com>\r\n");
    s.send_c_str("Cc: theboss@example.com\r\n");
    s.send_c_str("Date: Tue, 15 Jan 2008 16:02:43 -0500\r\n");
    s.send_c_str("Subject: Test message\r\n");
    s.send_c_str("\r\n");
    s.send_c_str("Hello Alice.\r\n");
    s.send_c_str("This is a test message with 5 header fields and 4 lines in the message body.\r\n");
    s.send_c_str("Your friend,\r\n");
    s.send_c_str("Bob\r\n");
    s.send_c_str(".\r\n");
    
    test_InitRecvLoop(s, recvBuff);
    test_result((char*) recvBuff.data(), "250 OK\r\n", "test_SMTP::DATA-CONTENT");
    
    s.send_c_str("QUIT \r\n");
    test_InitRecvLoop(s, recvBuff);
    test_result((char*) recvBuff.data(), "221 Bye\r\n", "test_SMTP::QUIT");
}



int main(int argc, const char * argv[]) {
    if (argc > 1 && 0 == strcmp(argv[1], "--server")) {
            MyServer server;
            server.run(2525);
            
    } else {
        test_SMTP_normal();
    }
    

    
    
    
    std::cout << "\n== program ends here ==\n";
    return 0;
}
