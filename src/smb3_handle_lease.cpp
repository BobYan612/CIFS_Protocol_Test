/************************************************************

  FileName: smb3_handle_lease

  Description:    The module is to test directory handle lease
                  using multiple processes on different Windows 8 clients
                  to list and update files under the common directory

  Version:        1.0

  Function List:


  History:
         bobyan      create the first build.

***********************************************************/
#include "utils.h"
#include <vector>
#include <list>
#include <fstream>

BOOL storeFilenameIntoFile(LPSTR dir, HANDLE tmpFileHandle);
BOOL storeDirStructure(LPSTR dir);
BOOL updateCksumForTree(LPSTR subtree);
bool comparePath(LPSTR first, LPSTR second);
BOOL createFileInDir(LPSTR dir);
BOOL renameFileInDir(LPSTR dir);
BOOL handleSMB3Tree(char *lpPath, BOOL (*run) (char * mydir));
BOOL deleteFileInDir(LPSTR dir);
long getCountOfFolders(int level, int width);
LPSTR getFolderByIndex(int index, int level, int width, LPSTR dirPrefix, char * path);
LPSTR getRandomFolder(int width, LPSTR dirPrefix, LPSTR curpath, LPSTR subpath);
BOOL handleRandomFolder(char *lpPath, BOOL (*run) (char * mydir));
BOOL storeDirStructure(LPSTR dir);
BOOL storeAllDirInfo(LPSTR workdir);
BOOL storeCksum(LPSTR workdir);
/*************************************************

  Function:       smb3_handle_release_test

  Description:    The function is to create , rename and delete a random file in a random folder
                  under the subtree(smb3_source_file). However it alwayse open the directory with
                  FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE during every operation.
                  (see also handleRandomFolder subroutine).

  Input:          *None*

  Output:         *None*

  Return:         BOOL  the status returned by the operation.

  Others:         The test shall create a temporary file to store the subtree hierachy and also
                  calculate the checksum into smb3_dbfile.

*************************************************/
BOOL smb3_handle_lease_test()
{
    int section_no = 0;
    LPSTR source_filepath, destination_filepath;
    LPSTR p;
    char sourceFilepath[MAX_PATH];
    HANDLE sourceFile, destFile;
    time_t rawtime;
    long start, section_size;
    int section_num;
    int duration;
    int count = 1;
    int smb3_status;
    char timestr[200];
    char subtree[MAX_PATH] = {0};
    char pre_subtree[MAX_PATH]={0};
    time_t startpoint;
    long totalTime;
    long averageTime;

    strcpy(sourceFilepath, smb3_workdir);
    p = &sourceFilepath[strlen(sourceFilepath) - 1];
    //remove the '\' at the tail of the path.
    while(*p == '\\')
        *p = '\0';
    duration = smb3_duration;
    rawtime = time(NULL);
    do
    {
        getRandomFolder(smb3_dir_width, smb3_dir_prefix, sourceFilepath, subtree);
        //To avoid choose the same subtree twice.
        while(strcmp(subtree, pre_subtree) == 0){
            getRandomFolder(smb3_dir_width, smb3_dir_prefix, sourceFilepath, subtree);
        }
        strcpy(pre_subtree, subtree);
        if(smb3_debug_level >= 1)
            cout<<smb3_get_time(timestr)<<": Creating files in random folders above the subtree "<<subtree<<endl;
        if(!handleRandomFolder(subtree, createFileInDir)){
            cout<<"Error "<<__FILE__<<","<<__LINE__<<": failed  to create file in "<<subtree<<endl;
            return FALSE;
        }
        startpoint = time(NULL);
        if(!updateCksumForTree(subtree)){
            cout<<"Error "<<__FILE__<<","<<__LINE__<<": failed  to calculate checksum in "<<subtree<<endl;
            return FALSE;
        }
        totalTime = (long)(time(NULL) - startpoint);
        if(smb3_debug_level >= 1)
            cout<<"Created files in random folders above the subtree "<<subtree<<endl;
        Sleep(1000);
        if(smb3_debug_level >= 1)
            cout<<smb3_get_time(timestr)<<": Renaming files in random folders above the subtree "<<subtree<<endl;
        if(!handleRandomFolder(subtree, renameFileInDir)){
            cout<<"Error "<<__FILE__<<","<<__LINE__<<": failed  to rename files in "<<subtree<<endl;
            return FALSE;
        }
        startpoint = time(NULL);
        if(!updateCksumForTree(subtree)){
            cout<<"Error "<<__FILE__<<","<<__LINE__<<": failed  to calculate checksum in "<<subtree<<endl;
            return FALSE;
        }
        totalTime += (long)(time(NULL) - startpoint);
        if(smb3_debug_level >= 1)
            cout<<smb3_get_time(timestr)<<": Renamed files in random folders above the subtree "<<subtree<<endl;
        Sleep(1000);
        if(smb3_debug_level >= 1)
            cout<<smb3_get_time(timestr)<<": Deleting files in random folders above the subtree "<<subtree<<endl;
        if(!handleRandomFolder(subtree, deleteFileInDir)){
            cout<<"Error "<<__FILE__<<","<<__LINE__<<": failed  to delete files in "<<subtree<<endl;
            return FALSE;
        }
        startpoint = time(NULL);
        if(!updateCksumForTree(subtree)){
            cout<<"Error "<<__FILE__<<","<<__LINE__<<": failed  to calculate checksum in "<<subtree<<endl;
            return FALSE;
        }
        totalTime += (long)(time(NULL) - startpoint);
        if(smb3_debug_level >= 1)
            cout<<smb3_get_time(timestr)<<": Deleted files in random folders above the subtree "<<subtree<<endl;
        averageTime = (long)(totalTime/3);
        if((time(NULL) + averageTime * 2) < (rawtime + duration )){
           Sleep((long)(averageTime) * 1000);
        }
        else{
           break;
        }

    }while(time(NULL) <= (rawtime + duration ));
    if(smb3_debug_level >= 1)
        cout<<smb3_get_time(timestr)<<":this is smb3_handle_release_test:"<<rawtime<<endl;

    return TRUE;
}

