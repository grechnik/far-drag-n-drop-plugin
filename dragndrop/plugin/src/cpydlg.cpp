#include <utils.h>

#include "cpydlg.h"
#include "cperrdlg.h"
#include "dndguids.h"
#include "ddlng.h"

#define ITEM_X1 5
#define ITEM_X2 44
#define ITEM_LENGTH (ITEM_X2 - ITEM_X1 + 1)

CopyDialog::CopyDialogItems CopyDialog::copyDialogItemsTemplate =
{
        /*  0 */{DI_DOUBLEBOX,ITEM_X1-2,1,ITEM_X2+2,13,0,0,0,0,(wchar_t*)MCopy},
        /*  1 */{DI_TEXT,ITEM_X1, 2,ITEM_X2,0,0,0,0,0,(wchar_t*)MCopyingTheFile},
        /*  2 */{DI_TEXT,ITEM_X1, 3,ITEM_X2,0,0,0,0,0,L""},
        /*  3 */{DI_TEXT,ITEM_X1, 4,ITEM_X2,0,0,0,0,0,(wchar_t*)MCopyingTo},
        /*  4 */{DI_TEXT,ITEM_X1, 5,ITEM_X2,0,0,0,0,0,L""},
        /*  5 */{DI_TEXT,ITEM_X1, 6,ITEM_X2,0,0,0,0,0,L"\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591"}, // u+2591 0xb0
        /*  6 */{DI_TEXT,ITEM_X1, 7,ITEM_X2,0,0,0,0,0,L"Total size:"},
        /*  7 */{DI_TEXT,ITEM_X1, 8,ITEM_X2,0,0,0,0,0,L"\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591\x2591"},
        /*  8 */{DI_TEXT,ITEM_X1, 9,ITEM_X2,0,0,0,0,0,L"\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500"}, // u+2500 0xc4
        /*  9 */{DI_TEXT,ITEM_X1,10,ITEM_X2,0,0,0,0,0,L""},
        /* 10 */{DI_TEXT,ITEM_X1,11,ITEM_X2,0,0,0,0,0,L"\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500"}, // u+2500 0xc4
        /* 11 */{DI_TEXT,ITEM_X1,12,ITEM_X2,0,0,0,0,0,L"Time: %.2d:%.2d:%.2d Left: %.2d:%.2d:%.2d %6dKb/s"}
};

#define getMyItemId(structItem) getFarDlgItemId(CopyDialogItems,structItem)
#define NANOSECPERSEC 10000000

const GUID& CopyDialog::Id() const
{
    return copyDialogGuid;
}

CopyDialog::CopyDialog(bool move): FarDialog(), _items(copyDialogItemsTemplate),
    _totalProcessedSize(0), _totalSize(0), _currentProcessedSize(0),
    _currentSize(0), _filesToProcess(0),
    _filesProcessed(-1), _fileListProcessed(0), _timeStart(0)
{
    if (move)
    {
        _items.frame.Data = (wchar_t*)MMove;
        _items.label0.Data = (wchar_t*)MMovingTheFile;
    }
}

bool CopyDialog::onInit()
{
    updateTimesAndSpeed();
    updateFilesProcessed();
    updatePercents();
    updateTimesAndSpeed();
    return true;
}

bool CopyDialog::onClose(intptr_t id)
{
    id;
    /** @todo Ask user are they sure to cancel copy/move operation */
    TRACE("Copy Dialog is about to be closed with %d code\n", id);
    return true;
}

/*
 * This function converts integer numbers into string using
 * comma as 'Digit grouping symbol'.
 * I.e. number 999999999 will be converted to "999,999,999".
 */
static wchar_t* sizeToString(wchar_t* buff, const int64_t& value)
{
    wsprintf(buff, L"%I64d", value);

    size_t len = lstrlen(buff);
    size_t mod = len % 3;
    size_t div = len / 3;

    while (div)
    {
        if (mod)
        {
            memmove(&buff[mod + 1], &buff[mod], (len - mod + 2)*sizeof(*buff));
            buff[mod] = L',';
            len++;
            mod++;
        }

        mod += 3;
        div--;
    }

    return buff;
}

