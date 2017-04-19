#include "AP_BattMonitor_SMBus.h"

#define AP_BATTMONITOR_SMBUS_PEC_POLYNOME 0x07 // Polynome for CRC generation

// read word from register
// returns true if read was successful, false if failed
bool AP_BattMonitor_SMBus::read_word(uint8_t reg, uint16_t& data) const
{
    // buffer to hold results (1 extra byte returned holding PEC)
    const uint8_t read_size = 2 + (_pec_supported ? 1 : 0);
    uint8_t buff[read_size];    // buffer to hold results

    // read the appropriate register from the device
    if (!_dev->read_registers(reg, buff, sizeof(buff))) {
        return false;
    }

    // check PEC
    if (_pec_supported) {
        const uint8_t pec = get_PEC(AP_BATTMONITOR_SMBUS_I2C_ADDR, reg, true, buff, 2);
        if (pec != buff[2]) {
            return false;
        }
    }

    // convert buffer to word
    data = (uint16_t)buff[1]<<8 | (uint16_t)buff[0];

    // return success
    return true;
}

/// get_PEC - calculate packet error correction code of buffer
uint8_t AP_BattMonitor_SMBus::get_PEC(const uint8_t i2c_addr, uint8_t cmd, bool reading, const uint8_t buff[], uint8_t len) const
{
    // exit immediately if no data
    if (len == 0) {
        return 0;
    }

    // prepare temp buffer for calcing crc
    uint8_t tmp_buff[len+3];
    tmp_buff[0] = i2c_addr << 1;
    tmp_buff[1] = cmd;
    tmp_buff[2] = tmp_buff[0] | (uint8_t)reading;
    memcpy(&tmp_buff[3],buff,len);

    // initialise crc to zero
    uint8_t crc = 0;
    uint8_t shift_reg = 0;
    bool do_invert;

    // for each byte in the stream
    for (uint8_t i=0; i<sizeof(tmp_buff); i++) {
        // load next data byte into the shift register
        shift_reg = tmp_buff[i];
        // for each bit in the current byte
        for (uint8_t j=0; j<8; j++) {
            do_invert = (crc ^ shift_reg) & 0x80;
            crc <<= 1;
            shift_reg <<= 1;
            if(do_invert) {
                crc ^= AP_BATTMONITOR_SMBUS_PEC_POLYNOME;
            }
        }
    }

    // return result
    return crc;
}

