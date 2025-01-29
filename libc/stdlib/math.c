#include <stdint.h> 
#include <math.h>



#ifndef NAN
#define NAN (0.0 / 0.0) // Define NAN if it's not already defined
#endif


int isinf(double x) {
    union {
        double d;
        uint64_t u;
    } u;
    u.d = x;
    uint64_t exponent = (u.u >> 52) & 0x7FF;
    uint64_t mantissa = u.u & 0x000FFFFFFFFFFFFFULL;
    return (exponent == 0x7FF && mantissa == 0);
}

// Trigonometric functions
double cos(double x) {
    // Using Taylor series approximation for cosine
    double result = 1.0;
    double term = 1.0;
    for (int i = 1; i <= 10; i++) {
        term *= -x * x / ((2 * i) * (2 * i - 1));
        result += term;
    }
    return result;
}

double sin(double x) {
    // Using Taylor series approximation for sine
    double result = x;
    double term = x;
    for (int i = 1; i <= 10; i++) {
        term *= -x * x / ((2 * i) * (2 * i + 1));
        result += term;
    }
    return result;
}

double tan(double x) {
    return sin(x) / cos(x);
}

// Decompose x into fractional and integer parts
double modf(double x, double* intpart) {
    *intpart = (long long)x;
    return x - *intpart;
}

// Logarithmic functions
double log(double x) {
    if (x <= 0) return NAN; // Logarithm of non-positive number is undefined
    // Using Newton's method for natural logarithm
    double result = 0.0;
    while (x >= 2.0) {
        x /= 2.0;
        result += 0.69314718056; // ln(2)
    }
    while (x < 1.0) {
        x *= 2.0;
        result -= 0.69314718056; // ln(2)
    }
    x -= 1.0;
    double term = x;
    result += term;
    for (int i = 2; i <= 10; i++) {
        term *= -x;
        result += term / i;
    }
    return result;
}

double log10(double x) {
    return log(x) / 2.30258509299; // ln(10)
}

// Power function
double pow(double base, double exponent) {
    if (exponent == 0) return 1.0;
    if (base == 0) return 0.0;
    double result = 1.0;
    for (int i = 0; i < (int)exponent; i++) {
        result *= base;
    }
    return result;
}

// Square root function
double sqrt(double x) {
    if (x < 0) return NAN; // Square root of negative number is undefined
    double guess = x / 2.0;
    for (int i = 0; i < 10; i++) {
        guess = 0.5 * (guess + x / guess);
    }
    return guess;
}

// Rounding functions
double ceil(double x) {
    if (x == (long long)x) return x;
    return x > 0 ? (long long)x + 1 : (long long)x;
}

double floor(double x) {
    if (x == (long long)x) return x;
    return x > 0 ? (long long)x : (long long)x - 1;
}

double fmod(double numer, double denom) {
    if (denom == 0) return NAN; // Division by zero
    return numer - (long long)(numer / denom) * denom;
}

double trunc(double x) {
    return (long long)x;
}

float truncf(float x) {
    return (long long)x;
}

long double truncl(long double x) {
    return (long long)x;
}

double round(double x) {
    return x >= 0 ? (long long)(x + 0.5) : (long long)(x - 0.5);
}

float roundf(float x) {
    return x >= 0 ? (long long)(x + 0.5f) : (long long)(x - 0.5f);
}

long double roundl(long double x) {
    return x >= 0 ? (long long)(x + 0.5L) : (long long)(x - 0.5L);
}

// Minimum, Maximum, and difference functions
double fmin(double x, double y) {
    return x < y ? x : y;
}

float fminf(float x, float y) {
    return x < y ? x : y;
}

long double fminl(long double x, long double y) {
    return x < y ? x : y;
}

double fmax(double x, double y) {
    return x > y ? x : y;
}

float fmaxf(float x, float y) {
    return x > y ? x : y;
}

long double fmaxl(long double x, long double y) {
    return x > y ? x : y;
}

// Absolute value functions
double fabs(double x) {
    return x < 0 ? -x : x;
}


int abs(int x) {
    return x < 0 ? -x : x;
}

float absf(float x) {
    return x < 0 ? -x : x;
}

long double absld(long double x) {
    return x < 0 ? -x : x;
}