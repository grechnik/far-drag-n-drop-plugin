// $Id: filelist.h 26 2008-04-20 18:48:32Z eleskine $

#ifndef __KARBAZOL_DRAGNDROP_2_0__FILELIST_H__
#define __KARBAZOL_DRAGNDROP_2_0__FILELIST_H__

#include <windows.h>
#include "mystring.h"
#include "ddlock.h"

class FileListEntry
{
private:
    MyStringW _srcpath;
    size_t _subpath;
    WIN32_FIND_DATA _data;
public:
    FileListEntry();
    FileListEntry(const FileListEntry& e);
    FileListEntry(const MyStringW& cmnDir, const wchar_t* subpath, 
            const WIN32_FIND_DATA& data);
    ~FileListEntry(){_subpath = 0;}
    FileListEntry& operator=(const FileListEntry& e);
    inline const wchar_t* srcpath() const {return _srcpath;}
    const wchar_t* subpath() const;
    inline const WIN32_FIND_DATA& data() const {return _data;}
};

struct FileListNotify
{
    virtual bool onNextEntry(const int reason, const FileListEntry& e) = 0;
    virtual bool onAllProcessed() = 0;
};

class FileListEntryContainer;

class FileList
{
private:
    FileListNotify* _notify;
    HANDLE _threadEnd;
    HANDLE _haveEntries;
    HANDLE _stopThread;
    const wchar_t* _files;
    CriticalSection _entriesGuard;
    FileListEntryContainer* _head;
    FileListEntryContainer* _tail;
    unsigned int createTheList(const wchar_t* files);
    bool processEntry(const wchar_t* file);
    bool processDir(const wchar_t* prefix, const FileListEntry& e);
    static DWORD WINAPI runMe(FileList* This);
    bool appendEntry(const FileListEntry& e);
public:
    FileList(const wchar_t* files, FileListNotify* fn=0);
    ~FileList();
    bool next(FileListEntry& e);
};

#endif // __KARBAZOL_DRAGNDROP_2_0__FILELIST_H__
