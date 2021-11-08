#ifndef ERROR_HPP_SENTRY
#define ERROR_HPP_SENTRY

#include "common.hpp"

class Error {
        const char *object;
        const char *message;
public:
        Error(const char *obj, const char *msg)
                { object = dupstr(obj); message = dupstr(msg); }
        Error(const Error& err)
                { object = dupstr(err.object); message = dupstr(err.message); }
        ~Error() { delete[] object; delete[] message; }
        void Report() const;
};

class SyntaxError : public Error {
        class LexItem *lex;
public:
        SyntaxError(const char *msg, LexItem *ptr) : Error("error", msg)
                { lex = ptr; }
        ~SyntaxError() {}
        void Report() const;
        LexItem *Token() const { return lex; }
};

class RuntimeError : public Error {
public:
        RuntimeError(const char *obj, const char *msg) : Error(obj, msg) {}
        ~RuntimeError() {}
};

#endif

