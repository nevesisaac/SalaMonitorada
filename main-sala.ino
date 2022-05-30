#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <WiFiClient.h>
#include <WiFi.h>
#include <Arduino.h>

#include <ThingSpeak.h>
#include "DHT.h"

#include <MFRC522.h>
#include <SPI.h>

#define DHTPIN 33     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 22  (AM2302), AM2321
  
#define SS_PIN 5 // rfid
#define RST_PIN 27 //rfid
#define LIVREPINO 32 //pino de acesso
#define NEGADOPINO 21 // pino de acesso
#define WIFI_NETWORK "YourNET"           
#define WIFI_PASSWORD "YourPassword"    
#define server "https://api.thingspeak.com/channels/1675434/feeds.json?metadata=true&api_key=SXPGWJBN6UVI7N79"

MFRC522 mfrc522(SS_PIN, RST_PIN);

WiFiClient client; 

DHT dht(DHTPIN, DHTTYPE);

HTTPClient http;
 
const long channelID = 1675434;
const char *WriteAPIKey = "E0KY94EHCR6P0NSP";
const char *channelReadKey = "SXPGWJBN6UVI7N79";

unsigned  long tempo; 
int ocupantes = 0; 
int reservado = 0;
String cartoesCadastrados = "";
String cartoesAutorizados = "";

void setup() {
  Serial.begin(9600); // inicia serial para testes
  
  WiFi.begin(WIFI_NETWORK,WIFI_PASSWORD); // inicia WIFI

  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay (500);
  }
  
  Serial.println("conectado");
  
  SPI.begin(); //Inicia conexões SPI
  mfrc522.PCD_Init();// Inicia cartão RFID
  
  dht.begin(); //Inicia Sensor

  ThingSpeak.begin(client); //Inicia Client do ThingSpeak (nuvem)

  carregaCadastrados(); //Carrega os  cartões cadastrados que estão indicados na nuvem
  
  pinMode(LIVREPINO,OUTPUT); //Identifica o LED de acesso
  pinMode(NEGADOPINO, OUTPUT); //Identifica o LED de negação
  
  ledcSetup(0,600,12);
  ledcAttachPin(25,0); // pino do buzzer e canal (identific funcionamento do Buzzer)
}

// Essa função é responsável por ler os metadados do canal onde
//os cartões são cadastrados e descadastrados e  carregar na ESP
// Por meio de uma requisição HTTP.

void carregaCadastrados(){ 

 String path = "https://api.thingspeak.com/channels/1675434/feeds.json?metadata=true&api_key=SXPGWJBN6UVI7N79";
 http.begin(path);
 int httpCode = http.GET();
  
 String payload = "{}"; 
  
 if (httpCode>0) {
  Serial.print("HTTP Response code: ");
  Serial.println(httpCode);
  payload = http.getString();
  }
 else {
  Serial.print("Error code: ");
  Serial.println(httpCode);
  }
  // Free resources
 http.end();
 JSONVar myObject = JSON.parse(payload);
 if (JSON.typeof(myObject) == "undefined") {
  Serial.println("Parsing input failed!");
  return;
  }
 cartoesCadastrados = myObject["channel"]["metadata"];
 
 Serial.println(cartoesCadastrados);
}

// Envia os dados adquridos para a nuvem

void enviaDados(){ 
  Serial.println("funcionaaaa");
  // Configura os campos com os valores
  if(isnan(dht.readTemperature()) || isnan(dht.readHumidity())){
    Serial.println("error no sensor");
  }
  else{
   ThingSpeak.setField(1,dht.readTemperature());
   ThingSpeak.setField(2,dht.readHumidity());
  }
  ThingSpeak.setField(3,ocupantes);
  ThingSpeak.setField(4,reservado);
  
  Serial.println(dht.readTemperature());
 
  int x = ThingSpeak.writeFields(channelID,WriteAPIKey );
  if (x == 200) {
    Serial.println("Update realizado com sucesso");
  }
  else {
    Serial.println("Problema no canal - erro HTTP " + String(x));
  }

  Serial.println("dados enviador");

return;
}

