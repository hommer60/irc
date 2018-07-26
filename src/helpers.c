#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helpers.h"

char* strtok_r(
    char *str, 
    const char *delim, 
    char **nextp)
{
    char *ret;
    if (str == NULL)
    {
        str = *nextp;
    }

    str += strspn(str, delim);
    if (*str == '\0')
    {
        return NULL;
    }

    ret = str;
    str += strcspn(str, delim);

    if (*str)
    {
        *str++ = '\0';
    }
    *nextp = str;
    return ret;
}