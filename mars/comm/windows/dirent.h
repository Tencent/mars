/*
 * dirent.h - dirent API for Microsoft Visual Studio
 * 
 * Copyright (C) 2006 Toni Ronkko
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * ``Software''), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL TONI RONKKO BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * Mar 4, 2007, Toni Ronkko
 * Bug fix: due to the strncpy_s() function this file only compiled in
 * Visual Studio 2005.  Using the new string functions only when the
 * compiler version allows.
 * 
 * Nov  2, 2006, Toni Ronkko
 * Major update: removed support for Watcom C, MS-DOS and Turbo C to
 * simplify the file, updated the code to compile cleanly on Visual
 * Studio 2005 with both unicode and multi-byte character strings,
 * removed rewinddir() as it had a bug.
 * 
 * Aug 20, 2006, Toni Ronkko
 * Removed all remarks about MSVC 1.0, which is antiqued now.  Simplified
 * comments by removing SGML tags.
 *
 * May 14 2002, Toni Ronkko
 * Embedded the function definitions directly to the header so that no
 * source modules need to be included in the Visual Studio project.  Removed
 * all the dependencies to other projects so that this very header can be
 * used independently.
 *
 * May 28 1998, Toni Ronkko
 * First version.
 */
#ifndef DIRENT_H
#define DIRENT_H

#include <windows.h>
#include <tchar.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>


typedef struct dirent {
  /* name of current directory entry (a multi-byte character string) */
  char d_name[MAX_PATH + 1];

  /* file attributes */
  WIN32_FIND_DATA data;
} dirent;


typedef struct DIR {
  /* current directory entry */
  dirent current;

  /* is there an un-processed entry in current? */
  int cached;

  /* file search handle */
  HANDLE search_handle;

  /* search pattern (3 = zero terminator + pattern "\\*") */
  TCHAR patt[MAX_PATH + 3];
} DIR;


static DIR *opendir (const char *dirname);
static struct dirent *readdir (DIR *dirp);
static int closedir (DIR *dirp);


/* use the new safe string functions introduced in Visual Studio 2005 */
#if defined(_MSC_VER) && _MSC_VER >= 1400
# define STRNCPY(dest,src,size) strncpy_s((dest),(size),(src),_TRUNCATE)
#else
# define STRNCPY(dest,src,size) strncpy((dest),(src),(size))
#endif


/*
 * Open directory stream DIRNAME for read and return a pointer to the
 * internal working area that is used to retrieve individual directory
 * entries.
 */
static DIR*
opendir(
    const char *dirname)
{
  DIR *dirp;
  assert (dirname != NULL);
  assert (strlen (dirname) < MAX_PATH);

  /* construct new DIR structure */
  dirp = (DIR*) malloc (sizeof (struct DIR));
  if (dirp != NULL) {
    TCHAR *p;
    
    /* prepare search pattern */
#ifdef _UNICODE

    /* convert directory name to wide character string */
    MultiByteToWideChar(
        CP_ACP,                                /* code page */
        0,                                     /* conversion flags */
        dirname,                               /* mb-string to convert */
        -1,                                    /* length of mb-string */
        dirp->patt,                            /* wc-string to produce */
        MAX_PATH);                             /* max length of wc-string */
    dirp->patt[MAX_PATH] = '\0';
    
    /* append search pattern to directory name */
    p = wcschr (dirp->patt, '\0');
    if (dirp->patt < p  &&  *(p-1) != '\\'  &&  *(p-1) != ':') {
      *p++ = '\\';
    }
    *p++ = '*';
    *p = '\0';

#else /* !_UNICODE */
    
    /* take directory name... */
    STRNCPY (dirp->patt, dirname, sizeof(dirp->patt));
    dirp->patt[MAX_PATH] = '\0';
    
    /* ... and append search pattern to it */
    p = strchr (dirp->patt, '\0');
    if (dirp->patt < p  &&  *(p-1) != '\\'  &&  *(p-1) != ':') {
      *p++ = '\\';
    }
    *p++ = '*';
    *p = '\0';
    
#endif /* !_UNICODE */

    /* open stream and retrieve first file */
	dirp->search_handle = FindFirstFileEx (dirp->patt, FindExInfoStandard, &dirp->current.data, FindExSearchNameMatch, NULL, 0);
    if (dirp->search_handle == INVALID_HANDLE_VALUE) {
      /* invalid search pattern? */
      free (dirp);
      return NULL;
    }

    /* there is an un-processed directory entry in memory now */
    dirp->cached = 1;
    
  }
  return dirp;
}

  
/*
 * Read a directory entry, and return a pointer to a dirent structure
 * containing the name of the entry in d_name field.  Individual directory
 * entries returned by this very function include regular files,
 * sub-directories, pseudo-directories "." and "..", but also volume labels,
 * hidden files and system files may be returned.  
 */
static struct dirent *
readdir(
    DIR *dirp)
{
  assert (dirp != NULL);

  if (dirp->search_handle == INVALID_HANDLE_VALUE) {
    /* directory stream was opened/rewound incorrectly or it ended normally */
    return NULL;
  }

  /* get next directory entry */
  if (dirp->cached != 0) {
    /* a valid directory entry already in memory */
    dirp->cached = 0;
  } else {
    /* read next directory entry from disk */
    if (FindNextFile (dirp->search_handle, &dirp->current.data) == FALSE) {
      /* the very last file has been processed or an error occured */
      FindClose (dirp->search_handle);
      dirp->search_handle = INVALID_HANDLE_VALUE;
      return NULL;
    }
  }

  /* copy directory entry to d_name */
#ifdef _UNICODE
  
  /* convert entry name to multibyte */
  WideCharToMultiByte(
      CP_ACP,                                  /* code page */
      0,                                       /* conversion flags */
      dirp->current.data.cFileName,            /* wc-string to convert */
      -1,                                      /* length of wc-string */
      dirp->current.d_name,                    /* mb-string to produce */
      MAX_PATH,                                /* max length of mb-string */
      NULL,                                    /* use sys default character */
      NULL);                                   /* don't care  */
  dirp->current.d_name[MAX_PATH] = '\0';
  
#else /* !_UNICODE */

  /* copy as a multibyte character string */
  STRNCPY (dirp->current.d_name, dirp->current.data.cFileName, sizeof(dirp->current.d_name));
  dirp->current.d_name[MAX_PATH] = '\0';

#endif /* !_UNICODE */
  
  return &dirp->current;
}


/*
 * Close directory stream opened by opendir() function.  Close of the
 * directory stream invalidates the DIR structure as well as any previously
 * read directory entry.
 */
static int
closedir(
    DIR *dirp)
{
  assert (dirp != NULL);
 
  /* release search handle */
  if (dirp->search_handle != INVALID_HANDLE_VALUE) {
    FindClose (dirp->search_handle);
    dirp->search_handle = INVALID_HANDLE_VALUE;
  }

  /* release directory handle */
  free (dirp);
  return 0;
}


#endif /*DIRENT_H*/
