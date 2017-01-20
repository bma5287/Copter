#pragma once

#include "Display.h"
#include "Display_Backend.h"
#include <AP_HAL/I2CDevice.h>

#define SSD1306_COLUMNS 128		// display columns
#define SSD1306_ROWS 64		    // display rows
#define SSD1306_ROWS_PER_PAGE 8

class Display_SSD1306_I2C: public Display_Backend {

public:
    Display_SSD1306_I2C(AP_HAL::OwnPtr<AP_HAL::Device> dev);

    bool hw_init() override;
    bool hw_update() override;
    bool set_pixel(uint16_t x, uint16_t y) override;
    bool clear_pixel(uint16_t x, uint16_t y) override;
    bool clear_screen() override;

private:
    void _timer();

    AP_HAL::OwnPtr<AP_HAL::Device> _dev;
    uint8_t _displaybuffer[SSD1306_COLUMNS * SSD1306_ROWS_PER_PAGE];
    bool _need_hw_update;
};
