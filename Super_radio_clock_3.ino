/*********************************************************************
Firmware program for my new radio system, available at
https://www.tindie.com/products/microwavemont/dsp-radio-chip-based-lw-mw-sw-fm-receiver/
This program is released "as is" and without any warranty.
*********************************************************************/

#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <FaBoRTC_PCF2129.h>
#include <EEPROM.h>

#define OLED_RESET 20
Adafruit_SSD1306 display(OLED_RESET);
FaBoRTC_PCF2129 faboRTC;
#define RADIO 0x35
#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

File myFile;
char read_char;
int current_hour,count;

int terminal_1  = 10;
int terminal_2  = 12;
volatile int encorder_val,old_val=0;
volatile char old_state = 0;
volatile int event = 0, event2= 0;
volatile int rtc_setmode=0,old_set;
volatile unsigned long time_prev = 0, time_now;
unsigned long time_chat = 500;
volatile int sw = LOW;
int s_year, s_month, s_day, s_hour, s_min, utcpm, disp_h,disp_d,disp_m, ee_utcpm,chstp;
int channel_s;
int disp_min;
int setting_mode=0,timeset_done=0;

////////// radio related parameters///////////
int read_byte,raw_upper,upper,lower,mode,read_byte2;
    int rssi,stereo,infreq;
float freq, old_freq;
unsigned int channel_num,s_upper,s_lower;
unsigned char s_upper2, s_lower2, s_upper3;
unsigned int initial_num;
volatile int mode_set=0; /// mode_set=0:AM, mode_set=1:FM
volatile int band_mode = LOW;
float listen_freq;
int ct,pt;
int rssi_count;
int rssi_count2=0;
//int current_hour;
boolean first_table=0;

////////radio related function//////
void i2c_write(int device_address, int memory_address, int value, int value2)
{
  Wire.beginTransmission(device_address);
  Wire.write(memory_address);
    delay(5);
  Wire.write(value);
    delay(5);
  Wire.write(value2);
    delay(5);
  Wire.endTransmission();
}

void i2c_read(int device_address, int memory_address)
{
Wire.beginTransmission(device_address);
Wire.write(memory_address);
Wire.endTransmission(false);
Wire.requestFrom(device_address, 2);
read_byte = Wire.read();
//Wire.requestFrom(device_address, 1);
read_byte2 = Wire.read();
Wire.endTransmission(true);
//delay(30);
}



void setup()   {                
    Wire.begin() ;
    faboRTC.searchDevice();
  attachInterrupt(0,Rotary_encorder,HIGH);
  attachInterrupt(1,mode_setting,LOW);
  attachInterrupt(2,rtc_setting,HIGH);
  pinMode(2, INPUT);
  pinMode(11, INPUT);
  pinMode(terminal_1, INPUT);
  pinMode(terminal_2, INPUT);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); 
  
  //display.display();
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.clearDisplay();
  rtc_setmode=0;
  old_set=0;
  s_year=2018;
  s_month=6;
  s_day=28;
  s_hour=13;
  s_min=30;
  if (!SD.begin(4)) {
  display.setCursor(0,32);
    display.println("SD card not found!");
    display.display();
    //return;
  }  

////radio initialization part/////
  int temp;
    temp=0;
    i2c_write(RADIO,0x04,0b01100000,0b10110000);
    i2c_write(RADIO,0x05,0b00010000,0b00100000);   
    i2c_write(RADIO,0x0F,0b10001000,0b00011100);
    i2c_write(RADIO,0x2E,0b00101000,0b10001100);
    i2c_write(RADIO,0x22,0b10100010,0b11001100);
    i2c_write(RADIO,0x33,0b11010100,0b00000001);
///digital volume control enable
    i2c_write(RADIO,0x1D,0b00000000,0b00000100);   
encorder_val=0;
    //display.setTextSize(1);
  initial_reception();
  DateTime now = faboRTC.now();
  current_hour=now.hour();
    
}


