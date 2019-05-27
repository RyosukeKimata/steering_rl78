/* main.c */

// SFR��`�̎g�p
#pragma sfr

// ���荞�ݗL��/����
#pragma DI
#pragma EI

// ���荞�݃x�N�^��`
#pragma interrupt INTTM01 dummy
#pragma interrupt INTAD AD_Interrupter

// include
#include "SoftUART.h"
#include "Servo.h"
#include "UART0.h"
#include "FIFO.h"
#include "Flash.h"
#include "System.h"
#include <stdlib.h>
#include <string.h>

// alias
#define puts SUART_PutStr

// prototype
void dummy(void);
void CTRL_Initialize(void);
void CMDLINE(void);
void CMD_control(int argc, char *argv[]);       // ����̗L��/����
void CMD_ad(int argc, char *argv[]);            // ADC�̎蓮����
void CMD_servo(int argc, char *argv[]);         // �T�[�{�̎蓮����
void CMD_servo_speed(int argc, char *argv[]);   // �T�[�{�̑��x�ݒ�
void CMD_fread(int argc, char *argv[]);         // �t���b�V����ǂݍ���ŕ\��
void CMD_fifo(int argc, char *argv[]);          // UART��FIFO�̃|�C���^��\��
void CMD_load(int argc, char *argv[]);          // �Ǌp�ݒ�̓ǂݍ���
void CMD_save(int argc, char *argv[]);          // �Ǌp�ݒ�̕ۑ�
void CMD_config(int argc, char *argv[]);        // �Ǌp��ݒ肷��

// global
volatile int last_ad[2];
volatile int last_servo[2];

void main(void)
{
    int retcode;

    // ������g����32MHz�ɂ���
    HOCODIV = 0x00;
    
    // RxD0, TxD0��P1.6, P1.7�Ɋ���U��
    PIOR |= 0x02; // PIOR1 = 1;

    P1.7 = 1;
    PM1.7 = 0; // TxD0 �o��
    PM1.6 = 1; // RxD0 ����

    // Initialize Timer-Array-Unit 0
    TAU0EN = 1;        //Enable Timer
    TPS0 = 0x0050;    //Select clock source (CK01 = 1MHz)
    TS0 = 0;        //Stop timer
    TOE0 = 0;        //Set output settings
    TO0 = 0;        //Set output
    TOL0 = 0;        //Set output level
    TOM0 = 0;        //Set output mode

    // Initialize Software-UART
    SUART_Initialize();

    // Enable Interrupt
    EI();

    exec("led 7");

    // ���Z�b�g�v����\��
    exec("resf");

    // Enable Analog-to-Digital Converter
    ADCEN = 1;

    // Enable Serial-Array-Unit 0
    SAU0EN = 1;

    // Initialize Hardware-UART 0
    UART0_Initialize();

    SUART_PutStr("SAU Enabled.\r\n");

    puts("Loading config...");
    retcode = exec("load 0");
    //retcode = 1;
    if (retcode != 0) { // �ǂݍ��ݎ��s�����ꍇ
        puts("Loading default config...");
        exec("default_config");
    }

    // �T�[�{���[�^����̏������ƊJ�n
    CTRL_Initialize();

    exec("led 3");

    SUART_PutStr("RL78/G13 debug console.\r\n");

    // �f�o�b�O�p�R�}���h���C�����N��
    CMDLINE();
}

//���荞�݃x�N�^�e�[�u���o�^�p�̃_�~�[�֐�
void dummy(void)
{
}

void CTRL_Initialize(void)
{
    //AD���[�h�ݒ�
    ADM0 = 0x30; //���x�ݒ�(2.75us)
    ADM1 = 0xA0; //�^�C�}01�̊��荞��(INTTM01)�ŕϊ��J�n, �n�[�h�E�F�A�g���K
    ADM2 = 0x00; 

    ADS = 2; // 2 = vertical, 3 = horizontal
    ADCE = 1;

    ADMK = 0;

    ADCS = 1;

    //�^�C�}01�ݒ�
    TMR01 = 0x40;
    TMMK01 = 0;
    TDR01 = 50000; // 1MHz / 50k = 20Hz

    TS0 = 0x02; //�^�C�}�J�n
}

void AD_Interrupter(void)
{
    int an;
    int adc_out;
    int servo_ch = 0;
    
    if (ADS == 2) {servo_ch = SERVO_TAIL_VERTICAL;}
    if (ADS == 3) {servo_ch = SERVO_TAIL_HORIZONTAL;}
    
    adc_out = (int) (ADCR>>6);
    
    // �ۑ�
    last_ad[servo_ch] = adc_out;

    //�Ǌp�𑗐M
    Servo_SetPos((char) servo_ch, adc_out);

    if (ADS == 2) {
        ADS = 3;
    } else {
        ADS = 2;
    }
}

void CMDLINE(void)
{
    char str[128];
    int retcode;

    while(1) {
        SUART_PutStr("> ");

        SUART_GetStr(str, 127);

        SUART_PutStr("\r\n");
        
        // �R�}���h�̎��s
        retcode = exec(str);
        
        // �R�}���h���Ȃ��ꍇ
        if (retcode == EXEC_COMMAND_NOT_FOUND) {
            puts("exec: command not found.\r\n");
            continue;
        }
        
        // �I���R�[�h��0�łȂ��ꍇ
        if (retcode != 0) {
            puts("ret = ");
            SUART_PutByte((unsigned char) retcode);
            puts("\r\n");
        }
    }
}

void CMD_servo_speed(int argc, char *argv[])
{
    int s_id, s_speed;
    
    s_id = (int) atoi(argv[1]);
    s_speed = (int) atoi(argv[2]);
    Servo_SetSpeed((char) s_id, s_speed);

    SUART_PutStr("ServoID 0x");
    SUART_PutByte((unsigned char) s_id & 0x7f);
    SUART_PutStr(" Speed => 0x");
    SUART_PutByte((unsigned char) s_speed & 0x7f);

    return;
}