/*************************************************

  Function:       createFileInDir

  Description:    The function is to create a file in the refered folder

  Input:          dir  -the random folder path

  Output:         *None*

  Return:         BOOL  the status returned by the operation.

  Others:         *None*

*************************************************/
BOOL createFileInDir(LPSTR dir)
{
     HANDLE dirHandle, hFind, wHandle;
     WIN32_FIND_DATA lpFindFileData;
     char filename[MAX_PATH]={0};
     char subdir[MAX_PATH]={0};
     LPVOID     lpMsgbuff = NULL;
     long block_size = smb3_block_size;
     long file_size = smb3_file_size;
     LPSTR filePrefix = smb3_file_prefix;
     LPSTR sep = smb3_separator;
     int i,j;
     time_t rawtime;

     rawtime = time(NULL);
     sprintf(filename, "%s%s%s_%d", dir, sep, filePrefix, rawtime);
     wHandle = openFile(filename, GENERIC_WRITE,
                        FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE // open with exclusive access
                      );
     if(wHandle == NULL){
         printf("ERROR(%s, %d): failed to open %s",
                   __FILE__, __LINE__, filename);
         return FALSE;
     }
     if(!writeRandomFile( wHandle, block_size, file_size))
     {
         printf("ERROR(%s, %d): failed to write %s",
                   __FILE__, __LINE__, filename);
         return FALSE;
     }
     if(!closeFile(wHandle)){
         printf("ERROR(%s, %d): failed to close %s",
                   __FILE__, __LINE__, filename);
         return FALSE;
     }
     return TRUE;
}

