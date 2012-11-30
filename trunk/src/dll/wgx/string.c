/*
 *  WGX - Windows GUI Extended Library.
 *  Copyright (c) 2007-2012 Dmitri Arkhangelski (dmitriar@gmail.com).
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/**
 * @file string.c
 * @brief Strings.
 * @addtogroup String
 * @{
 */

#include "wgx-internals.h"

/**
 * @brief Size of the buffer used by wgx_vsprintf
 * initially. Larger sizes tend to reduce time needed
 * to format long strings.
 */
#define WGX_VSPRINTF_BUFFER_SIZE 128

/**
 * @brief A robust and flexible alternative to _vsnprintf.
 * @param[in] format the format specification.
 * @param[in] arg pointer to the list of arguments.
 * @return Pointer to the formatted string, NULL
 * indicates failure. The string must be deallocated
 * by free() call after its use.
 * @note Optimized for speed, can allocate more memory than needed.
 */
char *wgx_vsprintf(const char *format,va_list arg)
{
    char *buffer;
    int size;
    int result;
    
    if(format == NULL)
        return NULL;
    
    /* set the initial buffer size */
    size = WGX_VSPRINTF_BUFFER_SIZE;
    do {
        buffer = winx_malloc(size);
        if(buffer == NULL)
            return NULL;
        memset(buffer,0,size); /* needed for _vsnprintf */
        result = _vsnprintf(buffer,size,format,arg);
        if(result != -1 && result != size)
            return buffer;
        /* buffer is too small; try to allocate two times larger */
        winx_free(buffer);
        size <<= 1;
        if(size <= 0)
            return NULL;
    } while(1);
    
    return NULL;
}

/**
 * @brief A robust and flexible alternative to _snprintf.
 * @param[in] format the format specification.
 * @param[in] ... the arguments.
 * @return Pointer to the formatted string, NULL
 * indicates failure. The string must be deallocated
 * by free() call after its use.
 * @note Optimized for speed, can allocate more memory than needed.
 */
char *wgx_sprintf(const char *format, ...)
{
    va_list arg;
    
    if(format){
        va_start(arg,format);
        return wgx_vsprintf(format,arg);
    }
    
    return NULL;
}

/** @} */
