#pragma sfr

#pragma interrupt INTST0 UART0_TxInterrupter

#include "UART0.h"
#include "FIFO.h"

FIFO UART0_Send, UART0_Recv;
unsigned char UART0_SendBuf[UART0_SendBufSize], UART0_RecvBuf[UART0_RecvBufSize];

void UART0_Initialize(void)
{
    SPS0 = (SPS0&0x00f0)|0x0001; //��N���b�N 16MHz
    
    FIFO_Initialize(&UART0_Send, UART0_SendBuf, UART0_SendBufSize);
    FIFO_Initialize(&UART0_Recv, UART0_RecvBuf, UART0_RecvBufSize);
    
    //TxD0
    SMR00 = 0x0023; //UART���M
    SCR00 = 0x8297; //UART���M, �����p���e�B, �X�g�b�v1b, �f�[�^8b
    SDR00 = 0x8A00; // 115.2kbps
    
    //RxD0
    SMR00 = 0x0122; //UART��M

    SOL0 &= 0b00000100;

    SO0  |= 0b00000001;
    SOE0 |= 0b00000001;
    SS0  |= 0b00000001;

    STMK0 = 0; //���荞�݋���
    
    return;
}

void UART0_PutChar(unsigned char c)
{
    if (SSR00 & 0x0020) {
        //���M���Ȃ̂Ńo�b�t�@�ɏ���
        FIFO_Write(&UART0_Send, c);
    } else {
        //���ڏ�������
        SDR00 = ((SDR00 & 0xff00) | c);
    }

    return;
}

void UART0_TxInterrupter(void)
{
    int c;
    
    c = FIFO_Read(&UART0_Send);

    if (c != -1) { //���M����f�[�^������
        SDR00 = ((SDR00 & 0xff00) | (c & 0xff));
    }
    
    return;
}