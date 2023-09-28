#include <esp_now.h>
#include <WiFi.h>

#define  BUTTON_PIN_BITMASK 0x10E000010
 
// Variables for test data
int nivel;
int bateria;
int battSwitch =  33;
int battReading = 34;
 
// MAC Address of responder
uint8_t broadcastAddress[] = {0xE0, 0x5A, 0x1B, 0xC8, 0x30, 0xCC};
 
// Define a data structure
typedef struct struct_message {
  char a[32];
  int b;
  int c;
} 
struct_message;
 
// Create a structured object
struct_message myData;
 
esp_now_peer_info_t peerInfo;
 

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
 
void setup() {

  digitalWrite(LED_BUILTIN, HIGH); 
  pinMode(27, INPUT_PULLUP);//27
  pinMode(26, INPUT_PULLUP);//
  pinMode(4, INPUT_PULLUP);//4
  pinMode(32, INPUT_PULLUP);//32
  pinMode(25, INPUT_PULLUP);

  //print_wakeup_reason();

// sensado de bateria ubicacion temporal

  pinMode(battSwitch, OUTPUT); 
  
  
  Serial.begin(115200);
 
  // Set ESP32 as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
 
  // Initilize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
 
  // Register the send callback
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 1;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}
 
void loop() {
 
  structured_DATA();
  //delay(1000);
  discriminante();
  delay(1500);
  
  esp_deep_sleep_start();
  
}

void lectura_NIVEL(){
  
  digitalWrite(battSwitch, HIGH);
  
  bateria = map(analogRead(battReading),3300, 4095, 0, 1000);
  int nivelCERO = digitalRead(27);
  int nivelBAJO = digitalRead(26);
  int nivelMEDIO = digitalRead(4);
  int nivelALTO = digitalRead(32);
  int nivelDESBORDE = digitalRead(25);
   
  if (nivelCERO==1&&nivelBAJO==1&&nivelMEDIO==1&&nivelALTO==01&&nivelDESBORDE==0){

    nivel=100;    
  }
  else if (nivelCERO==1&&nivelBAJO==1&&nivelMEDIO==1&&nivelALTO==0&&nivelDESBORDE==0){

    nivel=75;    
  }
  else if (nivelCERO==1&&nivelBAJO==1&&nivelMEDIO==0&&nivelALTO==0&&nivelDESBORDE==0){

    nivel=50;    
  }
  else if (nivelCERO==1&&nivelBAJO==0&&nivelMEDIO==0&&nivelALTO==0&&nivelDESBORDE==0){

    nivel=25;    
  }
  else if (nivelCERO==0&&nivelBAJO==0&&nivelMEDIO==0&&nivelALTO==0&&nivelDESBORDE==0){

    nivel=0;    
  }
   else if (nivelCERO==1&&nivelBAJO==1&&nivelMEDIO==1&&nivelALTO==1&&nivelDESBORDE==1){

    nivel=550;    
  } 
  /*   
   else {
    nivel=440;
  }
  */
  
 if (nivel==440){
  Serial.println("ERROR EN SENSOR");
  Serial.println("REVISE SONDA MANUALMENTE");    
   }
  if (nivel==550){
  Serial.println("DESBORDAMIENTO!!!");    
   }
 else{
  Serial.print(nivel);
  Serial.println("%");
   }
} 

void structured_DATA(){

  lectura_NIVEL();
  Serial.println(bateria);
 //bateria = random(1,100);
 
    // Format structured data
  strcpy(myData.a, "NODO ELECTRONIVEL R4");
  myData.b = nivel;
  myData.c = bateria;
  //myData.c = random(0,1000);
  
  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
  if (result == ESP_OK) {
    Serial.println("Sending confirmed");
  }
  else {
    Serial.println("Sending error");
  }
  }


  void discriminante()
{
  if(nivel == 0)
    {
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_27,1);                    //Wake up por puerto 27 cambio a 1
    }
  if(nivel == 25)
    {
    esp_sleep_enable_ext1_wakeup(0x8000000,ESP_EXT1_WAKEUP_ALL_LOW);   //Wake up por puerto 27 cambio a 0
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_26,1);                     //Wake up por puerto 26 cambio a 1
    }
  if(nivel == 50)
    {
    esp_sleep_enable_ext1_wakeup(0x4000000,ESP_EXT1_WAKEUP_ALL_LOW);      //Wake up por puerto 26 cambio a 0
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_4,1);                     //Wake up por puerto 4 cambio a 1
    }
  if(nivel == 75)
    {
    esp_sleep_enable_ext1_wakeup(0x10,ESP_EXT1_WAKEUP_ALL_LOW);     //Wake up por puerto 4 cambio a 0
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_32,1);                     //Wake up por puerto 32 cambio a 1
    }
  if(nivel == 100)
    {
    esp_sleep_enable_ext1_wakeup(0x100000000,ESP_EXT1_WAKEUP_ALL_LOW);     //Wake up por puerto 32 cambio a 0
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_25,1);                     //Wake up por puerto 25 cambio a 1
    }
  if(nivel == 550)
    {
    esp_sleep_enable_ext1_wakeup(0x2000000,ESP_EXT1_WAKEUP_ALL_LOW);     //Wake up por puerto 25 cambio a 0
    }

}

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}
