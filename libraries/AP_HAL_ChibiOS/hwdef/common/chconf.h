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
 * Modified for use in AP_HAL by Andrew Tridgell and Siddharth Bharat Purohit
 */

/**
 * @file    templates/chconf.h
 * @brief   Configuration file template.
 * @details A copy of this file must be placed in each project directory, it
 *          contains the application specific kernel settings.
 *
 * @addtogroup config
 * @details Kernel related settings and hooks.
 * @{
 */

#pragma once

#include "hwdef.h"

#define _CHIBIOS_RT_CONF_
#define _CHIBIOS_RT_CONF_VER_6_0_
/*===========================================================================*/
/**
 * @name System timers settings
 * @{
 */
/*===========================================================================*/

#if !defined(FALSE)
#define FALSE                               0
#endif

#if !defined(TRUE)
#define TRUE                                1
#endif

#ifdef HAL_CHIBIOS_ENABLE_ASSERTS
#define CH_DBG_ENABLE_ASSERTS TRUE
#define CH_DBG_ENABLE_CHECKS TRUE
#define CH_DBG_SYSTEM_STATE_CHECK TRUE
#undef CH_DBG_ENABLE_STACK_CHECK
#define CH_DBG_ENABLE_STACK_CHECK TRUE

// Generate assertions on a GPIO pin
#ifdef HAL_GPIO_PIN_FAULT
#ifndef _FROM_ASM_
#ifdef __cplusplus
extern "C" {
#endif
  void fault_printf(const char *fmt, ...);
#ifdef __cplusplus
}
#endif
#endif
#define osalDbgAssert(c, remark) do { if (!(c)) { fault_printf("%s:%d: %s", __FILE__, __LINE__, remark ); chDbgAssert(c, remark); } } while (0)
#endif

#define PORT_INT_REQUIRED_STACK 256
#endif

#if HAL_ENABLE_THREAD_STATISTICS
#define CH_DBG_STATISTICS TRUE
#else
#define CH_DBG_STATISTICS FALSE
#endif

/**
 * @brief   System time counter resolution.
 * @note    Allowed values are 16 or 32 bits.
 */
#ifndef CH_CFG_ST_RESOLUTION
#define CH_CFG_ST_RESOLUTION                32
#endif

/**
 * @brief   System tick frequency.
 * @details Frequency of the system timer that drives the system ticks. This
 *          setting also defines the system tick time unit. We set this to 1000000
 *          in ArduPilot so we get maximum resolution for timing of delays
 */
#ifndef CH_CFG_ST_FREQUENCY
#define CH_CFG_ST_FREQUENCY                 1000000
#endif

/**
 * @brief   Time intervals data size.
 * @note    Allowed values are 16, 32 or 64 bits.
 */
#if !defined(CH_CFG_INTERVALS_SIZE)
#define CH_CFG_INTERVALS_SIZE               32
#endif

/**
 * @brief   Time types data size.
 * @note    Allowed values are 16 or 32 bits.
 */
#if !defined(CH_CFG_TIME_TYPES_SIZE)
#define CH_CFG_TIME_TYPES_SIZE              32
#endif

/**
 * @brief   Time delta constant for the tick-less mode.
 * @note    If this value is zero then the system uses the classic
 *          periodic tick. This value represents the minimum number
 *          of ticks that is safe to specify in a timeout directive.
 *          The value one is not valid, timeouts are rounded up to
 *          this value.
 */
#ifndef CH_CFG_ST_TIMEDELTA
#define CH_CFG_ST_TIMEDELTA                 2
#endif

/*
  default to a large interrupt stack for now. We may trim this later
  if we become confident of our interrupt handler requirements. Note
  that we pay for this stack size in every thread, so it is quite
  expensive in memory
 */
#ifndef PORT_INT_REQUIRED_STACK
#define PORT_INT_REQUIRED_STACK 128
#endif


/** @} */

/*===========================================================================*/
/**
 * @name Kernel parameters and options
 * @{
 */
/*===========================================================================*/

