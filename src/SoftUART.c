#pragma sfr

#pragma interrupt INTTM07 SUART_Interrupter

#include "SoftUART.h"

// �s����`
#define P_TOOL0 P4.0
#define PM_TOOL0 PM4.0

// �{�[���[�g�J�E���^�̏����l
// SUART_TDR = CPU_FREQ / BAUD_RATE = 32,000,000 [cycle/sec] / 9600 [bps] = 3333 [cycle/baud]
#define SUART_TDR 3333 /* 9600bps */

struct SUARTInfo {
	char buf:8;
	char recvBuf:8;
	unsigned int tx:1;
	unsigned int rx:1;
	unsigned int cnt:4;
	unsigned int recv:1;
};

volatile struct SUARTInfo suart;

void SUART_TOOL0_High(void);
void SUART_TOOL0_Low(void);

void SUART_Initialize(void)
{
	TMR07 = 0x00; //���[�h�ݒ�
	TMMK07 = 0; //���荞�݂�����
	TDR07 = SUART_TDR / 2 - 1; //���荞�݊Ԋu

	//�\���̏�����
	suart.tx = 0;
	suart.rx = 0;
	suart.recv = 0;
	
	SUART_TOOL0_High();
	
	TS0 = 0x80; //�^�C�}�J�n

	return;
}

char SUART_GetChar(void)
{
	char c;

	while(!suart.recv) {}
	c = suart.recvBuf;
	suart.recv = 0;

	return(c);
}

void SUART_GetStr(char *c, int size)
{
	int i;
	
	i = 0;
	while(i < size) {
		c[i] = SUART_GetChar();
		
		if (c[i] == 0x08) { //BackSpace
			if (i > 0) {
				SUART_PutStr(" \x08");
				i--;
			} else {
				SUART_PutChar(' ');
			}
			continue;
		}
		
		if ((c[i] == '\n')||( c[i] == '\r')) {break;} //Enter
		
		i++;
	}
	
	c[i] = '\0';

	return;		
}

void SUART_PutChar(unsigned char c) //1�������M
{
	while(suart.tx||suart.rx||(P_TOOL0 == 0)) {} //���M��

	suart.buf = c;
	suart.cnt = 0;
	TDR07 = SUART_TDR - 1; //�J�E���^
	suart.tx = 1;
	
	return;
}

void SUART_PutByte(unsigned char c)
{
	char s[] = "0123456789ABCDEF";
	SUART_PutChar(s[c>>4]);
	SUART_PutChar(s[c&0x0f]);
	return;
}

void SUART_PutInt(int val)
{
    int i = 1;

    if (val < 0) {
        SUART_PutChar('-');
        val = -val;
    }

    while(val / i >= 10) {i *= 10;}

    while(i != 0) {
        SUART_PutChar('0' + (char) ((val / i) % 10));
        i /= 10;
    }

    return;
}

void SUART_PutStr(char *str) //�����񑗐M
{
	while(*str) {
		SUART_PutChar(*str++);
	}

	return;
}

void SUART_TOOL0_High(void)
{
	PM_TOOL0 = 1;
	
	return;
}

void SUART_TOOL0_Low(void)
{
	P_TOOL0 = 0;
	PM_TOOL0 = 0;
	
	return;
}

void SUART_Interrupter(void)
{
	if (suart.rx) { //��M
		switch(suart.cnt) {
			case 9:
				suart.rx = 0;
				suart.recv = 1;
				suart.recvBuf = suart.buf;
				break;
			case 0: //�L���v�`���Ԋu�����炷
				TDR07 = SUART_TDR - 1; //�J�E���^
				break;
			case 8: 
				TDR07 = SUART_TDR / 2 - 1; //���荞�݊Ԋu
			default:
				suart.buf = (suart.buf>>1) + (P_TOOL0<<7);
				break;
		}
		suart.cnt++;
	} else if (suart.tx) { // ���M
		switch(suart.cnt) {
			case 0:
				SUART_TOOL0_Low();
				break;
			case 9:
				SUART_TOOL0_High();
				suart.tx = 0;
				TDR07 = SUART_TDR / 2 - 1; //���荞�݊Ԋu
				break;
			default:
				if ((suart.buf >> (suart.cnt - 1))&0x01) {
					SUART_TOOL0_High();
				} else {
					SUART_TOOL0_Low();
				}
		}
		suart.cnt++;
	} else { //��M�ҋ@
		if (P4.0 == 0) { //�X�^�[�g�r�b�g�����o
			suart.rx = 1;
			suart.cnt = 0;
			TDR07 = SUART_TDR * 3 / 4 - 1;
		}
	}

	WDTE = 0xAC;
	return;
}