/*************************************************

  Function:       renameFileInDir

  Description:    The function is to rename a random file in the refered folder

  Input:          dir  -the random folder path

  Output:         *None*

  Return:         BOOL  the status returned by the operation.

  Others:         *None*

*************************************************/
BOOL renameFileInDir(LPSTR dir)
{
     HANDLE dirHandle, nameFind, wHandle;
     WIN32_FIND_DATA MyFindFileData;
     LPSTR filename;
     char newFilename[MAX_PATH] = {0};
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
    nameFind=::FindFirstFile(szDir,&MyFindFileData);
    if(INVALID_HANDLE_VALUE == nameFind)
        return TRUE;
    while(TRUE)
    {
        if(!(MyFindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
             filename = (LPSTR)malloc((strlen(MyFindFileData.cFileName)*sizeof(LPSTR)));
             strcpy(filename, MyFindFileData.cFileName);
             files.push_back(filename);

        }
        if(!FindNextFile(nameFind,&MyFindFileData)){
            break;
        }
    }
    FindClose(nameFind);
    count = files.size();
    if(count == 0){
       return TRUE;
    }
    index = rand()%count;
    strcpy(newFilename, files[index]);
    if(strlen(newFilename) > 60){
         strncpy(newFilename, newFilename, 20);
    }
    strcpy(fileFullPath, dir);
    strcat(fileFullPath,"\\");
    strcat(fileFullPath, files[index]);
    strcat(newFilename, "_new");
    strcpy(newFullpath, dir);
    strcat(newFullpath,smb3_separator);
    strcat(newFullpath,newFilename);
    if(smb3_debug_level >= 1)
        cout<<"rename from "<<fileFullPath<<" to "<<newFullpath<<endl;
    if(!MoveFile(fileFullPath, newFullpath)){
        cout<<"Error "<<__FILE__<<","<<__LINE__<<": failed  to move file from "<<fileFullPath<<" to "<<newFullpath<<endl;
        return FALSE;
    }
    for (i=0; i<count; i++) {
       free(files[i]);
    };
     return TRUE;
}


/*************************************************

  Function:       deleteFileInDir

  Description:    The function is to delete a random file in the refered folder

  Input:          dir  -the random folder path

  Output:         *None*

  Return:         BOOL  the status returned by the operation.

  Others:         *None*

*************************************************/
BOOL deleteFileInDir(LPSTR dir)
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
    nameFind=::FindFirstFile(szDir,&MyFindFileData);
    if(INVALID_HANDLE_VALUE == nameFind)
        return TRUE;
    while(TRUE)
    {
        if(!(MyFindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
             filename = (LPSTR)malloc((strlen(MyFindFileData.cFileName)*sizeof(LPSTR)));
             strcpy(filename, MyFindFileData.cFileName);
             files.push_back(filename);

        }
        if(!FindNextFile(nameFind,&MyFindFileData)){
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
    strcat(fileFullPath,smb3_separator);
    strcat(fileFullPath, files[index]);
    if(smb3_debug_level >= 1)
        cout<<"delete "<<fileFullPath<<endl;
    if(!DeleteFile(fileFullPath)){
        return FALSE;
    }
    for (i=0; i<count; i++) {
       free(files[i]);
    };
     return TRUE;
}

/*************************************************

  Function:       updateCksumForTree

  Description:    The function is to update the checksum of one subtree hierachy infomation

  Input:          subtree  -the path of the subtree

  Output:         *None*

  Return:         BOOL  the status returned by the operation.

  Others:         *None*

*************************************************/
BOOL updateCksumForTree(LPSTR subtree)
{
    char parentPath[MAX_PATH];
    char fileOfSubtreeInfo[MAX_PATH];
    char keyvalue[MAX_PATH] = {0};
    char newValue[MAX_PATH] = {0};
    char dbFilepath[MAX_PATH];
    LPSTR cksumValue;
    LPSTR p;
    int pos;

    if(!storeDirStructure(subtree))
        return FALSE;
    strcpy(fileOfSubtreeInfo, subtree);
    strcat(fileOfSubtreeInfo, ".txt");
    cksumValue = calcMD5(fileOfSubtreeInfo);

    strcpy(keyvalue, subtree);
    pos = strcspn(keyvalue, ":");
    p = &keyvalue[pos + 1];
    strcpy(keyvalue, p);
    strcat(keyvalue, ": ");
    strcpy(newValue, keyvalue);
    strcat(newValue, cksumValue);
    strcpy(parentPath, subtree);
    p = strrchr(parentPath, '\\');
    p[0] = '\0';
    strcpy(dbFilepath, parentPath);
    strcat(dbFilepath, smb3_separator);
    strcat(dbFilepath, smb3_dbfile);
    free(cksumValue);
    if(smb3_debug_level >=1 )
    {
        cout<<"update "<<dbFilepath<<":"<<keyvalue<<" to "<<newValue<<endl;
    }
    if(!replaceLineInFile(dbFilepath, keyvalue, newValue))
    {
        return FALSE;
    }
    return TRUE;
}

/*************************************************

  Function:       storeCksum

  Description:    The function is to calculate the checksum of every subtree according to
                  workdir/subtree.txt and store it into workdir/smb3_dbfile

  Input:          workdir -the workdir of the dirctory , all subtrees are under it.

  Output:         *None*

  Return:         BOOL  the status returned by the operation.

  Others:         *None*

*************************************************/
BOOL storeCksum(LPSTR workdir)
{
    HANDLE hFind, dbHandle;
    char dbFilepath[MAX_PATH] = {0};
    char parentPath[MAX_PATH] = {0};
    char buffer[MAX_PATH];
    LPSTR cksumValue;
    char szDir[MAX_PATH];

    LPSTR p;
    char lineValue[MAX_PATH];
    bool hasUpdated = false;
    int i, pos;
    WIN32_FIND_DATA FindFileData;
    LPVOID     lpMsgbuff = NULL;
    DWORD dwBytesToWrite = 0;
    DWORD dwBytesWritten = 0;
    BOOL bErrorFlag = FALSE;

    strcpy(dbFilepath, workdir);
    strcat(dbFilepath, "\\");
    strcat(dbFilepath, smb3_dbfile);
    dbHandle = openFile(dbFilepath, GENERIC_WRITE,
                        FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE
                      );
    if(dbHandle == NULL){
         printf("ERROR(%s, %d): failed to open %s",
               __FILE__, __LINE__, dbFilepath);
         return FALSE;
    }

    strcpy(szDir, workdir);
    strcat(szDir,"\\*");

    //find all subtree name and find related temp file which storing the
    //hierachy infomation
    hFind=FindFirstFileEx(szDir, FindExInfoStandard, &FindFileData,
             FindExSearchLimitToDirectories, NULL, 0);


    if(INVALID_HANDLE_VALUE == hFind)
        return TRUE;
    while(TRUE)
    {
        if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if(FindFileData.cFileName[0]!='.')
            {
                strcpy(szDir,workdir);
                strcat(szDir,"\\");
                strcat(szDir,FindFileData.cFileName);
                //if the folder includes smb3_dir_prefix, that means
                //it's the subtree root. then find its related temp file
                //(subtree_name.txt).
                if(strstr(FindFileData.cFileName, smb3_dir_prefix) != 0)
                {
                    strcpy(lineValue, szDir);
                    strcat(szDir, ".txt");
                    cksumValue = calcMD5(szDir);
                    if(cksumValue != NULL)
                    {
                        //remove the drive letter
                        pos = strcspn(lineValue, ":");
                        p = &lineValue[pos + 1];
                        strcpy(lineValue, p);
                        strcat(lineValue, ": ");
                        strcat(lineValue, cksumValue);
                        strcat(lineValue, "\r\n");
                        free(cksumValue);
                        dwBytesToWrite = strlen(lineValue) * sizeof(char);
                        bErrorFlag = WriteFile(
                                            dbHandle,           // open file handle
                                            lineValue,      // start of data to write
                                            dwBytesToWrite,  // number of bytes to write
                                            &dwBytesWritten, // number of bytes that were written
                                            NULL);            // no overlapped structure
                        if (FALSE == bErrorFlag)
                        {
                            FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgbuff , 0, NULL);
                            cout<<"Error "<<__FILE__<<","<<__LINE__<<": failed  to write "<<lineValue<<" with error code: "<<GetLastError()<<(LPSTR)lpMsgbuff<<endl;
                            return FALSE;
                        }
                        else
                        {
                            if (dwBytesWritten != dwBytesToWrite)
                            {
                                printf("Error: dwBytesWritten != dwBytesToWrite when writing %s\n", lineValue);
                                return FALSE;
                            }
                        }
                    }
                };
            }
        }
        if(!FindNextFile(hFind,&FindFileData))
            break;
    }
    FindClose(hFind);
    if(!closeFile(dbHandle)){
        printf("ERROR(%s, %d): failed to close %s\n",
               __FILE__, __LINE__, dbFilepath);
        return FALSE;
    }
    return TRUE;
}

/*************************************************

  Function:       storeDirStructure

  Description:    The function is to traverse a subtree and store its hierachy into a temp file

  Input:          dir -the workdir of the dirctory , all subtrees are under it.

  Output:         *None*

  Return:         BOOL  the status returned by the operation.

  Others:         It need to remove the old temp file and then create the temp file handle and
                  pass it to storeFilenameIntoFile.

*************************************************/
BOOL storeDirStructure(LPSTR dir)
{
    char szFile[MAX_PATH];
    HANDLE tmpFileHandle;
    char * p;
    char parentDir[MAX_PATH] = {0};
    char tmpFilename[MAX_PATH] = {0};
    int pos;

    strcpy(tmpFilename, dir);
    strcat(tmpFilename, ".txt");
    DeleteFile(tmpFilename);
    tmpFileHandle = openFile(tmpFilename, GENERIC_WRITE,
                        FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE
                      );
    if(tmpFileHandle == NULL){
         printf("ERROR(%s, %d): failed to open %s",
               __FILE__, __LINE__, tmpFilename);
         return FALSE;
    }
    if(!storeFilenameIntoFile(dir, tmpFileHandle))
    {
        printf("ERROR(%s, %d): failed to store the information of %s into %s",
               __FILE__, __LINE__, dir, tmpFilename);
         return FALSE;
    }
    if(smb3_debug_level > 1)
        cout<<" Finish write dir into "<<tmpFilename<<endl;
    if(!closeFile(tmpFileHandle)){
        printf("ERROR(%s, %d): failed to close %s\n",
               __FILE__, __LINE__, tmpFilename);
        return FALSE;
    }
    return TRUE;

}

/*************************************************

  Function:       storeFilenameIntoFile

  Description:    The function is to all filenames under the lpPath into temp file
                  This is a recurse subroutine , so the tmpFile just a HANDLE to avoid
                  too many open and close operation here.

  Input:          lpPath   the workdir of the subtree

                  tmpFileHandle  the HANDLE of the temp file to store the structure information

  Output:         *None*

  Return:         BOOL  the status returned by the operation.

  Others:         *None*

*************************************************/
BOOL storeFilenameIntoFile(LPSTR lpPath, HANDLE tmpFileHandle)
{
    char szFind[MAX_PATH] = {0};
    char szFile[MAX_PATH] = {0};
    char szDir[MAX_PATH] = {0};
    LPSTR p;
    HANDLE dirHandle;
    HANDLE hFind;
    LPSTR filepath;
    list<LPSTR> fileList;
    list<LPSTR>::iterator it;
    WIN32_FIND_DATA FindFileData;
    LPVOID     lpMsgbuff = NULL;
    DWORD dwBytesToWrite = 0;
    DWORD dwBytesWritten = 0;
    BOOL bErrorFlag = FALSE;
    int pos;

    strcpy(szDir, lpPath);
    strcat(szDir,"\\*");
    hFind=FindFirstFileEx(szDir, FindExInfoStandard, &FindFileData,
             FindExSearchLimitToDirectories, NULL, 0);

    if(INVALID_HANDLE_VALUE == hFind)
        return TRUE;
    //read all file paths into a list, so we may close the Find process before
    //searching child paths
    while(TRUE)
    {
        if(FindFileData.cFileName[0] !='.')
        {
            strcpy(szFile,lpPath);
            strcat(szFile,"\\");
            strcat(szFile,FindFileData.cFileName);
            filepath = (LPSTR)malloc((strlen(szFile) + 1) * sizeof(char));
            strcpy(filepath, szFile);
            fileList.push_back(filepath);
        }
        if(!FindNextFile(hFind,&FindFileData))
            break;
    }
    FindClose(hFind);
    fileList.sort(comparePath);
    //store all paths into tempFileHandle
    for (it=fileList.begin(); it!=fileList.end(); ++it)
    {
        strcpy(szFile, * it);
        pos = strcspn(szFile, ":");
        p = &szFile[pos + 1];
        strcpy(szFile, p);
        strcat(szFile, "\r\n");
        if(smb3_debug_level > 1)
            cout<<"writing "<<szFile<<endl;
        dwBytesToWrite = strlen(szFile) * sizeof(char);
        bErrorFlag = WriteFile(
                            tmpFileHandle,           // open file handle
                            szFile,      // start of data to write
                            dwBytesToWrite,  // number of bytes to write
                            &dwBytesWritten, // number of bytes that were written
                            NULL);            // no overlapped structure

        if (FALSE == bErrorFlag)
        {
            FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgbuff , 0, NULL);
            cout<<"Error "<<__FILE__<<","<<__LINE__<<": failed  to write "<<szFile<<" with error code: "<<GetLastError()<<(LPSTR)lpMsgbuff<<endl;
            return FALSE;
        }
        else
        {
            if (dwBytesWritten != dwBytesToWrite)
            {
                printf("Error: dwBytesWritten != dwBytesToWrite when writing %s\n", szFile);
                return FALSE;
            }
        }
    }
    //do above steps in child direcotries
    for (it=fileList.begin(); it!=fileList.end(); ++it)
    {
        filepath = * it;
        if(strstr(filepath, smb3_dir_prefix) >= 0)
        {
            if(!storeFilenameIntoFile(filepath, tmpFileHandle))
            {
                return FALSE;
            };
        }
        free(filepath);
    }

    return TRUE;
}