/**
 * @brief   Round robin interval.
 * @details This constant is the number of system ticks allowed for the
 *          threads before preemption occurs. Setting this value to zero
 *          disables the preemption for threads with equal priority and the
 *          round robin becomes cooperative. Note that higher priority
 *          threads can still preempt, the kernel is always preemptive.
 * @note    Disabling the round robin preemption makes the kernel more compact
 *          and generally faster.
 * @note    The round robin preemption is not supported in tickless mode and
 *          must be set to zero in that case.
 */
#if !defined(CH_CFG_TIME_QUANTUM)
#define CH_CFG_TIME_QUANTUM                 0
#endif

/**
 * @brief   Managed RAM size.
 * @details Size of the RAM area to be managed by the OS. If set to zero
 *          then the whole available RAM is used. The core memory is made
 *          available to the heap allocator and/or can be used directly through
 *          the simplified core memory allocator.
 *
 * @note    In order to let the OS manage the whole RAM the linker script must
 *          provide the @p __heap_base__ and @p __heap_end__ symbols.
 * @note    Requires @p CH_CFG_USE_MEMCORE.
 */
#if !defined(CH_CFG_MEMCORE_SIZE)
#define CH_CFG_MEMCORE_SIZE                 0
#endif

/**
 * @brief   Idle thread automatic spawn suppression.
 * @details When this option is activated the function @p chSysInit()
 *          does not spawn the idle thread. The application @p main()
 *          function becomes the idle thread and must implement an
 *          infinite loop.
 */
#if !defined(CH_CFG_NO_IDLE_THREAD)
#define CH_CFG_NO_IDLE_THREAD               FALSE
#endif

/** @} */

/*===========================================================================*/
/**
 * @name Performance options
 * @{
 */
/*===========================================================================*/

/**
 * @brief   OS optimization.
 * @details If enabled then time efficient rather than space efficient code
 *          is used when two possible implementations exist.
 *
 * @note    This is not related to the compiler optimization options.
 * @note    The default is @p TRUE.
 */
#if !defined(CH_CFG_OPTIMIZE_SPEED)
#define CH_CFG_OPTIMIZE_SPEED               TRUE
#endif

/** @} */

/*===========================================================================*/
/**
 * @name Subsystem options
 * @{
 */
/*===========================================================================*/

/**
 * @brief   Time Measurement APIs.
 * @details If enabled then the time measurement APIs are included in
 *          the kernel.
 *
 * @note    The default is @p TRUE.
 */
#if !defined(CH_CFG_USE_TM)
#define CH_CFG_USE_TM                       TRUE
#endif

/**
 * @brief   Threads registry APIs.
 * @details If enabled then the registry APIs are included in the kernel.
 *
 * @note    The default is @p TRUE.
 */
#if !defined(CH_CFG_USE_REGISTRY)
#define CH_CFG_USE_REGISTRY                 TRUE
#endif

/**
 * @brief   Threads synchronization APIs.
 * @details If enabled then the @p chThdWait() function is included in
 *          the kernel.
 *
 * @note    The default is @p TRUE.
 */
#if !defined(CH_CFG_USE_WAITEXIT)
#define CH_CFG_USE_WAITEXIT                 TRUE
#endif

/**
 * @brief   Semaphores APIs.
 * @details If enabled then the Semaphores APIs are included in the kernel.
 *
 * @note    The default is @p TRUE.
 */
#if !defined(CH_CFG_USE_SEMAPHORES)
#define CH_CFG_USE_SEMAPHORES               TRUE
#endif

/**
 * @brief   Semaphores queuing mode.
 * @details If enabled then the threads are enqueued on semaphores by
 *          priority rather than in FIFO order.
 *
 * @note    The default is @p FALSE. Enable this if you have special
 *          requirements.
 * @note    Requires @p CH_CFG_USE_SEMAPHORES.
 */
#if !defined(CH_CFG_USE_SEMAPHORES_PRIORITY)
#define CH_CFG_USE_SEMAPHORES_PRIORITY      FALSE
#endif

