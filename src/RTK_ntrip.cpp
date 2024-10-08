#include <iostream>
#include <sstream>
#include "serial/serial.h"
#include "nutil.h"
#include <iomanip>
#include <ros/ros.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h> /* basic socket definitions */
// #include  <netinet/in.h>      /* sockaddr_in{} and other Internet defns */
#include <arpa/inet.h> /* inet(3) functions */
#include <unistd.h>
#include <fcntl.h>
#include<K823_msgs/K823.h>
using namespace std;
ros::Publisher K823_pub;
K823_msgs::K823 k823;
void print_char(char *src, int len)
{
	for (int i = 0; i < len; ++i)
	{
		printf("%c", (unsigned char)src[i]);
	}
	printf("\n");
}

void print_char_hex(char *src, int len)
{
	for (int i = 0; i < len; ++i)
	{
		printf("%02x ", (unsigned char)src[i]);
	}
	printf("\n");
}

int check_sum(char *src)
{
	int sum = 0, num = 0;

	sscanf(src, "%*[^*]*%x", &num);
	for (int i = 1; src[i] != '*'; ++i)
	{
		sum ^= src[i];
	}
	return sum - num;
}

int ch2index(char ch)
{
	int i;
	for (i = 0; i < (int)strlen(base64_code_table); ++i)
	{
		if (ch == base64_code_table[i])
			return i;
	}
	return -1;
}

char index2chr(int index)
{
	return base64_code_table[index];
}

int base64_encode(char *src, char *result)
{
	char temp[3] = {0};
	int i = 0, j = 0, count = 0;
	int len = strlen(src);
	if (len == 0)
		return -1;

	if (len % 3 != 0)
	{
		count = 3 - len % 3;
	}

	while (i < len)
	{
		strncpy(temp, src + i, 3);
		result[j + 0] = index2chr((temp[0] & 0xFC) >> 2);
		result[j + 1] = index2chr(((temp[0] & 0x3) << 4) | ((temp[1] & 0xF0) >> 4));
		if (temp[1] == 0)
			break;
		result[j + 2] = index2chr(((temp[1] & 0xF) << 2) | ((temp[2] & 0xC0) >> 6));
		if (temp[2] == 0)
			break;
		result[j + 3] = index2chr(temp[2] & 0x3F);
		i += 3;
		j += 4;
		memset(temp, 0x0, 3);
	}

	while (count)
	{
		result[j + 4 - count] = '=';
		--count;
	}

	return 0;
}

int base64_decode(char *src, char *usr, char *pwd)
{
	char result[64] = {0};
	char temp[4] = {0};
	int i = 0;
	int j = 0;
	int len = strlen(src);
	if (len == 0 || len % 4 != 0)
		return -1;

	while (i < len)
	{
		strncpy(temp, src + i, 4);
		result[j + 0] = ((ch2index(temp[0]) & 0x3F) << 2) | ((ch2index(temp[1]) & 0x3F) >> 4);
		if (temp[2] == '=')
			break;
		result[j + 1] = ((ch2index(temp[1]) & 0xF) << 4) | ((ch2index(temp[2]) & 0x3F) >> 2);
		if (temp[3] == '=')
			break;
		result[j + 2] = ((ch2index(temp[2]) & 0x3) << 6) | ((ch2index(temp[3]) & 0x3F));
		i += 4;
		j += 3;
		memset(temp, 0x0, 4);
	}
	sscanf(result, "%[^:]%*c%[^\n]", usr, pwd);

	return 0;
}

int get_sourcetable(char *data, int data_len)
{
	int fsize = 0;

	FILE *fp = fopen("./config/sourcetable.dat", "r");
	if (fp != NULL)
	{
		fseek(fp, 0, SEEK_END);
		fsize = (int)ftell(fp);
		rewind(fp);
	}

	char *source = (char *)malloc(fsize);
	fread(source, sizeof(char), fsize, fp);

	snprintf(data, data_len, "SOURCETABLE 200 OK\r\n"
							 "Server: %s\r\n"
							 "Content-Type: text/plain\r\n"
							 "Content-Length: %d\r\n"
							 "%s"
							 "ENDSOURCETABLE\r\n",
			 caster_agent, fsize, source);

	fclose(fp);
	free(source);
	source = NULL;

	return 0;
}

// add 7 0
std::string format70(double value, int numDecimalPlaces)
{
	std::ostringstream streamObj;
	streamObj << std::fixed << std::setprecision(numDecimalPlaces) << value;
	return streamObj.str();
}
// xor
unsigned char xor_checksum(const std::string &gpgga)
{
	unsigned char checksum = 0;
	for (char c : gpgga)
	{
		checksum ^= static_cast<unsigned char>(c);
	}
	return checksum;
}

