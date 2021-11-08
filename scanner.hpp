#ifndef SCANNER_HPP_SENTRY
#define SCANNER_HPP_SENTRY

enum token_type {
        identifier,
        keyword,
        constant,
        string_literal,
        operation,
        punctuator,
        bad_token
};

struct LexItem {
        const char *token;
        enum token_type type;
        unsigned int line;
        LexItem *next;
};

class Scanner {
        enum state {
                home,
                ident,
                keywd,
                count,
                creal,
                quote,
                equal,
                comnt,
                error
        };
        enum state flag;
        bool delay;
        bool separate;
        int current_line;
        int allocated;
        int buf_used;
        char *buffer;
        LexItem *token_list;
        LexItem *last_ptr;
        static const char *const keywords[];
public:
        Scanner();
        ~Scanner();
        void Feed(char c);
        void Report() const;
        bool Success() const { return flag == home; }
        LexItem *LastToken() const { return last_ptr; }
        LexItem *GetTokenList() const { return token_list; }
private:
        void Step(char c);
        void HandleHome(char c);
        void HandleIdent(char c);
        void HandleKeywd(char c);
        void HandleCount(char c);
        void HandleCreal(char c);
        void HandleQuote(char c);
        void HandleEqual(char c);
        void HandleComnt(char c);
        void ControlDelimiter(char c);
        void Append(char c);
        void AddLexeme(enum token_type type);
        void AllocateBuffer();
        void DisposeBuffer();
        char *GetString() const;
        static bool IsOperation(char c);
        static bool IsPunctuator(char c);
        static bool IsDelimiter(char c);
        static bool IsKeyword(const char *token);
        static bool IsFunction(const char *token);
};

#endif

