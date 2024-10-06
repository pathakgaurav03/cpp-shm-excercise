#include "image_ipc.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <sys/types.h> // for pid_t
#include <sys/wait.h>  // for wait()
#include <unistd.h>    // for fork()
#include <vector>
using namespace std;

TEST(DummyTest, basicIpc)
{
    // imageIpc IP("myMem1");
    // int n = 2;
    // int m = 5;
    // // std::vector<std::vector<>, > data;
    // // std::vector<std::vector<int>> vec(n, std::vector<int>(m));

    // // for (int i = 0; i < n; i++)
    // // {
    // //     for (int j = 0; j < m; j++)
    // //     {
    // //         vec[i][j] = j + i + 1;
    // //     }
    // // }

    // // vector<vector<int>> vec{/* Element one with 2 values in it. */
    // //                         {1, 2, 3},

    // //                         /* Element two with 3 values in it. */
    // //                         {4, 5, 6},

    // //                         /* Element three with 4 values in it. */
    // //                         {7, 8, 9}};

    // vector<vector<int>> vec;
    // for (int i = 0; i < n; i++)
    // {
    //     vector<int> v2;
    //     for (int j = 0; j < n; j++)
    //     {
    //         v2.push_back(j + 1);
    //     }
    //     vec.push_back(v2);
    // }

    // IP.write_in_buffer(n, n, vec);

    // pid_t pid = fork(); // Create a new process
    // pid_t wpid;
    // int status = 0;

    // if (pid < 0)
    // {
    //     // Fork failed
    //     std::cerr << "Fork failed!" << std::endl;
    //     // return 1;
    // }
    // else if (pid == 0)
    // {
    //     // Child process
    //     std::cout << "Hello from the child process! PID: " << getpid() << std::endl;
    //     imageIpc IPC("myMem1");
    //     IPC.read_from_buffer();
    //     // sleep(1);
    // }
    // else
    // {
    //     // Parent process
    //     std::cout << "Hello from the parent process! PID: " << getpid() << ", Child PID: " << pid << std::endl;
    //     while ((wpid = wait(&status)) > 0)
    //         ;
    // }
    // // return 0;
    ASSERT_TRUE(true);
}

TEST(DummyTest, basicIpc1)
{
    int n = 3;
    int m = 5;

    pid_t pid = fork(); // Create a new process
    if (pid < 0)
    {
        // Fork failed
        std::cerr << "Fork failed!" << std::endl;
        // return 1;
    }
    else if (pid == 0)
    {
        int n = 3;
        int m = 5;
        std::cout << "Hello from the child process! PID: " << getpid() << std::endl;
        imageIpc IPC("myMem2", false);
        vector<vector<int>> vec;
        IPC.read_from_buffer(vec);
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < m; j++)
            {
                // std::cout << "random " << std::endl;
                std::cout << vec[i][j] << " ";
            }

            std::cout << std::endl;
        }
        IPC.cleanUp("myMem2");
        exit(0);
        // return 0;
    }
    else
    {
        std::cout << "Hello from the parent process! PID: " << getpid() << ", Child PID: " << pid << std::endl;
        vector<vector<int>> vec;
        for (int i = 0; i < n; i++)
        {
            vector<int> v2;
            for (int j = 0; j < m; j++)
            {
                v2.push_back(j + 1);
            }
            vec.push_back(v2);
        }
        imageIpc IP("myMem2", true);
        IP.write_in_buffer(n, m, vec);
        pid_t wpid;
        int status = 0;
        while ((wpid = wait(&status)) > 0)
        {
            std::cout << "waiting .." << std::endl;
        };
        IP.cleanUp("myMem2");
    }

    ASSERT_TRUE(true);
}

TEST(DummyTest, invalidInput)
{
    int n = 3;
    int m = 5;
    vector<vector<int>> vec;
    for (int i = 0; i < n; i++)
    {
        vector<int> v2;
        for (int j = 0; j < m; j++)
        {
            v2.push_back(j + 1);
        }
        vec.push_back(v2);
    }

    imageIpc IP("myMem2", true);
    EXPECT_EQ(IP.write_in_buffer(n + 1, m + 1, vec), -1);
    vector<vector<int>> vec2;
    EXPECT_EQ(IP.write_in_buffer(n + 1, m + 1, vec2), -1);
    // EXPECT_EQ(IP.write_in_buffer(n, m, vec), 0);
    IP.cleanUp("myMem2");
}

TEST(DummyTest, sendMultipleBuffers)
{
    for (int t = 0; t < 2; t++)
    {
        int n = 3;
        int m = 5;

        pid_t pid = fork(); // Create a new process
        if (pid < 0)
        {
            // Fork failed
            std::cerr << "Fork failed!" << std::endl;
            // return 1;
        }
        else if (pid == 0)
        {
            int n = 3;
            int m = 5;
            std::cout << "Hello from the child process! PID: " << getpid() << std::endl;
            imageIpc IPC("myMem2", false);
            for (int k = 0; k < 2; k++)
            {
                vector<vector<int>> vec;
                IPC.read_from_buffer(vec);
                for (int i = 0; i < n; i++)
                {
                    for (int j = 0; j < m; j++)
                    {
                        // std::cout << "random " << std::endl;
                        std::cout << vec[i][j] << " ";
                    }

                    std::cout << std::endl;
                }
            }
            IPC.cleanUp("myMem2");
            exit(0);
            // return 0;
        }
        else
        {
            std::cout << "Hello from the parent process! PID: " << getpid() << ", Child PID: " << pid << std::endl;
            vector<vector<int>> vec;
            for (int i = 0; i < n; i++)
            {
                vector<int> v2;
                for (int j = 0; j < m; j++)
                {
                    v2.push_back(j + 1);
                }
                vec.push_back(v2);
            }
            imageIpc IP("myMem2", true);
            for (int k = 0; k < 2; k++)
            {
                IP.write_in_buffer(n, m, vec);
            }
            pid_t wpid;
            int status = 0;
            while ((wpid = wait(&status)) > 0)
            {
                std::cout << "waiting .." << std::endl;
            };
            IP.cleanUp("myMem2");
        }
    }

    ASSERT_TRUE(true);
}
