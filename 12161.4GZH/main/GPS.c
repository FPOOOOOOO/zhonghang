#include "GPS.h"

// GPS NMEA-0183协议重要参数结构体定义
// 卫星信息
__packed typedef struct
{
    uint8_t num;     // 卫星编号
    uint8_t eledeg;  // 卫星仰角
    uint16_t azideg; // 卫星方位角
    uint8_t sn;      // 信噪比
} nmea_slmsg;

// UTC时间信息
__packed typedef struct
{
    uint16_t year; // 年份
    uint8_t month; // 月份
    uint8_t date;  // 日期
    uint8_t hour;  // 小时
    uint8_t min;   // 分钟
    uint8_t sec;   // 秒钟
} nmea_utc_time;

// NMEA 0183 协议解析后数据存放结构体
__packed typedef struct
{
    uint8_t svnum;             // 可见卫星数
    nmea_slmsg slmsg[12]; // 最多12颗卫星
    nmea_utc_time utc;    // UTC时间
    uint32_t latitude;         // 纬度 分扩大100000倍,实际要除以100000
    uint8_t nshemi;            // 北纬/南纬,N:北纬;S:南纬
    uint32_t longitude;        // 经度 分扩大100000倍,实际要除以100000
    uint8_t ewhemi;            // 东经/西经,E:东经;W:西经
    uint8_t gpssta;            // GPS状态:0,未定位;1,非差分定位;2,差分定位;6,正在估算.
    uint8_t posslnum;          // 用于定位的卫星数,0~12.
    uint8_t possl[12];         // 用于定位的卫星编号
    uint8_t fixmode;           // 定位类型:1,没有定位;2,2D定位;3,3D定位
    uint16_t pdop;             // 位置精度因子 0~500,对应实际值0~50.0
    uint16_t hdop;             // 水平精度因子 0~500,对应实际值0~50.0
    uint16_t vdop;             // 垂直精度因子 0~500,对应实际值0~50.0

    uint16_t course;   // 航向
    int altitude; // 海拔高度,放大了10倍,实际除以10.单位:0.1m
    uint32_t speed;    // 地面速率,放大了1000倍,实际除以10.单位:0.001公里/小时
} nmea_msg;

//从buf里面得到第cx个逗号所在的位置
//返回值:0~0XFE,代表逗号所在位置的偏移.
//       0XFF,代表不存在第cx个逗号                
uint8_t NMEA_Comma_Pos(uint8_t *buf,uint8_t cx)
{           
  uint8_t *p=buf;
  while(cx)
  {     
    if(*buf=='*'||*buf<' '||*buf>'z')return 0XFF;//遇到'*'或者非法字符,则不存在第cx个逗号
    if(*buf==',')cx--;
    buf++;
  }
  return buf-p;   
}

//m^n函数
//返回值:m^n次方.
uint32_t NMEA_Pow(uint8_t m,uint8_t n)
{
  uint32_t result=1;   
  while(n--)result*=m;    
  return result;
}

//str转换为数字,以','或者'*'结束
//buf:数字存储区
//dx:小数点位数,返回给调用函数
//返回值:转换后的数值
/*遇到冒号以及竖杠、注释的斜杠的时候进行返回*/
int NMEA_Str2num(uint8_t *buf,uint8_t*dx)
{
  uint8_t *p=buf;
  uint32_t ires=0,fres=0;
  uint8_t ilen=0,flen=0,i;
  uint8_t mask=0;
  int res;
  while(1) //得到整数和小数的长度
  {
    if(*p=='-'){mask|=0X02;p++;}//是负数
    if(*p==','||(*p=='*')||(*p=='|')||(*p==':')\
    ||(*p=='!')  ||(*p=='/'))break;//遇到结束了
    if(*p=='.'){mask|=0X01;p++;}//遇到小数点了
    else if(*p == 0)//截至符 0
    {
      break;
    }
    else if(*p>'9'||(*p<'0'))  //有非法字符
    {  
      ilen=0;
      flen=0;
      break;
    }  
    if(mask&0X01)flen++;
    else ilen++;
    p++;
  }
  if(mask&0X02)buf++;  //去掉负号
  for(i=0;i<ilen;i++)  //得到整数部分数据
  {  
    ires+=NMEA_Pow(10,ilen-1-i)*(buf[i]-'0');//
  }
  if(flen>5)flen=5;  //最多取5位小数
  *dx=flen;       //小数点位数
  for(i=0;i<flen;i++)  //得到小数部分数据
  {  
    fres+=NMEA_Pow(10,flen-1-i)*(buf[ilen+1+i]-'0');
  } 
  res=ires*NMEA_Pow(10,flen)+fres;
  if(mask&0X02)res=-res;       
  return res;
}

