/************************************************************

  FileName: utils.c

  Description:    The module is to provide some common methods

  Version:        1.0

  Function List:


  History:
         bobyan     create the first build.

***********************************************************/

#include "utils.h"
#include <iostream>

/*----------------Following is the public variables used by parameters--------*/
_PUBLIC_ long smb3_duration = -1;
_PUBLIC_ LPSTR smb3_source_filepath;
_PUBLIC_ LPSTR smb3_destination_filepath;
_PUBLIC_ LPSTR smb3_workdir;
_PUBLIC_ LPSTR smb3_separator = "\\";
_PUBLIC_ long smb3_section_size = 1024; //the unit is bite;
_PUBLIC_ int smb3_section_num = 5; //the unit is bite;
_PUBLIC_ long smb3_file_size = 10240;    //the unit is KB
_PUBLIC_ long  smb3_block_size = 1024 * 8;
_PUBLIC_ int smb3_files_per_folder = 10;
_PUBLIC_ int smb3_debug_level = 0;
_PUBLIC_ LPSTR gCaseName;
_PUBLIC_ TestCase * gTestCases = NULL;
_PUBLIC_ int smb3_dir_level = 3;
_PUBLIC_ int smb3_dir_width = 10;
_PUBLIC_ LPSTR smb3_dir_prefix = "smb3_win8_dir";
_PUBLIC_ LPSTR smb3_file_prefix = "smb3_win8_file";
_PUBLIC_ LPSTR smb3_dbfile = "smb3_win8_cksum.txt";
_PUBLIC_ float smb3_percent = 0.01;
_PUBLIC_ bool smb3_help = false;
_PUBLIC_ bool smb3_update_file = false;
/*************************************************

  Function:       parseArg

  Description:    The function is to parse the parameters in main then store the
                  value in public vairables.

  Input:          argc:  The count of the args

                  argv:  The array of the args

                  options: The array of the ArgOption which defines the arributes of
                           every parameter.

  Output:         *None*

  Return:         BOOL  the status returned by the operation.

  Others:         if the parameter is not validated, we need to list the help

*************************************************/
BOOL parseArg(int argc,char *argv[], struct ArgOption * options)
{
   int i = 0;
   ArgOption * option;
   LPSTR value;
   LPSTR arg;
   LPSTR p;
   LPSTR tmp_value;
   for( i = 1; i < argc; i++){
      p = argv[i];
      if(*p == '-'){
          if(*(++p) == '-'){
             p++;
             option = getOption(options, p);
          }
          else
             option = getOption(options, *p);
          if(smb3_debug_level >= 1)
              cout<<option->shortname<<"="<<argv[i+1]<<endl;
          if(!(option  == NULL)){
              if(option->type == POPT_ARG_STRING){
                  value = argv[++i];
                  *((const char **) option->arg) = (value)? strdup(value) : NULL;
              }
              else if(option->type == POPT_ARG_INT){
                  value = argv[++i];
                 *((int *)(option->arg)) = atoi(value);
              }
              else if(option->type == POPT_ARG_LONG){
                   value = argv[++i];
                  *((long *)(option->arg)) = atol(value);
              }
              else if(option->type == POPT_ARG_BOOL){
                  value = argv[++i];
                  if(atoi(value) == 1){
                    *((bool *)(option->arg)) = true;
                  }
                  else{
                    *((bool *)(option->arg)) = false;
                  }
              }
              else if(option->type == POPT_ARG_FLOAT){
                  value = argv[++i];
                  cout<<" value is "<<value<<endl;
                  *((float *)(option->arg)) = atof(value);
              }
          }
          else{
              if(smb3_debug_level >= 1)
                cout<<"Warning:can't recongnize "<<argv[i]<<endl;

              continue;
//              help(options);
//              return FALSE;
          }
      }
      else{
         value = argv[i];
         tmp_value = (value)? strdup(value) : NULL;
         if(getTestCase(tmp_value) == NULL){
              if(smb3_debug_level >= 1)
                cout<<"Warning:can't recongnize "<<argv[i]<<endl;
              continue;
         }
         else{
              gCaseName = (value)? strdup(value) : NULL;
         }
      }
   }
   return TRUE;
}

