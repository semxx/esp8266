/*******************************************************************
 *  this is a basic example how to program a Telegram Bot          *
 *  using TelegramBOT library on ESP8266                           *
 *                                                                 *
 *  Open a conversation with the bot, it will echo your messages   *
 *  https://web.telegram.org/#/im?p=@EchoBot_bot                   *
 *                                                                 *
 *  written by Giacarlo Bacchio                                    *
 *******************************************************************/


#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266TelegramBOT.h>


// Initialize Wifi connection to the router
char ssid[] = "Mimimi";              // your network SSID (name)
char password[] = "panatorium";                              // your network key

// Initialize Telegram BOT
// Initialize Telegram BOT

#define BOTtoken "165672905:AAFhk3XgHITZGDA_M2XEoxAhFaOdxl1Wf6Q"
//token of TestBOT
#define BOTname "Neato"
#define BOTusername "neatobot"
TelegramBOT bot(BOTtoken, BOTname, BOTusername);

int Bot_mtbs = 1000; //mean time between scan messages
long Bot_lasttime;   //last time messages' scan has been done


void setup() {

  Serial.begin(9600);
  delay(3000);

  // attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, password) != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);

  bot.begin();      // launch Bot functionalities
}

void loop() {

  if (millis() > Bot_lasttime + Bot_mtbs)  {
    bot.getUpdates(bot.message[0][1]);   // launch aPI GetUpdates up
to xxx message
    Bot_EchoMessages();   // reply to message with Echo
    Bot_lasttime = millis();
    delay(5000);
  }
}

/********************************************
 * EchoMessages - function to Echo messages *
 ********************************************/
void Bot_EchoMessages() {

  for (int i = 1; i < bot.message[0][0].toInt() + 1; i++)      {
    String EchoResponse = bot.message[i][5];
    EchoResponse = UnicodeASCII(EchoResponse);
    bot.sendMessage(bot.message[i][4], EchoResponse, "");
  }
  bot.message[0][0] = "";   // all messages have been replied - reset
new messages
}

String UnicodeASCII(String input) {
      String s = input;
  //  Serial.print("Input value in function replace: ");
  //  Serial.println(s);
    s.replace("\\u0410", "А");
    s.replace("\\u0411", "Б");
    s.replace("\\u0412", "В");
    s.replace("\\u0413", "Г");
    s.replace("\\u0414", "Д");
    s.replace("\\u0415", "Е");
    s.replace("\\u0416", "Ж");
    s.replace("\\u0417", "З");
    s.replace("\\u0418", "И");
    s.replace("\\u0419", "Й");
    s.replace("\\u041a", "К");
    s.replace("\\u041b", "Л");
    s.replace("\\u041c", "М");
    s.replace("\\u041d", "Н");
    s.replace("\\u041e", "О");
    s.replace("\\u041f", "П");
    s.replace("\\u0420", "Р");
    s.replace("\\u0421", "С");
    s.replace("\\u0422", "Т");
    s.replace("\\u0423", "У");
    s.replace("\\u0424", "Ф");
    s.replace("\\u0425", "Х");
    s.replace("\\u0426", "Ц");
    s.replace("\\u0427", "Ч");
    s.replace("\\u0428", "Ш");
    s.replace("\\u0429", "Щ");
    s.replace("\\u042a", "Ъ");
    s.replace("\\u042b", "Ы");
    s.replace("\\u042c", "Ь");
    s.replace("\\u042d", "Э");
    s.replace("\\u042e", "Ю");
    s.replace("\\u042f", "Я");
    s.replace("\\u0430", "а");
    s.replace("\\u0431", "б");
    s.replace("\\u0432", "в");
    s.replace("\\u0433", "г");
    s.replace("\\u0434", "д");
    s.replace("\\u0435", "е");
    s.replace("\\u0436", "ж");
    s.replace("\\u0437", "з");
    s.replace("\\u0438", "и");
    s.replace("\\u0439", "й");
    s.replace("\\u043a", "к");
    s.replace("\\u043b", "л");
    s.replace("\\u043c", "м");
    s.replace("\\u043d", "н");
    s.replace("\\u043e", "о");
    s.replace("\\u043f", "п");
    s.replace("\\u0440", "р");
    s.replace("\\u0441", "с");
    s.replace("\\u0442", "т");
    s.replace("\\u0443", "у");
    s.replace("\\u0444", "ф");
    s.replace("\\u0445", "х");
    s.replace("\\u0446", "ц");
    s.replace("\\u0447", "ч");
    s.replace("\\u0448", "ш");
    s.replace("\\u0449", "щ");
    s.replace("\\u044a", "ъ");
    s.replace("\\u044b", "ы");
    s.replace("\\u044c", "ь");
    s.replace("\\u044d", "э");
    s.replace("\\u044e", "ю");
    s.replace("\\u044f", "я");
    s.replace("\\u0401", "Ё");
    s.replace("\\u0451", "ё");
  //Serial.print("Replaced: ");
  //Serial.println(s);
  return s;
}
