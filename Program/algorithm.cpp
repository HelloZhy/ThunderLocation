#include "algorithm.h"

algorithm::algorithm(){
    fstream file;
    file.open(HANN_DAT_PATH, ios::binary|ios::in);
    double* pwn = wn;
    for(int i = 0; i < WN_LENGTH; ++i){
        file.read((char*)pwn, sizeof(double));
        pwn++;
    }
    file.close();
}

void algorithm::discreteFourierFast(
    const complex<double>* f, 
    int i_max, complex<double>* F, 
    fourier_transform_direction ftd){
	if (i_max <= 0 || ((i_max & (i_max - 1)) != 0)) throw INCORRECT_SPECTRUM_SIZE_FOR_FFT;

	double norm, exp_dir;
	switch (ftd){
	case ftdFunctionToSpectrum:
		norm = 1;
		exp_dir = -1;
		break;
	case ftdSpectrumToFunction:
		norm = 1.0 / i_max;
		exp_dir = 1;
		break;
	default:
		throw UNSUPPORTED_FTD;
	}

	int NN = i_max, digitsCount = 0;
	while (NN >>= 1) digitsCount++;

	// Allocating 2 buffers with n complex values
	complex<double>** buf = new complex<double>* [2];
	for (int i = 0; i < 2; i++){
		buf[i] = new complex<double>[i_max];
	}

	// Grouping function values according to the binary-reversed index order
	int cur_buf = 0;
	for (int i = 0; i < i_max; i++){
		buf[cur_buf][i] = f[reverseBits(digitsCount, i)];
	}

	int exp_divider = 1;
	int different_exps = 2;
	int values_in_row = i_max / 2;
	int next_buf = 1;

	for (int step = 0; step < digitsCount; step++){
		for (int n = 0; n < different_exps; n++){
			complex<double> xp = exp((double)(exp_dir * PI * n / exp_divider) * I);

			for (int k = 0; k < values_in_row; k++){
				complex<double>* pf = &buf[cur_buf][2 * k + (n % (different_exps / 2)) * (values_in_row * 2)];
				buf[next_buf][n * values_in_row + k] = (*pf) + (*(pf + 1)) * xp;
			}
		}

		exp_divider *= 2;
		different_exps *= 2;
		values_in_row /= 2;
		cur_buf = next_buf;
		next_buf = (cur_buf + 1) % 2;
	}

	// Norming, saving the result
	for (int i = 0; i < i_max; i++){
		F[i] = norm * buf[cur_buf][i];
	}

	// Freeing our temporary buffers
	for (int i = 0; i < 2; i++){
		delete [] buf[i];
	}
	delete [] buf;
}

tuple<int, double> algorithm::gccphat(vector<double>& xref, vector<double>& x){
    const int length = xref.size();
    const int N = pow(2, (int)log2(length));

    complex<double>* _xref = new complex<double>[N];
    complex<double>* _x = new complex<double>[N];
    for(int i = 0; i < N; ++i){
        _xref[i].real(xref[i]);
        _xref[i].imag(0);
        _x[i].real(x[i]);
        _x[i].imag(0);
    }

    complex<double>* XREF = new complex<double>[N];
    complex<double>* X = new complex<double>[N];
    complex<double>* XCORR = new complex<double>[N];
    discreteFourierFast(_xref, N, XREF, ftdFunctionToSpectrum);
    discreteFourierFast(_x, N, X, ftdFunctionToSpectrum);

    for(int i = 0; i < N; ++i)
        XCORR[i] = (conj(XREF[i]) * X[i]) / abs(conj(XREF[i]) * X[i]);
    
    complex<double>* xcorr = new complex<double>[N];
    discreteFourierFast(XCORR, N, xcorr, ftdSpectrumToFunction);

    double* _xcorr = new double[N];
    for(int i = 0; i < N; ++i)
        _xcorr[i] = abs(xcorr[i]);

    int argmax = 0;
    double valmax = _xcorr[0];
    for(int i = 1; i < N; ++i){
        if(_xcorr[i] > valmax){
            valmax = _xcorr[i];
            argmax = i;
        }
    }

    int tau_n;
    if(argmax >= N / 2){
        tau_n = -(int)(length * (1 - (float)argmax / N));
    }else{
        tau_n = (int)(length * (float)argmax / N);
    }
    tuple<int, double> res(tau_n, valmax);
    

    delete [] _xref;
    delete [] _x;
    delete [] XREF;
    delete [] X;
    delete [] XCORR;
    delete [] xcorr;
    delete [] _xcorr;

    return res;
}

bool algorithm::filter_energy(vector<double>& x){
    int length = x.size();
    double sum = 0;
    for(int i = 0; i < length; ++i)
        sum += abs(x[i]);
    sum /= length; 
	//cout << "sum" << sum << endl;
    return (sum > Energy_Threshold)? true: false;
}

tuple<double, double> algorithm::get_location(
    double t0,
    double tx,
    double ty,
    double tz
){
    double x = ((double)1 / (2 * D)) * (pow(D, 2) + (tz - tx) * (tz + tx - 2 * t0) * pow(V, 2));
    double y = ((double)1 / (2 * D)) * (pow(D, 2) + (tz - ty) * (tz + ty - 2 * t0) * pow(V, 2));
    cout << "x: " << x << "\ty: " << y << endl;
    double rho = pow(pow(x, 2) + pow(y, 2), 0.5);
    double _theta = atan(abs(y) / abs(x)) * 360 / (2 * PI);
    double theta;
    if(x >= 0 && y >= 0)
        theta = _theta;
    else if(x < 0 && y >= 0)
        theta = 180 - _theta;
    else if(x < 0 && y < 0)
        theta = 180 + _theta;
    else
        theta = 360 - _theta;

    tuple<double, double> res(rho, theta);
    return res;
}

