//
// Created by jb030 on 27/12/2023.
//

#ifndef TIME_RECORD_H
#define TIME_RECORD_H

#include "./macro.h"

#define ENABLE_TIME_RECORD ON

#ifdef __cplusplus
extern "C" {
#endif

    void _init_time_record();
    void _record_time(const char * name, double time);
    void _start_named_timer(const char * name);
    void _stop_named_timer_and_record(const char * name);
    void _clear_time_record();
    void _print_time_record();
    void _increase_counter(const char * name);

# if ENABLE_TIME_RECORD
    #define init_time_record() _init_time_record()
    #define record_time(name, time) _record_time(name, time)
    #define start_named_timer(name) _start_named_timer(name)
    #define stop_named_timer_and_record(name) _stop_named_timer_and_record(name)
    #define clear_time_record() _clear_time_record()
    #define print_time_record() _print_time_record()
    #define increase_counter(name) _increase_counter(name)
# else
    #define init_time_record()
    #define record_time(name, time)
    #define start_named_timer(name)
    #define stop_named_timer_and_record(name)
    #define clear_time_record()
    #define print_time_record()
    #define increase_counter(name)
# endif



#ifdef __cplusplus
}
#endif

#define TIME_RECORD_H

#endif //TIME_RECORD_H
