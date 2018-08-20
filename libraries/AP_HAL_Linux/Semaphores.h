#pragma once

#include <AP_HAL/AP_HAL_Boards.h>
#include <stdint.h>
#include <AP_HAL/AP_HAL_Macros.h>
#include <AP_HAL/Semaphores.h>
#include <pthread.h>

namespace Linux {

class Semaphore : public AP_HAL::Semaphore {
public:
    Semaphore();
    bool give();
    bool take(uint32_t timeout_ms);
    bool take_nonblocking();
protected:
    pthread_mutex_t _lock;
};

class Semaphore_Recursive : public Semaphore {
public:
    Semaphore_Recursive();
};
    
}
