#pragma once
double swap_bytes(double x);
int32_t swap_bytes(int32_t x);
int16_t swap_bytes(int16_t x);
uint16_t swap_bytes(uint16_t x);

template<typename T> void swapInPlace(T& x)
{
    x = swap_bytes(x);
}

