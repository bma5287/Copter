/*
  implement a file store for embedded firmware images
 */
#pragma once

#include <AP_HAL/AP_HAL.h>

class AP_ROMFS {
public:
    // find a file and de-compress, assumning gzip format. The
    // decompressed data will be allocated with malloc(). You must
    // call AP_ROMFS::free() on the return value after use. The next byte after
    // the file data is guaranteed to be null.
    static const uint8_t *find_decompress(const char *name, uint32_t &size);

    // free returned data
    static void free(const uint8_t *data);

private:
    // find an embedded file
    static const uint8_t *find_file(const char *name, uint32_t &size);

    struct embedded_file {
        const char *filename;
        uint32_t size;
        const uint8_t *contents;
    };
    static const struct embedded_file files[];
};
