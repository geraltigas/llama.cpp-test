//
// Created by jb030 on 28/12/2023.
//

#include <CL/cl.h>
#ifndef PRINT_INFO_H

#define ENABLE_UNIFIED_MEMORY_OPTIMIZATION
// #define ENABLE_MATRIX_COMPARE

extern bool fp16_support;
extern bool unified_memory_support;
extern cl_platform_id _global_platform;
extern cl_device_id _global_device;
extern cl_context _global_context;
extern cl_command_queue _global_queue;
extern cl_program _global_program;

enum class buffer_type {
    READ_ONLY,
    WRITE_ONLY,
    READ_WRITE
};

void init_logging(const char* argv0);
void print_platforms();
void print_gpus();
void print_now_using(cl_platform_id platform, cl_device_id device);
void print_now_using();
// return multiple values: platform and device, function name is get_platform_and_device(const char *platform_name, const char *device_name)
std::tuple<cl_platform_id, cl_device_id> get_platform_and_device(const char *platform_name, const char *device_name);
void set_platform_and_device();
void setup_opencl_env();
cl_mem create_buffer(const buffer_type buffer_type, const size_t size, void* host_ptr);
void write_buffer(const cl_mem buffer, const size_t size, const void* host_ptr);
void build_all_kernels();
cl_kernel get_kernel(const char* kernel_name);
void add_kernel(const char* kernel_name, cl_kernel kernel);
void set_kernel_arg(cl_kernel kernel, cl_uint arg_index, size_t arg_size, const void* arg_value);
void run_kernel(cl_kernel kernel, const size_t global_size, const size_t local_size);
void read_buffer(const cl_mem buffer, const size_t size, void* host_ptr);
void release_opencl_env();
void check_unified_memory_support();
bool compare_matrix(cl_mem x, cl_mem _x,size_t size_f);


#define PRINT_INFO_H

#endif //PRINT_INFO_H
