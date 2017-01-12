#include <MenuSystem.h> 
#include <Wire.h> 
#include <SPI.h> 
#include <Adafruit_GFX.h> 
#include <Adafruit_SSD1306.h> 

//Пины 
#define OLED_MOSI   9 
#define OLED_CLK   10 
#define OLED_DC    11 
#define OLED_CS    12 
#define OLED_RESET 13 
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS); 

// Menu variables 
MenuSystem ms; 
Menu mm(""); 
Menu mu1("Display"); 
MenuItem mu1_mi1("Display1"); 
MenuItem mu1_mi2("Display2"); 
MenuItem mu1_mi3("Display3"); 
Menu mu2("Settings"); 
MenuItem mu2_mi1("Settings1"); 
MenuItem mu2_mi2("Settings2"); 
MenuItem mu2_mi3("Settings3"); 

// Объявления переменных и констант 
const int buttonPin = 0;   // аналоговый вход для кнопок 

int buttonValue = 0;   // переменная для значения АЦП кнопки 
int UpButtonState = 0;     // переменная для кнопки вверх 
int DownButtonState = 0;   // переменная для кнопки вниз 
int SelectButtonState = 0; // переменная для кнопки выбора 
int BackButtonState = 0;   // переменная для кнопки возврата 

bool bRanCallback = false; 
bool bForward = true; 

int line = 30;             // переменная для установки настроек в линию 


// Функция меню обратного вызова 
// В этом примере все пункты меню можно использовать для обратного вызова. 

void on_display1_selected(MenuItem* p_menu_item) 
{ 
  Serial.println("DISPLAY1 Selected"); 
  display.setCursor(0,55); 
  display.print("DISPLAY1 Selected"); 
  bRanCallback = true; 
  bForward = true; 
} 
void on_display2_selected(MenuItem* p_menu_item) 
{ 
  Serial.println("DISPLAY2 Selected"); 
  display.setCursor(0,55); 
  display.print("DISPLAY2 Selected"); 
  //bRanCallback = false; 
  bForward = true; 
} 
void on_display3_selected(MenuItem* p_menu_item) 
{ 
  Serial.println("DISPLAY3 Selected"); 
  display.setCursor(0,55); 
  display.print("DISPLAY3 Selected"); 
  bRanCallback = false; 
  bForward = true; 
} 

void on_settings1_selected(MenuItem* p_menu_item) 
{ 
  Serial.println("SETTINGS1 Selected"); 
  display.setCursor(0,55); 
  display.print("SETTINGS1 Selected"); 
  bRanCallback = true; 
  bForward = true; 
} 
void on_settings2_selected(MenuItem* p_menu_item) 
{ 
  Serial.println("SETTINGS2 Selected"); 
  display.setCursor(0,55); 
  display.print("SETTINGS2 Selected"); 
  bRanCallback = false; 
  bForward = true; 
} 
void on_settings3_selected(MenuItem* p_menu_item) 
{ 
  Serial.println("SETTINGS3 Selected"); 
  display.setCursor(0,55); 
  display.print("SETTINGS3 Selected"); 
  //bRanCallback = false; 
  bForward = true; 
} 

// Standard arduino functions 

void setup() 
{ 
  Serial.begin(9600); 
   
// by default, we'll generate the high voltage from the 3.3v line internally! (neat!) 
  display.begin(SSD1306_SWITCHCAPVCC); 
   
   
  display.display();         // show splashscreen 
  delay(1000); 
  display.clearDisplay();   // clears the screen and buffer 
   
// Menu setup 
  mm.add_menu(&mu1); 
  mu1.add_item(&mu1_mi1, &on_display1_selected); 
  mu1.add_item(&mu1_mi2, &on_display2_selected); 
  mu1.add_item(&mu1_mi3, &on_display3_selected); 
  mm.add_menu(&mu2); 
  mu2.add_item(&mu2_mi1, &on_settings1_selected); 
  mu2.add_item(&mu2_mi2, &on_settings2_selected); 
  mu2.add_item(&mu2_mi3, &on_settings3_selected); 
  ms.set_root_menu(&mm); 
} 

void loop() 
{ 
  buttonValue = analogRead(buttonPin); 
  Serial.println(buttonValue); 
   
//OLED set up 
  display.display();          
  display.clearDisplay(); 
  display.setTextSize(1); 
  display.setTextColor(WHITE); 
  line=15; //line variable reset 
  //Serial.println(""); 
   
   
// Display Title 

  display.setCursor(10,0); 
  display.println("MENU CONFIGURATION"); 
  display.setCursor(0,5); 
  display.print("_____________________"); 
  display.setCursor(110,15); 
  
// Display the menu 
  Menu const* cp_menu = ms.get_current_menu(); 
  MenuComponent const* cp_menu_sel = cp_menu->get_selected(); 
  for (int i = 0; i < cp_menu->get_num_menu_components(); ++i) 
  { 
    MenuComponent const* cp_m_comp = cp_menu->get_menu_component(i); 
    //Serial.println(cp_m_comp->get_name()); 
    display.setCursor(10,line); 
    display.print(cp_m_comp->get_name()); 
     
    if (cp_menu_sel == cp_m_comp){ 
      //Serial.print("<<< "); 
      display.setCursor(0,line); 
      display.write(16); 
    } 
    line=line+10; 
    //Serial.println(""); 
  }  
  
// read the state of the pushbutton value: 
  UpButtonState = buttonValue; 
  if (UpButtonState > 830 && UpButtonState < 840) {     
    ms.prev();  
  }  
   
  DownButtonState = buttonValue; 
  if (UpButtonState > 860 && UpButtonState < 870) {    
    ms.next();   
  }  

  SelectButtonState = buttonValue; 
  if (UpButtonState > 480 && UpButtonState < 490) {    
    ms.select();   
  }  
   
  BackButtonState = buttonValue; 
  if (UpButtonState > 319 && UpButtonState < 325) {    
    ms.back();   
  }  
  
   
// Wait for two seconds so the output is viewable 
  delay(200); 
}  