// Checa se alguem está utilizando o sistema RFID e permite ou não a sua entrada

void RFID(){
  
  String cartao1 = "";
  
  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    delay(1);
    return;
  }
  // Seleciona um dos cartoes
  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    delay(1);
    return;
  }
  Serial.print(F("Card UID:"));
  
  for (byte i = 0; i < mfrc522.uid.size; i++) {  //Fazendo a leitura do cartão e  salvando na variável cartao1
   Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
   Serial.println(mfrc522.uid.uidByte[i], HEX);
    
   cartao1.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
   cartao1.concat(String(mfrc522.uid.uidByte[i], HEX));
   }
   
  cartao1 = cartao1 + " ;";
  Serial.println(cartao1);
  delay(350);
  // Checa se o cartao apresentado é um cartao cadastrado e se ele já entrou na sala
  // No primeiro caso identifica uma pessoa que já entrou na sala
  
  if(cartoesCadastrados.indexOf(cartao1) >=0 &&  cartoesAutorizados.indexOf(cartao1) >= 0){ 

    Serial.println("Volte sempre");
    ocupantes = ocupantes - 1;
    cartoesAutorizados.replace(cartao1,""); 
    Serial.println("cartoes Autorizados:");
    Serial.println(cartoesAutorizados);
    digitalWrite(LIVREPINO,HIGH);
    delay(300);
    digitalWrite(LIVREPINO,LOW);
     
    // retirar cartao1 de cartões autorizados


    //controle de uma pessoa específica que foi cadastrada
    if(cartao1 == " 5c c5 61 83 ;"){
      reservado = 0;
    }
    
  // Nesse caso uma pessoa que está entrando na sala e é autorizada
    
  }
  else if(cartoesCadastrados.indexOf(cartao1) >=0 && cartoesAutorizados.indexOf(cartao1) <= 0){
    Serial.println("Acesso liberado");
    
    ocupantes = ocupantes + 1;
    cartoesAutorizados = cartoesAutorizados + cartao1;
    Serial.println(" crtoes auttorizados:");
    Serial.println(cartoesAutorizados);
    // Adiciona cartao1 à cartoes autorizados
    digitalWrite(LIVREPINO,HIGH);
    delay(300);
    digitalWrite(LIVREPINO,LOW);

    //controle de uma pessoa específica
    if(cartao1 == " 5c c5 61 83 ;"){
      reservado = 1;
    }  
  
  }
  
  // No último caso uma pessoa não autorizada tenta entrar na sala e é  negada
  else{
    Serial.println("acesso negado");
    digitalWrite(NEGADOPINO,HIGH);
    delay(300);
    digitalWrite(NEGADOPINO,LOW);
  } 
}


// Essa função é um sistema quee checa limitações da sala, se essas limitações forem
// ultrapassadas o buzzer é acionado.

void acionaAlarme(){
  
  if(isnan(dht.readTemperature()) || isnan(dht.readHumidity())){
    Serial.println("error no sensor");
    delay (50);
    return;
  }
  
  if((dht.readTemperature()> 25.00) || (dht.readTemperature()< 20.00)){
   ledcWriteTone(0,800);
   delay(2000);
   ledcWriteTone(0,0);
   delay(50);
  }
  if((dht.readHumidity()> 95.00) || (dht.readHumidity()< 40.00)){  
   ledcWriteTone(0,800);
   delay(1500); 
   ledcWriteTone(0,0);
   delay(50);
   }
}

// MAIN onde as funções são chamadas

void loop() {
  if (client.connect(server,80)){
    Serial.println("conectado!");
  }
  
 tempo = millis();
 RFID();
 
 if(tempo % 20000 <= 60  ){ //thingspeak precisa de 15 segundos de delay para enviar;
   carregaCadastrados();
   enviaDados(); //alterad para enviar em intervalos e tempo definidos
   Serial.println("pagode");
   tempo = 0;
 }

 if(tempo%300 == 0){
  acionaAlarme();
 }
}
