//
// Environment.hpp
// Copyright (c) 2008 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_BUILD_TOOL_ENVIRONMENT_HPP_INCLUDED
#define SWEET_BUILD_TOOL_ENVIRONMENT_HPP_INCLUDED

#include "declspec.hpp"
#include <sweet/lua/LuaThread.hpp>
#include <sweet/path/Path.hpp>
#include <sweet/pointer/ptr.hpp>
#include <vector>

namespace sweet
{

namespace build_tool
{

class Job;
class Target;
class BuildTool;

/**
// Provides context for a script to interact with its outside environment.
*/
class Environment 
{
    int index_; ///< The index of this Environment.
    BuildTool* build_tool_; ///< The BuildTool that this Environment is part of.
    lua::LuaThread environment_thread_; ///< The LuaThread that this Environment uses to execute scripts in.
    ptr<Target> working_directory_; ///< The current working directory for this Environment.
    std::vector<path::Path> directories_; ///< The stack of working directories for this Environment (the element at the top is the current working directory).
    Job* job_; ///< The current Job for this Environment.
    int exit_code_; ///< The exit code from the Command that was most recently executed by this Environment.

    public:
        Environment( int index, const path::Path& directory, BuildTool* build_tool );

        lua::LuaThread& get_environment_thread();

        void set_working_directory( ptr<Target> target );
        ptr<Target> get_working_directory() const;

        void reset_directory( const path::Path& directory );
        void change_directory( const path::Path& directory );
        void push_directory( const path::Path& directory );
        void pop_directory();
        const path::Path& get_directory() const;

        void set_job( Job* job );
        Job* get_job() const;

        void set_echo( bool echo );
        bool is_echo() const;

        void set_exit_code( int exit_code );
        int get_exit_code() const;
};

}

}

#endif
