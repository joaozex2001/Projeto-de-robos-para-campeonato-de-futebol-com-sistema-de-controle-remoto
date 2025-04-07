#include <ESP8266WiFi.h>
#include <espnow.h>

#define CHANNEL 6

#define VERBOSE_MODE 1
#define MOTOR12_SPEED 128
#define MOTOR34_SPEED 128
#define MYID 0
#define IN1 D5
#define IN2 D6
#define IN3 D7
#define IN4 D8
#define STATE_S 0
#define STATE_F 1
#define STATE_B -1
#define STATE_CW 2
#define STATE_CC -2

char btn[5];
int8_t state;

void InitESPNow() {
  WiFi.disconnect();
  if(esp_now_init() == ERR_OK) {
    Serial.println("ESPNow Init Success");
  }
  else {
    Serial.println("ESPNow Init Failed");
    ESP.restart();
  }
}

// config AP SSID
void configDeviceAP() {
  String Prefix = "Slave:";
  String Mac = WiFi.macAddress();
  String SSID = Prefix + Mac;
  String Password = "123456789";
  bool result = WiFi.softAP(SSID.c_str(), Password.c_str(), CHANNEL, 0);
  if(VERBOSE_MODE) {
    if (!result) {
      Serial.println("AP Config failed.");
    } else {
      Serial.println("AP Config Success. Broadcasting with AP: " + String(SSID));
    }
  }
}

void Move() {
  switch(state) {
    case STATE_S: 
      // Desliga os motores quando o robô está parado
      analogWrite(IN1, 0);
      analogWrite(IN2, 0);
      analogWrite(IN3, 0);
      analogWrite(IN4, 0);
      break;
    
    case STATE_F:  // Move para frente
      analogWrite(IN1, MOTOR12_SPEED);
      analogWrite(IN2, 0);
      analogWrite(IN3, MOTOR34_SPEED);
      analogWrite(IN4, 0);
      break;
      
    case STATE_B:  // Move para trás
      analogWrite(IN1, 0);
      analogWrite(IN2, MOTOR12_SPEED);
      analogWrite(IN3, 0);
      analogWrite(IN4, MOTOR34_SPEED);
      break;
    
    case STATE_CW:  // Gira para a direita
      analogWrite(IN1, MOTOR12_SPEED);
      analogWrite(IN2, 0);
      analogWrite(IN3, 0);
      analogWrite(IN4, MOTOR34_SPEED);
      break;
      
    case STATE_CC:  // Gira para a esquerda
      analogWrite(IN1, 0);
      analogWrite(IN2, MOTOR12_SPEED);
      analogWrite(IN3, MOTOR34_SPEED);
      analogWrite(IN4, 0);
      break;
      
    default:
      // Caso o estado seja inválido, desliga todos os motores
      analogWrite(IN1, 0);
      analogWrite(IN2, 0);
      analogWrite(IN3, 0);
      analogWrite(IN4, 0);
      break;
  }
}

void UpdateSTATE() {
  // Lógica para acionar o movimento apenas quando o botão é pressionado
  if (btn[0] == '0') { 
    state = STATE_CW; // Se o botão para a direita for pressionado
  }
  else if (btn[1] == '0') {
    state = STATE_CC; // Se o botão para a esquerda for pressionado
  }
  else if (btn[2] == '0') {
    state = STATE_F; // Se o botão para frente for pressionado
  }
  else if (btn[3] == '0') {
    state = STATE_B; // Se o botão para trás for pressionado
  }
  else {
    state = STATE_S; // Caso nenhum botão esteja pressionado, o robô deve parar
  }

  if(VERBOSE_MODE) {
    Serial.print("state:");
    Serial.println(state);
  }
}

// callback when data is recv from Master
void OnDataRecv(uint8_t *mac_addr, uint8_t *data, uint8_t data_len) {
  
  int k = MYID*4;

  for(int i=0; i<4; ++i) {
    btn[i] = (char) data[k + i];
  }

  if(VERBOSE_MODE) {
    Serial.println(btn);
  }
  UpdateSTATE();
}

void setup() {
  Serial.begin(115200);
  Serial.println("ESPNow/Basic/Slave Example");
  //Set device in AP mode to begin with
  WiFi.mode(WIFI_AP);
  // configure device AP mode
  configDeviceAP();
  // This is the mac address of the Slave in AP Mode
  Serial.print("AP MAC: "); Serial.println(WiFi.softAPmacAddress());
  // Init ESPNow with a fallback logic
  InitESPNow();
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info.
  esp_now_register_recv_cb(OnDataRecv);
  btn[4] = '\0';
  state = STATE_S;
  pinMode(IN1,OUTPUT);
  pinMode(IN2,OUTPUT);
  pinMode(IN3,OUTPUT);
  pinMode(IN4,OUTPUT);
  Move();
}

void loop() {
  // Atualiza o estado com base nos dados recebidos
  UpdateSTATE();
  // Executa o movimento baseado no estado atualizado
  Move();
  delay(10);  // Pequeno delay para evitar sobrecarga no loop
}
