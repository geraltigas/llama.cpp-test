//
// Created by jb030 on 28/12/2023.
//
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <cstdlib>
#include <iostream>
#include <map>
#include <set>
#include <my_opencl.h>
#include <glog/logging.h>

// CHECK_ERROR is a macro, it is defined in info.h
#define CHECK_ERROR(err, msg) \
    if (err != CL_SUCCESS) { \
        LOG(ERROR) << msg << std::endl; \
        exit(EXIT_FAILURE); \
    }

bool fp16_support;
bool unified_memory_support;
cl_platform_id _global_platform;
cl_device_id _global_device;
cl_context _global_context;
cl_command_queue _global_queue;
cl_program _global_program;
// buffers set
static std::set<cl_mem> _global_buffers;
// kernel map, key is kernel name, value is kernel
static std::map<std::string, cl_kernel> _global_kernels;

char* platform_name = "Intel(R) OpenCL HD Graphics";
//"NVIDIA CUDA";
char* device_name = "Intel(R) UHD Graphics";
//"NVIDIA GeForce RTX 2060";
char* kernels_file = "../kernels.cl";


void init_logging(const char* argv0) {
    google::InitGoogleLogging(argv0);
    // level INFO
    FLAGS_logtostdout = true;
    FLAGS_minloglevel = 0;
}

void print_platforms() {
    LOG(INFO) << "OpenCL Platforms:" << std::endl;
    cl_int err;
    cl_uint numPlatforms;
    cl_platform_id* platforms;
    err = clGetPlatformIDs(0, nullptr, &numPlatforms);
    CHECK_ERROR(err, "Failed to get platform IDs")
    platforms = (cl_platform_id *)malloc(sizeof(cl_platform_id) * numPlatforms);
    err = clGetPlatformIDs(numPlatforms, platforms, nullptr);
    CHECK_ERROR(err, "Failed to get platform IDs")
    for (unsigned int i = 0; i < numPlatforms; i++) {
        char platformName[128];
        err = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 128, platformName, nullptr);
        CHECK_ERROR(err, "Failed to get platform info")
        LOG(INFO) << "Platform " << i << ": " << platformName << std::endl;
    }
    free(platforms);
}

void print_gpus() {
    LOG(INFO) << "OpenCL GPUs:" << std::endl;
    cl_int err;
    cl_uint numPlatforms, numDevices;
    cl_platform_id* platforms;
    cl_device_id* devices;
    err = clGetPlatformIDs(0, nullptr, &numPlatforms);
    CHECK_ERROR(err, "Failed to get platform IDs")
    platforms = (cl_platform_id *)malloc(sizeof(cl_platform_id) * numPlatforms);
    err = clGetPlatformIDs(numPlatforms, platforms, nullptr);
    CHECK_ERROR(err, "Failed to get platform IDs")
    for (unsigned int i = 0; i < numPlatforms; i++) {
        err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, nullptr, &numDevices);
        if (err != CL_SUCCESS) {
            continue;
        }
        devices = (cl_device_id *)malloc(sizeof(cl_device_id) * numDevices);
        err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, numDevices, devices, nullptr);
        CHECK_ERROR(err, "Failed to get device IDs")
        for ( unsigned int j = 0; j < numDevices; j++) {
            char deviceName[128];
            err = clGetDeviceInfo(devices[j], CL_DEVICE_NAME, 128, deviceName, nullptr);
            CHECK_ERROR(err, "Failed to get device info")
            LOG(INFO) << "Platform " << i << ", GPU " << j << ": " << deviceName << std::endl;
        }
        free(devices);
    }
    free(platforms);
}

void print_now_using(cl_platform_id platform, cl_device_id device) {
    LOG(INFO) << "Now Using:" << std::endl;
    char platformName[128];
    char deviceName[128];
    clGetPlatformInfo(platform, CL_PLATFORM_NAME, 128, platformName, nullptr);
    clGetDeviceInfo(device, CL_DEVICE_NAME, 128, deviceName, nullptr);
    LOG(INFO) << "Platform: " << platformName << std::endl;
    LOG(INFO) << "Device: " << deviceName << std::endl;
}

void print_now_using() {
    print_now_using(_global_platform, _global_device);
}

void check_unified_memory_support() {
    cl_bool unifiedMemory;
    cl_int err;

    // 获取设备的统一内存属性
    err = clGetDeviceInfo(_global_device, CL_DEVICE_HOST_UNIFIED_MEMORY, sizeof(cl_bool), &unifiedMemory, NULL);
    if (err != CL_SUCCESS) {
        // 错误处理
        printf("Error getting device info: %d\n", err);
        unified_memory_support = false;
    }else {
        unified_memory_support = (unifiedMemory == CL_TRUE);
    }
}

