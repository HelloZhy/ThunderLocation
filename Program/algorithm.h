#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <vector>
#include <array>
#include <tuple>
#include <complex>
#include <cmath>
#include <fstream>
#include <iostream>

#ifndef CONFIG_H
#include "config.h"
#endif

using namespace std;

class algorithm{
private:
    const double E_Threshold = E_THRESHOLD;
    const double S_Threshold = S_THRESHOLD;
    const double Energy_Threshold = ENERGY_THRESHOLD;
    const float S_Frame_Time = S_FRAME_TIME;
    const float E_Frame_Time = E_FRAME_TIME;
    const int INCORRECT_SPECTRUM_SIZE_FOR_FFT = 1;
    const int UNSUPPORTED_FTD = 2;
    enum fourier_transform_direction { ftdFunctionToSpectrum, ftdSpectrumToFunction };
    inline int reverseBits(unsigned short digitsCount, int value){
        if (value >> digitsCount > 0) return -1;
        int res = 0;
        for (int d = 0; d < digitsCount; d++){
            res = (res * 2 + (value % 2));
            value /= 2;
        }
        return res;
    }
    const complex<double> I{0, 1};
    const double PI = 3.14159265358979323846;
    double wn[WN_LENGTH];
public:
    algorithm();
    void discreteFourierFast(
        const complex<double>* f, 
        int i_max, complex<double>* F, 
        fourier_transform_direction ftd);

    tuple<int, double> gccphat(vector<double>& xref, vector<double>& x);

    bool filter_energy(vector<double>& x);

    tuple<double, double> get_location(
        double t0,
        double tx,
        double ty,
        double tz
    );

    tuple<bool, array<double, 3>> s_sig_identifier(
        array<vector<double>, 3>& sig,
        const int fs,
        int ctr
    );

    tuple<bool, double> e_sig_identifier(
        vector<double>& sig,
        const int fs
    );
};

#endif
