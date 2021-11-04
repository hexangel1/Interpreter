#ifndef ERROR_HPP_SENTRY
#define ERROR_HPP_SENTRY

#include "common.hpp"

class LexItem;

class Error {
        const char *message;
public:
        Error(const char *msg) { message = dupstr(msg); }
        Error(const Error& err) { message = dupstr(err.message); }
        ~Error() { delete[] message; }
        void Report() const;
};

class SyntaxError : public Error {
        LexItem *lex;
public:
        SyntaxError(const char *msg, LexItem *q ) : Error(msg), lex(q) {}
        ~SyntaxError() {}
        void Report() const;
        LexItem *Get() const { return lex; }
};

class RuntimeError : public Error {
        const char *func;
public:
        RuntimeError(const char *msg, const char *f) : Error(msg)
                { func = dupstr(f); }
        ~RuntimeError() { delete[] func; }
        void Report() const;
};

#endif