// int 2 hexstring
std::string intToHexString(int i)
{
	std::stringstream stream;
	stream << std::hex << i;
	return stream.str();
}
// gga ts
std::string ggats()
{
	std::time_t now = std::time(nullptr);
	std::tm *ptm = std::localtime(&now);
	std::ostringstream timeStream;
	timeStream << std::put_time(ptm, "%H%M%S");
	std::string timeStamp = timeStream.str() + ".00";
	return timeStamp;
}
// generation
// status:  0初始化，1单点定位，2码差分，3无效PPS，4固定解，5浮点解
//          6正在估，7人工输入固定值，8模拟模式，9WAAS差分
std::string gen_gpgga(std::string lat, std::string lng, int status)
{
	if (status < 0 || status > 9)
		status = 0;
	std::string _status = std::to_string(status);
	std::string NORS = "N";
	std::string EORW = "E";
	// convert
	double dlng = std::stod(lng);
	double dlat = std::stod(lat);
	// add 7 0
	std::string dlng7 = format70(dlng, 7);
	std::string dlat7 = format70(dlat, 7);
	// substr
	double dlng_dd = std::stod(dlng7.substr(0, 3));
	double dlng_ddd = std::stod(dlng7.substr(3));
	double dlat_dd = std::stod(dlat7.substr(0, 2));
	double dlat_ddd = std::stod(dlat7.substr(2));
	// calc
	double lng_new = dlng_dd + dlng_ddd / 60.0;
	double lat_new = dlat_dd + dlat_ddd / 60.0;
	// if
	if (lat_new < 0)
		NORS = "S";
	if (lng_new < 0)
		EORW = "W";
	std::string gga = "GPGGA," + ggats() + "," + lat + "," + NORS + "," + lng + "," + EORW + "," + _status + ",08,1.0,0.000,M,100.000,M,,";
	unsigned char checksum = xor_checksum(gga);
	return "$" + gga + "*" + intToHexString(static_cast<int>(checksum)) + "\r\n";
}

std::string gen_gpgga_sinan(std::string lat, std::string lng, int status)
{
	lat = std::to_string(atof(lat.c_str()) * 100);
	lng = std::to_string(atof(lng.c_str()) * 100);
	if (status < 0 || status > 9)
		status = 0;
	std::string _status = std::to_string(status);
	std::string NORS = "N";
	std::string EORW = "E";
	// convert
	double dlng = std::stod(lng);
	double dlat = std::stod(lat);
	// add 7 0
	std::string dlng7 = format70(dlng, 7);
	std::string dlat7 = format70(dlat, 7);
	// substr
	double dlng_dd = std::stod(dlng7.substr(0, 3));
	double dlng_ddd = std::stod(dlng7.substr(3));
	double dlat_dd = std::stod(dlat7.substr(0, 2));
	double dlat_ddd = std::stod(dlat7.substr(2));
	// calc
	double lng_new = dlng_dd + dlng_ddd / 60.0;
	double lat_new = dlat_dd + dlat_ddd / 60.0;
	// if
	if (lat_new < 0)
		NORS = "S";
	if (lng_new < 0)
		EORW = "W";
	std::string gga = "GPGGA," + ggats() + "," + lat + "," + NORS + "," + lng + "," + EORW + "," + _status + ",08,1.0,0.000,M,100.000,M,,";
	unsigned char checksum = xor_checksum(gga);
	return "$" + gga + "*" + intToHexString(static_cast<int>(checksum)) + "\r\n";
}

bool startsWith(const std::string &fullString, const std::string &starting)
{
	if (fullString.length() >= starting.length())
	{
		return (0 == fullString.compare(0, starting.length(), starting));
	}
	else
	{
		return false;
	}
}

bool isNum(const std::string &str)
{
std:;
	istringstream iss(str);
	double number;
	char ch;
	if (!(iss >> number))
	{
		return false;
	}
	if (iss >> ch)
	{
		return false;
	}
	return true;
}

bool startsWithIndex(const std::string &fullString, const std::string &starting)
{
	bool result = false;
	int index = -1, ret = -1;
	if (fullString.length() >= starting.length())
	{
		cout << "read data : " << fullString << endl;
		ret = fullString.find(starting);
		if (std::string::npos != ret)
		{
			if (0 == ret)
				result = true;
		}
	}
	return result;
}

