//// Sample program "BREAKOUT" for new Radio board
//// Jul, 2018 kodera2t
//// This program is not fully completed (it works as an example.)

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

#define SOUND 18

int terminal_1  = 10;
int terminal_2  = 12;
volatile int encorder_val=48,old_val=0;
volatile char old_state = 0;
volatile int event = 0, event2= 0;
volatile int rtc_setmode=0,old_set;
volatile unsigned long time_prev = 0, time_now;
unsigned long time_chat = 500;
volatile int sw = LOW;



//SSD_13XX oled = SSD_13XX(__CS1, __DC,__RST);


int xpos,ypos,xold,yold;
int color;


void setup() {

  randomSeed(analogRead(A0));
  pinMode(SOUND,OUTPUT);
  pinMode(2, INPUT);
  pinMode(11, INPUT);
  pinMode(terminal_1, INPUT);
  pinMode(terminal_2, INPUT);
  attachInterrupt(0,Rotary_encorder,HIGH);  
  byte direc,n_direc,o_direc;
  unsigned int i,j;
  uint8_t c;
  int error;
  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);   
  display.clearDisplay();

  display.setRotation(0);
  display.setTextSize(1);
  display.setTextColor(WHITE);


 




  xpos=48;
  display.display();
}

void loop() {

  int error,xval,yval,xad,yad;
  int xcell,ycell,drawx,drawy;
  int oldx,oldy;
  int x,numx,numy,rad,six,siy,tmpx,tmpy,xdi,ydi,newx,newy,rumx,rumy,sou,senseV,padposi,padh,padd,padw,fl;
  int count,i,blocknum,fpos,blockh,delb,blck,countx,k;
  int firstline[16];
  float blockw;
  
  six=96;
  siy=64;
  rad=2;
  sou=1;
  padh=2;
  padd=siy-2;
  padw=10;
  blocknum=15;
  blockw=6.0;
  blockh=4;
  fpos=20;
  fl=1;
  count=0;


///wall-touch check
  int left,right,up,down ; 
  double dT;
//////////////////////////  

  xold=xpos;
  yold=ypos;

//  xad=xval/600;
//  yval=yval-1100;
//  yad=yval/600;
//
//
  xpos=3*encorder_val;
//  ypos=ypos+yad;





  //xpos=encorder_val;
  oldx=xcell;
  oldy=ycell;

        numx = random(rad,six-rad);
        numy = random(rad,siy-rad);
        display.fillCircle(numx,numy,rad,BLACK);
        tmpx=numx;
        tmpy=numy;
        delay(500);
        display.clearDisplay();        
        numx = random(rad,six-rad);
        numy = random(rad,siy-rad);
        xdi=numx-tmpx;
        ydi=numy-tmpy;
        if(ydi>=0){
          ydi=1;
        } else {
          ydi=-1;
        }
        if(xdi>=0){
          xdi=1;
        } else {
          xdi=-1;
        }
        rumx = random(0,6);
        rumy = random(0,6);
        xdi=xdi+rumx-3;
        ydi=ydi+rumy-3;
        newx=tmpx;
        newy=tmpy;
        for(i=0;i<=blocknum;i++){
          firstline[i]=1;
        }

                
        do {
        x=x+1;


  xold=xpos;
  yold=ypos;

  xad=xval/200;
  yval=yval-1100;
  yad=yval/200;


  xpos=3*encorder_val;
  ypos=ypos+yad;

  if(xpos<3){
    xpos=3;
  }
  if(xpos>93){
    xpos=93;
  }





  
  oldx=xcell;
  oldy=ycell;





        if(newx>=six-rad||newx<=rad){
          rumx=random(-3,3);
          xdi=-xdi;
          ydi=rumx+ydi;
          rad=random(1,4);
          if(abs(ydi)>rad){
            ydi=3*sqrt(ydi*ydi)/ydi;
          };
        }
        if(newy<=rad){
          rumy=random(-2,2);
          ydi=-ydi;
          xdi=xdi+rumy;
          rad=random(1,4);
          if(abs(xdi)>rad){
            xdi=3*sqrt(xdi*xdi)/xdi;
          };
        }


      if((newx>six+1||newy>siy+1)||(newx<0||newy<0)){
          newx=six/2;
          newy=siy/3;
        }
        newx=newx+xdi;
        newy=newy+ydi;
        padposi=xpos;
          if(newy>62){
          if(newx>padposi && newx<padposi+padw){
            ydi=-ydi;
          fl=1;
          for (sou=1;sou<=80;sou++){
            digitalWrite(SOUND, HIGH);
            delayMicroseconds(400);
            digitalWrite(SOUND, LOW);            
          }
          } else {
            count=0;
            fl=0;
          }
          }
          
          if((newy>fpos)&&(newy<fpos+blockh)){
            delb=newx/blockw+1;
            blck=firstline[delb];
          if(blck==1){
            ydi=-ydi;
            firstline[delb] =0;
            count=count+1;
            for (sou=1;sou<=80;sou++){
            digitalWrite(SOUND, HIGH);
            delayMicroseconds(800);
            digitalWrite(SOUND, LOW);            
          }
          }
          }          
          
//// drawing block ////
        for( i=1;i<=blocknum;i++){
          if(firstline[i]==1){
          display.drawRect((i-1)*blockw,fpos,blockw,blockh,WHITE);
            //display.display();
          }
        }
        k=0;
        for( i=1;i<=blocknum;i++){
          if(firstline[i]==0){
          k=k+1;
          }
        }
        if(k==16){
          fl=0;
        }
        display.fillRect(padposi,padd-padh,padw,padd,WHITE);  
        display.fillCircle(newx,newy,rad,WHITE);
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(80,10);
        display.print(count);
          display.display();
        delay(60);
        display.clearDisplay();
        if(count==15){
          //game_clear();
                    for (sou=1;sou<=500;sou++){
            digitalWrite(SOUND, HIGH);
            delayMicroseconds(200);
            digitalWrite(SOUND, LOW);            
          }
        fl=0;
        }        
        } while(fl==1);
          display.display();
}








//void game_over(void){
//  display.fillScreen(BLACK);
//    display.setTextSize(2);  
//    display.setTextColor(WHITE);
//    display.setCursor(20,20);
//    display.print("GAME");
//    display.setCursor(20,40);
//    display.print("OVER!"); 
//    //display.display();   
//    xpos=0;
//    ypos=0;
//    color=1;
//    //display.display();
//    delay(2000);
//}

//void game_clear(void){
//  display.fillScreen(WHITE);
//    display.setTextSize(2);  
//    display.setTextColor(BLACK);
//    display.setCursor(20,20);
//    display.print("GAME");
//    display.setCursor(20,40);
//    display.print("CLEAR");    
//    xpos=0;
//    ypos=0;
//    color=1;
//    delay(3000);
//    //display.display();
//}

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