/**
 * @brief   Mutexes APIs.
 * @details If enabled then the mutexes APIs are included in the kernel.
 *
 * @note    The default is @p TRUE.
 */
#if !defined(CH_CFG_USE_MUTEXES)
#define CH_CFG_USE_MUTEXES                  TRUE
#endif

/**
 * @brief   Enables recursive behavior on mutexes.
 * @note    Recursive mutexes are heavier and have an increased
 *          memory footprint.
 *
 * @note    The default is @p FALSE.
 * @note    Requires @p CH_CFG_USE_MUTEXES.
 */
#if !defined(CH_CFG_USE_MUTEXES_RECURSIVE)
#define CH_CFG_USE_MUTEXES_RECURSIVE        TRUE
#endif

/**
 * @brief   Conditional Variables APIs.
 * @details If enabled then the conditional variables APIs are included
 *          in the kernel.
 *
 * @note    The default is @p TRUE.
 * @note    Requires @p CH_CFG_USE_MUTEXES.
 */
#if !defined(CH_CFG_USE_CONDVARS)
#define CH_CFG_USE_CONDVARS                 TRUE
#endif

/**
 * @brief   Conditional Variables APIs with timeout.
 * @details If enabled then the conditional variables APIs with timeout
 *          specification are included in the kernel.
 *
 * @note    The default is @p TRUE.
 * @note    Requires @p CH_CFG_USE_CONDVARS.
 */
#if !defined(CH_CFG_USE_CONDVARS_TIMEOUT)
#define CH_CFG_USE_CONDVARS_TIMEOUT         TRUE
#endif

/**
 * @brief   Events Flags APIs.
 * @details If enabled then the event flags APIs are included in the kernel.
 *
 * @note    The default is @p TRUE.
 */
#if !defined(CH_CFG_USE_EVENTS)
#define CH_CFG_USE_EVENTS                   TRUE
#endif

/**
 * @brief   Events Flags APIs with timeout.
 * @details If enabled then the events APIs with timeout specification
 *          are included in the kernel.
 *
 * @note    The default is @p TRUE.
 * @note    Requires @p CH_CFG_USE_EVENTS.
 */
#if !defined(CH_CFG_USE_EVENTS_TIMEOUT)
#define CH_CFG_USE_EVENTS_TIMEOUT           TRUE
#endif

/**
 * @brief   Synchronous Messages APIs.
 * @details If enabled then the synchronous messages APIs are included
 *          in the kernel.
 *
 * @note    The default is @p TRUE.
 */
#if !defined(CH_CFG_USE_MESSAGES)
#define CH_CFG_USE_MESSAGES                 TRUE
#endif

/**
 * @brief   Synchronous Messages queuing mode.
 * @details If enabled then messages are served by priority rather than in
 *          FIFO order.
 *
 * @note    The default is @p FALSE. Enable this if you have special
 *          requirements.
 * @note    Requires @p CH_CFG_USE_MESSAGES.
 */
#if !defined(CH_CFG_USE_MESSAGES_PRIORITY)
#define CH_CFG_USE_MESSAGES_PRIORITY        FALSE
#endif

/**
 * @brief   Mailboxes APIs.
 * @details If enabled then the asynchronous messages (mailboxes) APIs are
 *          included in the kernel.
 *
 * @note    The default is @p TRUE.
 * @note    Requires @p CH_CFG_USE_SEMAPHORES.
 */
#if !defined(CH_CFG_USE_MAILBOXES)
#define CH_CFG_USE_MAILBOXES                TRUE
#endif

/**
 * @brief   Core Memory Manager APIs.
 * @details If enabled then the core memory manager APIs are included
 *          in the kernel.
 *
 * @note    The default is @p TRUE.
 */
#if !defined(CH_CFG_USE_MEMCORE)
#define CH_CFG_USE_MEMCORE                  TRUE
#endif

