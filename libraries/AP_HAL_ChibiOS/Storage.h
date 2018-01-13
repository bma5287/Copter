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
 * Code by Andrew Tridgell and Siddharth Bharat Purohit
 */
#pragma once

#include <AP_HAL/AP_HAL.h>
#include "AP_HAL_ChibiOS_Namespace.h"
#include <AP_Common/Bitmask.h>
#include <AP_FlashStorage/AP_FlashStorage.h>
#include "hwdef/common/flash.h"
#include <AP_RAMTRON/AP_RAMTRON.h>

#define CH_STORAGE_SIZE HAL_STORAGE_SIZE

// when using flash storage we use a small line size to make storage
// compact and minimise the number of erase cycles needed
#define CH_STORAGE_LINE_SHIFT 3

#define CH_STORAGE_LINE_SIZE (1<<CH_STORAGE_LINE_SHIFT)
#define CH_STORAGE_NUM_LINES (CH_STORAGE_SIZE/CH_STORAGE_LINE_SIZE)

class ChibiOS::Storage : public AP_HAL::Storage {
public:
    void init() {}
    void read_block(void *dst, uint16_t src, size_t n);
    void write_block(uint16_t dst, const void* src, size_t n);

    void _timer_tick(void);

private:
    volatile bool _initialised;
    void _storage_create(void);
    void _storage_open(void);
    void _mark_dirty(uint16_t loc, uint16_t length);
    uint8_t _buffer[CH_STORAGE_SIZE] __attribute__((aligned(4)));
    Bitmask _dirty_mask{CH_STORAGE_NUM_LINES};

    bool _flash_write_data(uint8_t sector, uint32_t offset, const uint8_t *data, uint16_t length);
    bool _flash_read_data(uint8_t sector, uint32_t offset, uint8_t *data, uint16_t length);
    bool _flash_erase_sector(uint8_t sector);
    bool _flash_erase_ok(void);
    uint8_t _flash_page;
    bool _flash_failed;
    uint32_t _last_re_init_ms;
    
    AP_FlashStorage _flash{_buffer,
            stm32_flash_getpagesize(STORAGE_FLASH_PAGE),
            FUNCTOR_BIND_MEMBER(&Storage::_flash_write_data, bool, uint8_t, uint32_t, const uint8_t *, uint16_t),
            FUNCTOR_BIND_MEMBER(&Storage::_flash_read_data, bool, uint8_t, uint32_t, uint8_t *, uint16_t),
            FUNCTOR_BIND_MEMBER(&Storage::_flash_erase_sector, bool, uint8_t),
            FUNCTOR_BIND_MEMBER(&Storage::_flash_erase_ok, bool)};
    
    void _flash_load(void);
    void _flash_write(uint16_t line);

#if HAL_WITH_RAMTRON
    AP_RAMTRON fram;
    bool using_fram;
#endif
};
