//
// BasicPathTraits.cpp
// Copyright (c) 2008 - 2015 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include <sweet/fs/fs.hpp>

using namespace sweet::fs;

#if defined(BUILD_OS_WINDOWS)
const char* BasicPathTraits<char>::EMPTY = "";
const char* BasicPathTraits<char>::SEPARATORS = "\\/";
const char* BasicPathTraits<char>::SEPARATOR = "/";
const char* BasicPathTraits<char>::NATIVE_SEPARATOR = "\\";
const char  BasicPathTraits<char>::DRIVE = ':';
const char* BasicPathTraits<char>::CURRENT = ".";
const char* BasicPathTraits<char>::PARENT = "..";
const char  BasicPathTraits<char>::DELIMITER = '.';    

const wchar_t* BasicPathTraits<wchar_t>::EMPTY = L"";
const wchar_t* BasicPathTraits<wchar_t>::SEPARATORS = L"\\/";
const wchar_t* BasicPathTraits<wchar_t>::SEPARATOR = L"/";
const wchar_t* BasicPathTraits<wchar_t>::NATIVE_SEPARATOR = L"\\";
const wchar_t  BasicPathTraits<wchar_t>::DRIVE = L':';
const wchar_t* BasicPathTraits<wchar_t>::CURRENT = L".";
const wchar_t* BasicPathTraits<wchar_t>::PARENT = L"..";
const wchar_t  BasicPathTraits<wchar_t>::DELIMITER = L'.';    
#endif

#if defined(BUILD_OS_MACOSX) || defined(BUILD_OS_ANDROID) || defined(BUILD_OS_IOS)
const char* BasicPathTraits<char>::EMPTY = "";
const char* BasicPathTraits<char>::SEPARATORS = "/";
const char* BasicPathTraits<char>::SEPARATOR = "/";
const char* BasicPathTraits<char>::NATIVE_SEPARATOR = "/";
const char  BasicPathTraits<char>::DRIVE = ':';
const char* BasicPathTraits<char>::CURRENT = ".";
const char* BasicPathTraits<char>::PARENT = "..";
const char  BasicPathTraits<char>::DELIMITER = '.';    

const wchar_t* BasicPathTraits<wchar_t>::EMPTY = L"";
const wchar_t* BasicPathTraits<wchar_t>::SEPARATORS = L"/";
const wchar_t* BasicPathTraits<wchar_t>::SEPARATOR = L"/";
const wchar_t* BasicPathTraits<wchar_t>::NATIVE_SEPARATOR = L"/";
const wchar_t  BasicPathTraits<wchar_t>::DRIVE = L':';
const wchar_t* BasicPathTraits<wchar_t>::CURRENT = L".";
const wchar_t* BasicPathTraits<wchar_t>::PARENT = L"..";
const wchar_t  BasicPathTraits<wchar_t>::DELIMITER = L'.';    
#endif