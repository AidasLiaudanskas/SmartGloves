#include "mbed.h"

#ifndef SENSORS_H
#define SENSORS_H


// Register names, can look up in the datasheet

#define SMPRT_DIV    0x19 //
#define CONFIG       0x1A //
#define GYRO_CONFIG  0x1B //
#define ACCEL_CONFIG 0x1C //
#define FIFO_EN      0x23 //
#define I2C_MST_CTRL 0x24 //
#define INT_PIN_CFG  0x37 //
#define INT_ENABLE   0x38 //

#define ACCX158      0x3B //
#define ACCX70       0x3C //
#define ACCY158      0x3D //
#define ACCY70       0x3E //
#define ACCZ158      0x3F //
#define ACCZ70       0x40 //
#define TEMP158      0x41 //
#define TEMP70       0x42 // Temperature in C = ((signed int16_t) value)/340   + 36.53
#define GYROX158     0x43 //
#define GYROX70      0x44 //
#define GYROY158     0x45 //
#define GYROY70      0x46 //
#define GYROZ158     0x47 //
#define GYROZ70      0x48 //

#define USER_CTRL    0x6A //
#define PWR_MGMT_1   0x6B //
#define PWR_MGMT_2   0x6C //
#define WHO_AM_I     0x75 // Check device number to see if it works

const uint8_t ACC_LEFT  = 0x69; //    0b01101000
const uint8_t ACC_RIGHT = 0x68; //    0b01101001


typedef struct {

        uint16_t flex[12];
        // Order of the flex sensors:
        // 0 - right thumb, 4 - right pinky, 5 - R elbow;
        // 6 - left thumb, 10 left pinky, 11 - L elbow;
        uint16_t acc[2][7];
        // 7th value is the temperature value
        uint16_t temp;


} Datapacket;

typedef struct {

        uint8_t pixels[8];

} Screen;


void readacc(Datapacket data);

void setupacc(uint8_t ADDRESS);

void WriteBytes(uint8_t addr, uint8_t *pbuf, uint16_t length, uint8_t DEV_ADDR);

void WriteByte(uint8_t addr, uint8_t buf, uint8_t DEV_ADDR);

void ReadBytes(uint8_t addr, uint8_t *pbuf, uint16_t length, uint8_t DEV_ADDR);

void readflexs(Datapacket *data);

void readacc(Datapacket *data, uint8_t accadr);

void setupI2C(void);







#endif
