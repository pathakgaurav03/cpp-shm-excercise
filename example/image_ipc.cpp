#include "image_ipc.hpp"
#include <cstdlib>
#include <iostream>

using namespace boost::interprocess;

// Generates a random string of given length.
static void randomStrGen(int length, char name[])
{
    static std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";

    srand(time(NULL));
    for (int i = 0; i < length; i++)
        name[i] = charset[rand() % charset.length()];
}

// constructor
imageIpc::imageIpc(const char* _name, bool isProducer)
{
    name = _name;
    if (isProducer)
    {
        shared_memory_object::remove(_name);
        shm  = shared_memory_object(open_or_create, _name, read_write);
        addr = nullptr;
    }
    else
    {
        shm  = shared_memory_object(open_only, _name, read_write);
        addr = nullptr;
    }
}

void imageIpc::cleanUp(const char* _name)
{
    addr = nullptr;
    shared_memory_object::remove(_name);
}

imageIpc::ErrorCode imageIpc::producerSetup()
{
    shm.truncate(sizeof(struct genImage));

    // map the shared memory in process address space
    region = mapped_region(shm, read_write);

    // Get the address of the mapped region
    addr = region.get_address();
    return imageIpc::SUCCESS;
}

imageIpc::ErrorCode
imageIpc::write_in_buffer(const int width, const int height, const std::vector<std::vector<int>>& src)
{
    // Sanitize user provide input
    std::size_t srcSize = 0;
    for (auto&& i : src)
    {
        srcSize += i.size();
    }

    if (width == 0 || height == 0 || srcSize != static_cast<size_t>(width * height))
    {
        return imageIpc::ERROR_INVALID_INPUT;
    }

    genImage* image;
    if (addr == nullptr)
    {
        if (producerSetup() != imageIpc::SUCCESS)
        {
            return imageIpc::ERROR_UNKNOWN;
        };
        image = new (addr) genImage;
    }

    // allocate metadat object in shared memory
    image = static_cast<genImage*>(addr);
    {
        // initialize metadata
        scoped_lock<interprocess_mutex> lock(image->metaDataMutex);
        image->width  = width;
        image->height = height;
        image->size   = width * height;
        randomStrGen(20, image->name);
    }

    // Open data buffer shared memory and map it in the process
    shared_memory_object shMem{open_or_create, image->name, read_write};
    shMem.truncate(width * height);
    mapped_region regionData(shMem, read_write);

    // Get the address of the mapped region
    void* addr1 = regionData.get_address();
    int* data{new (addr1) int[width * height]{}};

    // Write data
    {
        scoped_lock<interprocess_mutex> lock(image->dataMutex);
        for (int i = 0; i < width; i++)
        {
            for (int j = 0; j < height; j++)
            {
                data[i * height + j] = src[i][j];
            }
        }
    }

    // Notify consumer of new data
    {
        scoped_lock<interprocess_mutex> lock(image->metaDataMutex);
        image->isConsumed = false;
        image->isProduced = true;
        image->cond_empty.notify_one();

        // wait for consumer to finish
        image->cond_full.wait(lock);
    }

    shared_memory_object::remove(image->name);
    return imageIpc::SUCCESS;
}

imageIpc::ErrorCode imageIpc::consumerSetup()
{
    // map the shared memory in process address space
    region = mapped_region(shm, read_write);

    // Get the address of the mapped region
    addr = region.get_address();
    return imageIpc::SUCCESS;
}

imageIpc::ErrorCode imageIpc::read_from_buffer(std::vector<std::vector<int>>& vec)
{
    if (addr == nullptr)
    {
        consumerSetup();
    }

    char* dataBufferName;
    genImage* image = static_cast<genImage*>(addr);
    int dataWidth, dataHeight;
    {
        // Wait for data to be produced in buffer
        scoped_lock<interprocess_mutex> lock(image->metaDataMutex);
        if (!image->isProduced)
        {
            image->cond_empty.wait(lock);
        }

        dataBufferName = image->name;
        dataWidth      = image->width;
        dataHeight     = image->height;
    }

    // // Open data buffer shared memory and map it in the process
    shared_memory_object shMem{open_only, dataBufferName, read_write};
    mapped_region regionData(shMem, read_write);
    void* addr1 = regionData.get_address();
    auto data   = static_cast<int*>(addr1);

    // Read data
    {
        scoped_lock<interprocess_mutex> lock(image->dataMutex);
        for (int i = 0; i < dataWidth; i++)
        {
            std::vector<int> v2;
            for (int j = 0; j < dataHeight; j++)
            {
                v2.push_back(data[i * dataHeight + j]);
            }
            vec.push_back(v2);
        }
    }

    // notify producer that data has been consumed
    {
        scoped_lock<interprocess_mutex> lock(image->metaDataMutex);
        image->isConsumed = true;
        image->isProduced = false;
        image->cond_full.notify_one();
    }
    shared_memory_object::remove(dataBufferName);
    return imageIpc::SUCCESS;
}