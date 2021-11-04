#include <cstring>
#include "common.hpp"

char *dupstr(const char *s)
{
        int len = strlen(s);
        char *copy = new char[len + 1];
        for (int i = 0; i < len; i++)
                copy[i] = s[i];
        copy[len] = 0;
        return copy;
}

char *concatenate(const char *s1, const char *s2)
{
        int len1 = strlen(s1);
        int len2 = strlen(s2);
        char *str = new char[len1 + len2 + 1];
        for (int i = 0; i < len1; i++)
                str[i] = s1[i];
        for (int i = 0; i < len2; i++)
                str[len1 + i] = s2[i];
        str[len1 + len2] = 0;
        return str;
}