/*************************************************

  Function:       getOption

  Description:    The function is to get the option struct
                  according to the shortName (a char)

  Input:          options:  The array of the ArgOption which defines the arributes of
                            every parameter.

                  shortName:  A char for certain public variable

  Output:         *None*

  Return:         struct ArgOption

  Others:         *None*

*************************************************/
struct ArgOption * getOption(struct ArgOption * options, char shortName)
{
   int i, count;
   ArgOption * curoption = options;
   while(curoption->shortname != '0'){
      if(curoption->shortname == shortName)
         return curoption;
      curoption++;
   }
   return NULL;
}

/*************************************************

  Function:       getOption

  Description:    The function is to get the option struct
                  according to the shortName (a char)

  Input:          options:  The array of the ArgOption which defines the arributes of
                            every parameter.

                  shortName:  A char for certain public variable

  Output:         *None*

  Return:         struct ArgOption

  Others:         *None*

*************************************************/
struct ArgOption * getOption(struct ArgOption * options, LPSTR name)
{
   int i, count;
   ArgOption * curoption = options;
   while(curoption->shortname != '0'){
      if(strcmp(curoption->name , name) == 0)
         return curoption;
      curoption++;
   }
   return NULL;
}

/*************************************************

  Function:       help

  Description:    The function is to list the help

  Input:          options:  The array of the ArgOption which defines the arributes of
                            every parameter.

                  shortName:  A char for certain public variable

  Output:         *None*

  Return:         struct ArgOption

  Others:         *None*

*************************************************/
void help(const struct ArgOption * options)
{
   struct TestCase * mycase;
   const ArgOption * curoption = options;
   cout<<"Usage: smb3.exe <options> casename"<<endl;
   int i, count;
   while(curoption->shortname != '0' ){
       cout<<"\t\t-"<<curoption->shortname<<"\t"<< curoption->descrition<<endl;
       curoption++;
   }
   cout<<"CaseName should be following:"<<endl;
   if(gTestCases == NULL){
       cout<<"\tCurrently has no any case available"<<endl;
   }
   else{
       mycase = gTestCases;
       do{
           printf("\t%s",mycase->case_name);
           mycase = mycase->next_t;
       }while(mycase!= NULL);
   }
   printf("\n");
   return;
}

/*************************************************

  Function:       registerCase

  Description:    The function is register the case, so the case name
                  may link to its method.

  Input:          casename: The case name used in CLI

                  run:  the reference of the method.

  Output:         *None*

  Return:         BOOL

  Others:         The gTestCases is one kind of linked list,
                  When a new case is joined, it just appends it
                  at the end of the list.

*************************************************/
BOOL registerCase(LPSTR casename, BOOL (*run) ())
{
   struct TestCase * mycase;
   struct TestCase * inCase;
   inCase = (struct TestCase *) malloc(sizeof(struct TestCase));
   inCase->case_name = (char *)calloc((strlen(casename) + 1) , sizeof(char));
   memcpy(inCase->case_name, casename, strlen(casename));
   inCase->run = run;
   inCase->next_t = NULL;
   inCase->pre_t = NULL;
   if(gTestCases == NULL)
       gTestCases = inCase;
   else{
       mycase = gTestCases;
       while(mycase->next_t != NULL)
           mycase = mycase->next_t;
       mycase->next_t = inCase;
       inCase->pre_t = mycase;
   }
   return TRUE;
}
/*************************************************

  Function:       getTestCase

  Description:    The function is to fetch the TestCase struct, every testcase
                  struct includes its name and related method.

  Input:          casename: The case name

  Output:         *None*

  Return:         struct TestCase

  Others:         *None*

*************************************************/
struct TestCase * getTestCase(LPSTR caseName)
{
   struct TestCase * mycase;
   if(caseName == NULL){
      return NULL;
   }
   if(gTestCases == NULL){
       if(smb3_debug_level >= 1)
           cout<<"case is NULL"<<endl;
       return NULL;
   }
   else{
       mycase = gTestCases;
       do{
           if(smb3_debug_level >= 1)
               cout<<mycase->case_name<<"<->"<<caseName<<endl;
           if(strcmp(mycase->case_name, caseName) == 0)
              return mycase;
           if(smb3_debug_level >= 1)
               cout<<"goto next compare"<<endl;
           mycase = mycase->next_t;
       }while(mycase != NULL);
   }
   return NULL;
}

