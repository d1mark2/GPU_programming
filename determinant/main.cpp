#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION

#include "Metal/Metal.hpp"
#include <iostream>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <ctime>
#include <vector>

#define MIN_VALUE -100.0f
#define MAX_VALUE 100.0f

MTL::ComputePipelineState* make_pipeline (MTL::Device* device, MTL::Library* library, const char* name)
{
    NS::Error* error = nullptr;
    MTL::Function* func = library->newFunction(NS::String::string(name, NS::UTF8StringEncoding));
    MTL::ComputePipelineState* pipeline = device->newComputePipelineState(func, &error);
    if (!pipeline)
    {
        std::cerr << "pipeline error: " << error->localizedDescription()->utf8String() << std::endl;
        exit(1);
    }
    func->release();
    return pipeline;
}

bool read_number (FILE* file, double* value)
{
    int c = fgetc(file);
    while (c != EOF && !isdigit(c) && c != '-' && c != '+' && c != '.')
        c = fgetc(file);
    if (c == EOF)
        return false;
    ungetc(c, file);
    return fscanf(file, "%lf", value) == 1;
}

bool read_matrix (FILE* file, uint32_t* n, std::vector<float>& data)
{
    double first = 0;
    int count = 0;
    if (read_number(file, &first))
        count = (int)first;
    if (count <= 0)
    {
        return false;
    }

    *n = (uint32_t)lround(sqrt((double)count));
    if ((*n) * (*n) != (uint32_t)count)
    {
        printf("count of elements %d is not n*n\n", count);
        return false;
    }

    data.resize(count);
    for (int i = 0; i < count; i++)
    {
        double value = 0;
        if (!read_number(file, &value))
        {
            printf("not enough elements\n");
            return false;
        }
        data[i] = (float)value;
    }
    return true;
}

bool open_file (const char* filename, uint32_t* n, std::vector<float>& data)
{
    FILE* file = fopen(filename, "r");
    if (!file)
    {
        printf("cant open file %s\n", filename);
        return false;
    }
    bool ok = read_matrix(file, n, data);
    fclose(file);
    return ok;
}

int main (int argc, char* argv[])
{
    uint32_t n = 0;
    std::vector<float> from_file;
    bool use_file = false;
    char filename[256] = "";
    int mode = 0;
    int size = 0;

    if (argc == 1)
    {
        printf("1 - random matrix\n2 - matrix from file\n3 - matrix from keyboard\nchoose mode: ");
        if (scanf("%d", &mode) != 1 || mode < 1 || mode > 3)
        {
            printf("wrong mode\n");
            return 1;
        }
        if (mode == 1)
        {
            printf("matrix size: ");
            if (scanf("%d", &size) != 1)
                size = 0;
        }
        else if (mode == 2)
        {
            printf("file name: ");
            if (scanf("%255s", filename) != 1)
                filename[0] = 0;
        }
    }
    else if (argc == 3 && strcmp(argv[1], "-f") == 0)
    {
        mode = 2;
        strncpy(filename, argv[2], 255);
    }
    else if (argc == 2)
    {
        mode = 1;
        size = atoi(argv[1]);
    }
    else
    {
        printf("usage: %s  or  %s <size>  or  %s -f <file>\n", argv[0], argv[0], argv[0]);
        return 1;
    }

    if (mode == 1)
    {
        if (size <= 0)
        {
            printf("matrix size must be > 0\n");
            return 1;
        }
        n = size;
    }
    else if (mode == 2)
    {
        if (!open_file(filename, &n, from_file))
            return 1;
        use_file = true;
    }
    else
    {
        printf("count of elements, then elements: ");
        if (!read_matrix(stdin, &n, from_file))
            return 1;
        use_file = true;
    }

    const uint32_t pivot_threads = 256;
    const uint32_t tile = 16;

    NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();

    MTL::Device* device = MTL::CreateSystemDefaultDevice();
    if (!device)
    {
        std::cerr << "Metal is not supported on this device" << std::endl;
        return 1;
    }

    NS::Error* error = nullptr;
    MTL::Library* library = device->newLibrary(NS::String::string("determinant.metallib", NS::UTF8StringEncoding), &error);
    if (!library)
    {
        std::cerr << "cant load determinant.metallib" << std::endl;
        return 1;
    }
    MTL::ComputePipelineState* pivot_func = make_pipeline(device, library, "pivot_swap");
    MTL::ComputePipelineState* elim_func = make_pipeline(device, library, "eliminate");
    MTL::CommandQueue* queue = device->newCommandQueue();

    MTL::Buffer* matrix_buff = device->newBuffer(n * n * sizeof(float), MTL::ResourceStorageModeShared);
    MTL::Buffer* swaps_buff = device->newBuffer(sizeof(uint32_t), MTL::ResourceStorageModeShared);

    float* matrix = (float*)matrix_buff->contents();
    if (use_file)
        memcpy(matrix, from_file.data(), n * n * sizeof(float));
    else
    {
        srand(time(NULL));
        for (uint32_t i = 0; i < n * n; i++)
            matrix[i] = MIN_VALUE + (MAX_VALUE - MIN_VALUE) * (float)rand() / (float)RAND_MAX;
    }
    *(uint32_t*)swaps_buff->contents() = 0;

    auto start = std::chrono::steady_clock::now();

    MTL::CommandBuffer* cmd_buf = queue->commandBuffer();
    MTL::ComputeCommandEncoder* enc = cmd_buf->computeCommandEncoder();

    for (uint32_t k = 0; k + 1 < n; k++)
    {
        enc->setComputePipelineState(pivot_func);
        enc->setBuffer(matrix_buff, 0, 0);
        enc->setBytes(&n, sizeof(n), 1);
        enc->setBytes(&k, sizeof(k), 2);
        enc->setBuffer(swaps_buff, 0, 3);
        enc->dispatchThreadgroups(MTL::Size::Make(1, 1, 1), MTL::Size::Make(pivot_threads, 1, 1));

        uint32_t m = n - k - 1;
        uint32_t groups = (m + tile - 1) / tile;
        enc->setComputePipelineState(elim_func);
        enc->setBuffer(matrix_buff, 0, 0);
        enc->setBytes(&n, sizeof(n), 1);
        enc->setBytes(&k, sizeof(k), 2);
        enc->dispatchThreadgroups(MTL::Size::Make(groups, groups, 1), MTL::Size::Make(tile, tile, 1));
    }
    enc->endEncoding();
    cmd_buf->commit();
    cmd_buf->waitUntilCompleted();

    uint32_t swaps = *(uint32_t*)swaps_buff->contents();
    int sign = (swaps % 2 == 0) ? 1 : -1;
    double log_det = 0;
    for (uint32_t i = 0; i < n; i++)
    {
        double d = matrix[i * n + i];
        if (d == 0)
        {
            sign = 0;
            log_det = -INFINITY;
            break;
        }
        if (d < 0)
            sign = -sign;
        log_det += log(fabs(d));
    }

    auto end = std::chrono::steady_clock::now();
    double time_ms = std::chrono::duration<double, std::milli>(end - start).count();

    printf("n: %u\n", n);
    printf("sign: %d\n", sign);
    printf("ln|det|: %lf\n", log_det);
    if (log_det < 700)
        printf("det: %e\n", sign * exp(log_det));
    printf("time: %lf ms\n", time_ms);

    matrix_buff->release();
    swaps_buff->release();
    pivot_func->release();
    elim_func->release();
    queue->release();
    library->release();
    device->release();
    pool->release();
    return 0;
}
