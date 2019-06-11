#ifndef DATA_FIFO_H
#define DATA_FIFO_H

#include <iostream>
#include <tuple>
#include <array>
#include <vector>

#ifndef CONFIG_H
#include "config.h"
#endif
#ifndef TCP_CLIENT_H
#include "tcp_client.h"
#endif

using namespace std;

class data_fifo{
private:
    static const int SOUND_FRAME = SOUND_RX_PACK_LEN;
    static const int ELECTRICAL_FRAME = ELECTRICAL_RX_PACK_LEN;
    static const int E_LENGTH = ELECTRICAL_FIFO_LENGTH + 1;
    static const int S_LENGTH = SOUND_FIFO_LENGTH + 1;
    int e_front;
    int e_rear;
    int s_front;
    int s_rear;
public:
    double e_sig[E_LENGTH];
    double s_sig[3][S_LENGTH];
    
    data_fifo();
    void fifo_clear();
    bool fifo_e_enqueue(double data);
    bool fifo_s_enqueue(array<double, 3> data);
    tuple<bool, double> fifo_e_dequeue();
    tuple<bool, array<double, 3>> fifo_s_dequeue();
    bool fifo_isempty(char mark); 
    bool fifo_isfull(char mark);
    int  fifo_get_length(char mark);
    inline int fifo_get_rest(char mark){
        int len = fifo_get_length(mark);
        return (mark == 's')? SOUND_FIFO_LENGTH - len: ELECTRICAL_FIFO_LENGTH - len;
    }
    bool fifo_insert_pack(tcp_client::DATA &data);
    tuple<bool, array<vector<double>, 3>> fifo_get_s_data();
    tuple<bool, array<vector<double>, 1>> fifo_get_e_data();
    tuple<bool, array<vector<double>, 3>> fifo_dequeue_s_frame();
    tuple<bool, array<vector<double>, 1>> fifo_dequeue_e_frame();
};

#endif
