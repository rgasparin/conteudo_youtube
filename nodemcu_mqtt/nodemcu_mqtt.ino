#include <ESP8266WiFi.h> //lib do wifi para o ESP8266
//#include <ESP8266WiFiMulti.h> //lib do wifi para o ESP8266
#include <ArduinoOTA.h> //lib do ArduinoOTA
#include <PubSubClient.h>

#define ledVerde 16 //este led sinaliza que o ESP foi conectado
#define ledVermelho 5 //este led é usado como exemplo de atualização


//ESP8266WiFiMulti wifiMulti;

const char* ssid = ""; //nome da rede
const char* password = ""; //senha da rede

const int sensorMag = 4;
const int PIN_BUZZER = 14;


//SERVIDOR MQTT
const char* mqttServer = "";   //server
const char* mqttUser = "";              //user
const char* mqttPassword = "";      //password
const int mqttPort = 1883;                     //port
const char* mqttTopicSub ="nodemcu/portao";            //tópico que sera assinado

WiFiClient espClient;
PubSubClient client(espClient);

void setup()
{
   //define pino como saída
   pinMode(ledVerde,OUTPUT);

   pinMode(sensorMag, INPUT_PULLUP); 
   
   //define pino como saída
   //código da atualização 
   pinMode(ledVermelho,OUTPUT);

   pinMode(PIN_BUZZER, OUTPUT);
   
   //inicia serial com 115200 bits por segundo
   Serial.begin(115200);
   
   Serial.println("Booting");   
   
   WiFi.begin(ssid, password); 
   
   Serial.println("Connecting ...");
   
   while (WiFi.status() != WL_CONNECTED)
   {       
       delay(250);
       Serial.print('.');
   }

  // A porta fica default como 8266
  // ArduinoOTA.setPort(8266);

  // Define o hostname (opcional)
  ArduinoOTA.setHostname("myesp8266");

  // Define a senha (opcional)
  ArduinoOTA.setPassword("12345678");

  // É possível definir uma criptografia hash md5 para a senha usando a função "setPasswordHash"
  // Exemplo de MD5 para senha "admin" = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  //define o que será executado quando o ArduinoOTA iniciar
  ArduinoOTA.onStart([](){ startOTA(); }); //startOTA é uma função criada para simplificar o código 

  //define o que será executado quando o ArduinoOTA terminar
  ArduinoOTA.onEnd([](){ endOTA(); }); //endOTA é uma função criada para simplificar o código 

  //define o que será executado quando o ArduinoOTA estiver gravando
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) { progressOTA(progress,total); }); //progressOTA é uma função criada para simplificar o código 

  //define o que será executado quando o ArduinoOTA encontrar um erro
  ArduinoOTA.onError([](ota_error_t error){ errorOTA(error); });//errorOTA é uma função criada para simplificar o código 
  
  //inicializa ArduinoOTA
  ArduinoOTA.begin();

  //exibe pronto e o ip utilizado pelo ESP
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  digitalWrite(ledVerde,HIGH);
  digitalWrite(ledVermelho,LOW);

  client.setServer(mqttServer, mqttPort);

  while (!client.connected()) {
    if (client.connect("ESP8266Client", mqttUser, mqttPassword )) {
       Serial.println("Conectado");
    } else {
      Serial.print("falha estado  ");
      Serial.print(client.state());
      delay(2000);
    }
  }
  
  
}

//#######################################################################################

//funções de exibição dos estágios de upload (start, progress, end e error) do ArduinoOTA
void startOTA()
{
   String type;
   
   //caso a atualização esteja sendo gravada na memória flash externa, então informa "flash"
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "flash";
    else  //caso a atualização seja feita pela memória interna (file system), então informa "filesystem"
      type = "filesystem"; // U_SPIFFS

    //exibe mensagem junto ao tipo de gravação
    Serial.println("Start updating " + type);
}

//exibe mensagem
void endOTA()
{
  Serial.println("\nEnd");
}

//exibe progresso em porcentagem
void progressOTA(unsigned int progress, unsigned int total)
{
   Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
}

//caso aconteça algum erro, exibe especificamente o tipo do erro
void errorOTA(ota_error_t error)
{  
      Serial.printf("Error[%u]: ", error);
      
      if (error == OTA_AUTH_ERROR) 
        Serial.println("Auth Failed");
      else
      if (error == OTA_BEGIN_ERROR)
        Serial.println("Begin Failed");
      else 
      if (error == OTA_CONNECT_ERROR)
        Serial.println("Connect Failed");
      else
      if (error == OTA_RECEIVE_ERROR) 
        Serial.println("Receive Failed");
      else 
      if (error == OTA_END_ERROR)
        Serial.println("End Failed");
}

//#######################################################################################

void loop() 
{
  //Handle é descritor que referencia variáveis no bloco de memória
  //Ele é usado como um "guia" para que o ESP possa se comunicar com o computador pela rede
  ArduinoOTA.handle();

  String msgMqtt = "OFF";
  if(digitalRead(sensorMag) == LOW){ //SE A LEITURA DO PINO FOR IGUAL A LOW, FAZ
      digitalWrite(ledVermelho, LOW);  //APAGA O LED
  }else{ //SENÃO, FAZ
    digitalWrite(ledVermelho, HIGH); //ACENDE O LED
    msgMqtt = "ON";
  }

  client.publish(mqttTopicSub, msgMqtt.c_str(), true);
  delay(1000);
}
