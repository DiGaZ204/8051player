#include<reg52.h>  // 包含標頭檔，一般情況不需要改動，標頭檔包含特殊功能寄存器的定義
#include <string.h>
#include <stdio.h>  // 添加此行來包含 sprintf 函數原型

#define MAX 20
typedef unsigned char byte;  // 8bit
typedef unsigned int  word;  // 16bit

#define DataPort P2
sbit LATCH1 = P0^0; //定義鎖存使能端口 段鎖存
sbit LATCH2 = P0^1;
unsigned char code dofly_DuanMa[10] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f}; // 顯示段碼值0~9
unsigned char code dofly_WeiMa[] = {0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f};                     //分別對應相應的數碼管點亮,即位碼
void Display(unsigned char FirstBit, unsigned char Num); //數碼管顯示函數
unsigned char TempData[8]; // 存儲顯示值的全局變量

byte buf[MAX];  // 存UART
byte head = 0;
byte secondChar[2];
byte get_0d = 0;
byte rec_flag = 0;


/*------------------------------------------------
                   函式宣告
------------------------------------------------*/
void Init_Timer0(void)
{
 TMOD |= 0x01;	  //使用模式1，16位元計時器，使用"|"符號可以在使用多個計時器時不受影響		     
 //TH0=0x00;	      //給定初值
 //TL0=0x00;
 EA=1;            //總中斷打開
 ET0=1;           //計時器中斷打開
 
 TR0=1;           //計時器開關打開
  
 
}
void Timer0_isr(void) interrupt 1 
{
 TH0=(65536-2000)/256;		  //重新賦值 2ms
 TL0=(65536-2000)%256;
 
 Display(0,8);       // 調用數碼管掃描
 
}

void SendStr(unsigned char *s);

/*------------------------------------------------
                    串口初始化
------------------------------------------------*/
void InitUART(void)
{
    SCON  = 0x50;  // SCON: 模式 1, 8-bit UART, 使能接收
    TMOD |= 0x20;  // TMOD: timer 1, mode 2, 8-bit 重裝
    TH1   = 0xFD;  // TH1: 重裝值 9600 串列傳輸速率 晶振 11.0592MHz
    TR1   = 1;     // TR1: timer 1 打開
    EA    = 1;     // 打開總中斷
    // ES = 1;      // 打開串口中斷
}

/*------------------------------------------------
                    主函數
------------------------------------------------*/
unsigned char ans[20]; // 存儲顯示值的全局變量
unsigned char k=0;
void main(void)
{
	word j;
    InitUART();

	Init_Timer0();
	ES = 1;  // 打開串口中斷
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
                    發送一個位元組
------------------------------------------------*/
void SendByte(unsigned char dat)
{
    SBUF = dat;
    while(!TI);
    TI = 0;
}

/*------------------------------------------------
                    發送一個字串
------------------------------------------------*/
void SendStr(unsigned char *s)
{
    while(*s != '\0')  // \0 表示字串結束標誌，通過檢測是否字串末尾
    {
        SendByte(*s);
        s++;
    }
}

/*------------------------------------------------
                     串口中斷程式
------------------------------------------------*/
void UART_SER(void) interrupt 4  // 串列中斷服務程式
{
    unsigned char tmp;  // 定義臨時變數

    if(RI)  // 判斷是接收中斷產生
    {
        RI = 0;  // 標誌位元清零
        tmp = SBUF;  // 讀入緩衝區的值
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

        // SBUF = tmp;  // 把接收到的值再發回電腦端
    }
    // if(TI)  // 如果是發送標誌位元，清零
    //     TI = 0;
}
void Display(unsigned char FirstBit, unsigned char Num) {
    static unsigned char i = 0;
    DataPort = 0;   //清空數據，防止有交替重影
    LATCH1 = 1;     //段鎖存
    LATCH1 = 0;

    DataPort = dofly_WeiMa[i + FirstBit]; //取位碼 
    LATCH2 = 1;     //位鎖存
    LATCH2 = 0;

    DataPort = TempData[i]; //取顯示數據，段碼
    LATCH1 = 1;     //段鎖存
    LATCH1 = 0;

    i++;
    if (i == Num)i = 0;
}

