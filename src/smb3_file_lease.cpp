/************************************************************

  FileName: smb3_file_lease

  Description:    The module is to test using multiple processes
  to read and update files across different clients

  Version:        1.0

  Function List:


  History:
         bobyan    create the first build.

***********************************************************/
#include "utils.h"
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>

BOOL updateRandomBlockInSection(HANDLE srcHandle, int sectionIndex ,long sectionSize, float percent);
BOOL getUniqueName(LPSTR path, int sectionIndex, LPSTR str);
BOOL updateMD5InFile(LPSTR dbfile, LPSTR uniqueName, LPSTR newLine);
BOOL updateRandomFile(LPSTR dir);
BOOL updateRandomeBlockInFile(LPSTR srcFile);


/*************************************************

  Function:       smb3_file_lease_test

  Description:    The function is to update a random file every 5 seconds

  Input:          *None*

  Output:         *None*

  Return:         BOOL  the status returned by the operation.

  Others:         If the section is locked by other process , the process shall
                  wait for its unlock then lock it and continue to write.

*************************************************/
BOOL smb3_file_lease_test()
{
    time_t rawtime;
    int running_duration = smb3_duration;
    rawtime = time(NULL);

    do{
        if(!updateRandomFile(smb3_workdir)){
            cout<<"ERROR: failed in update largefile in "<<smb3_workdir<<endl;
            return FALSE;
        }
        Sleep(10000);
    }while(time(NULL) <= (rawtime + running_duration ));

    return TRUE;
}

/*************************************************

  Function:       updateMD5InFile

  Description:    The function is to update the dbfile with new checksum info.

  Input:          dbfile      - the file to store the checksum
                  uniqueName  - the name including file path and section index
                  newLine     - the new line including uniqueName and new checksum

  Output:         *None*

  Return:         BOOL  the status returned by the operation.

  Others:         *None*

*************************************************/
BOOL updateMD5InFile(LPSTR dbfile, LPSTR uniqueName, LPSTR newLine)
{
      string inbuf;
      char tmpFile[MAX_PATH] = {0};
      fstream input_file(dbfile, ios::in);
      strcpy(tmpFile, dbfile);
      strcat(tmpFile, ".tmp");
      bool hasFound = false;
      DeleteFile(tmpFile);
      ofstream output_file(tmpFile);
      LPVOID     lpMsgbuff = NULL;

      if(input_file.is_open()){
            while (!input_file.eof())
            {
                getline(input_file, inbuf);
                if(inbuf.size() <= 0)
                    continue;
                int spot = inbuf.find(uniqueName);
                if(spot >= 0)
                {
                    hasFound = true;
                    inbuf = string(newLine);
                }
                output_file << inbuf << endl;
            }
            input_file.close();
            if(!DeleteFile(dbfile)){
                 FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                                  NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                  (LPTSTR) &lpMsgbuff , 0, NULL);
                 cout<<"Error "<<__FILE__<<","<<__LINE__<<": failed  to delete file("<<dbfile<<") with error code: "
                        <<GetLastError()<<(LPSTR)lpMsgbuff<<endl;
                 return FALSE;
            }
      }
      if(!hasFound)
          output_file << newLine << endl;


      output_file.close();
      if(!MoveFile(tmpFile, dbfile)){
         FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                          NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                          (LPTSTR) &lpMsgbuff , 0, NULL);
         cout<<"Error "<<__FILE__<<","<<__LINE__<<": failed  to rename to file("<<dbfile<<") with error code: "
                <<GetLastError()<<(LPSTR)lpMsgbuff<<endl;
         return FALSE;
      }
      return TRUE;
}