/// new add for decode gpgga
std::string read_gpgga(int fd)
{
	char buffer[256];
	std::string gpgga;
	while (true)
	{
		ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
		if (bytes_read > 0)
		{
			buffer[bytes_read] = '\0';
			gpgga += buffer;
			size_t found = gpgga.find("\r\n");
			if (found != std::string::npos)
			{
				std::string line = gpgga.substr(0, found);
				if (line.substr(0, 6) == "$GPYBM")
				{
					return line;
				}
				gpgga = gpgga.substr(found + 2);
			}
		}
		usleep(100000);
		/*
		else if (bytes_read == 0 || (errno != EAGAIN && errno != EWOULDBLOCK))
		{
			std::cerr << "Error reading from serial port" << std::endl;
			return "";
		}
		*/
	}
}

std::string degreesToDMS(double degrees)
{
    int deg = static_cast<int>(degrees); // 整数部分为度数  
    double temp = (degrees - deg) * 60; // 小数部分乘以60得到分和秒的临时值  
    int min = static_cast<int>(temp); // 分的整数部分  
    double sec = (temp - min) * 60; // 剩余的小数部分乘以60得到秒
    
    std::stringstream minstr;
    //保存至少两位数字字符串
    minstr << std::setfill('0') << std::setw(2) << min;

    std::stringstream secstr;
    secstr<< std::fixed << std::setprecision(6)<< sec;

    std::string result = secstr.str();
    size_t pos = result.find('.');
    while (pos != std::string::npos) {  
        result.erase(pos, 1); // 删除找到的'.'  
        pos = result.find('.', pos); // 继续从下一个位置查找  
    }  
    std::stringstream out;
    std::cout << deg << "."<< minstr.str()  << result << std::endl;
    std::stringstream ss;
    ss << to_string(deg) << "." << minstr.str() << result;
    //std::cout << ss.str() << std::endl;
	return ss.str();
}


void *serial_port_recv(void *arg)
{
	TransData *td = (TransData *)arg;
	std::string gpgga;
	std::string substr;
	int index = -1;
	while ((gpgga = read_gpgga(td->serial_fd)).size() > 0)
	{
		std::cout << "GPYBM: " << gpgga << std::endl;
		index = -1;
		stringstream ss(gpgga);
		while (getline(ss, substr, ','))
		{
			index++;
			switch (index)
			{
			case 3:
				if (isNum(substr))
					ros::param::set("lat", stod(substr)), k823.lat = stod(substr), td->lat = degreesToDMS(stod(substr));
				break;
			case 4:
				if (isNum(substr))
					ros::param::set("lng", stod(substr)), k823.lng = stod(substr), td->lng = degreesToDMS(stod(substr));
				break;
			case 6:
				cout << "heading:" << substr << endl;
				if (isNum(substr))
					ros::param::set("RTK_heading", stof(substr)), k823.RTK_angle = stof(substr);
				break;
			case 12:
				cout << "x:" << substr << endl;
				if (isNum(substr))
					ros::param::set("RTK_x", stod(substr)), k823.RTK_x = stod(substr);
				break;
			case 13:
				cout << "y:" << substr << endl;
				if (isNum(substr))
					ros::param::set("RTK_y", stod(substr)), k823.RTK_y = stod(substr);
				break;
			case 16:
				cout << "fix_pos:" << substr << endl;
				if (isNum(substr))
					ros::param::set("RTK_pos_flag", stoi(substr)), k823.position_flag = stoi(substr),td->position_flag = stoi(substr);
				break;
			case 17:
				cout << "fix_ori:" << substr << endl;
				if (isNum(substr))
					ros::param::set("RTK_Angle_flag", stoi(substr)), k823.orientation_flag = stoi(substr),td->position_flag = stoi(substr);
				break;
			}
		}

		k823.RTK_name = "Lao_wang";

		K823_pub.publish(k823);
	}
}


