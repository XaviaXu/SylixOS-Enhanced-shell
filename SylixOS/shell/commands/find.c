#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>



#define TRUE  1
#define FALSE 0

#define GREATER 1
#define LESS -1
#define EQUAL 0

char *nameRes = NULL;
int nameFlag = FALSE;
int inumRes = -1;
char *rootDir = NULL;
char *typeRes = NULL;
int maxDepth = -1;
long sizeRes = -1;
int sizeFlag = 0;
char *aTimeRes = NULL;
char *mTimeRes = NULL;
char *actionRes = NULL;

int resFlag = 0;


char currDir[] = ".";
char preDir[] = "..";
int mask = 0xfff;

static struct option options[] = {
    {"name",required_argument,NULL,'n'},
    {"iname",required_argument,NULL,'N'},
    {"perm",required_argument,NULL,'p'},
    {"depth",required_argument,NULL,'d'},
    {"type",required_argument,NULL,'t'},
    {"size",required_argument,NULL,'s'},
    {"amin",required_argument,NULL,'a'},
    {"mmin",required_argument,NULL,'m'},
    {"exec",required_argument,NULL,'e'},
    {NULL,0,NULL,0}
};

int transPermission(char * in){
    int perm = 0;
    int i;
    for (i = 0; i < 3; i++)
    {
        int tmp = in[i]-'0';
        perm += (tmp<<(3*(2-i)));
    }
    return perm;
}

long transFileSize(char *in){
    
    long totalSize = 0;
    char sizeType = in[strlen(in)-1];

    in[strlen(in)-1] = '\0';    
    if(in[0]=='+'){
        sizeFlag = GREATER;
        totalSize = atol(++in);
    }else if(in[0]=='-'){
        sizeFlag = LESS;
        totalSize = atol(++in);
    }else{
        sizeFlag = EQUAL;
        totalSize = atol(in);
    }

    switch (sizeType)
    {
    case 'b':
        totalSize*=512;
        break;
    case 'w':
        totalSize *= 2;
        break;
    case 'k':
        totalSize *= 1000;
        break;
    case 'M':
        totalSize *= 1000000;
        break;
    case 'G':
        totalSize *= 1000000000;
        break;
    default:
        break;
    }

    return totalSize;

}

int checkName(char *fileName){
    if(nameRes==NULL){return TRUE;}
    if(nameFlag){
        int res = strcasecmp(fileName,nameRes);
        return res==0?TRUE:FALSE;
    }else{
        return strcmp(fileName,nameRes)==0?TRUE:FALSE;
    }
}

int checkType(char fileType){
    if(typeRes==NULL){return TRUE;}
    if(strcmp(typeRes,"f")==0&&fileType==DT_REG){
        //f: regular file
        return TRUE;
    }else if(strcmp(typeRes,"l")==0&&fileType==DT_LNK){
        //l: symbolic link
        return TRUE;
    }else if(strcmp(typeRes,"d")==0&&fileType==DT_DIR){
        //d: directory
        return TRUE;
    }else if(strcmp(typeRes,"c")==0&&fileType==DT_CHR){
        //c: character device
        return TRUE;
    }else if(strcmp(typeRes,"b")==0&&fileType==DT_BLK){
        return TRUE;
    }else if(strcmp(typeRes,"s")==0&&fileType==DT_SOCK){
        return TRUE;
    }else if(strcmp(typeRes,"p")==0&&fileType==DT_FIFO){
        return TRUE;
    }
    return FALSE;
}

int checkSize(long fileSize){
    if(sizeRes==-1){return TRUE;}
    if(sizeFlag == EQUAL &&fileSize==sizeRes){
        return TRUE;
    }else if(sizeFlag==GREATER && fileSize > sizeRes){
        return TRUE;
    }else if(sizeFlag==LESS && fileSize < sizeRes){
        return TRUE;
    }
    return FALSE;
}

int checkTime(time_t fileTime, char *deltaTime){
    if(deltaTime==NULL){return TRUE;}
    char modifier;
    int mins;
    int delt;
    if(!isdigit(deltaTime[0])){
        modifier = deltaTime[0];
        mins = atoi(++deltaTime);
    }else{
        mins = atoi(deltaTime);
    }

    delt = (int)(time(0)-fileTime)/60;
    if(modifier=='+' && delt > mins){
        return TRUE;
    }else if(modifier=='-' && delt < mins){
        return TRUE;
    }else if(delt==mins){
        return TRUE;
    }

    return FALSE;

}

