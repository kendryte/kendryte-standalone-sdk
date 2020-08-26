#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "fft_soft.h"

#define PI 3.14159265358979323846
complex add(complex a, complex b)
{
    complex ret = {a.real + b.real, a.imag + b.imag};
    return ret;
}

complex sub(complex a, complex b)
{
    complex ret = {a.real - b.real, a.imag - b.imag};
    return ret;
}

complex mul(complex a, complex b)
{
    complex ret = {a.real * b.real - a.imag * b.imag, a.real * b.imag + a.imag * b.real};
    return ret;
}

void bitrev(complex *data, int n)
{
    int j = 0;
    int m = 0;
    for (int i = 0; i < n; i++)
    {
        if (j > i)
            SWAP(data[i], data[j]);
        m = n / 2;
        while (j >= m && m != 0)
        {
            j -= m;
            m >>= 1;
        }
        j += m;
    }
}

void fft_soft(complex *data, int n)
{
    int M = 0;
    for (int i = n; i > 1; i = i >> 1, M++);

    bitrev(data, n);

    for (int m = 0; m < M; m++)
    {
        int K = n >> (m + 1);
        for (int k = 0; k < K; k++)
        {
            int J = 2 << m;
            int base = k * J;
            for (int j = 0; j < J / 2; j++)
            {
                int t = base + j;
                complex w = {cos(-2 * PI * j * K / n), sin(-2 * PI * j * K / n)};
                complex wn = mul(data[t + J / 2], w);
                complex temp = data[t];
                data[t] = add(data[t], wn);
                data[t + J / 2] = sub(temp, wn); 
            }
        }
    }
}

void ifft_soft(complex *data, int n)
{
    int M = 0;
    for (int i = n; i > 1; i = i >> 1, M++);

    bitrev(data, n);

    for (int m = 0; m < M; m++)
    {
        int K = n >> (m + 1);
        for (int k = 0; k < K; k++)
        {
            int J = 2 << m;
            int base = k * J;
            for (int j = 0; j < J / 2; j++)
            {
                int t = base + j;
                complex w = {cos(2 * PI * j * K / n), sin(2 * PI * j * K / n)};
                complex wn = mul(data[t + J / 2], w);
                complex temp = data[t];
                data[t] = add(data[t], wn);
                data[t + J / 2] = sub(temp, wn); 
            }
        }
    }

    for (int i = 0; i < n; i++)
    {
        data[i].real /= n;
        data[i].imag /= n;
    }
}
