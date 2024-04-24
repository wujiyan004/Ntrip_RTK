#include "nutil.h"

void print_char(char *src, int len)
{
	for(int i = 0; i < len; ++i){
		printf("%c", (unsigned char)src[i]);
	}
	printf("\n");
}

void print_char_hex(char *src, int len)
{
	for(int i = 0; i < len; ++i){
		printf("%02x ", (unsigned char)src[i]);
	}
	printf("\n");
}

int check_sum(char *src)
{
	int sum = 0, num = 0;

	sscanf(src, "%*[^*]*%x", &num);
	for(int i = 1; src[i] != '*'; ++i){
		sum ^= src[i];
	}
	return sum - num;
}

int ch2index(char ch)
{
	int i;
	for(i=0; i < (int)strlen(base64_code_table); ++i){
		if(ch == base64_code_table[i])
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
	if(len==0)
		return -1;

	if(len%3 != 0){
		count = 3 - len%3;
	}

	while(i < len){
		strncpy(temp, src+i, 3);
		result[j+0] = index2chr((temp[0]&0xFC)>>2);
		result[j+1] = index2chr(((temp[0]&0x3)<<4) | ((temp[1]&0xF0)>>4));
		if(temp[1] == 0)
			break;
		result[j+2] = index2chr(((temp[1]&0xF)<<2) | ((temp[2]&0xC0)>>6));
		if(temp[2] == 0)
			break;
		result[j+3] = index2chr(temp[2]&0x3F);
		i+=3;
		j+=4;
		memset(temp, 0x0, 3);
	}

	while(count){
		result[j+4-count] = '=';
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
	if(len==0 || len%4!=0)
		return -1;

	while(i < len){
		strncpy(temp, src+i, 4);
		result[j+0] = ((ch2index(temp[0])&0x3F) << 2) | ((ch2index(temp[1])&0x3F) >> 4);
		if(temp[2] == '=')
			break;
		result[j+1] = ((ch2index(temp[1])&0xF) << 4) | ((ch2index(temp[2])&0x3F) >> 2);
		if(temp[3] == '=')
			break;
		result[j+2] = ((ch2index(temp[2])&0x3) << 6) | ((ch2index(temp[3])&0x3F));
		i+=4;
		j+=3;
		memset(temp, 0x0, 4);
	}
	sscanf(result, "%[^:]%*c%[^\n]", usr, pwd);

	return 0;
}


int get_sourcetable(char *data, int data_len)
{
	int fsize = 0;

	FILE *fp = fopen("./config/sourcetable.dat", "r");
	if (fp != NULL) {
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
		caster_agent, fsize , source);

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

bool startsWith(const std::string& fullString, const std::string& starting) {
    if (fullString.length() >= starting.length()) {
        return (0 == fullString.compare(0, starting.length(), starting));
    } else {
        return false;
    }
}