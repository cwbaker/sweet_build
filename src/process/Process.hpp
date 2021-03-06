#ifndef SWEET_PROCESS_PROCESS_HPP_INCLUDED
#define SWEET_PROCESS_PROCESS_HPP_INCLUDED

#include <build.hpp>
#include <vector>
#include <stdint.h>

#if defined(BUILD_OS_MACOS) || defined(BUILD_OS_LINUX)
#include <sys/types.h>
#endif

namespace sweet
{

namespace process
{

enum Pipes
{
    PIPE_STDIN,
    PIPE_STDOUT,
    PIPE_STDERR,
    PIPE_USER_0,
    PIPE_COUNT
};

class Environment;

/**
// An operating system process.
*/
class Process
{
    struct Pipe
    {
        int child_fd; ///< The file descriptor to dup2() into in the child.
        intptr_t read_fd; ///< The read file descriptor to the pipe.
        intptr_t write_fd; ///< The write file desciptor to the pipe.
    };

    const char* executable_;
    const char* directory_;
    const Environment* environment_;
    bool start_suspended_;
    bool inherit_environment_;
    std::vector<Pipe> pipes_;

#if defined(BUILD_OS_WINDOWS)
    void* process_; ///< The handle to this Process.
    void* suspended_thread_; ///< The handle to the suspended main thread of this Process.
#endif

#if defined(BUILD_OS_MACOS)
    pid_t process_;
    int exit_code_;
    bool suspended_;
#endif

#if defined(BUILD_OS_LINUX)
    pid_t process_;
    int exit_code_;
#endif

    public:
        Process();
        ~Process();

        void* process() const;
        void* thread() const;
        void* write_pipe( int index ) const;

        void executable( const char* executable );
        void directory( const char* directory );
        void environment( const Environment* environment );
        void start_suspended( bool start_suspended );
        void inherit_environment( bool inherit_environment );
        intptr_t pipe( int child_fd );
        void run( const char* arguments );

        void resume();
        void wait();
        int exit_code();
};

}

}

#endif
