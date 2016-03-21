/// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-
#pragma once

#include <AP_Common/AP_Common.h>
#include <AP_HAL/AP_HAL.h>
#include <AP_Math/AP_Math.h>

#include "AP_Compass.h"
#include "AP_Compass_Backend.h"

class AP_Compass_LSM303D : public AP_Compass_Backend
{
public:
    static AP_Compass_Backend *probe(Compass &compass);

    bool init() override;
    void read() override;

private:
    AP_Compass_LSM303D(Compass &compass);

    bool _read_raw();
    uint8_t _register_read(uint8_t reg);
    void _register_write(uint8_t reg, uint8_t val);
    void _register_modify(uint8_t reg, uint8_t clearbits, uint8_t setbits);
    bool _data_ready();
    bool _hardware_init();
    void _update();
    void _disable_i2c();
    bool _mag_set_range(uint8_t max_ga);
    bool _mag_set_samplerate(uint16_t frequency);

    AP_HAL::SPIDeviceDriver *_spi;
    AP_HAL::Semaphore *_spi_sem;
    AP_HAL::DigitalSource *_drdy_pin_m;

    float _mag_range_scale;
    float _mag_x_accum;
    float _mag_y_accum;
    float _mag_z_accum;
    uint32_t _last_update_timestamp;
    int16_t _mag_x;
    int16_t _mag_y;
    int16_t _mag_z;
    uint8_t _accum_count;

    uint8_t _compass_instance;
    bool _initialised;

    uint8_t _mag_range_ga;
    uint8_t _mag_samplerate;
    uint8_t _reg7_expected;
};
