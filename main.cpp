#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION

#include "Metal/Metal.hpp"

#include <iostream> 

int main() {
    NS::AutoreleasePool* autool = NS::AutoreleasePool::alloc()->init();

    MTL::Device* device = MTL::CreateSystemDefaultDevice();
    if (!device) {
        std::cerr << "Metal is not supported on this device" << std::endl;
        return -1;
    }

    std::cout << "Metal device: " << device->name()->utf8String() << std::endl;

    device->release();
    autool->release();
    return 0;
}