/*************************************************

  Function:       handleRandomFolder

  Description:    The function is a control process , it firstly open the directory with
                  FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE. Then select a random
                  child or grandchild to transfer BOOL (*run) (create, rename or delete)

  Input:          lpPath   the workdir of the subtree

                  run      the subroutine to handle the randome folder.

  Output:         *None*

  Return:         BOOL  the status returned by the operation.

  Others:         *None*

*************************************************/
BOOL handleRandomFolder(char *subtree, BOOL (*run) (char * mydir))
{
    char szDir[MAX_PATH] = {0};
    float percent = smb3_percent;
    HANDLE dirHandle, subdirHandle;
    char tmpDir[MAX_PATH] = {0};
    LPSTR dirPrefix;
    int level, width, filesPerFolder;
    int totalFiles;
    int i, count, totalFolders;

    level = smb3_dir_level;
    width = smb3_dir_width;
    dirPrefix = smb3_dir_prefix;
    filesPerFolder = smb3_files_per_folder;
    totalFolders = getCountOfFolders(level, width);
    count = ceil(percent * totalFolders * filesPerFolder);

    dirHandle =  CreateFile(subtree,
                            GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE ,
                            0,
                            OPEN_EXISTING,
                            FILE_FLAG_BACKUP_SEMANTICS,
                            0);
    if(dirHandle == INVALID_HANDLE_VALUE)
    {
         printf("ERROR(%s, %d): failed to open %s\n",
            __FILE__, __LINE__, subtree);
         return FALSE;
    }
    else
    {
        for(i = 0; i < count ; i ++ )
        {
            strcpy(szDir, subtree);
            getFolderByIndex(int(rand()%totalFolders), level, width, dirPrefix, tmpDir);
            if(strlen(tmpDir) > 0)
            {
               strcat(szDir, "\\");
               strcat(szDir, tmpDir);
            }
            if(smb3_debug_level >= 1){
               cout<<"Find the path is :"<<szDir<<endl;
            }
            subdirHandle = CreateFile(szDir,
                            GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE ,
                            0,
                            OPEN_EXISTING,
                            FILE_FLAG_BACKUP_SEMANTICS,
                            0);
            if(subdirHandle == INVALID_HANDLE_VALUE)
            {
                 printf("ERROR(%s, %d): failed to open %s\n",
                    __FILE__, __LINE__, szDir);
                 return FALSE;
            }
            if(!run(szDir))
            {
                closeFile(dirHandle);
                closeFile(subdirHandle);
                return FALSE;
            }
            if(!closeFile(subdirHandle)){
                printf("ERROR(%s, %d): failed to close %s\n",
                       __FILE__, __LINE__, szDir);
                return FALSE;
            }
        }
    }

    if(!closeFile(dirHandle)){
        printf("ERROR(%s, %d): failed to close %s\n",
                       __FILE__, __LINE__, subtree);
        return FALSE;
    }
    return TRUE;
}

