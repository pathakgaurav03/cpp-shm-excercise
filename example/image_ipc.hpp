#include "image_ipc_types.hpp"
#include "shared_data.h"
#include <string>

class imageIpc
{
public:
    imageIpc();
    imageIpc(const char* name, bool isProducer);
    int write_in_buffer(int width, int height, const std::vector<std::vector<int>>& src);
    int read_from_buffer(std::vector<std::vector<int>>& vec);
    void cleanUp(const char* name);

private:
    int producerSetup();
    int consumerSetup();
    void* addr;
    const char* name;
    boost::interprocess::mapped_region region;
    boost::interprocess::shared_memory_object shm;
};
