#include <cstdio>
#include <cstring>
#include "scanner.hpp"

const char *const Scanner::keywords[] = {
        "program", "begin", "end",    "endl",
        "equ",     "and",   "or",     "xor", 
        "not",     "if",    "else",   "elseif",
        "goto",    "while", "repeat", "until",
        "alloc",   "free",  "print",  "scan",
        "inc",     "dec",   "true",   "false",
        "bool",    "int",   "double", "string"
};

Scanner::Scanner()
{
        flag = home;
        delay = false;
        separate = false;
        current_line = 1;
        allocated = 0;
        buf_used = 0;
        buffer = 0;
        token_list = 0;
        last_ptr = 0;
}

Scanner::~Scanner()
{
        LexItem *tmp;
        while (token_list) {
                tmp = token_list;
                token_list = token_list->next;
                delete[] tmp->token;
                delete tmp;
        }
        DisposeBuffer();

}

void Scanner::Feed(char c)
{
        Step(c);
        if (delay) {
                delay = false;
                Step(c);
        }
        if (c == '\n')
                current_line++;
}

void Scanner::Report() const
{
        if (flag == home)
                return;
        if (flag == error) {
                 fprintf(stderr, "token: %s\n", last_ptr->token);
                 fprintf(stderr, "line: %i\n", last_ptr->line);
                 fprintf(stderr, "error: unrecognized token\n");
        } else {
                 fprintf(stderr, "error: bad final state\n");
        }
}

void Scanner::Step(char c)
{
        ControlDelimiter(c);
        switch (flag) {
        case home:
                HandleHome(c);
                break;
        case ident:
                HandleIdent(c);
                break;
        case keywd:
                HandleKeywd(c);
                break;
        case count:
                HandleCount(c);
                break;
        case creal:
                HandleCreal(c);
                break;
        case quote:
                HandleQuote(c);
                break;
        case equal:
                HandleEqual(c);
                break;
        case comnt:
                HandleComnt(c);
                break;
        case error:
                ;
        }
}

void Scanner::HandleHome(char c)
{
        if (IsDelimiter(c))
                return;
        if (c == '#') {
                flag = comnt;
        } else if (c == '"') {
                flag = quote;
        } else {
                Append(c);
                if (IsOperation(c)) {
                        AddLexeme(operation);
                } else if (IsPunctuator(c)) {
                        AddLexeme(punctuator);
                } else if (c == '$' || c == '?' || c == '@') {
                        flag = ident;
                } else if (c >= 'a' && c <= 'z') {
                        flag = keywd;
                } else if (c >= '0' && c <= '9') {
                        flag = count;
                } else if (c == '=' || c == '>' || c == '<' || c == '!') {
                        flag = equal;
                } else {
                        flag = error;
                        AddLexeme(bad_token);
                }
        }
}

void Scanner::HandleIdent(char c)
{
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')||
            (c >= '0' && c <= '9') || (c == '_')) {
                Append(c);
        } else {
                flag = home;
                delay = true;
                separate = true;
                AddLexeme(identifier);
        }
}

void Scanner::HandleKeywd(char c)
{
        if (c >= 'a' && c <= 'z') {
                Append(c);
        } else {
                if (IsKeyword(buffer)) {
                        flag = home;
                        delay = true;
                        separate = true;
                        AddLexeme(keyword);
                } else {
                        flag = error;
                        AddLexeme(bad_token);
                }
        }
}

void Scanner::HandleCount(char c)
{
        if (c >= '0' && c <= '9'){
                Append(c);
        } else if (c == '.') {
                flag = creal;
                Append(c);
        } else {
                flag = home;
                delay = true;
                separate = true;
                AddLexeme(constant);
        }
}

void Scanner::HandleCreal(char c)
{
        if (c >= '0' && c <= '9'){
                Append(c);
        } else {
                flag = home;
                delay = true;
                separate = true;
                AddLexeme(constant);
        }
}

void Scanner::HandleQuote(char c)
{
        if (c != '"') {
                Append(c);
        } else {
                flag = home;
                separate = true;
                AddLexeme(string_literal);
        }
}

void Scanner::HandleEqual(char c)
{
        flag = home;
        if (c == '=') {
                Append(c);
                AddLexeme(operation);
        } else {
                delay = true;
                AddLexeme(operation);
        }
}

void Scanner::HandleComnt(char c)
{
        if (c == '\n')
                flag = home;
}

void Scanner::ControlDelimiter(char c)
{
        if (!separate)
                return;
        separate = false;
        if (c == '>' || c == '<' || c == '!' || c == '=' ||
            IsOperation(c) || IsPunctuator(c) || IsDelimiter(c))
                return;
        flag = error;
        Append(c);
        AddLexeme(bad_token);
}

void Scanner::Append(char c)
{
        if (buf_used == allocated)
                AllocateBuffer();
        buffer[buf_used] = c;
        buf_used++;
        buffer[buf_used] = 0;
}

void Scanner::AddLexeme(enum token_type type)
{
        if (buf_used == 0)
                return;
        if (last_ptr) {
                last_ptr->next = new LexItem;
                last_ptr = last_ptr->next;
        } else {
                token_list = new LexItem;
                last_ptr = token_list;
        }
        last_ptr->next = 0;
        last_ptr->token = GetString();
        last_ptr->type = type;
        last_ptr->line = current_line;
        DisposeBuffer();
}

void Scanner::AllocateBuffer()
{
        int old_size = allocated;
        allocated += 16;
        char *tmp = new char[allocated + 1];
        for (int i = 0; i < old_size; i++)
                tmp[i] = buffer[i];
        if (buffer)
                delete[] buffer;
        buffer = tmp;
}

void Scanner::DisposeBuffer()
{
        allocated = 0;
        buf_used = 0;
        if (buffer) {
                delete[] buffer;
                buffer = 0;
        }
}

char *Scanner::GetString() const
{
        char *str = new char[buf_used + 1];
        for (int i = 0; i < buf_used; i++)
                str[i] = buffer[i];
        str[buf_used] = 0;
        return str;
}

bool Scanner::IsOperation(char c)
{
        return strchr("+-*/%~^|&()[]", c);
}

bool Scanner::IsPunctuator(char c)
{
        return strchr("{};:,", c);
}

bool Scanner::IsDelimiter(char c)
{
        return strchr(" \t\n", c);
}

bool Scanner::IsKeyword(const char *token)
{
        unsigned int i;
        for (i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
                if (!strcmp(token, keywords[i]))
                        return true;
        }
        return false;
}