/**
 * @brief   Heap Allocator APIs.
 * @details If enabled then the memory heap allocator APIs are included
 *          in the kernel.
 *
 * @note    The default is @p TRUE.
 * @note    Requires @p CH_CFG_USE_MEMCORE and either @p CH_CFG_USE_MUTEXES or
 *          @p CH_CFG_USE_SEMAPHORES.
 * @note    Mutexes are recommended.
 */
#if !defined(CH_CFG_USE_HEAP)
#define CH_CFG_USE_HEAP                     TRUE
#endif

/**
 * @brief   Memory Pools Allocator APIs.
 * @details If enabled then the memory pools allocator APIs are included
 *          in the kernel.
 *
 * @note    The default is @p TRUE.
 */
#if !defined(CH_CFG_USE_MEMPOOLS)
#define CH_CFG_USE_MEMPOOLS                 TRUE
#endif

/**
 * @brief  Objects FIFOs APIs.
 * @details If enabled then the objects FIFOs APIs are included
 *          in the kernel.
 *
 * @note    The default is @p TRUE.
 */
#if !defined(CH_CFG_USE_OBJ_FIFOS)
#define CH_CFG_USE_OBJ_FIFOS                TRUE
#endif

/**
 * @brief   Pipes APIs.
 * @details If enabled then the pipes APIs are included
 *          in the kernel.
 *
 * @note    The default is @p TRUE.
 */
#if !defined(CH_CFG_USE_PIPES)
#define CH_CFG_USE_PIPES                    FALSE
#endif

/**
 * @brief   Dynamic Threads APIs.
 * @details If enabled then the dynamic threads creation APIs are included
 *          in the kernel.
 *
 * @note    The default is @p TRUE.
 * @note    Requires @p CH_CFG_USE_WAITEXIT.
 * @note    Requires @p CH_CFG_USE_HEAP and/or @p CH_CFG_USE_MEMPOOLS.
 */
#if !defined(CH_CFG_USE_DYNAMIC)
#define CH_CFG_USE_DYNAMIC                  TRUE
#endif

/** @} */

/*===========================================================================*/
/**
 * @name Objects factory options
 * @{
 */
/*===========================================================================*/

/**
 * @brief   Objects Factory APIs.
 * @details If enabled then the objects factory APIs are included in the
 *          kernel.
 *
 * @note    The default is @p FALSE.
 */
#if !defined(CH_CFG_USE_FACTORY)
#define CH_CFG_USE_FACTORY                  TRUE
#endif

/**
 * @brief   Maximum length for object names.
 * @details If the specified length is zero then the name is stored by
 *          pointer but this could have unintended side effects.
 */
#if !defined(CH_CFG_FACTORY_MAX_NAMES_LENGTH)
#define CH_CFG_FACTORY_MAX_NAMES_LENGTH     8
#endif

/**
 * @brief   Enables the registry of generic objects.
 */
#if !defined(CH_CFG_FACTORY_OBJECTS_REGISTRY)
#define CH_CFG_FACTORY_OBJECTS_REGISTRY     TRUE
#endif

/**
 * @brief   Enables factory for generic buffers.
 */
#if !defined(CH_CFG_FACTORY_GENERIC_BUFFERS)
#define CH_CFG_FACTORY_GENERIC_BUFFERS      TRUE
#endif

/**
 * @brief   Enables factory for semaphores.
 */
#if !defined(CH_CFG_FACTORY_SEMAPHORES)
#define CH_CFG_FACTORY_SEMAPHORES           TRUE
#endif

/**
 * @brief   Enables factory for mailboxes.
 */
#if !defined(CH_CFG_FACTORY_MAILBOXES)
#define CH_CFG_FACTORY_MAILBOXES            TRUE
#endif

/**
 * @brief   Enables factory for objects FIFOs.
 */
#if !defined(CH_CFG_FACTORY_OBJ_FIFOS)
#define CH_CFG_FACTORY_OBJ_FIFOS            TRUE
#endif

/**
 * @brief   Enables factory for Pipes.
 */
#if !defined(CH_CFG_FACTORY_PIPES) || defined(__DOXYGEN__)
#define CH_CFG_FACTORY_PIPES                FALSE
#endif

/** @} */

