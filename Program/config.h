#ifndef __CONFIG_H_
#define __CONFIG_H_

/*FIFO CONFIG*/
#define ELECTRICAL_FIFO_LENGTH  10 * E_FS
#define SOUND_FIFO_LENGTH       10 * S_FS

#define E_FS                    1000000
#define S_FS                    44100

/*TCP CONNECTION CONFIG*/
#define ELECTRICAL_RX_PACK_LEN  2 * E_FS
#define SOUND_RX_PACK_LEN       2 * S_FS
#define IP                      "127.0.0.1"
#define PORT                    2000

/*CORE ALGORITHM*/
#define V                       340
#define D                       1
#define S_THRESHOLD             0.001
#define E_THRESHOLD             153
#define ENERGY_THRESHOLD        10
#define S_FRAME_TIME            2
#define E_FRAME_TIME            0.0001
#define WN_LENGTH               S_FRAME_TIME * S_FS
#define HANN_DAT_PATH           "./hann.dat"

/*MAIN*/
#define MAX_CTR                 50

#endif