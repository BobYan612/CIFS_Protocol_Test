## CIFS Protocol Test

**The Project is one example to verfiy CIFS(SMB3) Protocol**
The project is written by C++ to implement the file/folder lock/unlock with multiple processes.

**Features**  
There are the below considerations during implementation:
 - using multiple processes to read and update files across different clients
 - test directory handle lease using multiple processes on different Windows clients to list and update files under the common directory.
 - test lock file , write section , then unlock in certain duration


**Dependency**  
MD5 http://www.bzflag.org  



**Usagage**  
 --workdir|-w       :work dir for the tree                            
 --section_size|-s  :The size of every section                         
 --num_section|-c   :The number of section                             
 --file_size|-f     :the file size in every folder                       
 --num_file|-n      :the number of file in every folder                  
 --folder_prefix|-o :the prefix name for folder                        
 --file_prefix|-e   :the prefix name for the files                     
 --operation|-o     :the operation  (create | delete | update |verify)  
 --duration|-u      :the duration for update or verify                
 --help|-h          :Help                                             

> [!TIP]  
> Any suggestions or questions please contact biao.yan612@gmail.com
