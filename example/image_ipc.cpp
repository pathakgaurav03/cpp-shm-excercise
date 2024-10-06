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

int imageIpc::producerSetup()
{
    shm.truncate(sizeof(struct genImage));

    // map the shared memory in process address space
    region = mapped_region(shm, read_write);

    // Get the address of the mapped region
    addr = region.get_address();
    return 0;
}

int imageIpc::write_in_buffer(const int width, const int height, const std::vector<std::vector<int>>& src)
{
    // Sanitize user provide input
    std::size_t sum = 0;
    for (auto&& i : src)
    {
        sum += i.size();
    }

    if (width == 0 || height == 0 || (sum != static_cast<size_t>(width * height)))
    {
        std::cout << "invalid input" << "size =" << src.size() << std::endl;
        return -1;
    }

    genImage* image;
    if (addr == nullptr)
    {
        std::cout << "calling setup func" << std::endl;
        producerSetup();
        image = new (addr) genImage;
    }

    // allocate metadat object in shared memory
    image = static_cast<genImage*>(addr);
    {
        // initialize metadata
        scoped_lock<interprocess_mutex> lock(image->mutex);
        image->width  = width;
        image->height = height;
        image->size   = width * height;
        randomStrGen(20, image->name);
        std::cout << "name in parent=" << image->name << std::endl;
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
        scoped_lock<interprocess_mutex> lock(image->mutex);
        image->isConsumed = false;
        image->isProduced = true;
        image->cond_empty.notify_one();
    }

    // wait for consumer to finish
    {
        scoped_lock<interprocess_mutex> lock(image->mutex);
        if (!image->isConsumed)
        {
            std::cout << "waiting to be consumed..." << std::endl;
            image->cond_full.wait(lock);
            std::cout << "isConsumed " << image->isConsumed << " " << std::endl;
        }
    }
    shared_memory_object::remove(image->name);

    return 0;
}

int imageIpc::consumerSetup()
{
    // map the shared memory in process address space
    region = mapped_region(shm, read_write);

    // Get the address of the mapped region
    addr = region.get_address();
    return 0;
}

int imageIpc::read_from_buffer(std::vector<std::vector<int>>& vec)
{
    if (addr == nullptr)
    {
        std::cout << "calling consumer setup func" << std::endl;
        consumerSetup();
    }

    genImage* image = static_cast<genImage*>(addr);
    {
        // Wait for data to be produced in buffer
        scoped_lock<interprocess_mutex> lock(image->mutex);
        if (!image->isProduced)
        {
            image->cond_empty.wait(lock);
        }
    }

    // // Open data buffer shared memory and map it in the process
    shared_memory_object shMem{open_or_create, image->name, read_write};
    mapped_region regionData(shMem, read_write);
    void* addr1 = regionData.get_address();
    auto data   = static_cast<int*>(addr1);

    // Read data
    {
        scoped_lock<interprocess_mutex> lock(image->dataMutex);
        for (int i = 0; i < image->width; i++)
        {
            std::vector<int> v2;
            for (int j = 0; j < image->height; j++)
            {
                // std::cout << data[i * image->height + j] << " ";
                v2.push_back(data[i * image->height + j]);
            }
            vec.push_back(v2);
            // std::cout << std::endl;
        }
    }

    // notify producer that data has been consumed
    {
        std::cout << "notify Producer" << std::endl;
        scoped_lock<interprocess_mutex> lock(image->mutex);
        image->isConsumed = true;
        image->isProduced = false;
        image->cond_full.notify_one();
    }
    shared_memory_object::remove(image->name);

    return 0;
}

// g++ example/main.cpp -limage_ipc -L ./
// export LD_LIBRARY_PATH=/home/s0002010/exp/image/cpp-shm-exercise
// cmake --build ./

/// refinement points-
// use of generic types for pixels, static and constness, 3d pixel, API capabilities and flexibility
// Logging, Error handling, tests

// Think about circular buffer ... big buffer shared by processes.

// Most importantly synchronization between processes (simple case done)

// WHAT IF THERE ARE MORE THAN TWO PROCESSE

// multithreading