#ifndef __OLED_DATA_H
#define __OLED_DATA_H

#include <stdint.h>

/*�����ַ��ֽڿ���*/
#define OLED_CHN_CHAR_WIDTH			2		//UTF-8�����ʽ��3��GB2312�����ʽ��2

/*��ģ������Ԫ*/
typedef struct 
{
	char Index[OLED_CHN_CHAR_WIDTH + 1];	//��������
	uint8_t Data[32];						//��ģ����
} ChineseCell_t;

/*ASCII��ģ��������*/
extern const uint8_t OLED_F8x16[][16];
extern const uint8_t OLED_F6x8[][6];

/*������ģ��������*/
extern const ChineseCell_t OLED_CF16x16[];

/*ͼ����������*/
extern const uint8_t Diode[];
/*��������ĸ�ʽ�������λ�ü����µ�ͼ����������*/
//...

/*图像数据声明*/
extern const uint8_t Diode[];

extern const uint8_t HappyImage[];

extern const uint8_t AngryImage[];

extern const uint8_t SadImage[];

extern const uint8_t ListeningImage[];

extern const uint8_t SpeakingImage[];

extern const uint8_t IdleImage[];

#endif


/*****************��Э�Ƽ�|��Ȩ����****************/
/*****************jiangxiekeji.com*****************/
