
struct respondlogin{
	char account[12];
	unsigned char loginresult;
};

struct respondheartbeat{
	char account[12];
};

struct time{
	unsigned char hour;
	unsigned char minutes;
	unsigned char seconds;
	unsigned char tenms;
};

struct positioninfo{ 
	unsigned int id;
	unsigned char accuracy;
	unsigned char urgentposition;
	unsigned char multivaluesolution;
	struct time positiontime;
	unsigned int longitude;
	unsigned int latitude;
	short geodeticheight;
	short detlaelevation;
};

struct communicationinfo{
	unsigned int sendaddr;
	unsigned int recvaddr;
	struct time sendtime;
	unsigned char encodingtype;
	unsigned char message[210]; 
};

struct communicationreceipt{
	unsigned int sendaddr;
	unsigned int recvaddr;
	struct time receipttime;
};

struct encodeprotocol_respond{
	unsigned char messagetype;
	struct union{
		struct respondlogin* respondlogin;
		struct respondheartbeat* respondheartbeat;
		struct positioninfo* postioninfo;
		struct communicationinfo* communicationinfo;
		struct communicationreceipt* communicationreceipt;
	}message;
};

int encodelogin(struct respondlogin* respondlogin, unsigned char * buf, int& len){
	// $ZCFH长度(2bytes)登录账号(12 bytes)注册结果(8bytes)校验和(1bytes)
	//buf[0] = '$';buf[1] = 'Z'; buf[2] = 'C'; buf[3] = 'F'; buf[4] = 'H';
	memcpy(buf, "$ZCFH",5);
	memcpy(buf + 7, respondlogin.account, 12); 

}
