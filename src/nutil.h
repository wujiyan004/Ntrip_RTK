#ifndef __NTRIP_UTIL_H__
#define __NTRIP_UTIL_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <termios.h>
#include <malloc.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <pthread.h>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>

/* QX SERVICE PARAM */
#define QX_SERVER "203.107.45.154"
#define QX_PORT 8002
#define QX_MOUNT "AUTO"
#define QX_USER "qxwfue008"
#define QX_USERPASS "12cd52b"

/* MCU SERIAL */
#define SP_DEV "/dev/ttyUSB5"


typedef struct
{
    int serial_fd;
    std::string lat;
    std::string lng;
    int position_flag;
    int orientation_flag;
    char databuf[1024];
} TransData;

const char caster_agent[] = "NTRIP NTRIPCaster/20181206";
const char client_agent[] = "NTRIP NTRIPClient/20181206";
const char server_agent[] = "NTRIP NTRIPServer/20181206";
const char base64_code_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
void print_char(char *src, int len);
void print_char_hex(char *src, int len);
int check_sum(char *src);
int ch2index(char ch);
char index2chr(int index);
int base64_encode(char *src, char *result);
int base64_decode(char *src, char *usr, char *pwd);
int get_sourcetable(char *data, int data_len);

std::string format70(double value, int numDecimalPlaces);
unsigned char xor_checksum(const std::string &gpgga);
std::string intToHexString(int i);
std::string ggats();
std::string gen_gpgga(std::string lat, std::string lng, int status);
bool startsWith(const std::string& fullString, const std::string& starting);

#endif
