/************************************************************************************
 * Controle OSC com sensores LIDAR ou Ultrasônico                                   *
 *                                                                                  *
 * Esse projeto permite enviar mensagens OSC (Open Sound Control)                   *
 * a partir de um Arduino via porta serial, com otimizações e em alta velocidade,   *
 * para programas como Pure Data, SuperCollider, Ripper, etc                        *
 *                                                                                  *
 * Autor: Sandro Benigno                                                            *
 * Licença de uso: GNU GPL 2.0                                                      *
 *                                                                                  *
 * The GNU GPL is the most widely used free software license and has a strong       *
 * copyleft requirement. When distributing derived works, the source code of        *
 * the work must be made available under the same license. There are multiple       *
 * variants of the GNU GPL, each with different requirements.                       *
 *                                                                                  *
 **********************************************************************************/

#include <Arduino.h>
#include "OSCBundle.h"
#include "OSCBoards.h"

/*======================================
Parametros que são dependentes do modelo
de placa (Por exemplo UNO ou Leonardo)
======================================*/
#ifdef BOARD_HAS_USB_SERIAL
  #include <SLIPEncodedUSBSerial.h>
  SLIPEncodedUSBSerial SLIPSerial( thisBoardsSerialUSB );
#else
  #include <SLIPEncodedSerial.h>
  SLIPEncodedSerial SLIPSerial(Serial);
#endif

//Descomente o tipo de sensor que pretende utilizar
#define MAX_RANGE 70
//#define ULTRASOM //Sensor HC-SR04
#define LIDAR //Sensor VL53L0X do tipo LIDAR (Light Detection And Ranging)

#ifdef ULTRASOM
  #include <Ultrasonic.h>
  Ultrasonic US(2);
#endif

#ifdef LIDAR
  #include <Wire.h> //A comunicação com o sensor LIDAR VL53L0X é via protocolo i2c 
  #include "VL53L0X.h"
  VL53L0X LD;
#endif

void setup() {
  //Otimização da porta serial para envio de OSC (SLIP -> Serial Line Internet Protocol)
  SLIPSerial.begin(115200);
#ifdef ULTRASOM
  US.setTimeout(40000UL); //Inicializando o Sensor ultrasônico
#endif

#ifdef LIDAR
  Wire.begin();
  LD.setTimeout(500);
  if (!LD.init()) //Inicializando o sensor LIDAR
  {
    Serial.println("Falha ao inicializar o LIDAR!");
    while (1) {}
  }
#endif

}

bool validaPack(uint8_t dist){
  if(dist > MAX_RANGE){
    return false;
  }
  else return true;
}

void loop() {
  int32_t Dist;

/*=========================================
Leitura da distância via Ultrasom ou LIDAR
==========================================*/
#ifdef ULTRASOM
  Dist = uint32_t(US.read());
#endif

#ifdef LIDAR
  Dist = uint32_t(LD.readRangeSingleMillimeters()/10);
#endif


/*=========================================
Criando um grupo de mensagens e populando
com as mensagens com os dados a serem enviados
e enviando nos endereços via SLIP
==========================================*/

  OSCBundle b;
  if(validaPack(Dist)){
    b.add("/sensor").add(Dist); //leitura dentro do range útil
  }
  else{
    b.add("/sensor").add(MAX_RANGE); //leitura fora do range útil
  }
  
  //Preparando enviando e liberando a memória
  SLIPSerial.beginPacket();
  b.send(SLIPSerial);
  SLIPSerial.endPacket();
  b.empty();

#ifdef ULTRASOM
  delay(50); //Sensores Ultrasônicos são lentos, alguns precisam de até mais que 50 milisegundos
#endif

#ifdef LIDAR
  delay(10); //Intervalo de leitura compatível com o dispositivo LIDAR
#endif

}