//分析GNGLL信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GNGLL_Analysis(nmea_msg *gpsx,uint8_t *buf)
{
  uint8_t *p1,dx;       
  uint8_t posx;     
  uint32_t temp;     
  float rs;  
  
  p1 = (uint8_t*)strstr((const char *)buf,"GNRMC"); //GNSS
  if(p1 == NULL)
  {
    p1=(uint8_t*)strstr((const char *)buf,"GPRMC");//"$GPRMC",经常有&和GPRMC分开的情况,故只判断GPRMC.
  }
  
  posx=NMEA_Comma_Pos(p1, 1);                //得到UTC时间  hhmmss.ss
  if(posx!=0XFF)
  {
    temp=NMEA_Str2num(p1+posx,&dx)/NMEA_Pow(10,dx);     //得到UTC时间,去掉ms
    gpsx->utc.hour=temp/10000;
    gpsx->utc.min=(temp/100)%100;
    gpsx->utc.sec=temp%100;      
  }
  
  posx=NMEA_Comma_Pos(p1,2);/*判断RMC数据状态,A=数据有效 V=数据无效*/  
  if(posx!=0XFF)
  {
    uint8_t* p2=(uint8_t*)strstr((const char *)(p1+posx), "A");
    if(p2 == NULL)
    {
      posx = 0;  //数据无效 TODO
    }
  }
  
  posx=NMEA_Comma_Pos(p1,3);                //得到纬度 ddmm.mmmm
  if(posx!=0XFF)
  {
    temp=NMEA_Str2num(p1+posx,&dx);        
    gpsx->latitude=temp/NMEA_Pow(10,dx+2);  //得到°
    rs=temp%NMEA_Pow(10,dx+2);        //得到'     
    gpsx->latitude=gpsx->latitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60;//转换为° 
  }
  posx=NMEA_Comma_Pos(p1,4);                //南纬还是北纬 
  if(posx!=0XFF)
    gpsx->nshemi=*(p1+posx);           
   
  posx=NMEA_Comma_Pos(p1,5);                //得到经度 dddmm.mmmm
  if(posx!=0XFF)
  {                          
    temp=NMEA_Str2num(p1+posx,&dx);        
    gpsx->longitude=temp/NMEA_Pow(10,dx+2);  //得到°
    rs=temp%NMEA_Pow(10,dx+2);        //得到'     
    gpsx->longitude=gpsx->longitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60;//转换为° 
  }
  posx=NMEA_Comma_Pos(p1,6);                //东经还是西经
  if(posx!=0XFF)
    gpsx->ewhemi=*(p1+posx);  
   
  posx=NMEA_Comma_Pos(p1,8);                //得到方位 度
  if(posx!=0XFF)
  {                          
    temp=NMEA_Str2num(p1+posx,&dx);        
    gpsx->course = temp*10;                
  }
  
  posx=NMEA_Comma_Pos(p1, 9);                //得到UTC日期 ddmmyy
  if(posx!=0XFF)
  {
    temp=NMEA_Str2num(p1+posx, &dx);         
    gpsx->utc.date  = temp/10000;
    gpsx->utc.month = (temp/100)%100;
    gpsx->utc.year  = 2000+temp%100;      
  } 
}