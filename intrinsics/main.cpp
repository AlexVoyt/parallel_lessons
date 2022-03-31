#include <stdio.h>
#include <assert.h>
#include <emmintrin.h>
#include <pmmintrin.h>

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

void AddMatrix(const double* A, const double* B, std::size_t C, std::size_t R, double* Result)
{
    assert(C % 8 == 0);
    assert(R % 8 == 0);
}

int main()
{
    double Res = Integrate(1, 5, 100000);
    return 0;
}
