//#include <DHT.h>
//#include <OneWire.h>
//#include <DallasTemperature.h>
#include <Ethernet.h>
#include <ArduinoJson.h>
//#include <Servo.h>
#include <SPI.h>
/*
// Датчики температуры почвы
#define DS18B20_1 A1
#define DS18B20_2 A2
OneWire oneWire1(DS18B20_1);
OneWire oneWire2(DS18B20_2);
DallasTemperature ds_sensor1(&oneWire1);
DallasTemperature ds_sensor2(&oneWire2);

// Датчики температуры и влажности воздуха
#define DHTPIN 9
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Датчики влажности почвы и освещенности
#define MOISTURE_1 A3
#define MOISTURE_2 A4
#define LIGHT A0


// Выход ШИМ
#define PWM_LED 4

// Сервомоторы
#define Servo_PIN 7
Servo servo_1;
*/

// Выходы реле
#define rele2 46
#define rele1 47

//Входы - выходы дальномеров
#define trigPin_1 23
#define echoPin_1 22
#define trigPin_2 33
#define echoPin_2 32

int OUTPUT_count = 4;
int OUTPUT_pin[] = {rele1, rele2, trigPin_1, trigPin_2};

// Настройки сетевого адаптера
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// Local IP if DHCP fails
IPAddress ip(192, 168, 1,65);
IPAddress dnsServerIP(192, 168, 12, 1);
IPAddress gateway(192, 168, 1,65);
IPAddress subnet(255, 255, 255, 0);
EthernetClient client;

// Частота вызова функций
#define DIGITAL_UPDATE_TIME 3000
#define ANALOG_UPDATE_TIME 1000
#define IOT_UPDATE_TIME 10

// Таймеры
long timer_digital = 0;
long timer_analog = 0;
long timer_iot = 0;

// Параметры сервера
char IPserver[] = "192.168.1.8";
IPAddress iot_address(192,168,1,4);
char appKey[] = "511d1de0-3f48-47c4-8fd6-f4760443da27";
char thingName[] = "Thing_test";
char serviceName[] = "Service_test";

int barier = 0;
// Число параметров
#define sensorCount 4
char* sensorNames[] = {"dalnomer_1", "dalnomer_2", "dalnomer_3", "moution"};
float sensorValues[sensorCount];
// Датчики
#define dalnomer_1     0
#define dalnomer_2     1
#define dalnomer_3     2
#define moution        3
/*
#define dht11_temp     0
#define dht11_hum      1
#define sensor_light   2
#define ds18b20_temp1  3
#define ds18b20_temp2  4
#define soil_moisture1 5
#define soil_moisture2 6*/

// Максимальное время ожидания ответа от сервера
#define IOT_TIMEOUT1 5000
#define IOT_TIMEOUT2 100

// Таймер ожидания прихода символов с сервера
long timer_iot_timeout = 0;

// Размер приемного буффера
#define BUFF_LENGTH 256

// Приемный буфер
char buff[BUFF_LENGTH] = "";

void setup()
{
  for (int i =0; i<sensorCount; i++)sensorValues[i]=i+3;
  // Инициализация последовательного порта
  Serial.begin(115200);
  delay(1000);
  /*
  // Инициализация датчиков температуры DS18B20
  ds_sensor1.begin();
  ds_sensor2.begin();
  // Инициализация датчика DHT11
  dht.begin();
  */


  // Инициализация выходов реле
  /*
  pinMode(nagr, OUTPUT);
  pinMode(pomp, OUTPUT);
  pinMode(light, OUTPUT);
  pinMode(RELAY4, OUTPUT);

  pinMode(trigPin, OUTPUT); 
  */
  for (int i=0; i < OUTPUT_count; i++)pinMode(OUTPUT_pin[i], OUTPUT);

  pinMode(echoPin_1, INPUT); 
  pinMode(echoPin_2, INPUT); 
  // Инициализация портов для управления сервомоторами
  //servo_1.attach(Servo_PIN);
  // Инициализация сетевой платы
  if (Ethernet.begin(mac) == 0)
  {
    Serial.println("Failed to configure Ethernet using DHCP");
    Ethernet.begin(mac, ip, dnsServerIP, gateway, subnet);
  }
  Serial.print("LocalIP: ");
  Serial.println(Ethernet.localIP());
  Serial.print("SubnetMask: ");
  Serial.println(Ethernet.subnetMask());
  Serial.print("GatewayIP: ");
  Serial.println(Ethernet.gatewayIP());
  Serial.print("dnsServerIP: ");
  Serial.println(Ethernet.dnsServerIP());
  Serial.println("");
  /*
  // Однократный опрос датчиков
  readSensorDigital();
  readSensorAnalog();
  // Установка сервомотора в крайнее положение
  servo_1.write(90);
  */
}

void loop()
{
/*
  // Опрос датчика DHT11
  if (millis() > timer_digital + DIGITAL_UPDATE_TIME)
  {
    // Опрос датчиков
    readSensorDigital();
    // Сброс таймера
    timer_digital = millis();
  }
  // Опрос аналоговых датчиков
  if (millis() > timer_analog + ANALOG_UPDATE_TIME)
  {
    // Опрос датчиков
    readSensorAnalog();
    // Сброс таймера
    timer_analog = millis();
  }
*/

  //Вывод данных в монитор порта
  for (int i=0; i<sensorCount;i++)Serial.print(String(sensorNames[i])+ " : " + String(sensorValues[i]) + " ");
  Serial.println();

  // Вывод данных на сервер IoT
  if (millis() > timer_iot + IOT_UPDATE_TIME)
  {
    // Вывод данных на сервер IoT
    readDalnomer(trigPin_1, echoPin_1, dalnomer_1);
    readDalnomer(trigPin_2, echoPin_2, dalnomer_2);
    sendDataIot();
    // Сброс таймера
    timer_iot = millis();
  }
}