/*************************************************

  Function:       openFile

  Description:    The function is to open a file and return its HANDLE

  Input:          lpFileName: The fullpath of the filename

                  dwDesiredAccess: the access permission for the file such as GENERIC_READ | GENERIC_WRITE

                  dwShareMode: share mode of Windows API such as FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE

  Output:         *None*

  Return:         HANDLE  the handle of the file

  Others:         *None*

*************************************************/
HANDLE openFile( LPSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode)
{
   LPVOID     lpMsgbuff = NULL;
   HANDLE     hFile = NULL;

   hFile = CreateFile(lpFileName, dwDesiredAccess, dwShareMode,

                NULL,        // no security attributes
                OPEN_ALWAYS, // creating new file or opening if exists
                0,           // not overlapped I/O
                NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
         // Handle the error.
         FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgbuff , 0, NULL);
         cout<<"Error "<<__FILE__<<","<<__LINE__<<": failed  to open "<<lpFileName<<" with error code: "<<GetLastError()<<":"<<(LPSTR)lpMsgbuff<<endl;
         if (lpMsgbuff != NULL)
            LocalFree(lpMsgbuff);
         return NULL;
    }
    return hFile;
}

/*************************************************
openFile
  Function:       closeFile

  Description:    The function is to close a file

  Input:          hFile: The Handle of the file

  Output:         *None*

  Return:         BOOL

  Others:         *None*

*************************************************/
BOOL closeFile( HANDLE hFile )
{
    LPVOID     lpMsgbuff = NULL;
    BOOL fSuccess = CloseHandle (hFile);
    if (!fSuccess)
    {
       // Handle the error.
         FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgbuff , 0, NULL);
         cout<<"Error "<<__FILE__<<","<<__LINE__<<": failed  to close file with error code: "<<GetLastError()<<(LPSTR)lpMsgbuff<<endl;
         if (lpMsgbuff != NULL)
            LocalFree(lpMsgbuff);
        return FALSE;
    }
    return TRUE;
}

/*************************************************

  Function:       writeRandomFile

  Description:    The function is to write random block in a file

  Input:          hFile: The Handle of the file

                  block_size: the block_size

                  file_size: the file size

  Output:         *None*

  Return:         BOOL

  Others:         *None*

*************************************************/
BOOL writeRandomFile( HANDLE hFile, long block_size, long long file_size)
{
    byte * DataBuffer;
    long segSize = 0;
    LPVOID     lpMsgbuff = NULL;
    DWORD dwBytesToWrite = 0;
    DWORD dwBytesWritten = 0;
    BOOL bErrorFlag = FALSE;
    int i, j, count;

    count = ceil(file_size/(double)block_size);
	segSize = block_size;
    DataBuffer = (byte *)calloc((segSize ) , sizeof(byte));
    for(j=0; j<count; j++){
        if(j == count - 1){
           segSize = file_size - block_size * j;
           free(DataBuffer);
           DataBuffer = NULL;
           DataBuffer = (byte *)calloc((segSize ) , sizeof(byte));
        }

        dwBytesToWrite = (DWORD)(segSize);
        for(i=0; i < segSize ; i++){
            DataBuffer[i] = rand()%256;
        }
        bErrorFlag = WriteFile(
                        hFile,           // open file handle
                        DataBuffer,      // start of data to write
                        dwBytesToWrite,  // number of bytes to write
                        &dwBytesWritten, // number of bytes that were written
                        NULL);            // no overlapped structure

        if (FALSE == bErrorFlag)
        {
            FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgbuff , 0, NULL);
            cout<<"Error "<<__FILE__<<","<<__LINE__<<": failed  to write file with error code: "<<GetLastError()<<(LPSTR)lpMsgbuff<<endl;
            return FALSE;
        }
        else
        {
            if (dwBytesWritten != dwBytesToWrite)
            {
                // This is an error because a synchronous write that results in
                // success (WriteFile returns TRUE) should write all data as
                // requested. This would not necessarily be the case for
                // asynchronous writes.
                printf("Error: dwBytesWritten != dwBytesToWrite\n");
                return FALSE;
            }
        }
        if((j+1) % 1000 == 0 && !FlushFileBuffers(hFile)){
            printf("Error: can't flush the buffer to the file");
            return FALSE;
        }
    }
    if(!FlushFileBuffers(hFile)){
        FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgbuff , 0, NULL);
        cout<<"Error "<<__FILE__<<","<<__LINE__<<": failed  to flush file with error code: "<<GetLastError()<<(LPSTR)lpMsgbuff<<endl;
        return FALSE;
    }
    free(DataBuffer);
    DataBuffer = NULL;
    return TRUE;
}

