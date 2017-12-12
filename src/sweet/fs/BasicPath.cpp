//
// BasicPath.cpp
// Copyright (c) 2008 - 2015 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include <sweet/fs/fs.hpp>
#include <sweet/assert/assert.hpp>
#include <string>
#include <stdexcept>
#include <locale>

#if defined(BUILD_OS_MACOSX)
#include <unistd.h>
#endif

using std::wstring;
using namespace sweet::fs;

/**
// Get the current working directory.
//
// @return
//  The current working directory.
*/
WidePath sweet::fs::current_working_directory()
{
#if defined(BUILD_OS_WINDOWS)
//
// The length returned by GetCurrentDirectoryW includes the terminating
// null character so the wstring is initialized to have one less character 
// than that.
//   
    DWORD length = ::GetCurrentDirectoryW( 0, NULL );
    std::wstring directory( length - 1, L'\0' );
    ::GetCurrentDirectoryW( length, &directory[0] );
    return WidePath( directory );
#elif defined(BUILD_OS_MACOSX)
    char directory [4096];
    const char *result = getcwd( directory, sizeof(directory) );
    if ( !result )
    {
        directory[0] = '\0';
    }

    unsigned int length = strlen( directory );
    std::vector<wchar_t> buffer( length );    
    const std::ctype<wchar_t>* ctype = &std::use_facet<std::ctype<wchar_t> >( std::locale() );
    ctype->widen( directory, directory + length, &buffer[0] );
    return WidePath( wstring(&buffer[0], length) );
#else
    return WidePath();
#endif
}