std::tuple<cl_platform_id, cl_device_id> get_platform_and_device(const char* platform_name, const char* device_name) {
    LOG(INFO) << "Get Platform and Device:" << std::endl;
    cl_int err;
    cl_uint numPlatforms, numDevices;
    cl_platform_id* platforms;
    cl_device_id* devices;
    err = clGetPlatformIDs(0, nullptr, &numPlatforms);
    CHECK_ERROR(err, "Failed to get platform IDs")
    platforms = (cl_platform_id *)malloc(sizeof(cl_platform_id) * numPlatforms);
    err = clGetPlatformIDs(numPlatforms, platforms, nullptr);
    CHECK_ERROR(err, "Failed to get platform IDs")
    for (unsigned int i = 0; i < numPlatforms; i++) {
        char platformName[128];
        err = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 128, platformName, nullptr);
        CHECK_ERROR(err, "Failed to get platform info")
        if (strcmp(platformName, platform_name) == 0) {
            err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, nullptr, &numDevices);
            CHECK_ERROR(err, "Failed to get device IDs")
            devices = (cl_device_id *)malloc(sizeof(cl_device_id) * numDevices);
            err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, numDevices, devices, nullptr);
            CHECK_ERROR(err, "Failed to get device IDs")
            for ( unsigned int j = 0; j < numDevices; j++) {
                char deviceName[128];
                err = clGetDeviceInfo(devices[j], CL_DEVICE_NAME, 128, deviceName, nullptr);
                CHECK_ERROR(err, "Failed to get device info")
                if (strcmp(deviceName, device_name) == 0) {
                    LOG(INFO) << "Platform: " << platformName << std::endl;
                    LOG(INFO) << "Device: " << deviceName << std::endl;
                    return std::make_tuple(platforms[i], devices[j]);
                }
            }
            free(devices);
        }
    }
    free(platforms);
    return std::make_tuple(nullptr, nullptr);
}

void set_platform_and_device() {
    // use get_platform_and_device to get platform and device, and set them to _global_platform and _global_device
    auto tuple = get_platform_and_device(platform_name, device_name);
    _global_platform = std::get<0>(tuple);
    _global_device = std::get<1>(tuple);

    size_t ext_str_size;
    clGetDeviceInfo(_global_device, CL_DEVICE_EXTENSIONS, 0, NULL, &ext_str_size);
    char *ext_buffer = (char *)alloca(ext_str_size + 1);
    clGetDeviceInfo(_global_device, CL_DEVICE_EXTENSIONS, ext_str_size, ext_buffer, NULL);
    ext_buffer[ext_str_size] = '\0'; // ensure it is null terminated
    // Check if ext_buffer contains cl_khr_fp16
    fp16_support = strstr(ext_buffer, "cl_khr_fp16") != NULL;
    LOG(INFO) << "Device fp16 support: " << (fp16_support ? "true" : "false") << std::endl;

    // check unified memory support
    check_unified_memory_support();
    LOG(INFO) << "Device unified memory support: " << (unified_memory_support ? "true" : "false") << std::endl;
}

void setup_opencl_env() {
    // check _global_platform and _global_device
    if (_global_platform == nullptr || _global_device == nullptr) {
        LOG(ERROR) << "Platform or device is not initialized" << std::endl;
        exit(EXIT_FAILURE);
    }
    cl_context_properties properties[] = {
        (intptr_t)CL_CONTEXT_PLATFORM, (intptr_t)_global_platform, 0
    };
    // init context and queue
    cl_int err;
    _global_context = clCreateContext(properties, 1, &_global_device, NULL, NULL, &err);
        // clCreateContext(nullptr, 1, &_global_device, nullptr, nullptr, &err);
    CHECK_ERROR(err, "Failed to create context")
    _global_queue = clCreateCommandQueueWithProperties(_global_context, _global_device, nullptr, &err);
    CHECK_ERROR(err, "Failed to create command queue")
}

cl_mem create_buffer(const buffer_type buffer_type, const size_t size, void* host_ptr) {
    // check _global_context
    if (_global_context == nullptr) {
        LOG(ERROR) << "Context is not initialized" << std::endl;
        exit(EXIT_FAILURE);
    }
    cl_mem buffer;
    switch (buffer_type) {
        case buffer_type::READ_ONLY:
            buffer = clCreateBuffer(_global_context, CL_MEM_READ_ONLY, size, host_ptr, nullptr);
            break;
        case buffer_type::WRITE_ONLY:
            buffer = clCreateBuffer(_global_context, CL_MEM_WRITE_ONLY, size, host_ptr, nullptr);
            break;
        case buffer_type::READ_WRITE:
            buffer = clCreateBuffer(_global_context, CL_MEM_READ_WRITE, size, host_ptr, nullptr);
            break;
        default:
            LOG(ERROR) << "Unknown buffer type" << std::endl;
            exit(EXIT_FAILURE);
    }
    _global_buffers.insert(buffer);
    return buffer;
}

