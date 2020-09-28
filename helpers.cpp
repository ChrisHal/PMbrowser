// c(c) 2020 C. R. Halaszovich

#include<cinttypes>
#include"helpers.h"

// some tools

double swap_bytes(double x)
{
    char* a = reinterpret_cast<char*>(&x);
    uint64_t b;
    char* p = reinterpret_cast<char*>(&b);
    p[0] = a[7];
    p[1] = a[6];
    p[2] = a[5];
    p[3] = a[4];
    p[4] = a[3];
    p[5] = a[2];
    p[6] = a[1];
    p[7] = a[0];
    return *reinterpret_cast<double*>(p);
}

int32_t swap_bytes(int32_t x)
{
    char* a = reinterpret_cast<char*>(&x);
    int32_t b;
    char* p = reinterpret_cast<char*>(&b);
    p[0] = a[3];
    p[1] = a[2];
    p[2] = a[1];
    p[3] = a[0];
    return b;
}

int16_t swap_bytes(int16_t x)
{
    return ((x & 0xff00) >> 8)|((x & 0xff) << 8);
}

uint16_t swap_bytes(uint16_t x)
{
    return ((x & 0xff00) >> 8) | ((x & 0xff) << 8);
}
