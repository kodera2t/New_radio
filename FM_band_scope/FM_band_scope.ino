#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>

#define OLED_RESET 20
Adafruit_SSD1306 display(OLED_RESET);
#define RADIO 0x35
#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif
int read_byte,raw_upper,upper,lower,mode,read_byte2;
int rssi,stereo,infreq,temp;
int rssi_count;
int rssi_count2=0;
int mode_set=0;//FM
float listen_freq;
unsigned int initial_num;
unsigned int channel_num,s_upper,s_lower,s_upper2,s_lower2;
float freq, old_freq;
boolean event=0;

///// I2C command for Radio /////
void i2c_write(int device_address, int memory_address, int value, int value2)
{
  Wire.beginTransmission(device_address);
  Wire.write(memory_address);
    delay(1);
  Wire.write(value);
    delay(1);
  Wire.write(value2);
    delay(1);
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

void setup() {
  Wire.begin() ;
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);   
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.clearDisplay();

/// radio initialization /////
    temp=0;
    i2c_write(RADIO,0x04,0b01100000,0b10110000);
    i2c_write(RADIO,0x05,0b00010000,0b00100000);   
    i2c_write(RADIO,0x0F,0b10001000,0b00011100);
    i2c_write(RADIO,0x2E,0b00101000,0b10001100);
    i2c_write(RADIO,0x22,0b10100010,0b11001100);
    i2c_write(RADIO,0x33,0b11010100,0b00000001);
///digital volume control enable
    i2c_write(RADIO,0x1D,0b00000000,0b00000100); 
    mode_set==0;
    //display.print("This is test");
    display.display();

}

void loop() {
  int x,y,freq_count;
  display.setCursor(0,56);
  display.print("76");
  display.setCursor(106,56);
  display.print("108"); 
  display.setCursor(56,56);
  display.print("92");
  display.setCursor(27,56);
  display.print("84");
  display.setCursor(77,56);
  display.print("100");    
  for(freq_count=760;freq_count<1080;freq_count=freq_count+3){
  FM_recep(freq_count);
  rssi_read();
  x=(freq_count-760)/3;
  y=(-rssi)*2-16;
  if(y>54){
    y=54;
  }
  display.drawLine(x+8,54,x+8,y,WHITE);
  //display.fillRect(100,32,16,16, BLACK);
  //  display.display();
  //display.setCursor(100,32);
  //display.print(rssi);
  display.display();
  delay(70);
  }
  display.clearDisplay();
  //if
}

void rssi_read(){
  int pll_lock,old_rssi;
  if(mode_set==0){
//// display RSSI  every seconds. 
//  rssi_count=millis()-1000*rssi_count2; 
 // if(rssi_count>1000){
 //   rssi_count2++;
  i2c_read(RADIO,0x12);
  old_rssi=rssi;
  rssi=-100+(read_byte2>>3)*3;
  pll_lock=(read_byte&0b00001100);
  stereo=read_byte&0b00000011;
//  if(stereo==0b11){
//  display.setCursor(72,8);
//  display.print("stereo");
//  display.display();
//  }else{
//  display.setCursor(32,48);
//  display.print("      ");
//  display.display();
//  }
//  if(pll_lock!=12){
//  display.setCursor(32,48);
//  display.print("PLL UNLOCK"); 
//  display.display(); 
//  }
//  if(old_rssi != rssi){
//  display.fillRect(0,16,100+old_rssi,8,BLACK);
//  display.display();
//  display.setTextColor(WHITE); 
//  display.fillRect(0,16,100+rssi,8,WHITE);
//  display.display();
//  }
//    rssi_count=0;
  //}
}
}

void FM_recep(int FM_freq){

    i2c_write(RADIO,0x16,0b00000000,0b00000010);
//        i2c_write(RADIO,0x0F,0b10001000,0b00010000);
    i2c_write(RADIO,0x0F,0b10001000,0b00010000);
///digital volume control enable
    i2c_write(RADIO,0x1D,0b00000000,0b00000100);   
  listen_freq=float(FM_freq/10.0);  
  initial_num=listen_freq*20.0;
//  channel_num=initial_num+encorder_val*2;
  channel_num=initial_num;
  s_upper2=(channel_num>>8 | 0b10000000);
  s_lower2=channel_num&0b11111111;
  i2c_write(RADIO,0x02,0b00000000,0b00000111);
  i2c_write(RADIO,0x03,s_upper2,s_lower2);
  //display.fillRect(0,8,128,64, BLACK);
  //display.setCursor(0,8);
  //display.print("FM");
  freq=channel_num/20.0;
  //display.setCursor(16,0);
 /// display.print(encorder_val);
  
 //display.print(freq);
 //display.setCursor(48,0);
 //display.print("MHz");
   //display.display();
  event=0;  
}