void write_buffer(const cl_mem buffer, const size_t size, const void* host_ptr) {
    // check _global_queue
    if (_global_queue == nullptr) {
        LOG(ERROR) << "Command queue is not initialized" << std::endl;
        exit(EXIT_FAILURE);
    }
    cl_int err = clEnqueueWriteBuffer(_global_queue, buffer, CL_TRUE, 0, size, host_ptr, 0, nullptr, nullptr);
    CHECK_ERROR(err, "Failed to write buffer")
}

void build_all_kernels() {
    cl_int err;
    // read kernels.cl
    FILE* fp = fopen(kernels_file, "rb");
    if (fp == nullptr) {
        LOG(ERROR) << "Failed to open file " << kernels_file << std::endl;
        exit(EXIT_FAILURE);
    }
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    rewind(fp);
    char* source = (char *)malloc(size + 1);
    source[size] = '\0';
    // read file
    fread(source, sizeof(char), size, fp);
    fclose(fp);
    // create program
    _global_program = clCreateProgramWithSource(_global_context, 1, (const char **)&source, nullptr, &err);
    CHECK_ERROR(err, "Failed to create program")
    err = clBuildProgram(_global_program, 1, &_global_device, nullptr, nullptr, nullptr);
    if (err == CL_BUILD_PROGRAM_FAILURE) {
        size_t log_size;
        clGetProgramBuildInfo(_global_program, _global_device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &log_size);

        char* log = (char *)malloc(log_size);
        clGetProgramBuildInfo(_global_program, _global_device, CL_PROGRAM_BUILD_LOG, log_size, log, nullptr);

        LOG(ERROR) << "Build log:" << std::endl << log << std::endl;

        free(log);
    }
    CHECK_ERROR(err, "Failed to build program")
    // get kernel count
    cl_uint num_kernels;
    err = clCreateKernelsInProgram(_global_program, 0, nullptr, &num_kernels);
    CHECK_ERROR(err, "Failed to get kernel count")

    // get kernel names
    std::vector<cl_kernel> kernels(num_kernels);
    err = clCreateKernelsInProgram(_global_program, num_kernels, kernels.data(), nullptr);
    CHECK_ERROR(err, "Failed to get kernel names")

    for (unsigned int i = 0; i < num_kernels; i++) {
        char kernel_name[128];
        err = clGetKernelInfo(kernels[i], CL_KERNEL_FUNCTION_NAME, 128, kernel_name, nullptr);
        CHECK_ERROR(err, "Failed to get kernel name")
        _global_kernels[kernel_name] = kernels[i];
    }
}

void add_kernel(const char* kernel_name, cl_kernel kernel) {
    if (_global_kernels.find(kernel_name) != _global_kernels.end()) {
        LOG(ERROR) << "Kernel " << kernel_name << " already exists" << std::endl;
        exit(EXIT_FAILURE);
    }
    _global_kernels[kernel_name] = kernel;
}

cl_kernel get_kernel(const char* kernel_name) {
    // check _global_kernels
    if (_global_kernels.empty()) {
        LOG(ERROR) << "Kernels are not initialized" << std::endl;
        exit(EXIT_FAILURE);
    }
    auto kernel = _global_kernels.find(kernel_name);
    if (kernel == _global_kernels.end()) {
        LOG(ERROR) << "Kernel " << kernel_name << " not found" << std::endl;
        exit(EXIT_FAILURE);
    }
    return kernel->second;
}

void set_kernel_arg(cl_kernel kernel, cl_uint arg_index, size_t arg_size, const void* arg_value) {
    cl_int err = clSetKernelArg(kernel, arg_index, arg_size, arg_value);
    CHECK_ERROR(err, "Failed to set kernel arg")
}

void run_kernel(cl_kernel kernel, const size_t global_size, const size_t local_size) {
    cl_int err = clEnqueueNDRangeKernel(_global_queue, kernel, 1, nullptr, &global_size, &local_size, 0, nullptr,
                                        nullptr);
    CHECK_ERROR(err, "Failed to run kernel")
}

void read_buffer(const cl_mem buffer, const size_t size, void* host_ptr) {
    cl_int err = clEnqueueReadBuffer(_global_queue, buffer, CL_TRUE, 0, size, host_ptr, 0, nullptr, nullptr);
    CHECK_ERROR(err, "Failed to read buffer")
}

void release_opencl_env() {
    // release buffers
    for (auto buffer: _global_buffers) {
        clReleaseMemObject(buffer);
    }
    // release kernels
    for (auto kernel: _global_kernels) {
        clReleaseKernel(kernel.second);
    }
    // release program
    clReleaseProgram(_global_program);
    // release command queue
    clReleaseCommandQueue(_global_queue);
    // release context
    clReleaseContext(_global_context);
}
