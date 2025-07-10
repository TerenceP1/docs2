#include <CL/cl.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <Windows.h>
#include <thread>
using namespace cv;

#ifndef CL_QUEUE_PRIORITY_KHR
#define CL_QUEUE_PRIORITY_KHR 0x1096
#endif
#ifndef CL_QUEUE_PRIORITY_HIGH_KHR
#define CL_QUEUE_PRIORITY_HIGH_KHR 0x1097
#endif

#define width 3840  // 4K width
#define height 2160 // 4K height

int speclen;

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

bool checkPriorityHintsSupport(cl_device_id device)
{
    size_t ext_size = 0;
    clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, 0, nullptr, &ext_size);
    std::vector<char> ext_data(ext_size);
    clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, ext_size, ext_data.data(), nullptr);
    std::string extensions(ext_data.begin(), ext_data.end());

    std::cout << "Device extensions:\n"
              << extensions << "\n";

    return extensions.find("cl_khr_priority_hints") != std::string::npos;
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
    checkPriorityHintsSupport(device);
    // 3. Create context
    context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
    checkError(err, "clCreateContext");

    // 4. Create command queue
#if CL_TARGET_OPENCL_VERSION >= 200
    cl_queue_properties props_priority[] = {
        CL_QUEUE_PROPERTIES, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE,
        0};
    queue = clCreateCommandQueueWithProperties(context, device, props_priority, &err);
    checkError(err, "clCreateCommandQueueWithProperties");
#else
    queue = clCreateCommandQueue(context, device, 0, &err);
    checkError(err, "clCreateCommandQueue");
#endif

    // 5. Create and build program
    const char *kernelSource = R"(
void hueToRGB(float hue, int* r, int* g, int* b) {
    float s = 1.0f;
    float v = 1.0f;

    float c = v * s;
    float x = c * (1.0f - fabs(fmod(hue / 60.0f, 2.0f) - 1.0f));
    float m = v - c;

    float rf = 0.0f, gf = 0.0f, bf = 0.0f;
    if (hue < 60.0f) {
        rf = c; gf = x; bf = 0.0f;
    } else if (hue < 120.0f) {
        rf = x; gf = c; bf = 0.0f;
    } else if (hue < 180.0f) {
        rf = 0.0f; gf = c; bf = x;
    } else if (hue < 240.0f) {
        rf = 0.0f; gf = x; bf = c;
    } else if (hue < 300.0f) {
        rf = x; gf = 0.0f; bf = c;
    } else {
        rf = c; gf = 0.0f; bf = x;
    }

    *r = (int)((rf + m) * 255.0f);
    *g = (int)((gf + m) * 255.0f);
    *b = (int)((bf + m) * 255.0f);
}
#define width 3840 // 4K width
#define height 2160 // 4K height
__kernel void mandelbrot_thread(int max_itr,double re, double im, __global unsigned char *img, double zoom,int speclen){
    int x = get_global_id(0); // x-coordinate (width)
    int y = get_global_id(1); // y-coordinate (height)
    if (x>=width){
        return;
    }
    if (y>=height) return;
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
        hueToRGB((double)(i%speclen)/((double)speclen)*360.0,&r,&g,&b);
    }
    img[x*3+y*width*3]=b;
    img[x*3+y*width*3+1]=g;
    img[x*3+y*width*3+2]=r;
}
    )";
    /*     std::string stub = std::string("#define speclen ") + std::to_string(speclen) + "\n" + std::string(kernelSource);
        kernelSource = stub.c_str(); */
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
    kernel = clCreateKernel(program, "mandelbrot_thread", &err);
    checkError(err, "clCreateKernel");

    // Print device name
    char device_name[128];
    clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_name), device_name, nullptr);
    std::cout << "Using device: " << device_name << std::endl;

    img = clCreateBuffer(context, CL_MEM_READ_WRITE, 2160 * 3840 * 3, NULL, &err);
    checkError(err, "clCreateBuffer");
}

int max_itr;

