#include "u8-device.h"
#include "spi-context.h"

unsigned short CRC16 (unsigned char* pchMsg, unsigned short wDataLen)
{
        unsigned short wCRC = 0xFFFF;
        unsigned short i;
        unsigned char chChar;

        unsigned char tmp;
        unsigned short crcAccum = 0xFFFF;

        for (i = 0; i < wDataLen; i++)
        {
                chChar = *pchMsg++;

                tmp = chChar ^ (unsigned char)(crcAccum &0xff);
                tmp ^= (tmp<<4);
                crcAccum = (crcAccum>>8) ^ (tmp<<8) ^ (tmp <<3) ^ (tmp>>4);

        }
		
	return crcAccum;
}