//
// Created by jb030 on 27/12/2023.
//
#include <ctime>
#include <map>
#include <string>
#include "time_record.h"

// declare a map to store the time record
// key: name of the timer
// value: time of the timer
std::map<std::string, double> time_record;
std::map<std::string, double> delta_time_record;

void init_time_record() {
    time_record.clear();
}
void record_time(const char * name, double time) {
    time_record[name] = time;
}
void start_named_timer(const char * name) {
    delta_time_record[name] = clock();
}
void stop_named_timer_and_record(const char * name) {
    double delta_time = (clock() - delta_time_record[name]) / (double)CLOCKS_PER_SEC;
    time_record[name] = time_record[name] + delta_time;
}
void clear_time_record() {
    time_record.clear();
}
void print_time_record() {
    double total_time = 0;
    std::string max_op_name;
    double max_op_time = 0;
    for (auto it = time_record.begin(); it != time_record.end(); ++it) {
        printf("%s: %.3f\n", it->first.c_str(), it->second);
        total_time += it->second;
        if (it->second > max_op_time) {
            max_op_time = it->second;
            max_op_name = it->first;
        }
    }
    double other_time = total_time - max_op_time;
    double max_percent = max_op_time / total_time * 100;
    printf("Total time: %.3f\n", total_time);
    printf("Max op: %s, %.3f\n", max_op_name.c_str(), max_op_time);
    printf("Max percent: %.3f%%\n", max_percent);
    printf("Other time: %.3f\n", other_time);
}