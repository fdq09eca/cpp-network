//
//  MyClient.hpp
//  cpp-network-programming
//
//  Created by ChrisLam on 05/01/2024.
//

#ifndef MyClient_hpp
#define MyClient_hpp

#include <stdio.h>
#include "MySocket.hpp"

enum class State {
    UNKNOWN = 0,
    CONNECT,
    EHLO,
    MAIL,
    RCPT,
    DATA,
};


struct MyMail {
    std::string from;
    std::vector<std::string> to;
    std::string data;
    
    void reset(){
        from.clear();
        to.clear();
        data.clear();
    }
};

class MyClient : public NonCopyable {
    friend class MyServer;
private:
    MySocket _sock;
    std::string recvBuff;
    std::string lineBuff;
    std::string tokenBuff;
    State state = State::UNKNOWN;
    MyMail _mail;
    
    
    
public:
    MyClient() = default;
    
    MyClient(SOCKET raw_sock) : _sock(raw_sock) { }
    
    MyClient(MyClient&& rhs){
        *this = std::move(rhs);
    }
    
    void operator=(MyClient&& rhs) {
        _sock = std::move(rhs._sock);
    }
    
    bool operator==( const MyClient& rhs) {
        return _sock == rhs._sock;
    }
    
    SOCKET sock() { return _sock.sock(); }
    
    void send(std::string msg) {
        _sock.send_c_str(msg.c_str());
    }
    
    
    void setSock(MySocket&& sock) {
        _sock = std::move(sock);
    }
    
    void onConnect(){
        //send smtp greeting
        printf("[client] %p connected.\n", this);
        send("220 smtp.example.com ESMTP Postfix\r\n");
    }
    
    void close() {
        _sock.close();
    }
    
    bool isValid(){
        return _sock.sock() != INVALID_SOCKET;
    }
    
    
    
    void onRecvCommand(){
        
        if (state == State::DATA) {
            // TODO: handle data
            return;
        }
        
        printf("[onRecvCommand (len: %zu)] %s\n", lineBuff.size(), lineBuff.data());
        getToken();
        
        if (tokenBuff == "DATA") {
            if (state != State::RCPT)       { sendBadSeqCmd(); return; }
            send("354 \r\n");
            state = State::DATA;
            return;
        }
        else if (tokenBuff == "EHLO" || tokenBuff == "HELO") {
            if (state != State::CONNECT)    { sendBadSeqCmd(); return; }
            state = State::EHLO;
            sendOK();
        }
        else if (tokenBuff == "MAIL") {
            if (state != State::EHLO)    { sendBadSeqCmd(); return; }
            state = State::MAIL;
            
            const char* e = lineBuff.data() + lineBuff.size();
            const char* s = lineBuff.data() + tokenBuff.size() + 1;
            std::string buff;
            s = MyUtil::getToken(buff, s, ':');
            
            if (buff != "FROM") {
                sendSyntaxError();
                return;
            }
            
            _mail.from.assign(s + 1, e);
            sendOK();
        }
        
        else if (tokenBuff == "RCPT") {
            if (state != State::MAIL || state != State::RCPT)    { sendBadSeqCmd(); return; }
            
            state = State::RCPT;
            
            const char* e = lineBuff.data() + lineBuff.size();
            const char* s = lineBuff.data() + tokenBuff.size() + 1;
            
            std::string buff;
            s = MyUtil::getToken(buff, s, ':');

            if (buff != "TO") {
                sendSyntaxError();
                return;
            }

            auto& b = _mail.to.emplace_back(s + 1, e);
            printf("[mail.to] %s", b.c_str());
            sendOK();
        }
        
        else if (tokenBuff == "RSET") {
            // https://datatracker.ietf.org/doc/html/rfc5321#autoid-55
            _mail.reset();
            state = State::EHLO;
            sendOK();
            return;
        }
        
        else if (tokenBuff == "NOOP") {
            // https://datatracker.ietf.org/doc/html/rfc5321#autoid-59
            sendOK();
            return;
        }
        
        else if (tokenBuff == "HELP") {
            // https://datatracker.ietf.org/doc/html/rfc5321#autoid-58
            send("214 see https://datatracker.ietf.org/doc/html/rfc5321\r\n");
            return;
        }
        
        else if (tokenBuff == "VRFY") {
            // mailbox: https://datatracker.ietf.org/doc/html/rfc5321#autoid-22
            // vrfy: https://datatracker.ietf.org/doc/html/rfc5321#autoid-56
            // vrfy normal response: https://datatracker.ietf.org/doc/html/rfc5321#autoid-31
            send("502  Command not implemented\r\n");
            return;
        }
        
        else if (tokenBuff == "EXPN") {
            // mailbox: https://datatracker.ietf.org/doc/html/rfc5321#autoid-22
            // expn: https://datatracker.ietf.org/doc/html/rfc5321#autoid-57
            send("502  Command not implemented\r\n");
            return;
        }
        
        else if (tokenBuff == "QUIT") {
            // https://datatracker.ietf.org/doc/html/rfc5321#autoid-60
            send("221 Bye\r\n");
            close();
        } else {
            send("500 Syntax error command unrecognized\r\n");
        }
    }
    
    const char* lineEndChar(){
        if (state == State::DATA) {
            return "\r\n.\r\n";
        } else {
            return "\r\n";
        }
    }
    
    void sendOK(){
        send("200 OK\r\n");
    }
    
    void sendBadSeqCmd(){
        send("503 Bad sequence of commands\r\n");
    }
    
    void sendSyntaxError(){
        send("501 Syntax error in parameters or arguments\r\n");
    }
    
    void getToken(char sep = ' '){
        tokenBuff.clear();
        char* s = lineBuff.data();
        
        // trim;
        while (*s) {
            if (isspace(*s)) {
                s++;
            } else {
                break;
            }
        }
        
        
        char* e = s;
        while (*e) {
            if (*e == sep)
                break;
            
            if (isalnum(*e)) {
                e++;
                continue;
            }
        }
        MY_ASSERT(e >= s);
        tokenBuff.assign(s, e);
        printf("[getToken] %s\n", tokenBuff.data());
    }
    
    bool getLine(size_t lineStartPos, const char* lineEndChar = "\r\n"){
        size_t lineEndPos = recvBuff.find(lineEndChar, lineStartPos);
        
        if (lineEndPos == std::string::npos) {
            return false;
        } else {
            MY_ASSERT(lineEndPos >= lineStartPos);
            lineBuff.clear();
            
            char* s = recvBuff.data() + lineStartPos;
            char* e = recvBuff.data() + lineEndPos;
            
            lineBuff.assign(s, e);
            
            return true;
        }
    }
    
    void onRecv() {
        
        size_t n = _sock.nByteToRead();
        if (n == 0) {
            _sock.close();
            return;
        }
        
        recvBuff.clear();
        size_t lineStartPos = 0;
        
        
        size_t oldSize = recvBuff.size();
        size_t newSize = oldSize + n;
        
        recvBuff.resize(newSize);
        
        
        ssize_t r = _sock.recv((uint8_t*) recvBuff.data() + oldSize, n);
        
        recvBuff.resize(r);
        
        while (true) {
            bool result = getLine(lineStartPos, lineEndChar());
            
            if (!result)
                return;
            
            onRecvCommand();
            lineStartPos += lineBuff.size() + 2;
        }
        
        printf("[client %p] onRecv (%zu): %s\n",
               this, r, recvBuff.c_str());
    }
};


#endif /* MyClient_hpp */