void loop() {
  rssi_display();
  if((event==1)&&(rtc_setmode==0)){/////general radio operation start///
  if(mode_set==0){
  i2c_write(RADIO,0x16,0b00000000,0b00000010);
//        i2c_write(RADIO,0x0F,0b10001000,0b00010000);
    i2c_write(RADIO,0x0F,0b10001000,0b00010000);
///digital volume control enable
    i2c_write(RADIO,0x1D,0b00000000,0b00000100);   
  listen_freq=83.3;  
  initial_num=listen_freq*20.0;
  channel_num=initial_num+encorder_val*2;
  s_upper2=(channel_num>>8 | 0b10000000);
  s_lower2=channel_num&0b11111111;
  i2c_write(RADIO,0x02,0b00000000,0b00000111);
  i2c_write(RADIO,0x03,s_upper2,s_lower2);
  display.fillRect(0,8,128,64, BLACK);
  display.setCursor(0,8);
  display.print("FM");
  freq=channel_num/20.0;
  display.setCursor(16,8);
 /// display.print(encorder_val);
  
 display.print(freq);
 display.setCursor(48,8);
 display.print("MHz");
   display.display();
  event=0;  
  }
  else if(mode_set==1){
  i2c_write(RADIO,0x16,0b10000000,0b00000011);
//  i2c_write(RADIO,0x16,0b10000000,0b11000011);
    i2c_write(RADIO,0x22,0b10100010,0b10001100);
//  i2c_write(RADIO,0x22,0b01010100,0b00000000);
    i2c_write(RADIO,0x0F,0b10001000,0b00010000);
  listen_freq=810;  
  initial_num=listen_freq;
  channel_s=EEPROM.read(2);
      if(channel_s==9){
      channel_s=9;
    }else{
      channel_s=10;
    }
  channel_num=initial_num+encorder_val*channel_s;
  s_upper2=(channel_num>>8 | 0b10000000);
  s_lower2=channel_num&0b11111111;
  i2c_write(RADIO,0x02,0b00000000,0b00000111);

  i2c_write(RADIO,0x17,s_upper2,s_lower2);
  
  display.fillRect(0,8,128,64, BLACK);
  display.setCursor(0,8);
  display.print("AM");
  freq=channel_num;
  display.setCursor(16,8);
  display.print(freq);
  display.setCursor(64,8);
  display.print("kHz ");
   display.display();
  event=0;  
  } else if(mode_set==2){
    SW_rec(2250);
  }else if(mode_set==3){
      SW_rec(3150); 
  }else if(mode_set==4){
      SW_rec(3850);
  }else if(mode_set==5){
      SW_rec(4700);
  }else if(mode_set==6){
      SW_rec(5900);
  }else if(mode_set==7){
      SW_rec(7100);
  }else if(mode_set==8){
      SW_rec(9400);
  }else if(mode_set==9){
      SW_rec(11500);
  }else if(mode_set==10){
      SW_rec(13500);
  }else if(mode_set==11){
      SW_rec(15000);
  }else if(mode_set==12){
      SW_rec(17450);
  }else if(mode_set==13){
      SW_rec(18850);
  }else if(mode_set==14){
      SW_rec(21450);
  }else if(mode_set==15){
      SW_rec(25600);
      first_table=0;
  }
  }
  {////general radio operation stop////
  RTC_setting2();
}
  show_time();
  
    //display.display();
}

void rtc_setting(void){
  time_now = millis(); 
  if( time_now-time_prev > time_chat){
    if( sw == LOW )
     sw  = !sw;
    rtc_setmode = !rtc_setmode;
  }
  time_prev = time_now;
  event2=1;
}