/*===========================================================================*/
/**
 * @name Debug options
 * @{
 */
/*===========================================================================*/

/**
 * @brief   Debug option, kernel statistics.
 *
 * @note    The default is @p FALSE.
 */
#if !defined(CH_DBG_STATISTICS)
#define CH_DBG_STATISTICS                   FALSE
#endif

/**
 * @brief   Debug option, system state check.
 * @details If enabled the correct call protocol for system APIs is checked
 *          at runtime.
 *
 * @note    The default is @p FALSE.
 */
#if !defined(CH_DBG_SYSTEM_STATE_CHECK)
#define CH_DBG_SYSTEM_STATE_CHECK           FALSE
#endif

/**
 * @brief   Debug option, parameters checks.
 * @details If enabled then the checks on the API functions input
 *          parameters are activated.
 *
 * @note    The default is @p FALSE.
 */
#if !defined(CH_DBG_ENABLE_CHECKS)
#define CH_DBG_ENABLE_CHECKS                FALSE
#endif

/**
 * @brief   Debug option, consistency checks.
 * @details If enabled then all the assertions in the kernel code are
 *          activated. This includes consistency checks inside the kernel,
 *          runtime anomalies and port-defined checks.
 *
 * @note    The default is @p FALSE.
 */
#if !defined(CH_DBG_ENABLE_ASSERTS)
#define CH_DBG_ENABLE_ASSERTS               FALSE
#endif

/**
 * @brief   Debug option, trace buffer.
 * @details If enabled then the trace buffer is activated.
 *
 * @note    The default is @p CH_DBG_TRACE_MASK_DISABLED.
 */
#if !defined(CH_DBG_TRACE_MASK)
#define CH_DBG_TRACE_MASK                   CH_DBG_TRACE_MASK_DISABLED
#endif

/**
 * @brief   Trace buffer entries.
 * @note    The trace buffer is only allocated if @p CH_DBG_TRACE_MASK is
 *          different from @p CH_DBG_TRACE_MASK_DISABLED.
 */
#if !defined(CH_DBG_TRACE_BUFFER_SIZE)
#define CH_DBG_TRACE_BUFFER_SIZE            128
#endif

/**
 * @brief   Debug option, stack checks.
 * @details If enabled then a runtime stack check is performed.
 *
 * @note    The default is @p FALSE.
 * @note    The stack check is performed in a architecture/port dependent way.
 *          It may not be implemented or some ports.
 * @note    The default failure mode is to halt the system with the global
 *          @p panic_msg variable set to @p NULL.
 */
#if !defined(CH_DBG_ENABLE_STACK_CHECK)
#define CH_DBG_ENABLE_STACK_CHECK           TRUE
#endif

/**
 * @brief   Debug option, stacks initialization.
 * @details If enabled then the threads working area is filled with a byte
 *          value when a thread is created. This can be useful for the
 *          runtime measurement of the used stack.
 *
 * @note    The default is @p FALSE.
 */
#if !defined(CH_DBG_FILL_THREADS)
#define CH_DBG_FILL_THREADS                 TRUE
#endif

/**
 * @brief   Debug option, threads profiling.
 * @details If enabled then a field is added to the @p thread_t structure that
 *          counts the system ticks occurred while executing the thread.
 *
 * @note    The default is @p FALSE.
 * @note    This debug option is not currently compatible with the
 *          tickless mode.
 */
#if !defined(CH_DBG_THREADS_PROFILING)
#define CH_DBG_THREADS_PROFILING            FALSE
#endif

/** @} */

/*===========================================================================*/
/**
 * @name Kernel hooks
 * @{
 */
/*===========================================================================*/

/**
 * @brief   System structure extension.
 * @details User fields added to the end of the @p ch_system_t structure.
 */
#define CH_CFG_SYSTEM_EXTRA_FIELDS                                          \
  /* Add threads custom fields here.*/

/**
 * @brief   System initialization hook.
 * @details User initialization code added to the @p chSysInit() function
 *          just before interrupts are enabled globally.
 */
