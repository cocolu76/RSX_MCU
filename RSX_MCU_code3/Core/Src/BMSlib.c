#include "BMSlib.h"
#include "main.h"
#include "string.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
//#include "PEC.h"

const uint8_t ADCVSC[4] =   {0x5,0x67,0x74,0x5A}; //cmd and pec of cell voltage and sc conversion pole
const uint8_t RDSTATB[12]=   {0x0,0x12,0x70,0x24,0,0,0,0,0,0,0,0};
const uint8_t RDCFG[12] = {0x0,0x2,0x2B,0xA,0,0,0,0,0,0,0,0};
const uint8_t WRCFG[12] = {0x0,0x1,0x3D,0x6E,0,0,0,0,0,0,0,0};
const uint8_t WRCFG_data_starter[6] = {0x7A,0,0,0,0,0x12}; //uint8_ts 1-3, 4 are set with discharge and uv/ov
const uint8_t RDCVA[12] ={0x0,0x4,0x7,0xC2,0,0,0,0,0,0,0,0};
const uint8_t RDCVB[12] = {0x0,0x6,0x9A,0x94,0,0,0,0,0,0,0,0};
const uint8_t ADCV[4] = {0x3, 0x60, 0xf4, 0x6c};
const uint8_t CLRCELL[4] = {0x7,0x11,0xc9,0xc0};
volatile uint8_t spiTransferComplete = 0;

extern SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef huart3;



void SPItransfer(const uint8_t* buffer, uint16_t size){ //send buffer
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);  // CS LOW
  HAL_SPI_Transmit(&hspi1, buffer, size, 100);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
}


void SPItransferReceive(const uint8_t* buffer, uint8_t* rx, uint16_t size){
  //send buffer, receive rx at same time. rx and buffer have are "size" bytes
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);  // CS LOW
  HAL_SPI_TransmitReceive(&hspi1, buffer,rx, size, 100);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
}

void SPItransferDMA(const uint8_t* buffer, uint16_t size){ //send buffer
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);  // CS LOW
  HAL_SPI_Transmit_DMA(&hspi1, buffer, size);
}

void SPItransferReceiveDMA(const uint8_t* buffer, uint8_t* rx, uint16_t size){ //send buffer
  spiTransferComplete = 0;
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);  // CS LOW
  HAL_SPI_TransmitReceive_DMA(&hspi1, buffer,rx, size);
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
  while (__HAL_SPI_GET_FLAG(hspi, SPI_FLAG_BSY) != RESET);  // Wait for SPI to finish shifting out last bits
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);  // CS HIGH
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
  while (__HAL_SPI_GET_FLAG(hspi, SPI_FLAG_BSY) != RESET);  // Wait for SPI to finish shifting out last bits
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);  // CS HIGH
  spiTransferComplete = 1;
}

void print_msg(char * msg) {
  HAL_UART_Transmit(&huart3, (uint8_t *)msg, strlen(msg), 100);
}

void print_buffer(uint8_t* buffer, int buffer_size){
  char msg[100];
  for (int i = 0; i < buffer_size; i++){
	  sprintf(msg, "%X ", buffer[i]);
	  print_msg(msg);
  }
  print_msg("\n");
}

void adcvsc() {
  SPItransferDMA(ADCVSC,4);
}

void adcv() {
  SPItransferDMA(ADCV, 4);
}

void rdstatb(){
  uint8_t rxbuffer[12];
  SPItransferReceiveDMA(RDSTATB, rxbuffer, 12); //send and receive data at the same time
  print_msg("\nReceived: 0x");
  int print = 0;
  for (int i = 0; i < 12; i++){
    if(rxbuffer[i]!=0xFF) print = 1;
  }

  uint8_t datareceived[6]; 
  memcpy(datareceived, &rxbuffer[4], 6);
  uint8_t transfer2success = checkPEC(rxbuffer[10], rxbuffer[11], datareceived, 6);
  if (print && transfer2success) {
    print_buffer(rxbuffer, 12);
  }
  print_msg("\n");
  print_msg("Received done\n");
}


void rdcfg(uint8_t *CFGR, int CFGR_size){ 
  if (CFGR_size != 6){ //sanity check
    print_msg("RDCFG failed: CFGR is not size 6!\n");
    return;
  }
  uint8_t rxbuffer[12];
  SPItransferReceiveDMA(RDCFG,rxbuffer, 12);
  memcpy(CFGR, &rxbuffer[4], 6);
  checkPEC(rxbuffer[10], rxbuffer[11], CFGR, 6);
  print_buffer(rxbuffer,12);
}

void wrcfg(uint8_t* data, int data_size){
  if (data_size > 6){
    print_msg("WRCFG failed: data too big!\n");
    return;
  }
  uint8_t txbuffer[12];
  for (int i = 0; i < 4; i++){//copy cmd & pec
    txbuffer[i] = WRCFG[i];
  }
  for (int i = 4; i <10; i++){//copy cmd & pec
    txbuffer[i] = data[i-4];
  }
  uint16_t pec = pec15((char*)data, data_size);
  uint8_t pec1 = pec & 0xFF;
  uint8_t pec0 = (uint8_t)(pec >> 8);
  txbuffer[10] = pec0; //copy pec
  txbuffer[11] = pec1; //copy pec

  print_msg("sent 0x");
  print_buffer(txbuffer, 12);
  print_msg("\n");
  SPItransferDMA(txbuffer, 12); //send and receive data at the same time
  print_msg("wrcfg done\n");
}


