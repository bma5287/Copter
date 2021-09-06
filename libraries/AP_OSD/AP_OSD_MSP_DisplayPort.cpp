/*
 * This file is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
/*
  OSD backend for MSP
 */
#include <AP_MSP/AP_MSP.h>
#include <AP_MSP/msp.h>
#include "AP_OSD_MSP_DisplayPort.h"

#if HAL_WITH_MSP_DISPLAYPORT

static const struct AP_Param::defaults_table_struct defaults_table[] = {
    /*
    { "PARAM_NAME",       value_float }
    */
};

extern const AP_HAL::HAL &hal;
constexpr uint8_t AP_OSD_MSP_DisplayPort::symbols[AP_OSD_NUM_SYMBOLS];

// initialise backend
bool AP_OSD_MSP_DisplayPort::init(void)
{
    // check if we have a DisplayPort backend to use
    const AP_MSP *msp = AP::msp();
    if (msp == nullptr) {
        gcs().send_text(MAV_SEVERITY_WARNING,"MSP backend not available");
        return false;
    }
    _displayport = msp->find_protocol(AP_SerialManager::SerialProtocol_MSP_DisplayPort);
    if (_displayport == nullptr) {
        gcs().send_text(MAV_SEVERITY_WARNING,"MSP DisplayPort uart not available");
        return false;
    }
    // re-init port here for use in this thread
    _displayport->init_uart();
    return true;
}

void AP_OSD_MSP_DisplayPort::clear(void)
{
    // clear remote MSP screen
    _displayport->msp_displayport_clear_screen();

    // toggle flashing @2Hz
    const uint32_t now = AP_HAL::millis();
    if (((now / 500) & 0x01) != _blink_on) {
        _blink_on = !_blink_on;
    }
}

void AP_OSD_MSP_DisplayPort::write(uint8_t x, uint8_t y, const char* text)
{
    _displayport->msp_displayport_write_string(x, y, 0, text);
}

void AP_OSD_MSP_DisplayPort::write(uint8_t x, uint8_t y, bool blink, const char *fmt, ...)
{
    if (blink && !_blink_on) {
        return;
    }
    char buf[32+1]; // +1 for snprintf null-termination
    va_list ap;
    va_start(ap, fmt);
    int res = hal.util->vsnprintf(buf, sizeof(buf), fmt, ap);
    res = MIN(res, int(sizeof(buf)));
    if (res < int(sizeof(buf))-1) {
        _displayport->msp_displayport_write_string(x, y, blink, buf);
    }
    va_end(ap);
}

void AP_OSD_MSP_DisplayPort::flush(void)
{
    // grab the screen and force a redraw
    _displayport->msp_displayport_grab();
    _displayport->msp_displayport_draw_screen();

    // ok done processing displayport data
    // let's process incoming MSP frames (and reply if needed)
    _displayport->process_incoming_data();
}

void AP_OSD_MSP_DisplayPort::init_symbol_set(uint8_t *lookup_table, const uint8_t size)
{
    const AP_MSP *msp = AP::msp();
    // do we use backend specific symbols table?
    if (msp && msp->is_option_enabled(AP_MSP::Option::DISPLAYPORT_BTFL_SYMBOLS)) {
        memcpy(lookup_table, symbols, size);
    } else {
        memcpy(lookup_table, AP_OSD_Backend::symbols, size);
    }
}

// override built in positions with defaults for MSP OSD
void AP_OSD_MSP_DisplayPort::setup_defaults(void)
{
    AP_Param::set_defaults_from_table(defaults_table, ARRAY_SIZE(defaults_table));
}

AP_OSD_Backend *AP_OSD_MSP_DisplayPort::probe(AP_OSD &osd)
{
    AP_OSD_MSP_DisplayPort *backend = new AP_OSD_MSP_DisplayPort(osd);
    if (!backend) {
        return nullptr;
    }
    if (!backend->init()) {
        delete backend;
        return nullptr;
    }
    return backend;
}
#endif