/*************************************************

  Function:       getRandomFolder

  Description:    The function is to get a random folder under path.

  Input:          width  the width of the subtree

                  dirPrefix  the prefix for the directory

                  curpath   current workdir.

                  subpath   the subtree created by the subroutine.

  Output:         *None*

  Return:         LPSTR  the subtree of new one.

  Others:         *None*

*************************************************/
LPSTR getRandomFolder(int width, LPSTR dirPrefix, LPSTR curpath, LPSTR subpath)
{
    int randWidth;
    char indexStr[10];
    randWidth = int(rand()%width);
    itoa(randWidth, indexStr, 10);
    strcpy(subpath, curpath);
    strcat(subpath,"\\");
    strcat(subpath, dirPrefix);
    strcat(subpath, indexStr);
    return subpath;
}

/*************************************************

  Function:       getFolderByIndex

  Description:    The function is to create a path according to index
                  Here index is a random value from 0 through total sub-folders.

  Input:          index  a random data

                  level  the level of the subtree

                  width  the width of the subtree

                  dirPrefix  the prefix for the directory

                  path   the path the subtree.

  Output:         *None*

  Return:         BOOL  the status returned by the operation.

  Others:         *None*

*************************************************/
LPSTR getFolderByIndex(int index, int level, int width, LPSTR dirPrefix, char * path)
{
    vector<LPSTR> paths;
    vector<int> indexOfPath;
    int totalFiles;
    int dirLevel = 0;
    int value;
    int tmpValue = index;
    string tmpPath;
    char intValue[10];
    int i;
    if(index == 0){
        return "";
    }
    totalFiles = getCountOfFolders(level, width);
    if(index > totalFiles){
        index = index%totalFiles;
    }
    while(tmpValue > 0) {
       tmpValue  -= getCountOfFolders(++dirLevel, width);
    }
    tmpValue = index - getCountOfFolders(dirLevel - 1, width);
    do{
        value = tmpValue%width;
        itoa(value, intValue, 10);
        if(tmpPath.length() > 0)
            tmpPath = string(dirPrefix) + string(intValue) + string("\\") +  tmpPath;
        else
            tmpPath = string(dirPrefix) + string(intValue);
        tmpValue = int(tmpValue/width);
        dirLevel --;
    }while(dirLevel > 1);
    strcpy(path, tmpPath.data());
    if(smb3_debug_level >= 1){
       cout<<"get folder is :"<<path<<endl;
    }
    return path;
}

/*************************************************

  Function:       getCountOfFolders

  Description:    The function is to calculate the number of folder according to the
                  level and width.

  Input:          level  the level of the subtree

                  width  the width of the subtree


  Output:         *None*

  Return:         long  -the count of the folders(including the subtree folder itself);

  Others:         *None*

*************************************************/
long getCountOfFolders(int level, int width)
{
    int i;
    int numFolders = 1;
    for(i = 1; i < level; i++){
       numFolders += pow(width, i);
    }
    return numFolders;
}

/*************************************************

  Function:       comparePath

  Description:    The function is to compare two path so that list may sort the path
                  as a fix order

  Input:          *None*

  Output:         *None*

  Return:         bool  -the result of strcomp.

  Others:         *None*

*************************************************/
bool comparePath(LPSTR first, LPSTR second)
{
   return (strcmp(first, second) <= 0);
}