/*
// Чтение датчика DHT11
void readSensorDigital()
{
  // DHT11
  sensorValues[dht11_hum] = dht.readHumidity();
  sensorValues[dht11_temp] = dht.readTemperature();

  // DS18B20
  ds_sensor1.requestTemperatures();
  sensorValues[ds18b20_temp1] = ds_sensor1.getTempCByIndex(0);

  // DS18B20
  ds_sensor2.requestTemperatures();
  sensorValues[ds18b20_temp2] = ds_sensor2.getTempCByIndex(0);
}

// Чтение аналоговых датчиков
void readSensorAnalog()
{
  // Аналоговые датчики
  sensorValues[sensor_light] = (1023.0 - analogRead(LIGHT)) / 1023.0 * 100.0;
  sensorValues[soil_moisture1] = analogRead(MOISTURE_1) / 1023.0 * 100.0;
  sensorValues[soil_moisture2] = analogRead(MOISTURE_2) / 1023.0 * 100.0;
}
*/ 

void readDalnomer (int trig_Pin, int echo_Pin, int sensor_id){
  int duration;
  digitalWrite(trig_Pin, LOW); 
  delayMicroseconds(2); 
  digitalWrite(trig_Pin, HIGH);
  delayMicroseconds(10); 
  digitalWrite(trig_Pin, LOW); 
  duration = pulseIn(echo_Pin, HIGH); 
  sensorValues[sensor_id] = duration / 58;
}

// Подключение к серверу IoT ThingWorx
void sendDataIot()
{
  // Подключение к серверу
  Serial.println("Connecting to IoT server...");
  if (client.connect(iot_address, 8080))
  {
    // Проверка установления соединения
    if (client.connected())
    {
      // Отправка заголовка сетевого пакета
      Serial.println("Sending data to IoT server...\n");
      Serial.print("POST /Thingworx/Things/");
      client.print("POST /Thingworx/Things/");
      Serial.print(thingName);
      client.print(thingName);
      Serial.print("/Services/");
      client.print("/Services/");
      Serial.print(serviceName);
      client.print(serviceName);
      Serial.print("?appKey=");
      client.print("?appKey=");
      Serial.print(appKey);
      client.print(appKey);
      Serial.print("&method=post&x-thingworx-session=true");
      client.print("&method=post&x-thingworx-session=true");
      // Отправка данных с датчиков
      for (int idx = 0; idx < sensorCount; idx ++)
      {
        Serial.print("&");
        client.print("&");
        Serial.print(sensorNames[idx]);
        client.print(sensorNames[idx]);
        Serial.print("=");
        client.print("=");
        Serial.print(sensorValues[idx]);
        client.print(sensorValues[idx]);
      }
      // Закрываем пакет
      Serial.println(" HTTP/1.1");
      client.println(" HTTP/1.1");
      Serial.println("Accept: application/json");
      client.println("Accept: application/json");
      Serial.print("Host: ");
      client.print("Host: ");
      Serial.println(IPserver);
      client.println(IPserver);
      Serial.println("Content-Type: application/json");
      client.println("Content-Type: application/json");
      Serial.println();
      client.println();

      // Ждем ответа от сервера
      timer_iot_timeout = millis();
      while ((client.available() == 0) && (millis() < timer_iot_timeout + IOT_TIMEOUT1));

      // Выводим ответ от сервера, и, если медленное соединение, ждем выход по таймауту
      int iii = 0;
      bool currentLineIsBlank = true;
      bool flagJSON = false;
      timer_iot_timeout = millis();
      while ((millis() < timer_iot_timeout + IOT_TIMEOUT2) && (client.connected()))
      {
        while (client.available() > 0)
        {
          char symb = client.read();
          Serial.print(symb);
          if (symb == '{')
          {
            flagJSON = true;
          }
          else if (symb == '}')
          {
            flagJSON = false;
          }
          if (flagJSON == true)
          {
            buff[iii] = symb;
            iii ++;
        
          }
         
          timer_iot_timeout = millis();
        }
      }
      buff[iii] = '}';
      buff[iii + 1] = '\0';
      Serial.println(buff);
      // Закрываем соединение
      client.stop();

      
      // Расшифровываем параметры
      StaticJsonBuffer<BUFF_LENGTH> jsonBuffer;
      JsonObject& json_array = jsonBuffer.parseObject(buff);
      if (json_array.success())
      {
      barier = json_array["barier"];
      Serial.println("Nagr state:  " + String(barier));
      /*
      int window_state = json_array["window_state"];
      int light_state = json_array["light_state"];
      int nagr_state = json_array["nagr_state"];
      Serial.println("Pump state:  " + String(pump_state));
      Serial.println("Light state: " + String(light_state));
      Serial.println("Window state:" + String(window_state));
      */
      }
      // Делаем управление устройствами
      controlDevices();

      Serial.println("Packet successfully sent!");
      Serial.println();
    }
  }
}

// Управление исполнительными устройствами
void controlDevices()
{
  if (barier==1) {
    digitalWrite(rele1, LOW);
    digitalWrite(rele2, HIGH);
  }
  else if (barier==-1){    
    digitalWrite(rele1, HIGH);
    digitalWrite(rele2, LOW);
  }

}

