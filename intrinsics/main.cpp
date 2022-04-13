#include <stdio.h>
#include <cstdlib>
#include <assert.h>
#include <emmintrin.h>
#include <pmmintrin.h>
#include <stdint.h>

typedef uint32_t u32;

double Integrate(double a, double b, unsigned n)
{
    double dx = (b-a)/n;
    double Result = 0;
    __m128d X = _mm_set_pd(a+dx, a);
    __m128d DX = _mm_set_pd1(2*dx);
    __m128d Sum = _mm_set_pd1(0);

    unsigned Stride = sizeof(__m128d)/sizeof(double);
    assert(n >= Stride);
    assert(n % Stride == 0);
    for(unsigned int i = 0; i < n; i += Stride)
    {
        Sum = _mm_add_pd(Sum, _mm_mul_pd(X,X));
        X = _mm_add_pd(X, DX);
    }
    Result = _mm_cvtsd_f64(_mm_hadd_pd(Sum, Sum));

    Result *= dx;
    return Result;
}

void AddMatrixSSE(const double* A, const double* B, std::size_t C, std::size_t R, double* Result)
{
    assert(C % 8 == 0);
    assert(R % 8 == 0);

    u32 ElementsInRegister = sizeof(__m128d)/sizeof(double);
    assert(ElementsInRegister == 2);
    u32 Iterations = C*R/ElementsInRegister;

    for(u32 I = 0; I < Iterations; I++)
    {
        __m128d ATerm = _mm_load_pd(A);
        __m128d BTerm = _mm_load_pd(B);
        __m128d Sum = _mm_add_pd(ATerm, BTerm);
        _mm_storeu_pd(Result, Sum);

        A += ElementsInRegister;
        B += ElementsInRegister;
        Result += ElementsInRegister;
    }

}

void Print(const double* A, u32 C, u32 R)
{
    for(u32 Y = 0; Y < R; Y++)
    {
        for(u32 X = 0; X < C; X++)
        {
            printf("%f ", A[Y * C + X]);
        }
        printf("\n");
    }
}

int main()
{
    double IntRes = Integrate(-1, 1, 1000000);
    printf("%f\n", IntRes);

    u32 C = 8;
    u32 R = 16;
    double* A = (double* )malloc(C * R * sizeof(double));
    for(u32 I = 0; I < C * R; I++)
    {
        A[I] = I + 20;
    }
    // Print(A, C, R);

    double* B = (double* )malloc(C * R * sizeof(double));
    for(u32 I = 0; I < C * R; I++)
    {
        B[I] = I + 30;
    }
    double* Res = (double* )malloc(C * R * sizeof(double));

    AddMatrixSSE(A, B, C, R, Res);
    Print(Res, C, R);
    return 0;
}
