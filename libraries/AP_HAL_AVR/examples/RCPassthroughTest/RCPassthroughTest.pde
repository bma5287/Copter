
#include <AP_Common.h>
#include <AP_HAL.h>
#include <AP_HAL_AVR.h>

const AP_HAL::HAL& hal = AP_HAL_AVR_APM2;

void multiread(AP_HAL::RCInput* in, uint16_t* channels) {
    /* Multi-channel read method: */
    uint8_t valid;
    valid = in->read(channels, 8);
    hal.uart0->printf_P(PSTR("multi      read %d: %d %d %d %d %d %d %d %d\r\n"),
            (int) valid, 
            channels[0], channels[1], channels[2], channels[3],
            channels[4], channels[5], channels[6], channels[7]);
}

void individualread(AP_HAL::RCInput* in, uint16_t* channels) {
    /* individual channel read method: */
    uint8_t valid;
    valid = in->valid();
    for (int i = 0; i < 8; i++) {
        channels[i] = in->read(i);
    }
    hal.uart0->printf_P(PSTR("individual read %d: %d %d %d %d %d %d %d %d\r\n"),
            (int) valid, 
            channels[0], channels[1], channels[2], channels[3],
            channels[4], channels[5], channels[6], channels[7]);
}


void multiwrite(AP_HAL::RCOutput* out, uint16_t* channels) {
    out->write(0, channels, 8);
}

void individualwrite(AP_HAL::RCOutput* out, uint16_t* channels) {
    for (int ch = 0; ch < 8; ch++) {
        out->write(ch, channels[ch]); 
    }

}

void loop (void) {
    static int ctr = 0;
    uint16_t channels[8];

    hal.gpio->write(27, 1);

    
    /* Cycle between using the individual read method
     * and the multi read method*/
    if (ctr < 500) {
        multiread(hal.rcin, channels);
    } else {
        individualread(hal.rcin, channels);
        if (ctr > 1000)  ctr = 0;
    }

    /* Cycle between individual output and multichannel output */
    if (ctr % 500 < 250) {
        multiwrite(hal.rcout, channels);
    } else {
        individualwrite(hal.rcout, channels);
    }

    hal.gpio->write(27, 0);
    hal.scheduler->delay(4);
    ctr++;
}

void setup (void) {
    hal.uart0->begin(115200);
    hal.uart0->printf_P(PSTR("reading rc in:"));
    hal.gpio->pinMode(27, GPIO_OUTPUT);
    hal.gpio->write(27, 0);
}


extern "C" {
int main (void) {
    hal.init(NULL);
    setup();
    for(;;) loop();
    return 0;
}
}
