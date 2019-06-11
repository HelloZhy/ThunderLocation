#include "includes.h"

void test(){
    algorithm algo;
    data_fifo* fifo = new data_fifo();
    tcp_client client;
    tcp_client::DATA d;
    while(1){
        client.get_data(d);
        if(!fifo->fifo_insert_pack(d))
            break;
    }
    cout << "ok" << endl;
    auto tmp = fifo->fifo_get_s_data();
    auto data = get<1>(tmp);
    tuple<bool, array<double, 3>> a = algo.s_sig_identifier(data, S_FS, 1);
    cout << ((bool)get<0>(a)? "true": "false") << endl;
    cout << "tx: " << get<1>(a)[0] << "\tty: " <<  get<1>(a)[1] << "\ttz: " << get<1>(a)[2] << endl;
    tuple<int, double> res = algo.get_location(0.001, get<1>(a)[0], get<1>(a)[1], get<1>(a)[2]);
    cout << "rho: " << get<0>(res) << "\ttheta: " << get<1>(res) << endl;

    delete fifo;
}

fstream file;

int main(int argc, char* argv[]){
    algorithm alg;
    data_fifo* fifo = new data_fifo();
    tcp_client rx_client;
    //fill the fifo
    tcp_client::DATA d;
	/*
    do{
        rx_client.get_data(d);
    }while(fifo->fifo_insert_pack(d));
	*/
	while(1){
		if(!fifo->fifo_isfull('s')){
			rx_client.get_data(d);
			if(!fifo->fifo_insert_pack(d))
				break;
		}else
			break;
	}
    //main process
    double t0 = -1;
    int ctr = 1;
    tuple<bool, array<vector<double>, 1>> e_dat;
    tuple<bool, array<vector<double>, 3>> s_dat;
    bool mark = true;
    while(mark){
        //get electrical data from fifo
        e_dat = fifo->fifo_get_e_data();
        tuple<bool, double> e_res = alg.e_sig_identifier(get<1>(e_dat)[0], E_FS);
        if(get<0>(e_res) == true){
            cout << "electrical signal identify" << endl;
            t0 = get<1>(e_res);
            while(ctr < MAX_CTR && t0 > 0){
                //get sound data from fifo
                s_dat = fifo->fifo_get_s_data();

               ////for debug
               //file.open("tmp.mat", ios::out|ios::app);
               //auto _dat = get<1>(s_dat);
               //for(int i = 0; i < _dat[1].size(); ++i)
               //    file << (int)_dat[1][i] << ' ';
               //file << endl; 
               //file.close();

                tuple<bool, array<double, 3>> s_res = alg.s_sig_identifier(get<1>(s_dat), S_FS, ctr);
                if(get<0>(s_res) == true){
                    //compute location
                    cout << "get!!!!!" << endl;
                    tuple<int, double> res = alg.get_location(t0, get<1>(s_res)[0], get<1>(s_res)[1], get<1>(s_res)[2]);
                    cout << "rho: " << get<0>(res) << "\ttheta: " << get<1>(res) << endl;
                    fifo->fifo_clear();
                    break;
                }else{
                    //drop a pack of data
                    fifo->fifo_dequeue_e_frame();
                    fifo->fifo_dequeue_s_frame();
                    //get new package
                    rx_client.get_data(d);
                    if(!d.isvalid){
                        mark = false;
                        break;
                    }
                    fifo->fifo_insert_pack(d);
                    ctr++;
                }
            }
        }else{
            //drop a pack of data
            fifo->fifo_dequeue_e_frame();
            fifo->fifo_dequeue_s_frame();
            //get new package
            rx_client.get_data(d);
            if(!d.isvalid){
                mark = false;
                break;
            }
            fifo->fifo_insert_pack(d);
        }
        //reset
        ctr = 1;
        t0 = -1;
    }
    
    delete fifo;
    return 0;
}
