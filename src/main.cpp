#include "utils.h"

void registerTestCases(){
    registerCase("smb3.lock_range", smb3_write_range_test);
    registerCase("smb3.handle_lease", smb3_handle_lease_test);
    registerCase("smb3.file_lease", smb3_file_lease_test);
}


int main(int argc,char *argv[])
{
    struct TestCase * mycase;
    BOOL (*fn) ();
    char path[MAX_PATH];
    ArgOption options[] = {
        {"workdir",'W', POPT_ARG_STRING, &smb3_workdir, "The workdir for the process"},
        {"src_file",'S', POPT_ARG_STRING, &smb3_source_filepath, "The full path of the source file"},
        {"dst_file", 'D', POPT_ARG_STRING, &smb3_destination_filepath, "The full path of the source file"},
        {"section_size", 's', POPT_ARG_LONG, &smb3_section_size, "The section size"},
        {"num_section", 'c', POPT_ARG_INT, &smb3_section_num, "The section count"},
        {"file_size", 'F', POPT_ARG_LONG, &smb3_file_size, "The file size in KB, the default is 10M"},
        {"num_file_per_folder", 'n', POPT_ARG_INT, &smb3_files_per_folder, "The number of files per folder , the default is 10"},
        {"block_size", 'B', POPT_ARG_LONG, &smb3_block_size, "The block size in byte, default is 8K"},
        {"duration", 'u', POPT_ARG_LONG, &smb3_duration, "The duration for running the case (seconds)"},
        {"debug", 'd', POPT_ARG_INT, &smb3_debug_level, "The debug level for the process"},
        {"width", 'w', POPT_ARG_INT, &smb3_dir_width, "The width of directory"},
        {"level", 'l', POPT_ARG_INT, &smb3_dir_level, "The level of directory"},
        {"file_prefix", 'p', POPT_ARG_STRING, &smb3_file_prefix, "The prefix of the file"},
        {"dir_prefix", 'P', POPT_ARG_STRING, &smb3_dir_prefix, "The prefix of the file"},
        {"operation", 'o', POPT_ARG_STRING, &gCaseName, "The case name for the test"},
        {"percent", 'e', POPT_ARG_FLOAT, &smb3_percent, "The percent defined outside"},
        {"update_file", 'e', POPT_ARG_BOOL, &smb3_update_file, "To update the file content or not"},
        {"help", 'P', POPT_ARG_BOOL, &smb3_help, "The prefix of the file"},
        //the last line is to set the NULL struct , so that the later functions may know
        //it's the end of the array.
        {NULL,'0',0,NULL,"" }
    };
    srand(time(NULL));
    registerTestCases();
    if(parseArg(argc, argv, options) == FALSE ){
       return -1;
    };
    if(smb3_help){
       help(options);
       return 0;
    }
    cout<<"debug_level="<<smb3_debug_level<<";case_name="<<gCaseName<<endl;

    if(getTestCase(gCaseName) == NULL){
        if(gCaseName == NULL)
            cout<<"casename has not defined"<<endl;
        else
            cout<<"can't fine the casename "<<gCaseName<<endl;
       help(options);
       return -1;
    }
    cout<<"start "<<gCaseName<<endl;
    mycase = getTestCase(gCaseName);
    fn = mycase->run;
    if(!fn()){
       cout<<"Error: Failed on "<<gCaseName<<endl;
       return -1;
    }
    cout<<"Success:"<<gCaseName<<" is finished"<<endl;


    return 0;
}