void rdvab(uint16_t* CV, int CVsize){ //get voltages, CV must has size of 6
  if (CVsize < 6){
    print_msg("CV array too small\n");
    return;
  }
 //4 uint8_ts sent, 8 uint8_ts received
  uint8_t rxbuffer_a[12], rxbuffer_b[12];

  SPItransferReceiveDMA(RDCVA,rxbuffer_a, 12); //send and receive data at the same time
  uint8_t datareceived_a[6];
  memcpy(datareceived_a, &rxbuffer_a[4], 6);
  uint8_t transfer1success = checkPEC(rxbuffer_a[10], rxbuffer_a[11], datareceived_a, 6);
  print_msg("Received 1: 0x");
  print_buffer(rxbuffer_a, 12);

  SPItransferReceivDMA(RDCVB,rxbuffer_b, 12);
  uint8_t datareceived_b[6];
  memcpy(datareceived_b, &rxbuffer_b[4], 6);
  uint8_t transfer2success = checkPEC(rxbuffer_b[10], rxbuffer_b[11], datareceived_b, 6);
  print_msg("Received 2: 0x");
  print_buffer(rxbuffer_b, 12);

  CV[0] = (rxbuffer_a[5]<<8) | (rxbuffer_a[4] & 0xFF);
  CV[1] = (rxbuffer_a[7]<<8) | (rxbuffer_a[6] & 0xFF);
  CV[2] = (rxbuffer_a[9]<<8) | (rxbuffer_a[8] & 0xFF);
  CV[3] = (rxbuffer_b[5]<<8) | (rxbuffer_b[4] & 0xFF);
  CV[4] = (rxbuffer_b[7]<<8) | (rxbuffer_b[6] & 0xFF);
  CV[5] = (rxbuffer_b[9]<<8) | (rxbuffer_b[8] & 0xFF);

  char msg[100];
  print_msg("CV:");
  for (int i = 0; i < 6; i++){
	  sprintf(msg, "%d;", CV[i]);
    print_msg(msg);
  }
  print_msg("\n");
}

void clearcellreg(){
  SPItransferDMA(CLRCELL,4);
}

uint8_t checkPEC(uint8_t pec0_received, uint8_t pec1_received, uint8_t* data, int data_len){
  uint16_t expected_pec = pec15((char*)data, data_len);
  uint8_t expected_pec1 = expected_pec & 0xFF;
  uint8_t expected_pec0 = (uint8_t)(expected_pec >> 8);
  char toprint[100];
  if (pec0_received == expected_pec0 && pec1_received == expected_pec1) {
    print_msg("pec check passed");
    return 1;
  }
  else {
    print_msg("pec check failed");
    sprintf(toprint,"exp pec0 %X; exp pec1 %X; rec pec0 %X; rec pec1 %X\n",
      expected_pec0, expected_pec1, pec0_received, pec1_received);
    print_msg(toprint);
    return 0;
  }
}

void dischargeCellX(uint8_t* data, int data_size, int cell_x){ //cell_x takes values 1 to 6
  if ((data_size > 6)||(cell_x > 6)||(cell_x < 1)){
    print_msg("discharge failed: bad arguments!");
    return;
  }

  /*//check if any cell is discharging (only 1 adjacent cell at a time)
  uint8_t currentCFGR[6] = {0};
  rdcfg(currentCFGR, 6);
  if(currentCFGR[4] != 0){
    char err[100];
    sprint_msgf(err, "discharge failed: another cell is discharging! (CFGR4=0x%X)", currentCFGR[4]);
    print_msgln(err);
  }
  //*/

  for (int i = 0; i<data_size; i++){//CFGR0 & CFGR5
    data[i]=WRCFG_data_starter[i];
  }
  data[4] = 1<<(cell_x-1); //CFGR4
  
  wrcfg(data, data_size);
  print_msg("discharge sent\n");
}

void dataCollection(uint8_t c0, uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4) {
  // C2 DATA COLLECTION BELOW
/*
  uint8_t C40V, C4UV, C30V, C3UV, C20V, C2UV, C10V, C1UV;
  uint8_t C60V, C6UV, C50V, C5UV;
  

  C40V = bitRead(c2, 7);
  C4UV = bitRead(c2, 6);
  C30V = bitRead(c2, 5);
  C3UV = bitRead(c2, 4);
  C20V = bitRead(c2, 3);
  C2UV = bitRead(c2, 2);
  C10V = bitRead(c2, 1);
  C1UV = bitRead(c2, 0);

  print_msg("C2 Data...");
  print_msg(C40V);
  print_msg(C4UV);
  print_msg(C30V);
  print_msg(C3UV);
  print_msg(C20V);
  print_msg(C2UV);
  print_msg(C10V);
  print_msg(C1UV);

  //C3 DATA COLLECTION BELOW
  C60V = bitRead(c3, 3);
  C6UV = bitRead(c3, 2);
  C50V = bitRead(c3, 1);
  C5UV = bitRead(c3, 0);

  print_msg("\nC3 Data...");
  print_msg(C60V);
  print_msg(C6UV);
  print_msg(C50V);
  print_msg(C5UV);
  */

}
