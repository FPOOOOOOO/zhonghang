#include "GPS.h"

// GPS NMEA-0183Э����Ҫ�����ṹ�嶨��
// ������Ϣ
__packed typedef struct
{
    uint8_t num;     // ���Ǳ��
    uint8_t eledeg;  // ��������
    uint16_t azideg; // ���Ƿ�λ��
    uint8_t sn;      // �����
} nmea_slmsg;

// UTCʱ����Ϣ
__packed typedef struct
{
    uint16_t year; // ���
    uint8_t month; // �·�
    uint8_t date;  // ����
    uint8_t hour;  // Сʱ
    uint8_t min;   // ����
    uint8_t sec;   // ����
} nmea_utc_time;

// NMEA 0183 Э����������ݴ�Žṹ��
__packed typedef struct
{
    uint8_t svnum;             // �ɼ�������
    nmea_slmsg slmsg[12]; // ���12������
    nmea_utc_time utc;    // UTCʱ��
    uint32_t latitude;         // γ�� ������100000��,ʵ��Ҫ����100000
    uint8_t nshemi;            // ��γ/��γ,N:��γ;S:��γ
    uint32_t longitude;        // ���� ������100000��,ʵ��Ҫ����100000
    uint8_t ewhemi;            // ����/����,E:����;W:����
    uint8_t gpssta;            // GPS״̬:0,δ��λ;1,�ǲ�ֶ�λ;2,��ֶ�λ;6,���ڹ���.
    uint8_t posslnum;          // ���ڶ�λ��������,0~12.
    uint8_t possl[12];         // ���ڶ�λ�����Ǳ��
    uint8_t fixmode;           // ��λ����:1,û�ж�λ;2,2D��λ;3,3D��λ
    uint16_t pdop;             // λ�þ������� 0~500,��Ӧʵ��ֵ0~50.0
    uint16_t hdop;             // ˮƽ�������� 0~500,��Ӧʵ��ֵ0~50.0
    uint16_t vdop;             // ��ֱ�������� 0~500,��Ӧʵ��ֵ0~50.0

    uint16_t course;   // ����
    int altitude; // ���θ߶�,�Ŵ���10��,ʵ�ʳ���10.��λ:0.1m
    uint32_t speed;    // ��������,�Ŵ���1000��,ʵ�ʳ���10.��λ:0.001����/Сʱ
} nmea_msg;

//��buf����õ���cx���������ڵ�λ��
//����ֵ:0~0XFE,����������λ�õ�ƫ��.
//       0XFF,�������ڵ�cx������                
uint8_t NMEA_Comma_Pos(uint8_t *buf,uint8_t cx)
{           
  uint8_t *p=buf;
  while(cx)
  {     
    if(*buf=='*'||*buf<' '||*buf>'z')return 0XFF;//����'*'���߷Ƿ��ַ�,�򲻴��ڵ�cx������
    if(*buf==',')cx--;
    buf++;
  }
  return buf-p;   
}

//m^n����
//����ֵ:m^n�η�.
uint32_t NMEA_Pow(uint8_t m,uint8_t n)
{
  uint32_t result=1;   
  while(n--)result*=m;    
  return result;
}

//strת��Ϊ����,��','����'*'����
//buf:���ִ洢��
//dx:С����λ��,���ظ����ú���
//����ֵ:ת�������ֵ
/*����ð���Լ����ܡ�ע�͵�б�ܵ�ʱ����з���*/
int NMEA_Str2num(uint8_t *buf,uint8_t*dx)
{
  uint8_t *p=buf;
  uint32_t ires=0,fres=0;
  uint8_t ilen=0,flen=0,i;
  uint8_t mask=0;
  int res;
  while(1) //�õ�������С���ĳ���
  {
    if(*p=='-'){mask|=0X02;p++;}//�Ǹ���
    if(*p==','||(*p=='*')||(*p=='|')||(*p==':')\
    ||(*p=='!')  ||(*p=='/'))break;//����������
    if(*p=='.'){mask|=0X01;p++;}//����С������
    else if(*p == 0)//������ 0
    {
      break;
    }
    else if(*p>'9'||(*p<'0'))  //�зǷ��ַ�
    {  
      ilen=0;
      flen=0;
      break;
    }  
    if(mask&0X01)flen++;
    else ilen++;
    p++;
  }
  if(mask&0X02)buf++;  //ȥ������
  for(i=0;i<ilen;i++)  //�õ�������������
  {  
    ires+=NMEA_Pow(10,ilen-1-i)*(buf[i]-'0');//
  }
  if(flen>5)flen=5;  //���ȡ5λС��
  *dx=flen;       //С����λ��
  for(i=0;i<flen;i++)  //�õ�С����������
  {  
    fres+=NMEA_Pow(10,flen-1-i)*(buf[ilen+1+i]-'0');
  } 
  res=ires*NMEA_Pow(10,flen)+fres;
  if(mask&0X02)res=-res;       
  return res;
}

