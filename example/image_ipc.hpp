#ifndef CPPPROJECT_IMAGE_IPC_H
#define CPPPROJECT_IMAGE_IPC_H

#include "image_ipc_types.hpp"
#include "shared_data.h"
#include <string>

class imageIpc
{
public:
    enum ErrorCode
    {
        SUCCESS = 0,
        ERROR_INVALID_INPUT,
        ERROR_UNKNOWN,
    };

    imageIpc(const char* name, bool isProducer);
    ErrorCode write_in_buffer(int width, int height, const std::vector<std::vector<int>>& src);
    ErrorCode read_from_buffer(std::vector<std::vector<int>>& vec);
    void cleanUp(const char* name);

private:
    ErrorCode producerSetup();
    ErrorCode consumerSetup();
    void* addr;
    const char* name;
    boost::interprocess::mapped_region region;
    boost::interprocess::shared_memory_object shm;
};
#endif