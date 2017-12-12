//
// FileSystem.cpp
// Copyright (c) Charles Baker.  All rights reserved.
//

#include "FileSystem.hpp"
#include "BasicPath.hpp"
#if defined(BUILD_OS_WINDOWS)
#include <windows.h>
#elif defined(BUILD_OS_MACOSX)
#include <unistd.h>
#include <time.h>
#include <mach-o/dyld.h>
#endif

using namespace sweet;
using namespace sweet::fs;

FileSystem::FileSystem()
: root_(),
  initial_(),
  executable_(),
  home_(),
  directory_stack_( NULL )
{
    refresh_executable();
    refresh_home();
}

const fs::Path& FileSystem::root() const
{
    return root_;
}

const fs::Path& FileSystem::initial() const
{
    return initial_;
}

const fs::Path& FileSystem::executable() const
{
    return executable_;
}

const fs::Path& FileSystem::home() const
{
    return home_;
}

fs::Path FileSystem::root( const fs::Path& path ) const
{
    if ( fs::Path(path).is_absolute() )
    {
        return path;
    }

    fs::Path absolute_path( root_ );
    absolute_path /= path;
    absolute_path.normalize();
    return absolute_path;
}

fs::Path FileSystem::initial( const fs::Path& path ) const
{
    if ( fs::Path(path).is_absolute() )
    {
        return path;
    }

    fs::Path absolute_path( initial_ );
    absolute_path /= path;
    absolute_path.normalize();
    return absolute_path;
}

fs::Path FileSystem::executable( const fs::Path& path ) const
{
    if ( fs::Path(path).is_absolute() )
    {
        return path;
    }

    fs::Path absolute_path( executable_ );
    absolute_path /= path;
    absolute_path.normalize();
    return absolute_path;
}

fs::Path FileSystem::home( const fs::Path& path ) const
{
    if ( fs::Path(path).is_absolute() )
    {
        return path;
    }

    fs::Path absolute_path( home_ );
    absolute_path /= path;
    absolute_path.normalize();
    return absolute_path;
}

void FileSystem::set_root( const fs::Path& root )
{
    root_ = root;
}

void FileSystem::set_initial( const fs::Path& initial )
{
    initial_ = initial;
}

void FileSystem::refresh_executable()
{
#if defined(BUILD_OS_WINDOWS)
    char path [MAX_PATH + 1];
    int size = ::GetModuleFileNameA( NULL, path, sizeof(path) );
    path [sizeof(path) - 1] = 0;
    executable_ = fs::Path( path );
#elif defined(BUILD_OS_MACOSX)
    uint32_t size = 0;
    _NSGetExecutablePath( NULL, &size );
    char path [size];
    _NSGetExecutablePath( path, &size );
    executable_ = fs::Path( path );
#endif
}

void FileSystem::refresh_home()
{
#if defined(BUILD_OS_WINDOWS)
    char path [MAX_PATH + 1];
    int size = ::GetModuleFileNameA( NULL, path, sizeof(path) );
    path [sizeof(path) - 1] = 0;
    home_ = fs::Path( path );
#elif defined(BUILD_OS_MACOSX)
    uint32_t size = 0;
    _NSGetExecutablePath( NULL, &size );
    char path [size];
    _NSGetExecutablePath( path, &size );
    home_ = fs::Path( path );
#endif
}

void FileSystem::set_directory_stack( DirectoryStack* directory_stack )
{
    directory_stack_ = directory_stack;
}

DirectoryStack* FileSystem::directory_stack() const
{
    return directory_stack_;
}
