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
    READY,
    EHLO,
    MAIL,
    RCPT,
    DATA,
};


struct MyMail {
    std::string from;
    std::vector<std::string> to;
    std::string data;
    
    void clear(){
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
    size_t recvOffSet = 0;


public:
    MyClient() = default;
    
    MyClient(SOCKET raw_sock) : _sock(raw_sock) { }
    
    SOCKET sock() { return _sock.sock(); }
    
    void send(std::string msg) { _sock.send_c_str(msg.c_str()); }
    
    void setSock(MySocket&& sock) { _sock = std::move(sock); }
    
    void onConnect(){
        //send smtp greeting
        printf("[client] %p connected.\n", this);
        send("220 smtp.example.com ESMTP Postfix\r\n");
        state = State::READY;
    }
    
    void close() { _sock.close(); }
    bool isValid(){ return _sock.sock() != INVALID_SOCKET; }
    
    
    
    void onRecvCommand(){
        printf("[onRecvCommand (len: %zu)] %s\n", lineBuff.size(), lineBuff.data());
        getToken();
        
        if (tokenBuff == "DATA") {
            if (state != State::RCPT)       { sendBadSeqCmd(); return; }
            send("354 End data with <CR><LF>.<CR><LF>\r\n");
            state = State::DATA;
            return;
        }
        
        else if (tokenBuff == "EHLO" || tokenBuff == "HELO") {
            if (state != State::READY)    { sendBadSeqCmd(); return; }
            state = State::EHLO;
            sendOK();
        }
        
        else if (tokenBuff == "MAIL") {
            if (state != State::EHLO)    {
                if (state != State::MAIL) {
                    sendBadSeqCmd();
                    return;
                }
            }
            
            state = State::MAIL;
            _mail.clear();
            
            const char* e = lineBuff.data() + lineBuff.size();
            const char* s = lineBuff.data() + tokenBuff.size();
            std::string buff;
            s = MyUtil::getToken(buff, s, ':');
            
            if (!s || buff != "FROM") {
                sendSyntaxError();
                return;
            }
            
            _mail.from.assign(s + 1, e);
            printf("[mail.from] %s\n", _mail.from.c_str());
            sendOK();
        }
        
        else if (tokenBuff == "RCPT") {
            if (state != State::RCPT)    {
                if (state == State::MAIL) {
                    state = State::RCPT;
                } else {
                    sendBadSeqCmd();
                    return;
                }
            }
            
            const char* e = lineBuff.data() + lineBuff.size();
            const char* s = lineBuff.data() + tokenBuff.size();
            
            std::string buff;
            s = MyUtil::getToken(buff, s, ':');

            if (!s || buff != "TO") {
                sendSyntaxError();
                return;
            }

            auto& b = _mail.to.emplace_back(s + 1, e);
            printf("[mail.to] %s\n", b.c_str());
            sendOK();
        }
        
        else if (tokenBuff == "RSET") {
            // https://datatracker.ietf.org/doc/html/rfc5321#autoid-55
            _mail.clear();
            state = State::MAIL;
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

    void sendOK()           { send("250 OK\r\n"); }
    void sendBadSeqCmd()    { send("503 Bad sequence of commands\r\n" ); }
    void sendSyntaxError()  { send("501 Syntax error in parameters or arguments\r\n"); }
    
    
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
        
        char* c = s;
        while (*c) {
            if (*c == sep)
                break;
            
            if (isalnum(*c)) {
                c++;
                continue;
            }
        }
        
        MY_ASSERT(c >= s);
        tokenBuff.assign(s, c);
        printf("[getToken] %s\n", tokenBuff.data());
    }
    
    bool getLine(size_t offset, const char* lineEndChar = "\r\n"){
        size_t lineEndPos = recvBuff.find(lineEndChar, offset);
        
        if (lineEndPos == std::string::npos) {
            return false;
        } else {
            MY_ASSERT(lineEndPos >= offset);
            lineBuff.clear();
            
            char* s = recvBuff.data() + offset;
            char* e = recvBuff.data() + lineEndPos;
            
            lineBuff.assign(s, e);
            printf("[getLine] %s\n", lineBuff.c_str());
            return true;
        }
    }
    
    void onRecvData(){
        MY_ASSERT(state == State::DATA);
        size_t endPos = recvBuff.find("\r\n.\r\n", recvOffSet);
        if (endPos == std::string::npos)
            return;
        
        _mail.data.assign(recvBuff.data() + recvOffSet, recvBuff.data() + endPos);
        recvOffSet += _mail.data.size() + 5;
        sendOK();
        state = State::READY;
    }
    
    void onRecv() {
        
        size_t n = _sock.nByteToRead();
        if (n == 0) {
            _sock.close();
            return;
        }
        
        size_t oldSize = recvBuff.size();
        size_t newSize = oldSize + n;
        recvBuff.resize(newSize);
        
        auto* recvPtr = recvBuff.data() + oldSize;
        ssize_t r = _sock.recv((uint8_t*) recvPtr, n);
        
        std::cout << "[recvBuff] " << recvBuff; //TODO: start using libfmt
        
        if (state == State::DATA) {
            onRecvData();
            return;
        }
        
        while (true) {
            
            bool result = getLine(recvOffSet, "\r\n");
            
            if (!result) return;
            
            recvOffSet += lineBuff.size() + 2;
            onRecvCommand();
        }
        
        printf("[client %p] onRecv (%zu): %s\n", this, r, recvBuff.c_str());
    }
};


#endif /* MyClient_hpp */
