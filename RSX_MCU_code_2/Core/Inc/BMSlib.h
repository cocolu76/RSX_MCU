#ifndef BMS_HEADER
#define BMS_HEADER
#include <stdint.h>
extern const uint8_t ADCVSC[4]; //cmd and pec of cell voltage and sc conversion pole
extern const uint8_t RDSTATB[12];
extern const uint8_t RDCFG[12];
extern const uint8_t WRCFG[12];
extern const uint8_t WRCFG_data_starter[6]; //uint8_ts 1-3, 4 are set with discharge and uv/ov
extern const uint8_t RDCVA[12];
extern const uint8_t RDCVB[12];
extern const uint8_t ADCV[4];
extern const uint8_t CLRCELL[4];

void wakeup(); //wake up spi interface (not working)
void adcvsc(); //measure all cells and sum of all cells
void adcv();  //measure all cells
void rdstatb(); //read status register b
void rdcfg(uint8_t *CFGR, int CFGR_size); //read configuration register into size 6 uint8_t array "CFGR"
void wrcfg(uint8_t* data, int data_size); //write size 6 uint8_t array "data" into configuration register
void rdvab(uint16_t* CV, int CVsize); //read voltage registers a & b into uint16_t size 6 array "CV"
void clearcellreg(); //clear voltage registers
uint8_t checkPEC(uint8_t pec0_received, uint8_t pec1_received, uint8_t* data, int data_len); //check pec of received uint8_ts
void dataCollection(uint8_t c0, uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4);
void dischargeCellX(uint8_t* data, int data_size, int cell_x); //cell_x takes values 1 to 6
void SPItransfer(const uint8_t* buffer, uint16_t size);
void print_msg(char * msg);
void print_buffer(uint8_t * buffer, int buffer_size);
void SPItransferReceive(const uint8_t* buffer, uint8_t* rx, uint16_t size);
void SPItransferDMA(const uint8_t* buffer, uint16_t size);
void SPItransferReceiveDMA(const uint8_t* buffer, uint8_t* rx, uint16_t size);



#endif