/*************************************************

  Function:       copyFile

  Description:    The function is to read a section from source file
                  and then write the section into destination file

  Input:          inFile: The Handle of the source file

                  outFile: The handle of the destination file

                  start: the offset of the file

                  segmentSize: the length of the segment.

  Output:         *None*

  Return:         BOOL

  Others:         *None*

*************************************************/
BOOL copyFile(HANDLE inFile, HANDLE outFile, long start, long segmentSize)
{
     OVERLAPPED overlapstruc = {0};
     byte * Base;
     LPVOID     lpMsgbuff = NULL;
     BOOL bErrorFlag = FALSE;
     int count,j;
     long block_size = smb3_block_size;
     long segSize;


    count = ceil(segmentSize/(double)block_size);
	segSize = block_size;
    Base = (byte *)calloc(segSize,sizeof(byte));
    if(smb3_debug_level >= 1){
        cout<<"start position:"<<start<<endl;
        cout<<"count:"<<count<<endl;
    }
    for(j=0; j<count; j++){
         if(j == count - 1){
            segSize = segmentSize - block_size * j;
            free(Base);
            Base = NULL;
            Base = (byte *)calloc((segSize ) , sizeof(byte));
         }
         overlapstruc.Offset = start + block_size * j;
         overlapstruc.OffsetHigh = 0;
         if(smb3_debug_level >= 1)
            cout<<"read from source file"<<endl;
         bErrorFlag = ReadFileEx(inFile,Base,segSize,&overlapstruc,NULL);
         if (FALSE == bErrorFlag)
         {
            FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgbuff , 0, NULL);
            cout<<"Error "<<__FILE__<<","<<__LINE__<<": failed  to read file with error code: "<<GetLastError()<<(LPSTR)lpMsgbuff<<endl;
            return FALSE;
         }
         if(smb3_debug_level >= 2)
            cout<<"output:"<<Base<<endl;
         overlapstruc.Offset = start + segSize * j;
         overlapstruc.OffsetHigh = 0;
         if(smb3_debug_level >= 1)
            cout<<"write to destination file"<<endl;
         bErrorFlag = WriteFileEx(outFile,Base,segSize,&overlapstruc, NULL);
         if (FALSE == bErrorFlag)
         {
            FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgbuff , 0, NULL);
            cout<<"Error "<<__FILE__<<","<<__LINE__<<": failed  to write file with error code: "<<GetLastError()<<(LPSTR)lpMsgbuff<<endl;
            return FALSE;
         }
         if((j+1) % 100 == 0 && !FlushFileBuffers(outFile)){
            FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgbuff , 0, NULL);
            cout<<"Error "<<__FILE__<<","<<__LINE__<<": failed  to flush file with error code: "<<GetLastError()<<(LPSTR)lpMsgbuff<<endl;
            return FALSE;
         }
         Sleep(100);
     }
     free(Base);
     return TRUE;
}
/*************************************************

  Function:       lockFile

  Description:    The function is to lock a section of the file

  Input:          inFile: The Handle of the source file

                  start: the offset of the file

                  segmentSize: the length of the segment.

  Output:         *None*

  Return:         BOOL

  Others:         *None*

*************************************************/
int lockFile(HANDLE inFile, long start, long segmentSize)
{
     OVERLAPPED overlapstruc = {0};
     LPVOID    lpMsgbuff = NULL;
     BOOL bErrorFlag = FALSE;

     overlapstruc.Offset=start;
     overlapstruc.OffsetHigh = 0;
     bErrorFlag = LockFileEx(inFile, LOCKFILE_EXCLUSIVE_LOCK|LOCKFILE_FAIL_IMMEDIATELY, 0, segmentSize, 0, &overlapstruc);
     if(FALSE == bErrorFlag){
        //if  ERROR_IO_PENDING or ERROR_ACCESS_DENIED or ERROR_LOCK_VIOLATION means the section is locked by other process, so skip it.
        if(GetLastError() ==  ERROR_IO_PENDING ){
            return SMB3_STATUS_SKIP;
        }
        if(GetLastError() ==  ERROR_ACCESS_DENIED ){
            return SMB3_STATUS_SKIP;
        }
        if(GetLastError() ==  ERROR_LOCK_VIOLATION ){
            return SMB3_STATUS_SKIP;
        }

        FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgbuff , 0, NULL);
        cout<<"Error "<<__FILE__<<","<<__LINE__<<": failed  to lock file with error code: "<<GetLastError()<<(LPSTR)lpMsgbuff<<endl;
        return SMB3_STATUS_FAILURE;
     }
     return SMB3_STATUS_SUCCESS;
}

