#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "util.h"
#include "server.h"

static const int hex2bin_tbl[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

/* Does the reverse of bin2hex but does not allocate any ram */
bool hex2bin(unsigned char *p, const char *hexstr, size_t len)
{
    int nibble1, nibble2;
    unsigned char idx;
    bool ret = false;

    while (*hexstr && len) {
        if (!hexstr[1]) {
            log_msg(E_DEBUG, "hex2bin str truncated");
            return ret;
        }

        idx = *hexstr++;
        nibble1 = hex2bin_tbl[idx];
        idx = *hexstr++;
        nibble2 = hex2bin_tbl[idx];

        if (unlikely((nibble1 < 0) || (nibble2 < 0))) {
            log_msg(E_DEBUG, "hex2bin scan failed");
            return ret;
        }

        *p++ = (((unsigned char)nibble1) << 4) | ((unsigned char)nibble2);
        --len;
    }

    if (len == 0)
        ret = true;
    return ret;
}

/* Adequate size s==len*2 + 1 must be alloced to use this variant */
void __bin2hex(char *s, const unsigned char *p, size_t len)
{
    int i;
    static const char hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    for (i = 0; i < (int)len; i++) {
            *s++ = hex[p[i] >> 4];
            *s++ = hex[p[i] & 0xF];
        }
    *s++ = '\0';
}

/* Returns a malloced array string of a binary value of arbitrary length. The
 *  * array is rounded up to a 4 byte size to appease architectures that need
 *   * aligned array  sizes */
char *bin2hex(const unsigned char *p, size_t len)
{
    ssize_t slen;
    char *s;

    slen = len * 2 + 1;
    if (slen % 4)
        slen += 4 - (slen % 4);
    s = calloc(slen, 1);
    __bin2hex(s, p, len);

    return s;
}




