static wchar_t* centerAndFill(wchar_t* buff, size_t size /*of buff including NULL treminator*/,
        wchar_t filler)
{
    size_t len = lstrlen(buff);
    size_t pos = (size - 1 - len) / 2;

    memmove(&buff[pos], buff, len*sizeof(*buff));

    size_t i;
    size_t pos2 = pos + len;
    for (i = 0; i < pos; i++)
    {
        buff[i] = buff[pos2++] = filler;
    }

    if (pos2 < size-1)
        buff[pos2++] = filler;

    buff[pos2] = L'\0';

    return buff;
}

void CopyDialog::updateTotalSize()
{
    wchar_t totalSizeValue[] = L"9,223,372,036,854,775,807"; // Maximum value of int64
    wchar_t totalSizeString[ITEM_LENGTH + 1];

    wsprintf(totalSizeString, GetMsg(MTotalSize), sizeToString(totalSizeValue, _totalSize));

    setText(getMyItemId(lblTotalSize),
            centerAndFill(totalSizeString, LENGTH(totalSizeString), L'\x2500'));
}

void CopyDialog::updateFilesProcessed()
{
    wchar_t filesProcessed[ITEM_LENGTH + 1];

    int processed;
    if (_filesProcessed < 0)
        processed = 0;
    else
        processed = _filesProcessed;

    wsprintf(filesProcessed, GetMsg(MFilesProcessed), processed, _filesToProcess);

    setText(getMyItemId(lblFilesProcessed), filesProcessed);
}

static int calcPercents(const int64_t& value, const int64_t& base, int len)
{
    if (!value)
    {
        return 0;
    }

    if (value >= base)
    {
        return len;
    }

    return static_cast<int>(value * len / base);
}

void CopyDialog::updatePercents()
{
    int percents;
    if (_fileListProcessed)
    {
        percents = calcPercents(_totalProcessedSize, _totalSize, ITEM_LENGTH);
        updateProgressBar(percents, getMyItemId(progressTotal));
    }

    percents = calcPercents(_currentProcessedSize, _currentSize, ITEM_LENGTH);
    updateProgressBar(percents, getMyItemId(progressCurrent));
}

void CopyDialog::updateProgressBar(int value, int controlId)
{
    wchar_t stringValue[ITEM_LENGTH + 1];

    ASSERT(value < LENGTH(stringValue));

    int i = value - 1;
    int j = value;
    while (i >= 0 || j < LENGTH(stringValue)-1)
    {
        if (i >=0)
            stringValue[i--] = L'\x2588';
        if (j < LENGTH(stringValue)-1)
            stringValue[j++] = L'\x2591';
    }
    stringValue[j] = L'\0';

    setText(controlId, stringValue);
}

void CopyDialog::updateTimesAndSpeed()
{
    int64_t spent;
    GetSystemTimeAsFileTime(reinterpret_cast<LPFILETIME>(&spent));
    if (!_timeStart)
    {
        _timeStart = spent;
        spent = 0;
    }
    else
    {
        spent -= _timeStart;
    }

    int64_t left = 0;
    int speed = 0;
    if (spent && _totalProcessedSize)
    {
        left = (_totalSize - _totalProcessedSize) * spent / _totalProcessedSize;
        speed = static_cast<int>(_totalProcessedSize * NANOSECPERSEC / spent);
    }

    SYSTEMTIME spentTime, leftTime;

    FileTimeToSystemTime(reinterpret_cast<LPFILETIME>(&spent), &spentTime);
    FileTimeToSystemTime(reinterpret_cast<LPFILETIME>(&left), &leftTime);

    wchar_t timeString[MAX_PATH];

    wsprintf(timeString,
            GetMsg(MFileCopyingTimes), // "Time: %.2d:%.2d:%.2d Left: %.2d:%.2d:%.2d %6dKb/s"
            spentTime.wHour, spentTime.wMinute, spentTime.wSecond,
            leftTime.wHour, leftTime.wMinute, leftTime.wSecond,
            // TODO Use specific suffix for large speed
            speed ? speed >> 10 : 0);

    setText(getMyItemId(lblTimeInfo), timeString);
}