void show_RTC_set_mode(void){
  if((rtc_setmode==1)){
  display.clearDisplay();
  //display.display();
  display.setCursor(0,0);
  display.print("RTC set mode (in UTC)");
    display.setCursor(0,8);
  display.print("Year: ");
  display.println(s_year); //0
  display.print("Month: ");
  display.println(s_month);//1 
  display.print("Day: ");
  display.println(s_day);//2
  display.print("Hour: ");
  display.println(s_hour); //3
  display.print("Min: ");
  display.println(s_min);  //4
  display.print("UTC +-: ");
  display.println(utcpm); //5
  display.display();
  display.print("MW Ch.step:");
  display.println(chstp); //6
  display.display();
  }else{
  display.clearDisplay();
  //display.display();
  display.setCursor(0,0);
  display.print("exit RTC set");
  display.display();  
  }
  event2=0;
}

void Rotary_encorder(void)
{
    old_val=encorder_val;
  if(!digitalRead(terminal_1)){
    if(digitalRead(terminal_2)){
      old_state = 'R';
    } else {
      old_state = 'L';
    }
  } else {
    if(digitalRead(terminal_2)){
      if(old_state == 'L'){ 
        encorder_val++;
      }
    } else {
      if(old_state == 'R'){
        encorder_val--;
      }
    }
    old_state = 0;
    event=1;
  }
}


void mode_setting(){
int sw,k;
ct=millis();
delay(1);
sw=digitalRead(3);
if(sw==LOW && (ct-pt)>50){
band_mode=HIGH;
mode_set=mode_set+1;
}
pt=ct;
if(mode_set>15){
  mode_set=0;
}
event=1;
encorder_val=0;
k=0;
setting_mode=mode_set;
}



void value_setting(void){
  //encorder_val=0;
  switch(setting_mode){
    case 0://Year
    s_year=2018+encorder_val;
    break;
    case 1://Month
    s_month=encorder_val;
    break;
    case 2://Day
    s_day=encorder_val;
    break;
    case 3://Hour
    s_hour=encorder_val;
    break;
    case 4://Min
    s_min=encorder_val;
    break;
    case 5://UTC set
    utcpm=encorder_val;
    ee_utcpm=utcpm+12;
    EEPROM.write(1, ee_utcpm);
    break;
    case 6://MW channel step setting
    chstp=encorder_val;
    EEPROM.write(2, chstp);
    break;
    case 7://Min
    timeset_done=1;
    faboRTC.setDate(s_year,s_month,s_day,s_hour,s_min,0);
    break;
    default:
    break;
  }
}

void RTC_setting2(void){
    if((event==1)&&(rtc_setmode==1)){
    //encorder_val=0;
        value_setting();
    show_RTC_set_mode();

    event=0;
  }
  if(timeset_done==1){
    faboRTC.setDate(s_year,s_month,s_day,s_hour,s_min,0);
    timeset_done=0;
    display.clearDisplay();
  }
}

void show_time(void){
boolean zeroflag1=0,zeroflag2=0;
    time_now=millis();
    utcpm=EEPROM.read(1)-12;
  if(rtc_setmode==0){
  if(time_now-time_prev > 1000){
        DateTime now = faboRTC.now();
    disp_h=now.hour()+utcpm;
    disp_min=now.minute();
    if(disp_min<10){
      zeroflag2=1;
    }    
    disp_d=now.day();
    disp_m=now.month();
    if(disp_h>24){
      disp_h=disp_h-24;
      disp_d=disp_d+1;
    }
    if(disp_h<0){
      disp_h=24+disp_h;
      disp_d=disp_d-1;
      if(disp_d<=0){
        disp_m=disp_m-1;
      }
    }
        if(disp_h<10){
      zeroflag1=1;
    } 
    display.fillRect(0,0,128,8,BLACK);
    display.setCursor(0,0);
    display.print(now.year());
    display.print(",");
    display.print(disp_m);
    display.print(",");
    display.print(disp_d);
    display.print(",");
    if(zeroflag1==1){
      display.print("0");   
    }
    display.print(disp_h);
    display.print(":");
    if(zeroflag2==1){
      display.print("0");   
    }
    display.print(now.minute());
    display.print(":");
    display.print(now.second());  
    display.display();
    time_prev = time_now;
  }
  }
}

