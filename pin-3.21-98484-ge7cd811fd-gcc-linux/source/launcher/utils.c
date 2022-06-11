/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "utils.h"
#if defined(PIN_CRT) && defined(TARGET_LINUX)
#include "pincrt_file_utils.h"
#endif

#define PATH_MAX_SIZE 4096

/*
 * Only on linux the launcher_u linked with PinCRT which implements strnlen_s.
 * Instead on macOS* uses the system library function strnlen.
 */
size_t get_strlen(const char* str)
{
    size_t len;
#if defined(PIN_CRT) && defined(TARGET_LINUX)
    len = strnlen_s(str, PATH_MAX_SIZE);
#else
    len = strnlen(str, PATH_MAX_SIZE);
#endif
    assert(len > 0);
    return len;
}

char* appendPath(const char* s1, const char* s2, const char* s3)
{
    int n   = 1;
    char* p = 0;
    if (s1) n += get_strlen(s1);
    if (s2) n += get_strlen(s2);
    if (s3) n += get_strlen(s3);
    p = (char*)malloc(sizeof(char) * n);
    if (p == NULL) abort();
    if (s1) strcpy(p, s1);
    if (s2) strcat(p, s2);
    if (s3) strcat(p, s3);
    return p;
}

void check_retval(int retval, const char* str)
{
    if (retval != 0)
    {
        perror(str);
        exit(1);
    }
}

/*!
 * @brief Checks if the file exist and readable.
 * @param fn The file path
 * @return True if exist, readable and executable
 */
unsigned int check_file_exists(const char* fn)
{
    unsigned int okay = 1;
    if (access(fn, R_OK) == -1) okay = 0;
    return okay;
}

/*!
 * @brief Checks for file properties and prints an error message is requirements are not met.
 * @param fn The file path
 */
void check_file(const char* fn)
{
    unsigned int okay = check_file_exists(fn);
    if (!okay)
    {
        fprintf(stderr, "Error. File doesn't exist (%s)\n\n", fn);
        exit(1);
    }
}

/*!
 * Check if the file path is a directory
 * @param fn The file path
 * @return 1 if the file path is not a directory
 */
unsigned int check_not_directory(const char* fn)
{
#if defined(PIN_CRT) && defined(TARGET_LINUX)
    return is_directory_file(fn);
#else
    unsigned int okay = 1;
    struct stat st;
    if (stat(fn, &st) == -1 || !S_ISREG(st.st_mode)) okay = 0;
    return okay;
#endif
}

/*!
 * Checks that a file exists in the directory and that it is not a directory.
 * @param fn The file path to be checked
 * @param dir The directory path to be checked in
 * @param buff A buffer to create the complete path
 * @return 1 if file exist and not a directory
 */
unsigned int check_file_in_dir(const char* fn, const char* dir, char* buff)
{
    buff = appendPath(dir, "/", fn);
    return check_file_exists(buff) && check_not_directory(buff);
}

/*!
 * Searches for the given executable in the directories list at the PATH environment variable,
 * and returns the directory it was found in.
 * @param exename The executable path
 * @return The directory it was found in, or null if not found.
 */
char* search_in_path(const char* exename)
{
    const char dirsepchar = ':';
    char* env_path;
    char* syspath;
    char* buff;
    char* dir;
    char* dirsep;
    char* path = 0;

    env_path = getenv("PATH");
    assert(env_path != NULL);

    syspath = strdup(env_path);
    assert(syspath != NULL);

    buff = (char*)malloc(get_strlen(syspath) + get_strlen(exename) + 2);
    assert(buff != NULL);

    dir    = syspath;
    dirsep = strchr(dir, dirsepchar);

    while (dirsep)
    {
        *dirsep = 0;
        if (check_file_in_dir(exename, dir, buff))
        {
            path = strdup(dir);
            free(syspath);
            free(buff);
            return path;
        }
        dir    = dirsep + 1;
        dirsep = strchr(dir, dirsepchar);
    }
    if (check_file_in_dir(exename, dir, buff))
    {
        path = strdup(dir);
    }
    free(syspath);
    free(buff);
    return path;
}

/*!
 * Finds the base path (containing directory) of the given executable.
 * @param filename The file path
 * @return The base path
 */
char* find_base_path(char* filename)
{
    char* x;
    char* path = strdup(filename);
    x          = strrchr(path, '/');
    if (x)
    {
        *x = 0;
    }
    else
    {
        x = search_in_path(filename);
        if (x)
        {
            free(path);
            return x;
        }
    }
    return path;
}
