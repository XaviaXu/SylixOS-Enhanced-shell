#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

#define MEET 1
#define MISS 0

#define GREATER 1
#define LESS -1
#define EQUAL 0

char *nameRes = NULL;
int inumRes = -1;
char *rootDir = NULL;
char *typeRes = NULL;
int maxDepth = -1;
long sizeRes = -1;
int sizeFlag = 0;

char currDir[] = ".";
char preDir[] = "..";
int mask = 0xfff;

static struct option options[] = {
    {"name",required_argument,NULL,'n'},
    {"perm",required_argument,NULL,'p'},
    {"depth",required_argument,NULL,'d'},
    {"type",required_argument,NULL,'t'},
    {"size",required_argument,NULL,'s'},
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

int checkType(char fileType){
    if(strcmp(typeRes,"f")==0&&fileType==DT_REG){
        //f: regular file
        return 1;
    }else if(strcmp(typeRes,"l")==0&&fileType==DT_LNK){
        //l: symbolic link
        return 1;
    }else if(strcmp(typeRes,"d")==0&&fileType==DT_DIR){
        //d: directory
        return 1;
    }else if(strcmp(typeRes,"c")==0&&fileType==DT_CHR){
        //c: character device
        return 1;
    }else if(strcmp(typeRes,"b")==0&&fileType==DT_BLK){
        return 1;
    }else if(strcmp(typeRes,"s")==0&&fileType==DT_SOCK){
        return 1;
    }else if(strcmp(typeRes,"p")==0&&fileType==DT_FIFO){
        return 1;
    }
    return 0;
}

int checkSize(long fileSize){
    if(sizeRes == -1){return 1;}
    if(sizeFlag == EQUAL &&fileSize==sizeRes){
        return 1;
    }else if(sizeFlag==GREATER && fileSize > sizeRes){
        return 1;
    }else if(sizeFlag==LESS && fileSize < sizeRes){
        return 1;
    }
    return 0;
}

int checkStandard(char *filePath,char *fileName,char fileType){
    int flag = MEET;
    struct stat fileStat;
    stat(filePath,&fileStat);

    if(nameRes!=NULL){
        //add: regex & * ?
        if(strcmp(fileName,nameRes)!=0){
            flag = MISS;
        }
    }
    if(inumRes!=-1){
        int status = fileStat.st_mode & mask;
        if(inumRes!=status){
            flag = MISS;
        }
    }
    if(typeRes!=NULL){
        if(checkType(fileType)==0){
            flag = MISS;
        }
    }
    flag &= checkSize(fileStat.st_size);

    return flag;

}

void find(char* path,int depth){
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
                    printf("%s\n",fullPath);
                }

                if(fileType == DT_DIR&&(maxDepth==-1||depth>0)){
                    find(fullPath,depth-1);
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

    while ((c = getopt_long_only(argc, argv, "n:p:d:", options, NULL)) != -1) {

        /* code */
        switch (c)
        {
        case 'n':
            //search by name
            nameRes = optarg;
            printf("%s\n",nameRes);
            break;
        case 'p':
            //search by rights
            inumRes = transPermission(optarg);
            break;
        case 'd':
            maxDepth = atoi(optarg)-1;
            break;
        case 't':
            typeRes = optarg;
            break;
        case 's':
            sizeRes = transFileSize(optarg);
            break;
        default:
            valid = 0;
            break;
        }

    }

    argv += optind;
    argc -= optind;

    //output initial location?
    if(nameRes==NULL&&typeRes==NULL){

    }
    //find
    if(valid){
        find(rootDir,maxDepth);
    }
    return 0;

}