int main(int argc, char **argv)
{

	ros::init(argc, argv, "RTK_NTrip");
	ros::NodeHandle nh;
	// sub8 = nh.subscribe("PC_to_Server",1,callback8);
    K823_pub = nh.advertise<K823_msgs::K823>("K823", 1, true);
	ros::Rate loop_rate(5);
	// memset(&addr_serv.sin_zero, 0, sizeof(addr_serv.sin_zero));//medmory apply ps:we can ignore it

	pthread_t sp_read;
	TransData td;
	//td.lat = "32.45795135";
	//td.lng = "120.29880371";
	//td.lat = "32.240046348";
	//td.lng = "119.320203369";
	td.lat ="0";
	td.lng ="0";
	td.position_flag = 0;
	// init for serial
	int serial_fd;
	int ret_dev = 0;
	termios *ter_s = (termios *)malloc(sizeof(termios));
	td.serial_fd = open(SP_DEV, O_RDWR | O_NOCTTY | O_NDELAY);
	if (td.serial_fd < 0)
	{
		printf("%s open faild\r\n", SP_DEV);
		return -1;
	}
	bzero(ter_s, sizeof(*ter_s));
	// def 8n1
	ter_s->c_cflag |= CLOCAL | CREAD;
	ter_s->c_cflag &= ~CSIZE;
	ter_s->c_cflag |= CS8;
	ter_s->c_cflag &= ~CSTOPB;
	ter_s->c_cflag &= ~PARENB;
	ter_s->c_cc[VTIME] = 0;
	ter_s->c_cc[VMIN] = 0;
	cfsetispeed(ter_s, B115200);
	cfsetospeed(ter_s, B115200);
	tcflush(td.serial_fd, TCIFLUSH);
	if (tcsetattr(td.serial_fd, TCSANOW, ter_s) != 0)
	{
		printf("com set error!\r\n");
	}
	pthread_create(&sp_read, NULL, serial_port_recv, (void *)&td);
	while (ros::ok())
	{
		if (stod(td.lat) != 0 && stod(td.lng) != 0)
		{
			break;
		}
		ROS_INFO("wait for serial GPYBM lat and lng------");
		loop_rate.sleep();
	}

	// init for socket
	int m_sock;
	time_t start, stop;
	char recv_buf[1500] = {0};
	char request_data[1024] = {0};
	char userinfo_raw[48] = {0};
	char userinfo[64] = {0};
	char server_ip[] = QX_SERVER;
	int server_port = QX_PORT;
	char mountpoint[] = QX_MOUNT;
	char user[] = QX_USER;
	char passwd[] = QX_USERPASS;
	// gen gpgga
	std::string test_gga = gen_gpgga_sinan(td.lat, td.lng, td.position_flag);
	char gpgga[test_gga.length() + 1];
	memset(gpgga, '\0', sizeof(gpgga));
	strcpy(gpgga, test_gga.c_str());
	// test gpgga
	// char gpgga[] = "$GPGGA,083552.00,3000.0000000,N,11900.0000000,E,1,08,1.0,0.000,M,100.000,M,,*57\r\n";
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	server_addr.sin_addr.s_addr = inet_addr(server_ip);
	sprintf(userinfo_raw, "%s:%s", user, passwd);
	base64_encode(userinfo_raw, userinfo);
	sprintf(request_data,
			"GET /%s HTTP/1.1\r\n"
			"User-Agent: %s\r\n"
			"Accept: */*\r\n"
			"Connection: close\r\n"
			"Authorization: Basic %s\r\n"
			"\r\n",
			mountpoint, client_agent, userinfo);
	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_sock < 0)
	{
		printf("create socket fail\n");
		exit(1);
	}
	int ret = connect(m_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));
	if (ret < 0)
	{
		printf("connect caster fail\n");
		exit(1);
	}
	ret = send(m_sock, request_data, strlen(request_data), 0);
	if (ret < 0)
	{
		printf("send request fail\n");
		exit(1);
	}

	while (ros::ok())
	{
		    
		while (ros::ok())
		{
			ret = recv(m_sock, (void *)recv_buf, sizeof(recv_buf), 0);
			ROS_INFO("-----------------------recv_buf:[%s] ret=[%d]", recv_buf, ret);
			if (ret > 0 && !strncmp(recv_buf, "ICY 200 OK\r\n", 12))
			{
				ret = send(m_sock, gpgga, strlen(gpgga), 0);
				ROS_INFO("---------------strncmp");
			//ROS_INFO("------------------------------------test_gga: [%s]",test_gga.c_str());
				if (ret < 0)
				{
					printf("send gpgga data fail\n");
					exit(1);
				}
				break;
			}
		}
		while (ros::ok())
		{
			start = time(NULL);
			ret = recv(m_sock, (void *)recv_buf, sizeof(recv_buf), 0);
			stop = time(NULL);
			if (ret > 0)
			{
				printf("recv data:[%d] used time:[%d]\n", ret, (int)(stop - start));
				ret_dev = write(td.serial_fd, recv_buf, ret);
				printf("send data:[%d] to serial port\n", ret_dev);
			}
			else
			{
				printf("remote socket close!!!\n");
				break;
			}
			test_gga = gen_gpgga_sinan(td.lat, td.lng, td.position_flag);
			char gpgga[test_gga.length() + 1];
			memset(gpgga, '\0', sizeof(gpgga));
			strcpy(gpgga, test_gga.c_str());
			ROS_INFO("------------------------------------test_gga: [%s]",test_gga.c_str());

			ret = -1;
			ret = send(m_sock, gpgga, strlen(gpgga), 0);
			if (ret < 0)
			{
				printf("send gpgga data fail\n");
			}
		}

		// ros::param::get("gailv",str2.Water_volume);
		 //ROS_INFO("send: [%s]",tcp_message.c_str());
	}
	ros::spin();
	free(ter_s);
	close(m_sock);
	return 0;
}
