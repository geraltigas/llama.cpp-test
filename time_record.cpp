//
// Created by jb030 on 27/12/2023.
//
#include <ctime>
#include <map>
#include <string>
#include "time_record.h"
#include <mutex>

// declare a map to store the time record
// key: name of the timer
// value: time of the timer
std::map<std::string, double> time_record;
std::map<std::string, double> delta_time_record;
// record times record
std::map<std::string, int> record_times;
// counter
std::map<std::string, int> counter;
// make this thread safe
std::mutex time_record_mutex;
std::mutex delta_time_record_mutex;
std::mutex record_times_mutex;
std::mutex counter_mutex;

void _init_time_record() {
    time_record.clear();
}
// make this thread safe
void _record_time(const char * name, double time) {
    // time_record[name] = time;

    time_record_mutex.lock();
    record_times_mutex.lock();
    record_times[name] = record_times[name] + 1;
    time_record[name] = time_record[name] + time;
    record_times_mutex.unlock();
    time_record_mutex.unlock();

}

void _increase_counter(const char * name) {
    counter_mutex.lock();
    if (counter.find(name) == counter.end()) {
        counter[name] = 0;
    }
    counter[name] = counter[name] + 1;
    counter_mutex.unlock();
}

void _start_named_timer(const char * name) {

    delta_time_record_mutex.lock();
    record_times_mutex.lock();
    record_times[name] = record_times[name] + 1;
    delta_time_record[name] = clock();
    record_times_mutex.unlock();
    delta_time_record_mutex.unlock();

}
void _stop_named_timer_and_record(const char * name) {

    delta_time_record_mutex.lock();
    time_record_mutex.lock();
    double delta_time = (clock() - delta_time_record[name])  * 1000 / (double)CLOCKS_PER_SEC;
    time_record[name] = time_record[name] + delta_time;
    time_record_mutex.unlock();
    delta_time_record_mutex.unlock();

}
void _clear_time_record() {

    time_record_mutex.lock();
    record_times_mutex.lock();
    time_record.clear();
    record_times.clear();
    record_times_mutex.unlock();
    time_record_mutex.unlock();

}

void print_begin_with(const char * prefix) {
    for (auto it = time_record.begin(); it != time_record.end(); ++it) {
        if (it->first.find(prefix) == 0) {
            printf("%s: %.3f, called %d times\n", it->first.c_str(), it->second, record_times[it->first]);
        }
    }
}

void print_begin_with_addup(const char * prefix) {
    double total_time = 0;
    for (auto it = time_record.begin(); it != time_record.end(); ++it) {
        if (it->first.find(prefix) == 0) {
            printf("%s: %.3f, called %d times\n", it->first.c_str(), it->second, record_times[it->first]);
            total_time += it->second;
        }
    }
    printf("Total time: %.3f\n", total_time);
}

std::map<std::string, std::map<std::string, size_t>> statistic_map;

void _create_statistic_map(const char * name) {
    // add a new map
    statistic_map[name] = std::map<std::string, size_t>();
}

void _add_statistic(const char * name, const char * key, size_t value) {
    statistic_map[name][key] = value;
}

void _self_increase_statistic(const char * name, const char * key) {
    if (statistic_map.find(name) == statistic_map.end()) {
        statistic_map[name] = std::map<std::string, size_t>();
    }
    if (statistic_map[name].find(key) == statistic_map[name].end()) {
        statistic_map[name][key] = 0;
    }
    statistic_map[name][key] = statistic_map[name][key] + 1;
}

void _print_statistic_map(const char * name) {
    printf("------ statistic map: %s\n", name);
    for (auto it = statistic_map[name].begin(); it != statistic_map[name].end(); ++it) {
        printf("%s: %zu\n", it->first.c_str(), it->second);
    }
}

void _print_time_record() {
    double total_time = 0;
    std::string max_op_name;
    double max_op_time = 0;
    for (auto it = time_record.begin(); it != time_record.end(); ++it) {
        // if the name start with _
        if (it->first[0] == '_') {
            continue;
        }
        printf("%s: %.3f, called %d times\n", it->first.c_str(), it->second, record_times[it->first]);
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

    printf("------ time record\n");
    print_begin_with_addup("_opencl");
    printf("------ kernel record\n");
    print_begin_with_addup("_opencl_kernel");
    printf("------ counter\n");
    for (auto it = counter.begin(); it != counter.end(); ++it) {
        printf("%s: %d\n", it->first.c_str(), it->second);
    }
    _print_statistic_map("GEMM");
}