tuple<bool, array<double, 3>> algorithm::s_sig_identifier(
    array<vector<double>, 3>& sig,
    const int fs,
    int ctr
){
    int length = sig[0].size();
    double buf_tau[3] = {0};
    int buf_i[3] = {0};
    double buf_ref = 0;
    double buf_val[3] = {0};
    bool buf_mark[3] = {false};
    int frame_length = (int)S_Frame_Time * fs;
    int inc_val = frame_length / 10;
    vector<double> xref;
    vector<double> x;
    for(int i_ref = 0; i_ref < length - frame_length; i_ref += inc_val){
        //cout << "i_ref: " << (ctr - 1) * 2 * S_FS + i_ref << endl;
        xref.clear();
        for(int k = i_ref; k < i_ref + frame_length - 1; ++k)
            xref.push_back(sig[0][k]);
        //xref .* wn
        for(int k = 0; k < frame_length; ++k)
            xref[k] = (xref[k] - 128) * wn[k];
        if(filter_energy(xref)){
            for(int i_ch = 1; i_ch < 3; ++i_ch){
                for(int i = 0; i < length - frame_length; i += inc_val){
                    //cout << "\tch: " << (ctr - 1) * 2 * S_FS + i_ch << "\ti: " << i << endl;
                    x.clear();
                    for(int k = i; k < i + frame_length - 1; ++k)
                        x.push_back(sig[i_ch][k]);
                    //x .* wn
                    for(int k = 0; k < frame_length; ++k)
                        x[k] = (x[k] - 128) * wn[k];
                    if(filter_energy(x)){
                        tuple<int, double> res = gccphat(xref, x);
                        int tau = get<0>(res);
                        double val = get<1>(res);
                        if(val > S_Threshold && val > buf_val[i_ch]){
                            cout << "\t[ch]: " << i_ch << "\tnew gcc val: " << val << endl;
                            buf_val[i_ch] = val;
                            buf_i[i_ch] = i;
                            buf_tau[i_ch] = tau;
                            buf_mark[i_ch] = true;
                        }
                    }
                }
            }
            cout << "[0]: " << buf_i[0];
            cout << "\t[1]: " << buf_i[1];
            cout << "\t[2]: " << buf_i[2] << endl;
            cout << "[0]: " << buf_tau[0];
            cout << "\t[1]: " << buf_tau[1];
            cout << "\t[2]: " << buf_tau[2] << endl;
            if(buf_mark[1] && buf_mark[2]){
                double avr_val = (buf_val[1] + buf_val[2]) / 2;
                if(avr_val > buf_ref){
                    buf_ref = avr_val;
                    buf_i[0] = i_ref;
                    buf_mark[0] = true;
                }
            }
        }
    }
    if(buf_mark[0]){
        double tau1 = ((ctr - 1) * S_FRAME_TIME * S_FS + (double)buf_i[0]) / fs;
        double tau2 = ((ctr - 1) * S_FRAME_TIME * S_FS + (double)buf_i[1] + buf_tau[1]) / fs;
        double tau3 = ((ctr - 1) * S_FRAME_TIME * S_FS + (double)buf_i[2] + buf_tau[2]) / fs;
        array<double, 3> _res_tau({tau1, tau2, tau3});
        tuple<bool, array<double, 3>> res(true, _res_tau);
        return res;
    }else{
        array<double, 3> _res_tau({0, 0, 0});
        tuple<bool, array<double, 3>> res(false, _res_tau);
        return res;
    }
}

tuple<bool, double> algorithm::e_sig_identifier(
    vector<double>& sig,
    const int fs
){
    int length = sig.size();
    int frame_length = E_Frame_Time * fs;
    int dx_inc = frame_length / 10;

    vector<double> x;
    for(int i = 0; i < length - frame_length; i += dx_inc){
        x.clear();
        for(int j = 0; j < frame_length; ++j)
            x.push_back(sig[i + j]);

        double valmax = x[0]; 
        int top_point = 0;
        for(int j = 1; j < frame_length; ++j){
            if(x[j] > valmax){
                valmax = x[j];
                top_point = j;
            }
        }

        int start_point = -1;
        for(int j = 0; j < frame_length; ++j){
            if(x[j] > E_Threshold){
                start_point = j;
                break;
            }
        }

        int end_point = top_point;
        for(int j = top_point; j < frame_length; ++j){
            if(x[j] > E_Threshold)
                end_point = j;
        }
        if(
            valmax > E_Threshold &&
            start_point != -1 &&
            end_point != top_point &&
            top_point - start_point > 0 &&
            top_point - start_point < 18 &&
            end_point - top_point > 10
            ){
            double tau = (double)(i + start_point) / fs;
            //if is a valid signal, returned here
            return tuple<bool, double>(true, tau);
        }
    }
    //if not a valid signal, returned here
    return tuple<bool, double>(false, 0.0);
}
