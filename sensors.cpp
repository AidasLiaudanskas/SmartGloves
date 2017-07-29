#include "sensors.h"
#include "mbed.h"
#include "wire.h"


AnalogIn analogL(P0_4); // Analog senson input polling A3; Left In
AnalogIn analogR(P0_5); // Analog senson input polling A4; Right In

DigitalOut flexA(P0_7); // Both Hand Finger Selectors:  A
DigitalOut flexB(P0_6); // B
DigitalOut flexC(P0_15); //  C

I2C i2c(p29, p28);



void WriteBytes(uint8_t addr, uint8_t *pbuf, uint16_t length, uint8_t DEV_ADDR)
{

        i2c.start();
        i2c.write( DEV_ADDR <<1 );
        i2c.write( addr );
        for (int i =0; i<length; i++) {
                i2c.write( *pbuf );
                pbuf++;
        }
        i2c.stop();
}

void ReadBytes(uint8_t regaddr, uint8_t *pbuf, uint16_t length, uint8_t DEV_ADDR)
{
        i2c.start();
        char data[length];
        // i2c.write( (DEV_ADDR <<1) +1);
        i2c.write( (DEV_ADDR << 1));
        i2c.write( regaddr);
        i2c.start();
        i2c.read( (DEV_ADDR<<1), data, length);

        for(int i=0; i<length; i++) {   // equate the addresses
                pbuf[i]=data[i];
        }
}

void WriteByte(uint8_t addr, uint8_t buf, uint8_t DEV_ADDR)
{

        i2c.start();
        i2c.write( DEV_ADDR <<1 );
        i2c.write( addr );
        i2c.write( buf );
        i2c.stop();
}

void setupI2C(void){
        // Wire.begin(SCL, SDA, TWI_FREQUENCY_100K);
}

void setupacc(uint8_t ADDRESS){
        WriteByte(SMPRT_DIV, 0x13, ADDRESS); // Set rate divider to 19, so sample rate is 50hz
        WriteByte(CONFIG, 0x05, ADDRESS); // Set the digital low pass filter to 10Hz Cutoff for both Gyro and acc
        WriteByte(GYRO_CONFIG, 0x00, ADDRESS); // Set the gyro range to \pm 250 degrees/sec
        WriteByte(ACCEL_CONFIG, 0x00, ADDRESS); // Set the acc range to \pm 2g
        WriteByte(FIFO_EN, 0x00, ADDRESS); // Disable FIFO
        WriteByte(I2C_MST_CTRL, 0x00, ADDRESS); // Disabling external I2C
        WriteByte(INT_PIN_CFG, 0x30, ADDRESS); // Active high, push-pull, high until cleared, cleared on any read,
        WriteByte(INT_ENABLE, 0x00, ADDRESS); // DataRDY is the last bit if needed
        WriteByte(USER_CTRL, 0x00, ADDRESS); // No FIFO or I2C Master set
        WriteByte(PWR_MGMT_1, 0x00, ADDRESS); // TODO: Sleepmode is here; Disabled now; Cycle -> LP_WAKE_CTRL
        WriteByte(PWR_MGMT_2, 0x00, ADDRESS); // No lowpower mode, all sensors active

}

void readacc(Datapacket *data, uint8_t accadr){
        uint8_t rorl = 1; //
        if(accadr == ACC_RIGHT) {rorl=0; };
        uint8_t buffer[14]={0};
        uint16_t temp;
        ReadBytes(0x3B, buffer, 14, accadr);
        temp = ((buffer[0]<<8)+buffer[1]);
        for (int i=0; i<7; i++) {

                temp=0;
                temp = ((buffer[2*i]<<8)+buffer[2*i + 1]);
                data->acc[rorl][i]= (uint16_t) temp;
        }
        // Acc data structure: ACC X, Y, Z; Temperature measurement; GYRO X, Y, Z;

}




void readflexs(Datapacket *data){

        // Order of the flex sensors:
        // 0 - right thumb, 4 - right pinky, 5 - R elbow;
        // 6 - left thumb, 10 left pinky, 11 - L elbow;
        // Read right hand in:

        // AnalogIn analogL(P0_4); // Analog senson input polling A3; Left In
        // AnalogIn analogR(P0_5); // Analog senson input polling A4; Right In
        // Address is CBA
        // Read thumbs:
        flexC = 0;
        flexB = 0;
        flexA = 0;
        data->flex[6] = (uint16_t) (analogL.read()*65535);
        data->flex[0] = (uint16_t) (analogR.read()*65535);

        flexC = 0;
        flexB = 0;
        flexA = 1;
        data->flex[7] = (uint16_t) (analogL.read()*65535);
        data->flex[1] = (uint16_t) (analogR.read()*65535);

        flexC = 0;
        flexB = 1;
        flexA = 0;
        data->flex[8] = (uint16_t) (analogL.read()*65535);
        data->flex[2] = (uint16_t) (analogR.read()*65535);

        flexC = 0;
        flexB = 1;
        flexA = 1;
        data->flex[9] = (uint16_t) (analogL.read()*65535);
        data->flex[3] = (uint16_t) (analogR.read()*65535);

        flexC = 1;
        flexB = 0;
        flexA = 0;
        data->flex[10] = (uint16_t) (analogL.read()*65535);
        data->flex[4] = (uint16_t) (analogR.read()*65535);


        flexC = 1;
        flexB = 0;
        flexA = 1;
        data->flex[11] = (uint16_t) (analogL.read()*65535);
        data->flex[5] = (uint16_t) (analogR.read()*65535);


}
