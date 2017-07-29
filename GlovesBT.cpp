#include "mbed.h"
#include "ble/BLE.h"
#include "sensors.h"
#include <Serial.h>




DigitalOut led(LED1);
uint16_t customServiceUUID  = 0xABCD;
uint16_t readCharUUID       = 0xABCE;
uint16_t writeCharUUID      = 0xABCF;

static volatile bool triggerSensorPolling = false;

Serial pc(USBTX, USBRX);



const static char DEVICE_NAME[]        = "WorkGloves";
static const uint16_t uuid16_list[]        = {0xFFFF}; //Custom UUID, FFFF is reserved for development

/* Set Up custom Characteristics */
static uint16_t readValue[26] = {0x55, 0x33};
ReadOnlyArrayGattCharacteristic<uint16_t, sizeof(readValue)> readChar(readCharUUID, readValue);

static uint8_t writeValue[8] = {0x00};
WriteOnlyArrayGattCharacteristic<uint8_t, sizeof(writeValue)> writeChar(writeCharUUID, writeValue);


/* Set up custom service */
GattCharacteristic *characteristics[] = {&readChar, &writeChar};
GattService customService(customServiceUUID, characteristics, sizeof(characteristics) / sizeof(GattCharacteristic *));

void datapackettoarray(Datapacket todump, uint16_t *dest)
{       //Need a uint16_t array[26] input for correct functioning
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


/*
 *  Restart advertising when phone app disconnects
 */
void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *)
{
        BLE::Instance(BLE::DEFAULT_INSTANCE).gap().startAdvertising();
}

void periodicCallback(void)
{
        led = !led; /* Do blinky on LED1 while we're waiting for BLE events */

        /* Note that the periodicCallback() executes in interrupt context, so it is safer to do
         * heavy-weight sensor polling from the main thread. */
        triggerSensorPolling = true;
}

/*
 *  Handle writes to writeCharacteristic for screen data from phone
 */
void writeCharCallback(const GattWriteCallbackParams *params)
{
        /* Check to see what characteristic was written, by handle */
        if(params->handle == writeChar.getValueHandle()) {
                /* Update the readChar with the value of writeChar */
//                BLE::Instance(BLE::DEFAULT_INSTANCE).gattServer().write(readChar.getValueHandle(), params->data, params->len);
        }
}
/*
 * Initialization callback
 */
void bleInitComplete(BLE::InitializationCompleteCallbackContext *params)
{
        BLE &ble          = params->ble;
        ble_error_t error = params->error;

        if (error != BLE_ERROR_NONE) {
                return;
        }

        ble.gap().onDisconnection(disconnectionCallback);
        ble.gattServer().onDataWritten(writeCharCallback); // TODO: update to flush screen !!!

        /* Setup advertising */
        ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE); // BLE only, no classic BT
        ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED); // advertising type
        ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME)); // add name
        ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, (uint8_t *)uuid16_list, sizeof(uuid16_list)); // UUID's broadcast in advertising packet
        ble.gap().setAdvertisingInterval(250); // 250ms.

        /* Add our custom service */
        ble.addService(customService);

        /* Start advertising */
        ble.gap().startAdvertising();
}


void chararrraytoUART(uint8_t *array, uint8_t length){
        if(pc.writeable())
                for(int i=0; i<length; i++) {
                        pc.putc(*array);
                        array++;

                }

}

/*
 *  Main loop
 */
int main(void)
{

        pc.baud(115200);
        pc.printf("IIC Demo Start \r\n");
        // For debugging into PC terminal
        /* initialize stuff */
//        printf("\n\r********* Starting Main Loop *********\n\r");
        Datapacket readings;
        setupI2C();
        setupacc(ACC_LEFT);
        setupacc(ACC_RIGHT);
        pc.printf("Setup complete \r\n");
        led = 1;
        Ticker ticker;
        ticker.attach(periodicCallback, 1); // blink LED every 1 second
        BLE& ble = BLE::Instance(BLE::DEFAULT_INSTANCE);
        ble.init(bleInitComplete);
        uint16_t array[26];
        /* SpinWait for initialization to complete. This is necessary because the
         * BLE object is used in the main loop below. */
        while (ble.hasInitialized()  == false) { /* spin loop */ }
        /* Infinite loop waiting for BLE interrupt events */
        while (1) {
                // check for trigger// from periodicCallback()
                if (triggerSensorPolling) {
                        triggerSensorPolling = false;
//                        data[0] = data[0]+1;//
                        readflexs(&readings);
                        readacc(&readings, ACC_LEFT );
                        readacc(&readings, ACC_RIGHT );
                        datapackettoarray(readings, array);
                        pc.printf("Values read: \r\n");
                        chararrraytoUART((uint8_t*) &array, 52);
                        BLE::Instance(BLE::DEFAULT_INSTANCE).gattServer().write(readChar.getValueHandle(), (uint8_t*)array, 52 );

                }
                else {
                        ble.waitForEvent(); // low power wait for event
                }
        }
}
