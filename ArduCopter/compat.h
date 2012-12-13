
#ifndef __COMPAT_H__
#define __COMPAT_H__

#define OUTPUT GPIO_OUTPUT
#define INPUT GPIO_INPUT

#define HIGH 1
#define LOW 0

/* Forward declarations to avoid broken auto-prototyper (coughs on '::'?) */
static void run_cli(AP_HAL::UARTDriver *port);

// XXX AP_HAL tofix - what is bitRead? temporarily disable.
#define bitRead(a,b) false

#endif // __COMPAT_H__

