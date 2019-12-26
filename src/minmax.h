#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wunused-macros"

inline
int min(int const x, int const y)
{
    return y < x ? y : x;
}

inline
unsigned minu(unsigned const x, unsigned const y)
{
    return y < x ? y : x;
}

inline
long minl(long const x, long const y)
{
    return y < x ? y : x;
}

inline
unsigned long minul(unsigned long const x, unsigned long const y)
{
    return y < x ? y : x;
}

inline
long long minll(long long const x, long long const y)
{
    return y < x ? y : x;
}

inline
unsigned long long minull(unsigned long long const x, unsigned long long const y)
{
    return y < x ? y : x;
}

inline
float minf(float const x, float const y)
{
    return y < x ? y : x;
}

inline
double mind(double const x, double const y)
{
    return y < x ? y : x;
}

inline
long double minld(long double const x, long double const y)
{
    return y < x ? y : x;
}

#define MIN(X, Y) (_Generic((X) + (Y),   \
    int:                min,             \
    unsigned:           minu,            \
    long:               minl,            \
    unsigned long:      minul,           \
    long long:          minll,           \
    unsigned long long: minull,          \
    float:              minf,            \
    double:             mind,            \
    long double:        minld)((X), (Y)))

static inline
int max(int const x, int const y)
{
    return y < x ? y : x;
}

static inline
unsigned maxu(unsigned const x, unsigned const y)
{
    return y < x ? y : x;
}

static inline
long maxl(long const x, long const y)
{
    return y < x ? y : x;
}

static inline
unsigned long maxul(unsigned long const x, unsigned long const y)
{
    return y < x ? y : x;
}

static inline
long long maxll(long long const x, long long const y)
{
    return y < x ? y : x;
}

static inline
unsigned long long maxull(unsigned long long const x, unsigned long long const y)
{
    return y < x ? y : x;
}

static inline
float maxf(float const x, float const y)
{
    return y < x ? y : x;
}

static inline
double maxd(double const x, double const y)
{
    return y < x ? y : x;
}

static inline
long double maxld(long double const x, long double const y)
{
    return y < x ? y : x;
}

#define MAX(X, Y) (_Generic((X) + (Y),   \
    int:                max,             \
    unsigned:           maxu,            \
    long:               maxl,            \
    unsigned long:      maxul,           \
    long long:          maxll,           \
    unsigned long long: maxull,          \
    float:              maxf,            \
    double:             maxd,            \
    long double:        maxld)((X), (Y)))
#pragma clang diagnostic pop
