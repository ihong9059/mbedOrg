#ifndef _BH1750_I2C_H
#define _BH1750_I2C_H

#define DeEpromAddr       0xA0U 	//AT24c02
//#define DeBH1750Addr        (0x46U >> 1)	//BH1750

#define DeWriteDelay 0.001
#define DeReadDelay 0.0001
#define DePageWriteDelay 0.001

#define De256WriteDelay 0.005
#define DeReadDelay 0.0001
#define De256PageWriteDelay 0.005
typedef struct{
	uint8_t addr;
	char data;
} ep02ByteWrite_t;

typedef struct{
	uint8_t addr;
	char data[16];
} ep02PageWrite_t;

typedef struct{
	uint16_t addr;
	char data;
} epByteWrite_t;

typedef struct{
	uint16_t addr;
	char data[16];
} epPageWrite_t;

typedef struct{
	uint16_t id;
	uint16_t gid;
	uint8_t pid;
	uint8_t site;
	uint8_t mac[6];		//6bytes
	uint8_t tbd[4];		//4bytes
} gDb_t;

class eprom
{
private:
	int m_addr;

public:
	eprom();
	eprom(int);
	char readByte02(int);
	int readPage02(int, char*);
	int writeByte02(int, char);
	int writePage02(int, char*);
	char readByte256(uint16_t);
	int readPage256(uint16_t,char*);
	int writeByte256(uint16_t, char);
	int writePage256(uint16_t, char*);
	uint16_t swapByte(uint16_t);
	void testByte(uint16_t,uint16_t);
	void testPage(uint16_t, uint16_t);
	void test256Byte(uint16_t,uint16_t);
	void test256Page(uint16_t, uint16_t);
};


#endif