Mat genImg(double x, double y, int max_itr, double zoom)
{

    // Init kernel

    clSetKernelArg(kernel, 0, sizeof(int), &max_itr);
    clSetKernelArg(kernel, 1, sizeof(double), &x);
    clSetKernelArg(kernel, 2, sizeof(double), &y);
    clSetKernelArg(kernel, 3, sizeof(cl_mem), &img);
    clSetKernelArg(kernel, 4, sizeof(double), &zoom);
    clSetKernelArg(kernel, 5, sizeof(int), &speclen);

    // Launch kernel

    size_t global_offset[2] = {0, 0};
    size_t global_size[2] = {width, height};
    size_t maxWorkGroupSize;
    clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &maxWorkGroupSize, NULL);
    size_t localWorkSize[2] = {maxWorkGroupSize, 1};
    clEnqueueNDRangeKernel(queue, kernel, 2, global_offset, global_size, localWorkSize, 0, NULL, NULL);

    // Wait for kernel to complete

    clFlush(queue);
    clFinish(queue);

    // Copy data back to the cv::Mat

    unsigned char *resBuf = new unsigned char[width * height * 3];
    clEnqueueReadBuffer(queue, img, true, 0, width * height * 3, resBuf, 0, NULL, NULL);
    Mat res2(height, width, CV_8UC3, resBuf);
    Mat res = res2.clone();
    delete[] resBuf;

    return res;
}

Mat latestImage = Mat::zeros(Size(width, height), CV_8UC3);
std::mutex imageMutex;

void passive()
{
    cv::namedWindow("Preview", cv::WINDOW_NORMAL); // Allow window resizing
    cv::setWindowProperty("Preview", cv::WND_PROP_FULLSCREEN, cv::WINDOW_FULLSCREEN);
    while (true)
    {
        Sleep(1000);
        std::lock_guard<std::mutex> lock(imageMutex);
        imshow("Preview", latestImage);
        waitKey(10);
    }
}

int main()
{
    init_cl();

    // Create a blank black image (3-channel BGR)
    /*Mat res = genImg(0.0, 0.0, 1000, 1.5);
    imwrite("mandelbrot.png", res);
    imshow("Mandelbrot Set", res);
    waitKey(0);
    destroyAllWindows(); // Close all OpenCV windows*/
    double x, y, zoom;
    std::cout << "x: ";
    std::cin >> x;
    std::cout << "y: ";
    std::cin >> y;
    std::cout << "zoom: ";
    std::cin >> zoom;
    std::cout << "MaxItr (default 1000): ";
    int maxitr;
    std::cin >> maxitr;
    std::cout << "Spectrum length (default 100): ";
    std::cin >> speclen;
    double scale;
    std::cout << "Scale for display (reletive to 4k) (default 1): ";
    std::cin >> scale;
    zoom = 1.0 / zoom;
    int ind = 0;
    std::string path = "frames";
    std::cout << "File output name: ";
    std::string path2;
    std::cin >> path2;
    std::cout << "The generation is very GPU intensive so your computer may freeze. Intermediate frames before video finishes will be in the frames folder. A popup will appear with the current frame. Please fullscreen it or else it will make your tasknar disappear (IDK why). Are you sure?" << std::endl;
    system("pause > nul | <nul set /p \"=Are you 100% sure about this? Press any key to continue and close this window to cancel...\"");
    if (!std::filesystem::exists(path))
    {
        std::filesystem::create_directory(path);
    }
    VideoWriter writer(path2, VideoWriter::fourcc('a', 'v', 'c', '1'), 60, Size(width, height));
    std::thread passiveThread(passive); // Start a thread to keep the OpenCV window responsive
    passiveThread.detach();
    for (double czoom = 1.0; czoom >= zoom; czoom /= 1.0116194403)
    {
        std::cout << ind << std::endl;
        Mat img = genImg(x, y, maxitr, czoom);
        std::string filename = "frames/mandelbrot_" + std::to_string(ind++) + ".png";
        Mat resized;
        // double scale = 0.5; // or whatever fits your screen nicely
        resize(img, resized, Size(), scale, scale, cv::INTER_NEAREST);
        {

            std::lock_guard<std::mutex> lock(imageMutex);
            latestImage = resized;
        }
        imwrite(filename, img);
        waitKey(1);
        writer.write(img); // Write the frame to the video file
    }
    writer.release();
    system("start cmd /k echo DONE GENERATING MANDELBROT SET");

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
    if (img)
        clReleaseMemObject(img);

    return 0;
}
