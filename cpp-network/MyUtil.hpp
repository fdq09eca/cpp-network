//
//  MyUtil.hpp
//  cpp-network-programming
//
//  Created by ChrisLam on 26/12/2023.
//

#ifndef MyUtil_hpp
#define MyUtil_hpp

#include <stdio.h>
#include <iostream>
#include <vector>
#include <map>
#include <unistd.h>


#define MY_ASSERT(x) my_assert(x, #x, __LINE__, __FILE__)


inline void my_assert(bool x, const char* msg, int lineNumber, const char* filename) {
    assert(x);
    if (!x) printf("%s", msg);
}


class MyError : public std::exception {
public:
    MyError(const char* msg){
        printf("\n[ERR] %s: %s\n", msg, strerror(errno));
    }
};

class NonCopyable {
public:
    NonCopyable() = default;
    NonCopyable(NonCopyable&& rhs) = default;
    void operator=(NonCopyable&& rhs) {};
    
private:
    NonCopyable(const NonCopyable&) = delete;
    void operator=(const NonCopyable&) = delete;
};


class MyUtil {
public:
    inline static const char* trimFront(const char* c_str) {
        auto* p = c_str;
        while (isspace(*p)) {
            p++;
        }
        return p;
    }
    
    inline static const char* trimBack(const char* c_str) {
        auto e = c_str + strlen(c_str);
        while (e > c_str) {
            if (isspace(*e)) {
                e--;
                continue;
            }
            return e;
        }
        return c_str;
    }
    
    inline static const char* getToken(std::string& out_buff, const char* line, char sep = ' ') {
        out_buff.clear();
        
        auto* s = trimFront(line);
        auto* c = s;
        
        while (*c) {
            if (*c == sep) {
                out_buff.assign(s, c);
                return c; // return sep ptr;
            } else {
                c++;
            }
        }
        
        return nullptr; // no found sep
    }
    template<typename T>
    static void remove_unorder(std::vector<T>& vec, T& e){
        if (&e != &vec.back()) std::swap(e, vec.back());
        vec.resize(vec.size() - 1);
    }
    
};

inline void my_sleep(size_t sec){
#if _WIN32
    Sleep(sec * 1000);
#else
    ::sleep(static_cast<unsigned int>(sec));
#endif
    
}
#endif /* MyUtil_hpp */