//����GNGLL��Ϣ
//gpsx:nmea��Ϣ�ṹ��
//buf:���յ���GPS���ݻ������׵�ַ
void NMEA_GNGLL_Analysis(nmea_msg *gpsx,uint8_t *buf)
{
  uint8_t *p1,dx;       
  uint8_t posx;     
  uint32_t temp;     
  float rs;  
  
  p1 = (uint8_t*)strstr((const char *)buf,"GNRMC"); //GNSS
  if(p1 == NULL)
  {
    p1=(uint8_t*)strstr((const char *)buf,"GPRMC");//"$GPRMC",������&��GPRMC�ֿ������,��ֻ�ж�GPRMC.
  }
  
  posx=NMEA_Comma_Pos(p1, 1);                //�õ�UTCʱ��  hhmmss.ss
  if(posx!=0XFF)
  {
    temp=NMEA_Str2num(p1+posx,&dx)/NMEA_Pow(10,dx);     //�õ�UTCʱ��,ȥ��ms
    gpsx->utc.hour=temp/10000;
    gpsx->utc.min=(temp/100)%100;
    gpsx->utc.sec=temp%100;      
  }
  
  posx=NMEA_Comma_Pos(p1,2);/*�ж�RMC����״̬,A=������Ч V=������Ч*/  
  if(posx!=0XFF)
  {
    uint8_t* p2=(uint8_t*)strstr((const char *)(p1+posx), "A");
    if(p2 == NULL)
    {
      posx = 0;  //������Ч TODO
    }
  }
  
  posx=NMEA_Comma_Pos(p1,3);                //�õ�γ�� ddmm.mmmm
  if(posx!=0XFF)
  {
    temp=NMEA_Str2num(p1+posx,&dx);        
    gpsx->latitude=temp/NMEA_Pow(10,dx+2);  //�õ���
    rs=temp%NMEA_Pow(10,dx+2);        //�õ�'     
    gpsx->latitude=gpsx->latitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60;//ת��Ϊ�� 
  }
  posx=NMEA_Comma_Pos(p1,4);                //��γ���Ǳ�γ 
  if(posx!=0XFF)
    gpsx->nshemi=*(p1+posx);           
   
  posx=NMEA_Comma_Pos(p1,5);                //�õ����� dddmm.mmmm
  if(posx!=0XFF)
  {                          
    temp=NMEA_Str2num(p1+posx,&dx);        
    gpsx->longitude=temp/NMEA_Pow(10,dx+2);  //�õ���
    rs=temp%NMEA_Pow(10,dx+2);        //�õ�'     
    gpsx->longitude=gpsx->longitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60;//ת��Ϊ�� 
  }
  posx=NMEA_Comma_Pos(p1,6);                //������������
  if(posx!=0XFF)
    gpsx->ewhemi=*(p1+posx);  
   
  posx=NMEA_Comma_Pos(p1,8);                //�õ���λ ��
  if(posx!=0XFF)
  {                          
    temp=NMEA_Str2num(p1+posx,&dx);        
    gpsx->course = temp*10;                
  }
  
  posx=NMEA_Comma_Pos(p1, 9);                //�õ�UTC���� ddmmyy
  if(posx!=0XFF)
  {
    temp=NMEA_Str2num(p1+posx, &dx);         
    gpsx->utc.date  = temp/10000;
    gpsx->utc.month = (temp/100)%100;
    gpsx->utc.year  = 2000+temp%100;      
  } 
}