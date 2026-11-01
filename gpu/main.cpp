#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION

#include "Metal/Metal.hpp"
#include <iostream> 
#include <chrono>

void make_word (unsigned char* result, char* word)
{
    int len = strlen(word);
    strncpy((char*)result, (char*)word, len);
    result[len] = 0x80;
    result[63] = len*8; 
}

void parse_hash(char* hash, uint32_t* result_hash)
{
    for (int i = 0; i < 8; i++)
        sscanf(hash + 8 * i,"%8x",&result_hash[i]);
}

void index_to_password (uint64_t idx, const char* charset, int len, int passLen, char* out) 
{
    for (int i = passLen - 1; i >= 0; i--) {
        out[i] = charset[idx % len];
        idx /= len;
    }
    out[passLen] = '\0';
}

int main(int argc, char* argv[]) 
{
    const char* alphabet = "abcdefghijklmnopqrstuvwxyz";
    uint32_t charset_len = 26;
    uint64_t batchSize = 1u << 24;          
    uint64_t maxCandidates = ~0;     
    uint64_t offset = 0;
    bool found = false;
    uint32_t result_hash[8];

    if (argc == 2)
        parse_hash(argv[1], result_hash);
    else 
    {
        printf("add target hash");
        return 1;
    }   

    NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();

    MTL::Device* device = MTL::CreateSystemDefaultDevice();

    NS::Error* error = nullptr;
    NS::String* lib_path = NS::String::string("sha_256.metallib", NS::UTF8StringEncoding);
    MTL::Library* library = device->newLibrary(lib_path, &error);

    NS::String* func_name = NS::String::string("sha256_search", NS::UTF8StringEncoding);
    MTL::Function* func = library->newFunction(func_name);

    MTL::ComputePipelineState* func_gpu = device->newComputePipelineState(func, &error);
    MTL::CommandQueue* queue = device->newCommandQueue();

    MTL::Buffer* target_buff = device->newBuffer(8*sizeof(uint32_t), MTL::ResourceStorageModeShared);
    MTL::Buffer* charset_buff = device->newBuffer(charset_len, MTL::ResourceStorageModeShared);
    MTL::Buffer* flag_buff = device->newBuffer(sizeof(uint32_t), MTL::ResourceStorageModeShared);
    MTL::Buffer* found_buff = device->newBuffer(sizeof(uint64_t), MTL::ResourceStorageModeShared);
    
    memset(flag_buff->contents(), 0, sizeof(uint32_t));    
    memcpy(charset_buff->contents(), alphabet, charset_len);
    memcpy(target_buff->contents(), result_hash, 8*sizeof(uint32_t));   

    auto start = std::chrono::steady_clock::now();
  
    for (int len_pass = 1; len_pass < 12; len_pass++)
    {
        uint64_t max_elements = (uint64_t)pow(charset_len, len_pass);
        for (uint64_t index = 0; (index < max_elements) && !found; index += batchSize) 
        {
            uint64_t thisBatch = std::min(batchSize, maxCandidates - offset);

            MTL::CommandBuffer* cmdBuf = queue->commandBuffer();
            MTL::ComputeCommandEncoder* enc = cmdBuf->computeCommandEncoder();
            
            enc->setComputePipelineState(func_gpu);
            enc->setBuffer(charset_buff, 0, 0);
            enc->setBytes(&charset_len, sizeof(charset_len), 1);
            enc->setBytes(&len_pass, sizeof(len_pass), 2);
            enc->setBytes(&offset, sizeof(offset), 3);
            enc->setBuffer(target_buff, 0, 4);
            enc->setBuffer(flag_buff, 0, 5);
            enc->setBuffer(found_buff, 0, 6);

            enc->dispatchThreads(MTL::Size::Make(thisBatch,1,1), MTL::Size::Make(func_gpu->maxTotalThreadsPerThreadgroup(),1,1));
            enc->endEncoding();
            cmdBuf->commit();
            cmdBuf->waitUntilCompleted();

            offset += thisBatch;
            found = (*(uint32_t*)flag_buff->contents()) != 0;
        }

        if (found) 
        {
            auto end = std::chrono::steady_clock::now();
            auto elapsed = end - start;
            double elapsed_ms = (double)std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

            uint64_t found_index = *(uint64_t*)found_buff->contents();
            char password[8];
            index_to_password(found_index, alphabet, charset_len, len_pass, password);
            printf("password: %s\n", password);
            std::cout << "time: " << elapsed_ms/1000 << " s\n";
            pool->release();
            return 0;
        }
    }

    return 0;
}

