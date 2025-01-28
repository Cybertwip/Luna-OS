#ifndef __MATH_H__
#define __MATH_H__

extern double cos(double x);
extern double sin(double x);
extern double tan(double x);


extern double modf(double x, double* intpart);
extern double log(double x);
extern double log10(double x);

extern double pow(double base, double exponent);
extern double sqrt(double x);

extern double ceil(double x);
extern double floor(double x);
extern double fmod(double numer, double denom);

extern double trunc(double x);
extern float truncf(float x);
extern long double truncl(long double x);

extern double round(double x);
extern float roundf(float x);
extern long double roundl(long double x);

extern double fmin(double x, double y);
extern float fminf(float x, float y);
extern long double fminl(long double x, long double y);

extern double fmax(double x, double y);
extern float fmaxf(float x, float y);
extern long double fmaxl(long double x, long double y);

extern double fabs(double x);

extern double abs(double x);
extern float absf(float x);
extern long double absld(long double x);

#define M_PI 3.14159265358979323846264338327950288419


#endif // __MATH_H__