/*************************************************

  Function:       unlockFile

  Description:    The function is to unlock a section of the file

  Input:          inFile: The Handle of the source file

                  start: the offset of the file

                  segmentSize: the length of the segment.

  Output:         *None*

  Return:         int

  Others:         *None*

*************************************************/
int unlockFile(HANDLE inFile, long start, long segmentSize)
{
     OVERLAPPED overlapstruc = {0};
     LPVOID    lpMsgbuff = NULL;
     BOOL bErrorFlag = FALSE;

     overlapstruc.Offset=start;
     overlapstruc.OffsetHigh = 0;
     bErrorFlag =  UnlockFileEx(inFile, 0, segmentSize, 0, &overlapstruc);
     if(FALSE == bErrorFlag){
        if(GetLastError() == ERROR_NOT_LOCKED){
            return SMB3_STATUS_SKIP;
        }
        FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgbuff , 0, NULL);
        cout<<"Error "<<__FILE__<<","<<__LINE__<<": failed  to unlock file with error code: "<<GetLastError()<<(LPSTR)lpMsgbuff<<endl;
        return SMB3_STATUS_FAILURE;
     }
     return SMB3_STATUS_SUCCESS;
}

/*************************************************

  Function:       verifyFile

  Description:    The function is to compare the contents of the source file and destinatioin file

  Input:          inFile: The Handle of the source file

                  outFile: The handle of the destination file

                  start: the offset of the file

                  segmentSize: the length of the segment.

  Output:         *None*

  Return:         BOOL

  Others:         *None*

*************************************************/
BOOL verifyFile(HANDLE inFile, HANDLE outFile, long start, long segmentSize)
{
     OVERLAPPED overlapstruc = {0};
     byte * Base1, * Base2;
     int i;

     Base1 = (byte *)malloc(segmentSize * sizeof(byte));
     overlapstruc.Offset=start;
     overlapstruc.OffsetHigh = 0;
     ReadFileEx(inFile,Base1,segmentSize,&overlapstruc,NULL);
     Base2 = (byte *)malloc(segmentSize * sizeof(byte));
     overlapstruc.Offset=start;
     overlapstruc.OffsetHigh = 0;
     ReadFileEx(inFile,Base2,segmentSize,&overlapstruc,NULL);
     for(i=0; i<segmentSize; i++){
         if(Base1[i] != Base2[i]){
             cout<<"Error "<<__FILE__<<","<<__LINE__<<": failed to verify "<<Base1<<"<-->"<<Base2<<endl;
             free(Base1);
             free(Base2);
             return FALSE;
         }
     }
     free(Base1);
     free(Base2);
     return TRUE;
}

