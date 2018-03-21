#ifndef _DAC_I2C_H
#define _DAC_I2C_H

#define DeDacAddr       0xC0U 	//MCP4725
//#define DeDacAddr        (0xC0U >> 1)	//BH1750

typedef enum{
} MCP_t;

class dac
{
private:
	static int m_addr;
public:
	static bool m_enableFlag;

	dac();
	dac(int);
	void setMode(MCP_t  );
	void writeDac(float);
	void testDac();
};


#endif
