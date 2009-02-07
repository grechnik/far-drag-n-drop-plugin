// $Id: dircrtr.h 29 2008-04-21 11:00:40Z eleskine $
#ifndef __KARBAZOL_DRGNDROP_2_0__DIRCRTR_H__
#define __KARBAZOL_DRGNDROP_2_0__DIRCRTR_H__

#include "mystring.h"

class DirectoryCreator
{
private:
    MyStringW _root;
    bool createDirectoryRecursive(const wchar_t* subdir) const;
public:
    DirectoryCreator(): _root(){}
    DirectoryCreator(const wchar_t* dir);
    ~DirectoryCreator(){}
    /**
     * Tryes to create specified directory.
     * If the directory already exists the function fails.
     */
    bool createDirectory(const wchar_t* subdir) const;

    /**
     * Checks whether specified subdir exists.
     */
    bool checkDirectory(const wchar_t* subdir) const;

    /**
     * Creates specified directory.
     * If the directory already exists the functions does nothing
     * but succeeds.
     */
    bool ensureDirectory(const wchar_t* subdir) const;

    const MyStringW& root() const {return _root;}
};

#endif // __KARBAZOL_DRGNDROP_2_0__DIRCRTR_H__
// vim: set et ts=4 ai :

