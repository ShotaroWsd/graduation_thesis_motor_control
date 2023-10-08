#include <IRROBOT_EZController.h>
#include <FlexiTimer2.h>

#define ID_NUM 0

IRROBOT_EZController Easy(&Serial1);

bool status = 0;
uint16_t position = 4090;  // モータの指示位置
int final_position = 0;  // モータの最終目標位置
int present_position;  // モータの現在位置
int sleep_val;  // タイマー割り込みの周期
bool speed_control;  //0で最大速度，1でタイマを用いた速度制御 

#define RACIEVE_DATA_SIZE 4
#define SEND_DATA_SIZE 3
#define MAX_STROKE 4096
#define MIN_STROKE 0

#define INITIAL_POSITION 2000
#define POSITION_0mL 4090
#define POSITION_LAST 4000

uint8_t recieve_data[RACIEVE_DATA_SIZE];
uint8_t send_data[SEND_DATA_SIZE];


void setup() {
  Serial.begin(9600);
  Easy.MightyZap.begin(32);
  Easy.MightyZap.GoalPosition(ID_NUM, INITIAL_POSITION);

  while(!Serial); 
  Easy.MightyZap.GoalPosition(ID_NUM, POSITION_0mL);
  while(!Serial.available());
  Serial.readBytes(recieve_data, RACIEVE_DATA_SIZE);

  if(recieve_data[3] == 0){
    speed_control = 0;
    position = recieve_data[1] << 8 | recieve_data[2];
    Easy.MightyZap.GoalPosition(ID_NUM, position);
  }else{
    speed_control = 1;
    sleep_val = 255 - recieve_data[3];
    final_position = recieve_data[1] << 8 | recieve_data[2];
    FlexiTimer2::set(sleep_val, timerInterrept);
    FlexiTimer2::start();
  }
}


void loop() {  
  if(Serial.available()){  // シリアル通信を受信したとき
    Serial.readBytes(recieve_data, RACIEVE_DATA_SIZE);
    if(speed_control){  // 速度制御時は目標位置を現在位置とする
      present_position = position;  
    }else{  // 速度制御を行わないときは現在位置を取得する
      present_position = Easy.MightyZap.presentPosition(ID_NUM);
    }

    //present_position = 4000;
    send_data[0] = 0;
    send_data[1] = present_position >> 8;
    send_data[2] = present_position;

    Serial.write(send_data, SEND_DATA_SIZE);
    Serial.flush();
    if(recieve_data[0] == 2){
      Easy.MightyZap.GoalPosition(ID_NUM, POSITION_LAST);
    }
  }
}


// タイマー割り込みで呼び出される関数
void timerInterrept(){
    if(position > final_position){  // 目標位置まで到達していないとき
      position--;
      Easy.MightyZap.GoalPosition(ID_NUM, position);
    }
}