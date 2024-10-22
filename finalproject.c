#include<reg52.h>  // �]�t���Y�ɡA�@�뱡�p���ݭn��ʡA���Y�ɥ]�t�S��\��H�s�����w�q
#include <string.h>
#include <stdio.h>  // �K�[����ӥ]�t sprintf ��ƭ쫬

#define MAX 20
typedef unsigned char byte;  // 8bit
typedef unsigned int  word;  // 16bit

#define DataPort P2
sbit LATCH1 = P0^0; //�w�q��s�ϯ�ݤf �q��s
sbit LATCH2 = P0^1;
unsigned char code dofly_DuanMa[10] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f}; // ��ܬq�X��0~9
unsigned char code dofly_WeiMa[] = {0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f};                     //���O�����������ƽX���I�G,�Y��X
void Display(unsigned char FirstBit, unsigned char Num); //�ƽX����ܨ��
unsigned char TempData[8]; // �s�x��ܭȪ������ܶq

byte buf[MAX];  // �sUART
byte head = 0;
byte secondChar[2];
byte get_0d = 0;
byte rec_flag = 0;


/*------------------------------------------------
                   �禡�ŧi
------------------------------------------------*/
void Init_Timer0(void)
{
 TMOD |= 0x01;	  //�ϥμҦ�1�A16�줸�p�ɾ��A�ϥ�"|"�Ÿ��i�H�b�ϥΦh�ӭp�ɾ��ɤ����v�T		     
 //TH0=0x00;	      //���w���
 //TL0=0x00;
 EA=1;            //�`���_���}
 ET0=1;           //�p�ɾ����_���}
 
 TR0=1;           //�p�ɾ��}�����}
  
 
}
void Timer0_isr(void) interrupt 1 
{
 TH0=(65536-2000)/256;		  //���s��� 2ms
 TL0=(65536-2000)%256;
 
 Display(0,8);       // �եμƽX�ޱ��y
 
}

void SendStr(unsigned char *s);

/*------------------------------------------------
                    ��f��l��
------------------------------------------------*/
void InitUART(void)
{
    SCON  = 0x50;  // SCON: �Ҧ� 1, 8-bit UART, �ϯ౵��
    TMOD |= 0x20;  // TMOD: timer 1, mode 2, 8-bit ����
    TH1   = 0xFD;  // TH1: ���˭� 9600 ��C�ǿ�t�v ���� 11.0592MHz
    TR1   = 1;     // TR1: timer 1 ���}
    EA    = 1;     // ���}�`���_
    // ES = 1;      // ���}��f���_
}

/*------------------------------------------------
                    �D���
------------------------------------------------*/
unsigned char ans[20]; // �s�x��ܭȪ������ܶq
unsigned char k=0;
void main(void)
{
	word j;
    InitUART();

	Init_Timer0();
	ES = 1;  // ���}��f���_
    while (1)
    {	
        if (rec_flag == 1)
        {

            buf[head] = '\0';
            rec_flag = 0;
            head = 0;

			if (strcmp(buf,"R") == 0){
				TempData[0] = 0x50;  //r
                TempData[1] = 0x79;  //E
                TempData[2] = 0x6D;	 //S
				TempData[3] = 0x78;	 //t		
			}
			else if (strlen(buf) == 1){
				unsigned char i;
   				for(i = 1; i < 4 ; i++) 
      				TempData[i] = 0x00;
                TempData[0] = dofly_DuanMa[buf[0] - '0'];//0~9
            }
            else if(strlen(buf) == 2) {	
				unsigned char j=0;
				unsigned char i;
   				for(i = 2; i < 4 ; i++) 
      				TempData[i] = 0x00;
				TempData[0] = dofly_DuanMa[buf[0] - '0'];//0~9
				
				secondChar[0] = buf[1];
                secondChar[1] = '\0';
				if (strcmp(secondChar, "L") == 0) {
                    TempData[1] = 0x38; // L
                }
                else if (strcmp(secondChar, "H") == 0) {
                    TempData[1] = 0x76; // H
                }
				else{
					TempData[1] = dofly_DuanMa[buf[1] - '0'];//0~9
				}
            }
			else if(strlen(buf) == 3){
				TempData[3] = 0x00;
				TempData[0] = dofly_DuanMa[buf[0] - '0'];//0~9
				TempData[2] = dofly_DuanMa[buf[2] - '0'];//0~9
				secondChar[0] = buf[1];
                secondChar[1] = '\0';
				if (strcmp(secondChar, "L") == 0) {
                    TempData[1] = 0x38; // L
                }
                else if (strcmp(secondChar, "H") == 0) {
                    TempData[1] = 0x76; // H
                }
			}
        }
    }
}

/*------------------------------------------------
                    �o�e�@�Ӧ줸��
------------------------------------------------*/
void SendByte(unsigned char dat)
{
    SBUF = dat;
    while(!TI);
    TI = 0;
}

/*------------------------------------------------
                    �o�e�@�Ӧr��
------------------------------------------------*/
void SendStr(unsigned char *s)
{
    while(*s != '\0')  // \0 ��ܦr�굲���лx�A�q�L�˴��O�_�r�꥽��
    {
        SendByte(*s);
        s++;
    }
}

/*------------------------------------------------
                     ��f���_�{��
------------------------------------------------*/
void UART_SER(void) interrupt 4  // ��C���_�A�ȵ{��
{
    unsigned char tmp;  // �w�q�{���ܼ�

    if(RI)  // �P�_�O�������_����
    {
        RI = 0;  // �лx�줸�M�s
        tmp = SBUF;  // Ū�J�w�İϪ���
        if (get_0d == 0)
        {
            if (tmp == 0x0d)
                get_0d = 1;
            else
            {
                buf[head] = tmp;
                head++;
                if (head == MAX) head = 0;
            }
        }
        else if (get_0d == 1)
        {
            if (tmp != 0x0a)
            {
                head = 0;
                get_0d = 0;
            }
            else
            {
                rec_flag = 1;
                get_0d = 0;
            }
        }

        // SBUF = tmp;  // �Ⱶ���쪺�ȦA�o�^�q����
    }
    // if(TI)  // �p�G�O�o�e�лx�줸�A�M�s
    //     TI = 0;
}
void Display(unsigned char FirstBit, unsigned char Num) {
    static unsigned char i = 0;
    DataPort = 0;   //�M�żƾڡA���������v
    LATCH1 = 1;     //�q��s
    LATCH1 = 0;

    DataPort = dofly_WeiMa[i + FirstBit]; //����X 
    LATCH2 = 1;     //����s
    LATCH2 = 0;

    DataPort = TempData[i]; //����ܼƾڡA�q�X
    LATCH1 = 1;     //�q��s
    LATCH1 = 0;

    i++;
    if (i == Num)i = 0;
}

