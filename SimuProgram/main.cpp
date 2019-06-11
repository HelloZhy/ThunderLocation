#include <fstream>
#include <iostream> 
#include <tuple> 
#include <array>
#include <cmath>
#include <cstdio>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>


#define PORT_SERVER     2000
#define PORT_CLIENT     2000
#define MAX_CLIENT_NUM  10
#define MAX_INPUTS      8

#define V               340
#define D               1
#define PI              3.141592653

#define ELENGTH         20000000
#define SLENGTH         882000
#define E_FS            1000000
#define S_FS            44100
#define TCP_E_LENGTH    2 * E_FS
#define TCP_S_LENGTH    2 * S_FS

struct WAVHEAD{
    uint32_t big_ChunkID;
    uint32_t little_ChunkSize;
    uint32_t big_Format;
    uint32_t big_Subchunk1ID;
    uint32_t little_Subchunk1Size;
    uint16_t little_AudioFormate;
    uint16_t little_NumChannels;
    uint32_t little_SampleRate;
    uint32_t little_ByteRate;
    uint16_t little_BlockAlign;
    uint16_t little_BitsPerSample;
    uint32_t big_Subchunk2ID;
    uint32_t little_Subchunk2Size;
    //little endian data
};

struct SIGNALPACK {
    uint8_t e_signal[ELENGTH];
    uint8_t s_signal[3][SLENGTH];
};

struct SENDPACK {
    bool isvalid;
    uint8_t e_signal[TCP_E_LENGTH];
    uint8_t s_signal[3][TCP_S_LENGTH];
};

void load_signal(std::string dir, SIGNALPACK& pack){
	/*define wave head*/
	WAVHEAD electrical_head;
	WAVHEAD sound_ch1_head;
	WAVHEAD sound_ch2_head;
	WAVHEAD sound_ch3_head;
	/*read file*/
	std::fstream file;

	file.open(dir + "electrical.wav", std::ios::binary|std::ios::in);
    file.read(reinterpret_cast<char*>(&electrical_head), sizeof(WAVHEAD));
	for(int i = 0; i < ELENGTH; ++i)
		file.read(reinterpret_cast<char*>(&pack.e_signal[i]), sizeof(uint8_t));
	file.close();

	file.open(dir + "sound_ch1.wav", std::ios::binary|std::ios::in);
    file.read(reinterpret_cast<char*>(&sound_ch1_head), sizeof(WAVHEAD));
	for(int i = 0; i < SLENGTH; ++i)
		file.read(reinterpret_cast<char*>(&pack.s_signal[0][i]), sizeof(uint8_t));
	file.close();

	file.open(dir + "sound_ch2.wav", std::ios::binary|std::ios::in);
    file.read(reinterpret_cast<char*>(&sound_ch2_head), sizeof(WAVHEAD));
	for(int i = 0; i < SLENGTH; ++i)
		file.read(reinterpret_cast<char*>(&pack.s_signal[1][i]), sizeof(uint8_t));
	file.close();

	file.open(dir + "sound_ch3.wav", std::ios::binary|std::ios::in);
    file.read(reinterpret_cast<char*>(&sound_ch3_head), sizeof(WAVHEAD));
	for(int i = 0; i < SLENGTH; ++i)
		file.read(reinterpret_cast<char*>(&pack.s_signal[2][i]), sizeof(uint8_t));
	file.close();
}

int main(int argc, char* argv[]){
	if(argc != 2)
		return -1;
    SIGNALPACK* p = new SIGNALPACK;
    //load signal for simulation
	std::string d = argv[1];
    load_signal(d, *p);
    //establish connection as a server and wait for sending
    int server_fd;
    int client_fd;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t len;
    char buff[MAX_INPUTS];
    int ctr = 0;

    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		printf("error when creating a socket(server)\n");
        close(server_fd);
        return -1;
    }else printf("socket is created at %d\n", server_fd);
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_SERVER);
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    bzero(&(server_addr.sin_zero), sizeof(server_addr.sin_zero));

    if(bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0){
		printf("something error when using bind()\n");
		close(server_fd);
		return 1;
	}else{
		printf("bind finish\n");
	}

	if(listen(server_fd, MAX_CLIENT_NUM) == -1){
		printf("error when using listen()\n");
		close(server_fd);
		return 1;
	}else printf("listen established\n");

    len = sizeof(client_fd);
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &len);
	if(client_fd == -1){
		printf("error: cannot accept signal from client\n");
		close(client_fd);
		close(server_fd);
		return 1;
	}else{
		printf("connetion established!\n");
	}

    int e_mark = 0;
    int s_mark = 0;
    SENDPACK send_pack;
    while(1){
		ctr = recv(client_fd, buff, MAX_INPUTS, 0);
		if(ctr < 0){
			//error with client
			close(client_fd);
			break;
		}
		if(strncmp(buff, "__exit_", 8) == 0 || ctr == 0){
			//client normal exit;
			break;	
		}
		printf("recv from client: %s (length, %d)\n", buff, ctr);

        std::cout << "e mark: " << e_mark << std::endl;
        std::cout << "s mark: " << s_mark << std::endl;
        //data move
        if((e_mark <= ELENGTH - TCP_E_LENGTH) && (s_mark <= SLENGTH - TCP_S_LENGTH)){
            for(int i = 0; i < TCP_E_LENGTH; ++i)
                send_pack.e_signal[i] = p->e_signal[e_mark + i];
            for(int i = 0; i < TCP_S_LENGTH; ++i)
                for(int j = 0; j < 3; ++j)
                    send_pack.s_signal[j][i] = p->s_signal[j][s_mark + i];
            send_pack.isvalid = true;
        }else send_pack.isvalid = false;
        //send
        long int len = 0, _len = 0;
        long int counter = 0;
        uint8_t* p = (uint8_t*)&send_pack;
        long int rest = 2048;
        while(len < sizeof(send_pack)){
            _len = send(client_fd, p + len, rest, 0);
            if(_len > 0){
                len += _len;
                counter++;
            }else{
                std::cout << "send error: "  << _len << std::endl;
				close(client_fd);
				break;
            }
            //std::cout << "[" << counter << "] send length: " << _len << " rest:" << (long int)(sizeof(send_pack) - len) << std::endl;
            if((long int)(sizeof(send_pack) - len) < 2048 && (long int)(sizeof(send_pack) - len) > 0)
                rest = (long int)(sizeof(send_pack) - len);
        }
        //shift
        e_mark += TCP_E_LENGTH;
        s_mark += TCP_S_LENGTH;
        bzero(&buff, sizeof(buff));
	}

	close(server_fd);
    std::cout << "QUIT" << std::endl;

    //delete heap memory
    delete p;
    return 0;
}