int checkStandard(char *filePath,char *fileName,char fileType){
    int flag = TRUE;
    struct stat fileStat;
    stat(filePath,&fileStat);

    
    flag &= checkName(fileName);

    if(inumRes!=-1){
        int status = fileStat.st_mode & mask;
        if(inumRes!=status){
            flag = FALSE;
        }
    }
    //check file type
    flag &= checkType(fileType);

    //check last access/modification time
    flag &= checkTime(fileStat.st_atime,aTimeRes);
    flag &= checkTime(fileStat.st_mtime, mTimeRes);

    //check file size
    flag &= checkSize(fileStat.st_size);
    return flag;

}

void actionOnFile(char *filePath,char **remaining){
    if(actionRes==NULL){
        printf("%s\n",filePath);
        return;
    }
    
    char rm[] = "rm";
    char rmdir[] = "rmdir";
    char mv[] = "mv";
    char cat[] = "cat";
    char fullCmd[1024];
    if(strcmp(actionRes,rm)==0){
        snprintf(fullCmd,sizeof(fullCmd),"%s %s %s",rm,"-f",filePath);
//        char *argv = "rm -f /root/testfind/b";
        myExec(0,fullCmd);

    }else if(strcmp(actionRes,rmdir)==0){
        snprintf(fullCmd,sizeof(fullCmd),"%s %s",rmdir,filePath);
        //        char *argv = "rm -f /root/testfind/b";
        myExec(0,fullCmd);
        
    }else if(strcmp(actionRes,cat)==0){
        snprintf(fullCmd,sizeof(fullCmd),"%s %s",cat,filePath);
        myExec(0,fullCmd);
    }else if (strcmp(actionRes,mv)==0){
        char *tmp = *remaining;
        snprintf(fullCmd,sizeof(fullCmd),"%s %s %s",mv,filePath,tmp);
        myExec(0,fullCmd);


    }

}

void find(char* path,int depth,char **argv){
    DIR *dir;
    struct dirent *file;
    char *fileName;
    char fileType;
    char fullPath[1024];

    if(dir = opendir(path)){
        while ((file = readdir(dir))!=NULL)
        {
            fileName = file->d_name;
            fileType = file->d_type;
            if(strcmp(path,"/")==0){
                snprintf(fullPath,sizeof(fullPath),"%s%s",path,fileName);
            }else{
                snprintf(fullPath,sizeof(fullPath),"%s/%s",path,fileName);
            }
            
            

            if(!(strcmp(currDir,fileName)==0||strcmp(preDir,fileName)==0)){
                //selection
                if(checkStandard(fullPath,fileName,fileType)){
                    // printf("%s\n",fullPath);
                    actionOnFile(fullPath,argv);
                }

                if(fileType == DT_DIR&&(maxDepth==-1||depth>0)){
                    find(fullPath,depth-1,argv);
                }
            }
        }
        closedir(dir);

    }else{
        printf("Could not open directory %s\n", rootDir);
    }
}

int exec_find (int argc, char ** argv){
    
    char c;
    int valid = 1; 

    if(argc > 1 && argv[1][0] !='-'){
        //dir given
        rootDir = argv[1];
        argv++;
        argc--;
    }else{
        rootDir = ".";
    }

    while ((c = getopt_long_only(argc, argv, "n:N:p:d:t:s:a:m:e:", options, NULL)) != -1) {

        /* code */
        switch (c)
        {
        case 'N':
            nameFlag = TRUE;
        case 'n':
            //search by name
            nameRes = optarg;
            resFlag++;
            break;
        case 'p':
            //search by rights
            inumRes = transPermission(optarg);
            resFlag++;
            break;
        case 'd':
            maxDepth = atoi(optarg)-1;
            break;
        case 't':
            typeRes = optarg;
            resFlag++;
            break;
        case 's':
            sizeRes = transFileSize(optarg);
            resFlag++;
            break;
        case 'a':
            aTimeRes = optarg;
            resFlag++;
            break;
        case 'm':
            mTimeRes = optarg;
            resFlag++;
            break;
        case 'e':
            actionRes = optarg;
            resFlag++;
            break;
        default:
            valid = 0;
            break;
        }

    }

    argv += optind;
    argc -= optind;

    //output initial location?
    if(resFlag==0){
        printf("%s\n",rootDir);
    }
    //find
    if(valid){
        find(rootDir,maxDepth,argv);
    }

}
