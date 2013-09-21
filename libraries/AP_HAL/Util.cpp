#include <AP_HAL.h>
#include "Util.h"
#include "utility/print_vprintf.h"

/* Helper class implements AP_HAL::Print so we can use utility/vprintf */
class BufferPrinter : public AP_HAL::Print {
public:
    BufferPrinter(char* str, size_t size)  : _offs(0), _str(str), _size(size)  {}
    size_t write(uint8_t c) {
        if (_offs < _size) {
            _str[_offs] = c;
            _offs++;
            return 1;
        } else {
            return 0;
        }
    }
    size_t _offs; 
    char* const  _str;
    const size_t _size;
};

int AP_HAL::Util::snprintf(char* str, size_t size, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int res = this->vsnprintf(str, size, format, ap);
    va_end(ap);
    return res;
}

int AP_HAL::Util::snprintf_P(char* str, size_t size, const prog_char_t *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int res = this->vsnprintf_P(str, size, format, ap);
    va_end(ap);
    return res;
}


int AP_HAL::Util::vsnprintf(char* str, size_t size, const char *format, va_list ap)
{
    BufferPrinter buf(str, size);
    print_vprintf(&buf, 0, format, ap);
    // null terminate if possible
    buf.write(0);
    return (int) buf._offs;
}

int AP_HAL::Util::vsnprintf_P(char* str, size_t size, const prog_char_t *format,
                              va_list ap)
{
    BufferPrinter buf(str, size);
    print_vprintf(&buf, 1,(const char*) format, ap);
    // null terminate if possible
    buf.write(0);
    return (int) buf._offs;
}
