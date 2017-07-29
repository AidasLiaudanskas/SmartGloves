#include "mbed.h"
#include "sensors.h"


DigitalOut led(LED1);

static volatile bool triggerSensorPolling = false;

Serial pc(USBTX, USBRX);


void datapackettoarray(Datapacket todump, uint16_t *dest)
{
        uint8_t j,k;

        for (j=0; j<12; j++)
        {
                *dest=todump.flex[j];
                dest++;
        }
        for(k=0; k<7; k++)
        {
                *dest=todump.acc[0][k];
                dest++;
        }

        for(k=0; k<7; j++, k++)
        {
                *dest=todump.acc[1][k];
                dest++;
        }



}

char hex_from_nibble(uint8_t nibble)
{
        nibble &= 0xf;
        if (nibble < 0xa)
                return '0' + nibble;
        else
                return 'a' + (nibble - 0xa);
}

void put_byte(uint8_t byte)
{
        pc.putc(hex_from_nibble(byte >> 4));
        pc.putc(hex_from_nibble(byte));
}

void periodicCallback(void)
{
        led = !led; /* Do blinky on LED1 while we're waiting for events */

        /* Note that the periodicCallback() executes in interrupt context, so it is safer to do
         * heavy-weight sensor polling from the main thread. */
        triggerSensorPolling = true;
}

void chararrraytoUART(uint8_t *array, uint8_t length){
        if(pc.writeable()) {
                pc.printf("\r\n");
                for(int i=0; i<length; i++) {
                        put_byte(*array);
                        array++;
                }
        }
}


/*
 *  Main loop
 */
int main(void)
{
        pc.baud(115200);
        pc.printf("IIC Demo Start \r\n");
        Datapacket readings;
        setupI2C();
        setupacc(ACC_LEFT);
        setupacc(ACC_RIGHT);
        pc.printf("Setup complete \r\n");
        led = 1;
        Ticker ticker;
        ticker.attach(periodicCallback, 0.02); // blink LED at 50Hz
        uint16_t array[26];
        while (1) {
                // check for trigger// from periodicCallback()
                if (triggerSensorPolling) {
                        triggerSensorPolling = false;
                        readflexs(&readings);
                        readacc(&readings, ACC_LEFT );
                        readacc(&readings, ACC_RIGHT );
                        datapackettoarray(readings, array);
                        chararrraytoUART((uint8_t*) &array, 52);
                        pc.printf("\n");

                }
        }
}
