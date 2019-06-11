#include "data_fifo.h"

data_fifo::data_fifo(){
    e_front = e_rear = 0;
    s_front = s_rear = 0;
}

void data_fifo::fifo_clear(){
    e_front = e_rear;
    s_front = s_rear;
}

bool data_fifo::fifo_e_enqueue(double data){
    if(fifo_isfull('e'))
        return false;
    
    e_sig[e_rear] = data;
    e_rear = (e_rear + 1) % E_LENGTH;
    return true; 
}

bool data_fifo::fifo_s_enqueue(array<double, 3> data){
    if(fifo_isfull('s'))
        return false;

    s_sig[0][s_rear] = data[0];
    s_sig[1][s_rear] = data[1];
    s_sig[2][s_rear] = data[2];
    s_rear = (s_rear + 1) % S_LENGTH;
    return true;
}

tuple<bool, double> data_fifo::fifo_e_dequeue(){
    if(fifo_isempty('e'))
        return {false, 0};
    
    double res = e_sig[e_front];
    e_front = (e_front + 1) % E_LENGTH;
    return {true, res};
}

tuple<bool, array<double, 3>> data_fifo::fifo_s_dequeue(){
    array<double, 3> res{0, 0, 0};
    if(fifo_isempty('s'))
        return {false, res};
    res[0] = s_sig[0][s_front];
    res[1] = s_sig[1][s_front];
    res[2] = s_sig[2][s_front];
    s_front = (s_front + 1) % S_LENGTH;
    return {true, res};
}

bool data_fifo::fifo_isempty(char mark){
    if(mark == 's'){
        if(s_front == s_rear)
            return true;
        else
            return false;
    }else if(mark == 'e'){
        if(e_front == e_rear)
            return true;
        else
            return false;
    }else return false;
}

bool data_fifo::fifo_isfull(char mark){
    if(mark == 's'){
        if((s_rear + 1) % S_LENGTH == s_front)
            return true;
        else
            return false;
    }else if(mark == 'e'){
        if((e_rear + 1) % E_LENGTH == e_front)
            return true;
        else
            return false;
    }else return false;
}

int data_fifo::fifo_get_length(char mark){
    if(mark == 's'){
        return (s_rear + S_LENGTH - s_front) % S_LENGTH;
    }else if(mark == 'e'){
        return  (e_rear + E_LENGTH - e_front) % E_LENGTH;
    }else return -1;
}

bool data_fifo::fifo_insert_pack(tcp_client::DATA &data){
    bool mark_e = true;
    bool mark_s = true;
    if(fifo_get_rest('e') < tcp_client::E_SIGNAL_LEN)
        return false;
    else{
        for(int i = 0; i < tcp_client::E_SIGNAL_LEN; ++i){
            double tmp = (double)data.e_data[i];
            if(!fifo_e_enqueue(data.e_data[i])){
                mark_e = false;
                break;
            }
        }
    }

    if(fifo_get_rest('s') < tcp_client::S_SIGNAL_LEN)
        return false;
    else{
        array<double, 3> tmp;
        for(int i = 0; i < tcp_client::S_SIGNAL_LEN; ++i){
            tmp[0] = (double)data.s_data[0][i];
            tmp[1] = (double)data.s_data[1][i];
            tmp[2] = (double)data.s_data[2][i];
            if(!fifo_s_enqueue(tmp)){
                mark_e = false;
                break;
            }
        }
    }
    if(mark_s && mark_e)
        return true;
    else
        return false;
}

tuple<bool, array<vector<double>, 3>> data_fifo::fifo_get_s_data(){
    array<vector<double>, 3> res;
    if(!fifo_isempty('s')){
        int t_rear = (s_rear > s_front)? s_rear: s_rear + S_LENGTH;
        for(int i = s_front; i < t_rear; ++i){
            res[0].push_back(s_sig[0][i % S_LENGTH]);
            res[1].push_back(s_sig[1][i % S_LENGTH]);
            res[2].push_back(s_sig[2][i % S_LENGTH]);
        }
        return tuple<bool, array<vector<double>, 3>>(true, res);
    }else
        return tuple<bool, array<vector<double>, 3>>(false, res);
}

tuple<bool, array<vector<double>, 1>> data_fifo::fifo_get_e_data(){
    array<vector<double>, 1> res;
    if(!fifo_isempty('e')){
        int t_rear = (e_rear > e_front)? e_rear: e_rear + E_LENGTH;

        for(int i = e_front; i < e_rear; ++i)
            res[0].push_back(e_sig[i % E_LENGTH]);
        return tuple<bool, array<vector<double>, 1>>(true, res);
    }else
        return tuple<bool, array<vector<double>, 1>>(false, res);
}

tuple<bool, array<vector<double>, 3>> data_fifo::fifo_dequeue_s_frame(){
    array<vector<double>, 3> res;
    if(fifo_get_length('s') >= SOUND_FRAME){
        for(int i = 0; i < SOUND_FRAME; ++i){
            //loop to copy for 3 channels
            for(int j = 0; j < 3; ++j){
                res[j].push_back(s_sig[j][s_front]);
            }
            //dequeue
            s_front = (s_front + 1) % S_LENGTH;
        }
        return tuple<bool, array<vector<double>, 3>>(true, res);
    }else
        return tuple<bool, array<vector<double>, 3>>(false, res);
}

tuple<bool, array<vector<double>, 1>> data_fifo::fifo_dequeue_e_frame(){
    array<vector<double>, 1> res;
    if(fifo_get_length('e') >= ELECTRICAL_FRAME){
        for(int i = 0; i < ELECTRICAL_FRAME; ++i){
            res[0].push_back(e_sig[e_front]);
            e_front = (e_front + 1) % E_LENGTH;
        }
        return tuple<bool, array<vector<double>, 1>>(true, res);
    }else 
        return tuple<bool, array<vector<double>, 1>>(false, res);
}