/*************************************************

  Function:       calcMD5

  Description:    The function is to calculate the checksum of the file

  Input:          filename -the source file

  Output:         *None*

  Return:         LPSTR  -the value of the checksum for the file

  Others:         Note: the invoker outside need to free the memory of the returned value.

*************************************************/
LPSTR calcMD5(LPSTR filename)
{
    HANDLE sourceFile;
    BOOL fSuccess  = FALSE;
    DWORD dwRetVal = 0;
    UINT uRetVal   = 0;
    DWORD block_size = smb3_block_size;

    DWORD dwBytesRead    = 0;
    DWORD dwBytesWritten = 0;

    TCHAR szTempFileName[MAX_PATH];
    TCHAR lpTempPathBuffer[MAX_PATH];
    byte * chBuffer;
    MD5 md5 = MD5();
    LPCTSTR errMsg;
    char * md5value;


    chBuffer = (byte *)malloc(block_size * sizeof(byte));
    sourceFile = openFile(filename, GENERIC_READ, FILE_SHARE_READ );
    if(sourceFile == NULL)
       return NULL;
    do
    {
        if (ReadFile(sourceFile, chBuffer, block_size, &dwBytesRead, NULL))
        {
            //  Replaces lower case letters with upper case
            //  in place (using the same buffer). The return
            //  value is the number of replacements performed,
            //  which we aren't interested in for this demo.
            md5.update(chBuffer, dwBytesRead);

        }
        else
        {
            cout<<"ERROR:ReadFile failed"<<endl;
            return NULL;
        }
    //  Continues until the whole file is processed.
    } while (dwBytesRead == block_size);
     free(chBuffer);
     closeFile(sourceFile);
     md5.finalize();
     md5value = (char *)malloc(33* sizeof(char));
     memcpy(md5value, md5.hexdigest().data(), 33);
     return md5value;
}

/*************************************************

  Function:       calcSectionMD5

  Description:    The function is to calculate the checksum of one section in the file

  Input:          filename       -the source file
                  section_index  -the section index from 0 to max_section_no.
                  section_size   -the size of every section

  Output:         *None*

  Return:         LPSTR  -the value of the checksum for the file

  Others:         Note: the invoker outside need to free the memory of the returned value.

*************************************************/
LPSTR calcSectionMD5(LPSTR filename, int section_index, long section_size)
{
    OVERLAPPED overlapstruc = {0};
    LPVOID     lpMsgbuff = NULL;
    BOOL bErrorFlag = FALSE;
    int count,j;
    long block_size = smb3_block_size;
    long segSize;
    HANDLE sourceFile;
    DWORD dwBytesRead    = 0;
    DWORD fsSize;
    byte * chBuffer;
    MD5 md5 = MD5();
    char * md5value;
    long start;
    long sectionSize;

	segSize = block_size;
    fsSize =  GetFileSize(filename);;
    start = section_index * section_size;
    if((fsSize - start) >= section_size)
        sectionSize = section_size;
    else
        sectionSize = fsSize - start;
    count = ceil(sectionSize/(double)block_size);
    if(smb3_debug_level >= 2){
        cout<<"start position:"<<start<<endl;
        cout<<"count:"<<count<<endl;
    }
    chBuffer = (byte *)malloc(block_size * sizeof(byte));
    sourceFile = openFile(filename, GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE );
    if(sourceFile == NULL)
       return NULL;
    for(j=0; j<count; j++){
         if(j == count - 1){
            segSize = sectionSize - block_size * j;
            free(chBuffer);
            chBuffer = NULL;
            chBuffer = (byte *)calloc((segSize ) , sizeof(byte));
         }
         overlapstruc.Offset = start + block_size * j;
         overlapstruc.OffsetHigh = 0;;
         bErrorFlag = ReadFileEx(sourceFile,chBuffer,segSize,&overlapstruc,NULL);
         if (FALSE == bErrorFlag)
         {
            FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                          NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                          (LPTSTR) &lpMsgbuff , 0, NULL);
            cout<<"Error "<<__FILE__<<","<<__LINE__<<": failed  to read file with error code: "
                <<GetLastError()<<(LPSTR)lpMsgbuff<<endl;
            return FALSE;
         }
         if(smb3_debug_level >= 1){
            cout<<"sectionIndex="<<section_index<<",segSize="<<segSize<<",j="<<j<<":\n"<<endl;
         }
         if(smb3_debug_level >= 3){
            cout<<chBuffer<<endl;
         }

         md5.update(chBuffer, segSize);

     }
     free(chBuffer);
     closeFile(sourceFile);
     md5.finalize();
     md5value = (char *)malloc(33* sizeof(char));
     memcpy(md5value, md5.hexdigest().data(), 33);
     return md5value;

}