/*************************************************

  Function:       updateRandomFile

  Description:    The function is to update a random file in the dir

  Input:          dir      - the work dir

  Output:         *None*

  Return:         BOOL  the status returned by the operation.

  Others:         *None*

*************************************************/
BOOL updateRandomFile(LPSTR dir)
{
     HANDLE dirHandle, nameFind, wHandle;
     WIN32_FIND_DATA MyFindFileData;
     LPSTR filename;
     char szDir[MAX_PATH] = {0};
     char fileFullPath[MAX_PATH] = {0};
     char newFullpath[MAX_PATH] = {0};
     char subdir[MAX_PATH]={0};
     LPVOID   lpMsgbuff = NULL;
     long block_size = smb3_block_size;
     long file_size = smb3_file_size;
     LPSTR filePrefix = smb3_file_prefix;
     LPSTR sep = smb3_separator;
     vector<LPSTR> files ;
     int i,j;
     int count, index;
     time_t rawtime;

    strcpy(szDir, dir);
    strcat(szDir, "\\*");
    nameFind=FindFirstFile(szDir,&MyFindFileData);
    if(INVALID_HANDLE_VALUE == nameFind)
        return TRUE;
    while(TRUE)
    {
        if(!(MyFindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
             filename = (LPSTR)malloc((strlen(MyFindFileData.cFileName)*sizeof(LPSTR)));
             strcpy(filename, MyFindFileData.cFileName);
              if(strstr(filename, smb3_file_prefix) != NULL){
                files.push_back(filename);
             }

        }
        if(!FindNextFile(nameFind,&MyFindFileData)){
            if(smb3_debug_level >= 1)
                cout<<"Find is finished."<<endl;
            break;
        }
    }
    FindClose(nameFind);
    count = files.size();
    if(count == 0){
       return TRUE;
    }
    index = rand()%count;
    strcpy(fileFullPath, dir);
    strcat(fileFullPath,"\\");
    strcat(fileFullPath, files[index]);
    if(smb3_debug_level >= 1)
        cout<<"update file "<<fileFullPath<<endl;
    if(!updateRandomeBlockInFile(fileFullPath)){
        return FALSE;
    }
    for (i=0; i<count; i++) {
       free(files[i]);
    };
     return TRUE;
}

/*************************************************

  Function:       updateRandomFile

  Description:    The function is to update 10% random blocks in a file
                  When it finish a section, it shall update the checksum for the section.

  Input:          srcFile  - the source file

  Output:         *None*

  Return:         BOOL  the status returned by the operation.

  Others:         *None*

*************************************************/
BOOL updateRandomeBlockInFile(LPSTR srcFile)
{
    int section_no = 0;
    HANDLE sourceFile, destFile;
    time_t rawtime;
    int section_num;
    int running_duration;
    int count = 1;
    int smb3_status;
    DWORD fsSize;
    int i;
    float percent = smb3_percent;
    char uniqueName[MAX_PATH] = {0};
    char newline[MAX_PATH] = {0};
    char dbFilepath[MAX_PATH] = {0};
    LPSTR md5Value;
    NTSTATUS rlt;

    strcpy(dbFilepath, smb3_workdir);
    strcat(dbFilepath, smb3_separator);
    strcat(dbFilepath, smb3_dbfile);
    running_duration = smb3_duration;
    sourceFile = openFile(srcFile, GENERIC_WRITE ,
                        FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE // open with exclusive access
                      );
    if(sourceFile == NULL){
         printf("ERROR(%s, %d): failed to open %s",
               __FILE__, __LINE__, srcFile);
         return FALSE;
    }
    fsSize =  GetFileSize(srcFile);
    section_num = ceil(fsSize/(float)smb3_section_size);
    if(smb3_debug_level >= 1){
       cout<<"smb3_section_size="<<smb3_section_size<<",fsSize="<<fsSize<<endl;
    }
    for(i = 0; i < section_num; i++){
        rlt = updateRandomBlockInSection(sourceFile , i, smb3_section_size, percent);
        if(rlt == SMB3_STATUS_FAILURE){
            cout<<"ERROR: failed to update "<<srcFile<<endl;
            return FALSE;
        }
        else if (rlt == SMB3_STATUS_SKIP){
                continue;
        }
        md5Value = calcSectionMD5(srcFile, i, smb3_section_size);
        if(smb3_debug_level >= 1){
           cout<<"md5("<<srcFile<<",section="<<i<<")="<<md5Value<<endl;
        }
        getUniqueName(srcFile, i, uniqueName);
        strcpy(newline, uniqueName);
        strcat(newline, md5Value);
        free(md5Value);
        if(smb3_debug_level >= 1)
           cout<<"store "<<newline<<" in "<<dbFilepath<<endl;
        replaceLineInFile(dbFilepath, uniqueName, newline);
    }
    if(!closeFile(sourceFile)){
        printf("ERROR(%s, %d): failed to close %s",
               __FILE__, __LINE__, srcFile);
        return FALSE;
    }
    return TRUE;
}

/*************************************************

  Function:       getUniqueName

  Description:    The function is to create a unique name with following format:
                  filepath : section_index : checksum

  Input:          path          - the file path
                  sectionIndex  - the section index
                  str           - the pointer to a string to store the new uniquename

  Output:         *None*

  Return:         BOOL  the status returned by the operation.

  Others:         The file path need to remove the drive , so that the process on
                  other hut which has different drive for the same share
                  also may find the correct path.

*************************************************/
BOOL getUniqueName(LPSTR path, int sectionIndex, LPSTR str)
{
    int post;
    char lineValue[MAX_PATH];
    char * p;
    int pos;

    strcpy(lineValue, path);
    pos = strcspn(lineValue, ":");
    p = &lineValue[pos + 1];
    strcpy(lineValue, p);
    sprintf(lineValue, "%s%s%d%s", lineValue, " : ", sectionIndex, " : ");
    strcpy(str, lineValue);
    return TRUE;
}

/*************************************************

  Function:       updateRandomBlockInSection

  Description:    The function is to modify a section in a random block

  Input:          srcHandle     - the file path
                  sectionIndex  - the section index
                  sectionSize   - the section size
                  percent       - the percent of the block to be updated

  Output:         *None*

  Return:         BOOL  the status returned by the operation.

  Others:         The file path need to remove the drive , so that the process on
                  other hut which has different drive for the same share
                  also may find the correct path.

*************************************************/
NTSTATUS updateRandomBlockInSection(HANDLE srcHandle, int sectionIndex ,long sectionSize, float percent)
{
     OVERLAPPED overlapstruc = {0};
     byte * Base;
     LPVOID  lpMsgbuff = NULL;
     BOOL bErrorFlag = FALSE;
     int count,j, i;
     long block_size = smb3_block_size;
     long segSize;
     long start;
     int randvalue;
     NTSTATUS rlt = SMB3_STATUS_SKIP;

    count = ceil(sectionSize/(double)block_size);
	segSize = block_size;
    Base = (byte *)calloc(segSize,sizeof(byte));
    start = sectionIndex * smb3_section_size;
    if(smb3_debug_level >= 2){
        cout<<"start position:"<<start<<endl;
        cout<<"count:"<<count<<endl;
    }
    for(j=0; j<count; j++){
        //define whether current location is chosed.
         randvalue = rand()%100;
         if(randvalue%100 > percent * 100){
             continue;
         }
         if(rlt == SMB3_STATUS_SKIP){
            rlt = SMB3_STATUS_SUCCESS;
         }
         if(j == count - 1){
            segSize = sectionSize - block_size * j;
            free(Base);
            Base = NULL;
            Base = (byte *)calloc((segSize ) , sizeof(byte));
         }
         for(i=0; i < segSize ; i++){
            Base[i] = rand()%256;
         }
         overlapstruc.Offset = start + block_size * j;
         overlapstruc.OffsetHigh = 0;
         bErrorFlag = WriteFileEx(srcHandle,Base,segSize,&overlapstruc, NULL);
         if (FALSE == bErrorFlag)
         {
            FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgbuff , 0, NULL);
            cout<<"Error "<<__FILE__<<","<<__LINE__<<": failed  to write file with error code: "<<GetLastError()<<(LPSTR)lpMsgbuff<<endl;
            return SMB3_STATUS_FAILURE;
         }
         if(((j+1) % 1000 == 0 || j==count - 1) && !FlushFileBuffers(srcHandle)){
            FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgbuff , 0, NULL);
            cout<<"Error "<<__FILE__<<","<<__LINE__<<": failed  to flush file with error code: "<<GetLastError()<<(LPSTR)lpMsgbuff<<endl;
            return SMB3_STATUS_FAILURE;
         }

     }
     free(Base);
     return rlt;
}
