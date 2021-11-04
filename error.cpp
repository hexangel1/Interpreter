#include <cstdio>
#include "error.hpp"
#include "scanner.hpp"

void Error::Report() const
{
        fprintf(stderr, "%s\n", message);
}

void SyntaxError::Report() const
{
        fprintf(stderr, "token: %s\n", lex->token);
        fprintf(stderr, "line: %i\n", lex->line);
        fprintf(stderr, "error: ");
        Error::Report();
}

void RuntimeError::Report() const
{
        fprintf(stderr, "%s: ", func);
        Error::Report();
}