bool CopyDialog::appendFile(const int64_t& size, bool lastOne)
{
    if (!running())
    {
        return false;
    }

    if (lastOne)
    {
        InterlockedExchange(&_fileListProcessed, 1);
    }
    else
    {
        InterlockedExchangeAdd64(&_totalSize, size);
        InterlockedIncrement(&_filesToProcess);
        if (hwnd())
        {
            updateTotalSize();
            updateFilesProcessed();
        }
    }

    return true;
}

bool CopyDialog::nextFile(const wchar_t* src, const wchar_t* dest,
        const int64_t& size)
{
    if (running())
    {
        InterlockedIncrement(&_filesProcessed);
        MyStringW srcFile = src;
        MyStringW destFile = dest;

        _totalProcessedSize += (_currentSize - _currentProcessedSize);
        InterlockedExchange64(&_currentProcessedSize, 0);
        _currentSize = size;

        srcFile = TruncPathStr(srcFile, ITEM_LENGTH);
        destFile = TruncPathStr(destFile, ITEM_LENGTH);

        setText(getMyItemId(lblSrcFile), srcFile);
        setText(getMyItemId(lblDestFile), destFile);

        updateFilesProcessed();
        updatePercents();

        return true;
    }

    return false;
}

InitDialogItem* CopyDialog::items()
{
    return reinterpret_cast<InitDialogItem*>(&_items);
}

size_t CopyDialog::itemsCount()
{
    return sizeof(CopyDialogItems)/sizeof(InitDialogItem);
}

int CopyDialog::right()
{
    return _items.frame.X2+4;
}

int CopyDialog::bottom()
{
    return _items.frame.Y2+2;
}

DWORD CopyDialog::flags()
{
    return 0x10;
}

bool CopyDialog::onNextEntry(const int /*reason*/, const FileListEntry& e)
{
    if (!this)
    {
        return true;
    }

    LARGE_INTEGER size;
    size.LowPart  = e.data().nFileSizeLow;
    size.HighPart = e.data().nFileSizeHigh;

    if (e.type() == FileListEntry::File
            && !appendFile(size.QuadPart, false))
        return false;

    return true;
}

bool CopyDialog::onAllProcessed()
{
    if (!this)
    {
        return true;
    }

    if (!appendFile(0, true))
    {
        return false;
    }

    return true;
}

bool CopyDialog::onFileExists(const wchar_t* src, const wchar_t* dest)
{
    src;
    dest;

    /** @todo Ask user whether to overwrite files bug #3 */
    return true;
}

bool CopyDialog::onFileStep(const int64_t& step)
{
    if (running())
    {
        _currentProcessedSize += step;
        _totalProcessedSize += step;

        updateTimesAndSpeed();
        updatePercents();

        return true;
    }
    return false;
}

bool CopyDialog::onFileError(const wchar_t* src, const wchar_t* dest, DWORD errorNumber)
{
    /** @todo Implement error check */
    if (running())
    {
        // give the _dialog a chance to appear
        while (sendMessage(DM_GETTEXT, 0, 0) == 0)
        {
            Sleep(1);
        }
    }
    CopyErrorDialog* dlg = new CopyErrorDialog();
    if (!dlg)
    {
        return false;
    }

    dlg->addRef();
    int res = dlg->show(src, dest, errorNumber);
    dlg->release();
    if (CopyErrorDialog::cancel == res)
    {
        return true;
    }
    return false;
}

// vim: set et ts=4 ai :
