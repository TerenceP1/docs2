#include <CL/cl.h>
#include <iostream>
#include <vector>
#include <string>

// Global OpenCL variables
cl_platform_id platform = nullptr;
cl_device_id device = nullptr;
cl_context context = nullptr;
cl_command_queue queue = nullptr;
cl_program program = nullptr;
cl_kernel kernel = nullptr;
cl_mem img;

void checkError(cl_int err, const char *name)
{
    if (err != CL_SUCCESS)
    {
        std::cerr << "Error: " << name << " (" << err << ")" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void init_cl()
{
    cl_int err;

    // 1. Get platform
    cl_uint num_platforms = 0;
    err = clGetPlatformIDs(0, nullptr, &num_platforms);
    checkError(err, "clGetPlatformIDs count");
    if (num_platforms == 0)
    {
        std::cerr << "No OpenCL platforms found." << std::endl;
        exit(EXIT_FAILURE);
    }
    std::vector<cl_platform_id> platforms(num_platforms);
    err = clGetPlatformIDs(num_platforms, platforms.data(), nullptr);
    checkError(err, "clGetPlatformIDs");

    platform = platforms[0];

    // 2. Get device (GPU preferred, fallback CPU)
    cl_uint num_devices = 0;
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, nullptr, &num_devices);
    if (err != CL_SUCCESS || num_devices == 0)
    {
        std::cout << "No GPU device found. Trying CPU..." << std::endl;
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 0, nullptr, &num_devices);
        checkError(err, "clGetDeviceIDs CPU");
        if (num_devices == 0)
        {
            std::cerr << "No CPU devices found either." << std::endl;
            exit(EXIT_FAILURE);
        }
        std::vector<cl_device_id> devices(num_devices);
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, num_devices, devices.data(), nullptr);
        checkError(err, "clGetDeviceIDs CPU list");
        device = devices[0];
    }
    else
    {
        std::vector<cl_device_id> devices(num_devices);
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, num_devices, devices.data(), nullptr);
        checkError(err, "clGetDeviceIDs GPU list");
        device = devices[0];
    }

    // 3. Create context
    context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
    checkError(err, "clCreateContext");

    // 4. Create command queue
#if CL_TARGET_OPENCL_VERSION >= 200
    queue = clCreateCommandQueueWithProperties(context, device, 0, &err);
#else
    queue = clCreateCommandQueue(context, device, 0, &err);
#endif
    checkError(err, "clCreateCommandQueue");

    // 5. Create and build program
    const char *kernelSource = R"(
inline void hueToRGB(float hue, int* r, int* g, int* b) {
    float s = 1.0f;
    float v = 1.0f;

    float c = v * s;
    float x = c * (1 - fabsf(fmodf(hue / 60.0f, 2.0f) - 1));
    float m = v - c;

    float rf = 0, gf = 0, bf = 0;
    if (hue < 60) {
        rf = c; gf = x; bf = 0;
    } else if (hue < 120) {
        rf = x; gf = c; bf = 0;
    } else if (hue < 180) {
        rf = 0; gf = c; bf = x;
    } else if (hue < 240) {
        rf = 0; gf = x; bf = c;
    } else if (hue < 300) {
        rf = x; gf = 0; bf = c;
    } else {
        rf = c; gf = 0; bf = x;
    }

    *r = (int)((rf + m) * 255.0f);
    *g = (int)((gf + m) * 255.0f);
    *b = (int)((bf + m) * 255.0f);
}
#define width 3840 // 4K width
#define height 2160 // 4K height
__kernel void mandelbrot_thread(int max_itr,double re, double im, unsigned char *img, double zoom){
    int x = get_global_id(0); // x-coordinate (width)
    int y = get_global_id(1); // y-coordinate (height)
    double aspect=(double)width / height;
    double x0 = re + ((double)x / (double)width * 2.0 - 1.0) * zoom * aspect;
    double y0 = im + ((double)y / (double)height * 2.0 - 1.0) * zoom;
    double cRe=0.0,cIm=0.0,tIm,reSqTmp,imSqTmp;
    int i;
    for (i=0;i<max_itr;i++){
        imSqTmp=cIm*cIm;
        reSqTmp=cRe*cRe;
        cIm*=cRe;
        cIm*=2.0;
        cRe=reSqTmp-imSqTmp;
        cRe+=x0;
        cIm+=y0;
        reSqTmp+=imSqTmp;
        if (reSqTmp>4.0){
            break;
        }
    }
    int r=0,g=0,b=0;
    if (i!=max_itr){
        hueToRGB((double)(i%100)/100.0*360.0,&r,&g,&b);
    }
    img[x*3+y*width*3]=r;
    img[x*3+y*width*3+1]=g;
    img[x*3+y*width*3+2]=b;
}
    )";

    program = clCreateProgramWithSource(context, 1, &kernelSource, nullptr, &err);
    checkError(err, "clCreateProgramWithSource");

    err = clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);
    if (err != CL_SUCCESS)
    {
        // Print build log on error
        size_t log_size = 0;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &log_size);
        std::vector<char> log(log_size);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, log.data(), nullptr);
        std::cerr << "Build log:\n"
                  << log.data() << std::endl;
        checkError(err, "clBuildProgram");
    }

    // 6. Create kernel
    kernel = clCreateKernel(program, "dummy_kernel", &err);
    checkError(err, "clCreateKernel");

    // Print device name
    char device_name[128];
    clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_name), device_name, nullptr);
    std::cout << "Using device: " << device_name << std::endl;

    img = clCreateBuffer(context, CL_MEM_READ_WRITE, 2160 * 3840 * 3, NULL, &err);
    checkError(err, "clCreateBuffer");
}

int main()
{
    init_cl();

    // Cleanup OpenCL resources
    if (kernel)
        clReleaseKernel(kernel);
    if (program)
        clReleaseProgram(program);
    if (queue)
        clReleaseCommandQueue(queue);
    if (context)
        clReleaseContext(context);
    if (device)
        clReleaseDevice(device);

    return 0;
}