/*************************************************

  Function:       smb3_get_time

  Description:    The function is to fetch the time and store its value into timestr

  Input:          timestr -A string address to store the return value

  Output:         *None*

  Return:         LPSTR  -the value of time

  Others:         *None*

*************************************************/
 LPSTR smb3_get_time(LPSTR timestr)
 {
//	struct timespec tp;
    time_t rawtime;
    char * tp;
	struct tm *tmp;
    LPSTR p;
	time(&rawtime);
	tmp = localtime(&rawtime);
	if (!tmp) {
		cout<<"ERROR:"<<"localtime"<<endl;
		return timestr;
	}
    strcpy(timestr, asctime (tmp));
    p = strrchr(timestr, '\n');
    if(*p == '\n')
       *p = '\0';
    return timestr;
}

/*************************************************

  Function:       GetFileSize

  Description:    The function is to get the size of the file

  Input:          timestr -A string address to store the return value

  Output:         *None*

  Return:         LPSTR  -the value of time

  Others:         *None*

*************************************************/
off_t GetFileSize( LPSTR path )
{
    FILE *p_file = NULL;
    p_file = fopen(path,"rb");
    fseek(p_file,0,SEEK_END);
    off_t size = ftell(p_file);
    fclose(p_file);
    return size;

}
/*************************************************

  Function:       replaceLineInFile

  Description:    The function is to replace certain line where match the lineFilter

  Input:          filename -- the destination file
                  lineFilter --to match the line
                  newLine    --new value to the line

  Output:         *None*

  Return:         BOOL  - true if succeed.

  Others:         *None*

*************************************************/
BOOL replaceLineInFile(LPSTR filename, LPSTR lineFilter, LPSTR newLine)
{
    ifstream  in_file(filename);
    LPVOID lpMsgbuff = NULL;
    vector<LPSTR>  lines;
    LPSTR value;
    string line;
    bool isFound = false;
    bool isNewFile = false;
    int i=0;
    if(!in_file.is_open())
    {
        if(GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            isNewFile = true;
        }
        else
        {
            FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                          NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                          (LPTSTR) &lpMsgbuff , 0, NULL);
            cout<<"Error "<<__FILE__<<","<<__LINE__<<": failed  to open file("<<filename<<") with error code: "
                <<GetLastError()<<(LPSTR)lpMsgbuff<<endl;
            return FALSE;
        }
    }
    while(!isNewFile && !in_file.eof())
    {
        getline(in_file, line);
        if(line.size() == 0)
        {
            continue;
        }
        value = (LPSTR)malloc(line.size() * sizeof(LPSTR));
        if(line.find(lineFilter) != string::npos)
        {
            strcpy(value, newLine);
            isFound = true;
        }
        else
        {
            strcpy(value, line.data());
        }
        lines.push_back(value);
    }
    if(!isNewFile)
    {
        in_file.close();
    }
    if(!isFound)
    {
       value = (LPSTR)malloc(strlen(newLine) * sizeof(LPSTR));
       strcpy(value, newLine);
       lines.push_back(value);
    }
    ofstream  out_file(filename);
    for(int i=0;i <= lines.size()-1; i++)
    {
       out_file<<lines[i]<<endl;
       free(lines[i]);
    }
    out_file.close();
    return TRUE;
}
