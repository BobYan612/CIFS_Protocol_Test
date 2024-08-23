#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <conio.h>
#include <windows.h>
#include <math.h>
#include <iostream>
#include<fstream>
#include<vector>

#include "time.h"
#include "md5.h"
#include "locale.h"
#include <stdio.h>

#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#define _PUBLIC_ __attribute__((visibility("default")))
#define POPT_ARG_STRING		1	/*!< arg will be saved as string */
#define POPT_ARG_INT		2	/*!< arg will be converted to int */
#define POPT_ARG_LONG		3	/*!< arg will be converted to long */
#define POPT_ARG_BOOL       4   /*!< arg will be converted to bool */
#define POPT_ARG_FLOAT      5   /*!< arg will be converted to bool */

#define SMB3_STATUS_FAILURE 1
#define SMB3_STATUS_SUCCESS 0
#define SMB3_STATUS_SKIP    2
#define NTSTATUS int

using namespace std;

struct ArgOption {
   LPSTR name;
   char  shortname;
   byte  type;
   void * arg;
   LPSTR descrition;
};

struct TestCase{
   LPSTR case_name;
   BOOL (* run)();
   struct TestCase * next_t ;
   struct TestCase * pre_t ;
};

extern long smb3_duration;    //the unit is minutes
extern LPSTR smb3_source_filepath;
extern LPSTR smb3_destination_filepath;
extern LPSTR smb3_workdir;
extern LPSTR smb3_separator;
extern long smb3_section_size; //the unit is bite;
extern int smb3_section_num;
extern int smb3_debug_level;
extern int smb3_dir_level;
extern int smb3_dir_width;
extern int smb3_files_per_folder;
extern float smb3_percent;
extern LPSTR smb3_dir_prefix;
extern LPSTR smb3_file_prefix;
extern LPSTR smb3_dbfile;
extern long smb3_file_size;    //the unit is KB
extern long  smb3_block_size;
extern LPSTR gCaseName;
extern bool smb3_update_file;
extern TestCase * testCases;
extern bool smb3_help;

struct ArgOption * getOption(struct ArgOption * options, char shortName);
struct ArgOption * getOption(struct ArgOption * options, LPSTR name);

BOOL parseArg(int argc,char *argv[], struct ArgOption * options);
void help(const struct ArgOption * options);
LPSTR smb3_get_time(LPSTR timestr);

HANDLE openFile( LPSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode);
int closeFile( HANDLE hFile );
BOOL writeRandomFile( HANDLE hFile, long block_size, long long file_size);
BOOL copyFile(HANDLE inFile, HANDLE outFile, long start, long segmentSize);
BOOL verifyFile(HANDLE inFile, HANDLE outFile, long start, long segmentSize);
BOOL lockFile(HANDLE inFile, long start, long segmentSize);
BOOL replaceLineInFile(LPSTR filename, LPSTR lineFilter, LPSTR newLine);
BOOL unlockFile(HANDLE inFile, long start, long segmentSize);
LPSTR calcSectionMD5(LPSTR filename, int section_index, long section_size);
LPSTR calcMD5(LPSTR filename);
off_t GetFileSize( LPSTR path );

BOOL registerCase(LPSTR casename, BOOL (*run) ());
struct TestCase * getTestCase(LPSTR caseName);

BOOL smb3_write_range_test();
BOOL smb3_handle_lease_test();
BOOL smb3_file_lease_test();


#endif // UTILS_H_INCLUDED


