#pragma once

#include <optional>

#ifdef _WIN32
    #include <Windows.h>
#endif


struct file_mapping
{
#ifdef __unix__
    int fd;
#else
    HANDLE hfile;
    HANDLE map_handle;
#endif
    const char* addr;
    int length;
};

std::optional<file_mapping> map_file(const char* filepath);
void unmap_file(const file_mapping& fp);