/************************************************************

  FileName: smb3_range_lock.cpp

  Description:    The module is to test lock file , write section , then unlock in certain duration.

  Version:        1.0

  Function List:


  History:
         bobyan      create the first build.

***********************************************************/
#include "utils.h"



/*************************************************

  Function:       smb3_write_range_test

  Description:    The function is to test the lock,write,unlock in certain duration

  Input:          *None*

  Output:         *None*

  Return:         BOOL  the status returned by the operation.

  Others:         If the section is locked by other process , the process shall
                  wait for its unlock then lock it and continue to write.

*************************************************/
BOOL smb3_write_range_test()
{
    int section_no = 0;
    LPSTR source_filepath, destination_filepath;
    HANDLE sourceFile, destFile;
    time_t rawtime;
    long start, section_size;
    int section_num;
    int running_duration;
    int count = 1;
    int smb3_status;
    off_t fsSize;

    source_filepath = smb3_source_filepath;
    destination_filepath = smb3_destination_filepath;
    running_duration = smb3_duration;
 //   section_num = smb3_section_num;
    section_size = smb3_section_size;
    fsSize = GetFileSize(source_filepath);
    section_num = ceil(fsSize/(float)section_size);
    sourceFile = openFile(source_filepath, GENERIC_READ, FILE_SHARE_READ );
    if(smb3_debug_level >= 1){
        cout<<"the file size of "<<source_filepath<<" is"<<fsSize;
    }
    if(sourceFile == NULL){
         printf("ERROR(%s, %d): failed to open source file :%s",
               __FILE__, __LINE__, source_filepath);
         return FALSE;
    }
    destFile = openFile(destination_filepath, GENERIC_WRITE,
                        FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE // open with exclusive access
                      );
    if(destFile == NULL){
         printf("ERROR(%s, %d): failed to open destination file:%s",
               __FILE__, __LINE__, destination_filepath);
         return FALSE;
    }
    rawtime = time(NULL);
    do{
        section_no = rand()%section_num;
        start = section_no * smb3_section_size;
        if((fsSize - start) > smb3_section_size)
            section_size = smb3_section_size;
        else
            section_size = fsSize - start;
        if(smb3_debug_level >= 1)
            cout<<"lock "<<destination_filepath<<":"<<start <<","<<section_size<<endl;
        smb3_status = lockFile(destFile, start, section_size);
        if(smb3_status == SMB3_STATUS_FAILURE){
            printf("ERROR(%s, %d): failed to lock %s",
                __FILE__, __LINE__, destination_filepath);
            return FALSE;
        }
        else if (smb3_status == SMB3_STATUS_SKIP){
            Sleep(1000);
            continue;
        }
        if(smb3_debug_level >= 1)
            cout<<"copy from "<<source_filepath<<" to "<<destination_filepath<<endl;
        if(!copyFile(sourceFile, destFile,  start , section_size)){
            printf("ERROR(%s, %d): failed to copy %s to %s",
                __FILE__, __LINE__, source_filepath, destination_filepath);
            return FALSE;
        }
        if(smb3_debug_level >= 1)
            cout<<"unlock "<<destination_filepath<<":"<<start<<","<<section_size<<endl;
        smb3_status = unlockFile(destFile, start, section_size);
        if(smb3_status == SMB3_STATUS_FAILURE ){
            printf("ERROR(%s, %d): failed to unlock %s",
                __FILE__, __LINE__, destination_filepath);
            return FALSE;
        }
        else if (smb3_status == SMB3_STATUS_SKIP){
            Sleep(1000);
            continue;
        }
        if(smb3_debug_level >= 1)
            cout<<"begin waiting..."<<endl;
        //sleep 10000 milliseconds
        Sleep(10000);
        if(smb3_debug_level >= 1)
            cout<<"end waiting..."<<count++<<endl;
    }while(time(NULL) <= (rawtime + running_duration ));
    if(smb3_debug_level >= 1)
        cout<<"this is smb3_write_range_test:"<<rawtime<<endl;
    if(!closeFile(sourceFile)){
        printf("ERROR(%s, %d): failed to close %s",
               __FILE__, __LINE__, destination_filepath);
        return FALSE;
    }
    if(!closeFile(destFile)){
        printf("ERROR(%s, %d): failed to close %s",
               __FILE__, __LINE__, destination_filepath);
        return FALSE;
    }
    return TRUE;
}
