#include <math.h>
#include "pav_analysis.h"

float compute_power(const float *x, unsigned int N) {
    float signal = 0;
    for (unsigned int n = 0; n < N; n++) {
        signal += x[n] * x[n];
    }
    return 10*log10(signal / N);

    float num = 0.0;
    float den = 0.0;
    
    for (unsigned int n = 0; n < N; n++) {
        float w = 0.54 - 0.46 * cos(2.0 * 3.1415 * n / (N - 1));
        float xw = x[n] * w;
        num += xw * xw;
        den += w * w;
    }
    return 10.0 * log10(num / den);
}

float compute_am(const float *x, unsigned int N) {
    float signal = 0;
    for (unsigned int n = 0; n < N; n++) {
        signal += fabs(x[n]);
    }
    return signal / N;
}

float compute_zcr(const float *x, unsigned int N, float fm) {
    float zcr = 0;
    for (unsigned int n = 1; n < N; n++) {
        if ((x[n] >= 0 && x[n - 1] < 0) || (x[n] < 0 && x[n - 1] >= 0)) {
            zcr++;
        }
    }
    return (fm * zcr) / (2 * (N - 1));
}