void rssi_display(){
  int pll_lock,old_rssi;
  if(mode_set==0){
//// display RSSI  every seconds. 
  rssi_count=millis()-1000*rssi_count2; 
  if(rssi_count>1000){
    rssi_count2++;
  i2c_read(RADIO,0x12);
  old_rssi=rssi;
  rssi=-100+(read_byte2>>3)*3;
  pll_lock=(read_byte&0b00001100);
  stereo=read_byte&0b00000011;
  if(stereo==0b11){
  display.setCursor(72,8);
  display.print("stereo");
  display.display();
  }else{
  display.setCursor(32,48);
  display.print("      ");
  display.display();
  }
  if(pll_lock!=12){
  display.setCursor(32,48);
  display.print("PLL UNLOCK"); 
  display.display(); 
  }
  if(old_rssi != rssi){
  display.fillRect(0,16,100+old_rssi,8,BLACK);
  display.display();
  display.setTextColor(WHITE); 
  display.fillRect(0,16,100+rssi,8,WHITE);
  display.display();
  }
    rssi_count=0;
  }
}
}

void SW_rec(float listen_freq)
{
  if(first_table==0){
    display_timetable();
    first_table=1;
  }
  i2c_write(RADIO,0x16,0b10000000,0b00000011);
    i2c_write(RADIO,0x22,0b10100010,0b10001100);  
//  i2c_write(RADIO,0x22,0b01010100,0b00000000);
//  listen_freq=3000;  
  initial_num=listen_freq;
  channel_num=initial_num+encorder_val*5;
  s_upper2=(channel_num>>8 | 0b10000000);
  s_lower2=channel_num&0b11111111;
  i2c_write(RADIO,0x02,0b00000000,0b00000111);
  i2c_write(RADIO,0x17,s_upper2,s_lower2);
  old_freq=freq;
  //freq=channel_num;
  display.fillRect(0,8,128,8, BLACK);
  display.setCursor(0,8);
  display.print("SW");
  infreq=channel_num;
  display.setCursor(16,8);
  display.print(infreq);
  display.setCursor(48,8);
  display.print("kHz ");
  display.display();
  event=0; 
  mode=0;
}

void initial_reception(void){
    i2c_write(RADIO,0x16,0b00000000,0b00000010);
//        i2c_write(RADIO,0x0F,0b10001000,0b00010000);
    i2c_write(RADIO,0x0F,0b10001000,0b00010000);
///digital volume control enable
    i2c_write(RADIO,0x1D,0b00000000,0b00000100);   
  listen_freq=83.3;  
  initial_num=listen_freq*20.0;
  channel_num=initial_num+encorder_val*2;
  s_upper2=(channel_num>>8 | 0b10000000);
  s_lower2=channel_num&0b11111111;
  i2c_write(RADIO,0x02,0b00000000,0b00000111);
  i2c_write(RADIO,0x03,s_upper2,s_lower2);
  display.fillRect(0,8,128,64, BLACK);
  display.setCursor(0,8);
  display.print("FM");
  freq=channel_num/20.0;
  display.setCursor(16,8);
 /// display.print(encorder_val);
  
 display.print(freq);
 display.setCursor(48,8);
 display.print("MHz");
   display.display();
  event=0;
}


void display_timetable(void){
    myFile = SD.open("STATION.TXT", FILE_READ);
  display.setCursor(0,16);
  count=0;
  if (myFile) {
    while (myFile.available()) {
      read_char=myFile.read();
      if(read_char==0x0a){
        count++;
      }

      if(current_hour*6==count){
      //for(int i=0;i<6;i++){
      //Serial.write(myFile.read());
      //if(myFile.read()==0x35)
      //myFile.read();
        while (myFile.available()) {
      read_char=myFile.read();
      display.write(read_char);
      }
     
      }
            
    }
    // close the file:
    display.display();
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    display.println("No STATION.TXT found");
    display.display();    
  }
}








