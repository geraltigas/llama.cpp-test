//
// Created by jb030 on 27/12/2023.
//

#ifndef TIME_RECORD_H

#ifdef __cplusplus
extern "C" {
#endif

    void init_time_record();
    void record_time(const char * name, double time);
    void start_named_timer(const char * name);
    void stop_named_timer_and_record(const char * name);
    void clear_time_record();
    void print_time_record();

#ifdef __cplusplus
}
#endif

#define TIME_RECORD_H

#endif //TIME_RECORD_H
