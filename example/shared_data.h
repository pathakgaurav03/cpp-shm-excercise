#ifndef CPPPROJECT_SHARED_DATA_H
#define CPPPROJECT_SHARED_DATA_H

#include <array>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <iostream>
#include <memory>
#include <vector>

struct genImage
{
    genImage()
        : width(0)
        , height(0)
        , isProduced(false)
        , isConsumed(false) {};
    int width;
    int height;
    int size;
    boost::interprocess::interprocess_mutex metaDataMutex;
    boost::interprocess::interprocess_mutex dataMutex;
    boost::interprocess::interprocess_condition cond_empty;
    boost::interprocess::interprocess_condition cond_full;
    bool isProduced;
    bool isConsumed;
    char name[20];
};
#endif // CPPPROJECT_SHARED_DATA_H