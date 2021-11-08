#include <cstdio>
#include "error.hpp"
#include "scanner.hpp"

void Error::Report() const
{
        fprintf(stderr, "%s: %s\n", object, message);
}

void SyntaxError::Report() const
{
        if (lex) {
                fprintf(stderr, "token: %s\n", lex->token);
                fprintf(stderr, "line: %i\n", lex->line);
        }
        Error::Report();
}