#define CH_CFG_SYSTEM_INIT_HOOK() {                                         \
  /* Add threads initialization code here.*/                                \
}

/**
 * @brief   Threads descriptor structure extension.
 * @details User fields added to the end of the @p thread_t structure.
 */
#define CH_CFG_THREAD_EXTRA_FIELDS                                          \
  /* Add threads custom fields here.*/

/**
 * @brief   Threads initialization hook.
 * @details User initialization code added to the @p _thread_init() function.
 *
 * @note    It is invoked from within @p _thread_init() and implicitly from all
 *          the threads creation APIs.
 */
#define CH_CFG_THREAD_INIT_HOOK(tp) {                                       \
  /* Add threads initialization code here.*/                                \
}

/**
 * @brief   Threads finalization hook.
 * @details User finalization code added to the @p chThdExit() API.
 */
#define CH_CFG_THREAD_EXIT_HOOK(tp) {                                       \
  /* Add threads finalization code here.*/                                  \
}

/**
 * @brief   Context switch hook.
 * @details This hook is invoked just before switching between threads.
 */
#define CH_CFG_CONTEXT_SWITCH_HOOK(ntp, otp) {                              \
  /* Context switch code here.*/                                            \
}

/**
 * @brief   ISR enter hook.
 */
#define CH_CFG_IRQ_PROLOGUE_HOOK() {                                        \
  /* IRQ prologue code here.*/                                              \
}

/**
 * @brief   ISR exit hook.
 */
#define CH_CFG_IRQ_EPILOGUE_HOOK() {                                        \
  /* IRQ epilogue code here.*/                                              \
}

/**
 * @brief   Idle thread enter hook.
 * @note    This hook is invoked within a critical zone, no OS functions
 *          should be invoked from here.
 * @note    This macro can be used to activate a power saving mode.
 */
#define CH_CFG_IDLE_ENTER_HOOK() {                                          \
  /* Idle-enter code here.*/                                                \
}

/**
 * @brief   Idle thread leave hook.
 * @note    This hook is invoked within a critical zone, no OS functions
 *          should be invoked from here.
 * @note    This macro can be used to deactivate a power saving mode.
 */
#define CH_CFG_IDLE_LEAVE_HOOK() {                                          \
  /* Idle-leave code here.*/                                                \
}

/**
 * @brief   Idle Loop hook.
 * @details This hook is continuously invoked by the idle thread loop.
 */
#define CH_CFG_IDLE_LOOP_HOOK() {                                           \
  /* Idle loop code here.*/                                                 \
}

/**
 * @brief   System tick event hook.
 * @details This hook is invoked in the system tick handler immediately
 *          after processing the virtual timers queue.
 */
#define CH_CFG_SYSTEM_TICK_HOOK() {                                         \
  /* System tick event code here.*/                                         \
}

/**
 * @brief   System halt hook.
 * @details This hook is invoked in case to a system halting error before
 *          the system is halted.
 *
 * We flush all memory on STM32F7 to allow gdb to access variables currently
 * in the dcache
 */
#ifndef CH_CFG_SYSTEM_HALT_HOOK
#define CH_CFG_SYSTEM_HALT_HOOK(reason) do {                               \
        extern void memory_flush_all(void); \
        memory_flush_all(); \
        extern void system_halt_hook(void); \
        system_halt_hook(); \
} while(0)
#endif

/**
 * @brief   stack overflow event hook.
 * @details This hook is invoked when we have a stack overflow on task switch
 */
#define CH_CFG_STACK_OVERFLOW_HOOK(tp) {                                         \
  extern void stack_overflow(thread_t *tp); \
  stack_overflow(tp); \
}

/**
 * @brief   Trace hook.
 * @details This hook is invoked each time a new record is written in the
 *          trace buffer.
 */
#define CH_CFG_TRACE_HOOK(tep) {                                            \
  /* Trace code here.*/                                                     \
}

/** @} */

/*===========================================================================*/
/* Port-specific settings (override port settings defaulted in chcore.h).    */
/*===========================================================================*/


/** @} */
