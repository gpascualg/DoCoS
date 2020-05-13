#include "io/memmap.hpp"

#ifdef __unix__
    #include <fcntl.h>
    #include <sys/mman.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>
#else
    #include <Windows.h>
#endif

std::optional<file_mapping> map_file(const char* filepath)
{
    file_mapping file_mapping;

    #ifdef __unix__
        file_mapping.fd = open(filepath, O_RDONLY);
        if (file_mapping.fd == -1)
        {
            return {};
        }

        // obtain file size
        struct stat sb;
        if (fstat(file_mapping.fd, &sb) == -1)
        {
            return {};
        }

        file_mapping.length = sb.st_size;

        file_mapping.addr = static_cast<const char*>(mmap(NULL, file_mapping.length, PROT_READ, MAP_PRIVATE, file_mapping.fd, 0u));
        if (file_mapping.addr == MAP_FAILED)
        {
            return {};
        }
    #else
        file_mapping.hfile = CreateFileA(filepath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
        if (file_mapping.hfile == INVALID_HANDLE_VALUE)
        {
            return {};
        }

        file_mapping.length = GetFileSize(file_mapping.hfile, NULL);
        if (file_mapping.length == INVALID_FILE_SIZE)
        {
            CloseHandle(file_mapping.hfile);
            return {};
        }

        file_mapping.map_handle = CreateFileMapping(file_mapping.hfile, NULL, PAGE_READWRITE | SEC_RESERVE, 0, 0, 0);
        if (file_mapping.map_handle == NULL)
        {
            CloseHandle(file_mapping.hfile);
            return {};
        }

        file_mapping.addr = static_cast<const char*>(MapViewOfFile(file_mapping.map_handle, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, 0));
        if (file_mapping.addr == NULL)
        {
            CloseHandle(file_mapping.map_handle);
            CloseHandle(file_mapping.hfile);
            return {};
        }
    #endif

    return file_mapping;
}

void unmap_file(const file_mapping& fp)
{
    #ifdef __unix__
        munmap((void*)fp.addr, fp.length);
        close(fp.fd);
    #else
        UnmapViewOfFile(fp.addr);
        CloseHandle(fp.map_handle);
        CloseHandle(fp.hfile);
    #endif
}
