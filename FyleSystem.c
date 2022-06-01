#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


int array[5];

//[0] - files 
//[1] - lines 
//[2] - words 
//[3] - letters 
//[4] - digits 

struct file {
    char fileName[256];
    char fileContent[10000];
    time_t fileTimeAccess;
    time_t fileTimeMod;
};


struct folder {
    char folderName[256];
    struct file files[1024];
    struct folder* dirs;//every instance should be inited 
    int currFileIndex;//by default -1 except mainfolder
    int currDirIndex;//by default -1 -||-
    time_t dirTimeAccess;
    time_t dirTimeMod;
}MainFolder;//also initFunction should be implemented 
//note that after every folder adding you should malloc 256 blocks of struct folder object.

 void reverse(char s[])
 {
     int i, j;
     char c;

     for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
} 

 void itoa(int n, char s[])
 {
     int i, sign;

     if ((sign = n) < 0)  /* record sign */
         n = -n;          /* make n positive */
     i = 0;
     do {       /* generate digits in reverse order */
         s[i++] = n % 10 + '0';   /* get next digit */
     } while ((n /= 10) > 0);     /* delete it */
     if (sign < 0)
         s[i++] = '-';
     s[i] = '\0';
     reverse(s);
}  



char* extractingName(const char* path)//tested
{

    char* a = (char*)malloc(25 * sizeof(char));
    a[0] = '\0';
    int i = strlen(path) - 2;
    for (; i > 0; i--) {
        if (path[i] == '/')
            break;
    }
    path += (i + 1);
    strcpy(a, path);
    if (a[strlen(a) - 1] == '/')
    {
        a[strlen(a) - 1] = '\0';
    }
    return a;//dont forget to free
}


//useful, because paths are without / at the end.....
char* pathNoSlash(const char* path)//tested
{
    char* str = (char*)malloc((strlen(path) + 1) * sizeof(char));
    str[0] = '\0';
    strcpy(str, path);
    if (str[strlen(str) - 1] == '/')
        str[strlen(str) - 1] = '\0';
    return str;//don forget to free it 
}


//we should now into which folder we need to add somethin 
//or into which of them we should search needed file 
// so if the path /dir1/dir2/../dirn/filename, we need to get
//pointer to dirn object, BTW it contains into dir(n-1) folder xD

struct folder* getFolder(struct folder* dir, const char* foldName)//tested 
{//should be extractingName used before, or something like that.

    struct folder* dirPtr = NULL;
    for (int i = 0; i <= dir->currDirIndex; i++)
    {
        if (!strcmp(dir->dirs[i].folderName, foldName))
        {
            dirPtr=&(dir->dirs[i]);
            break;
        }
    }
    return dirPtr;
}//should make the same function for file ptr extracting u know

char * getSlash(const char * path)
{
    char * name;
    if(strlen(path)!=1){
    name = (char *)malloc((strlen(path)+2)*sizeof(char));
    strcpy(name,path);
    name[strlen(name)]='/';
    name[strlen(name)+1]='\0';
    }
    else {
    name = (char*)malloc((strlen(path)+1)*sizeof(char));
    strcpy(name,path);
    }
    return name;
}


//also, if the path is /dir1/*file or directory name* so it can not exist, be careful with that
struct folder* getCurrentFolder(const char* path)//tested
{ //dont forget if we need to add any folder, so before we need use function "pathNoSlash"
    path++;//yep we already into mountpoint directory
   
   
    //plan is : 
    //read name , find folder , get its pointer!!
    //btw we dont need to get the last dir ptr because 
    // it can not exist , so, again, if its "/dir1/dir2/" 
    //we should use f pathNoSlash befor THIS FUNCTION
    struct folder* current = &MainFolder;
    char CurStr[64];
    
    int j = 0;

    for (int i = 0; i < strlen(path); i++)
    {
        if (path[i] == '/') {
            CurStr[j] = '\0';
            current = getFolder(current, CurStr);
            j = 0;


        }
        else {
            CurStr[j] = path[i];
            j++;
        }
    }
    return current;

}

int isFolder(const char* path)//useless function, may be deleted, but tested =)
{
    if (!strcmp(path,"/"))
        return 1;
   // char* nsPath = pathNoSlash(path);
    char* foldname = extractingName(path);
    struct folder* curFold = getCurrentFolder(path);
    
    if (getFolder(curFold, foldname)){
       // free(nsPath);
        free(foldname);
        return 1;
    }
   // free(nsPath);
    free(foldname);
    
     return 0;
}

void initSystem() {

    printf("system inited\n");
    //so what should be done there?
    //im MainFolder should be allocated memory for directories, currentIndex should be set equal 0 because of /Books/ and /stats existing 
    //times should be set

    //Books dir and you know stats file too should be created 

    for( int i = 0 ; i < 5;i++)
        array[i]=0;

    time(&MainFolder.dirTimeAccess);
    time(&MainFolder.dirTimeMod);
    MainFolder.dirs = (struct folder*)malloc(10 * sizeof(struct folder));
    MainFolder.currDirIndex = 0;
    MainFolder.currFileIndex = 0;
    //короче это здорово, мы into mountfolder created space for 256 folders..huh..
    
    //names
    strcpy(MainFolder.files[0].fileName, "stats");
    strcpy(MainFolder.dirs[0].folderName, "Books");
    //times 
    time(&MainFolder.files[0].fileTimeAccess); time(&MainFolder.files[0].fileTimeMod);
    
    //content
    
    MainFolder.files[0].fileContent[0] = '\0';
    strcat(MainFolder.files[0].fileContent,"0 files\n0 lines\n0 words\n0 letters \n0 digits\n");

    //time 
    MainFolder.dirs[0].currDirIndex = -1;
    MainFolder.dirs[0].currFileIndex = -1;
    MainFolder.dirs[0].dirs = (struct folder*)malloc(10 * sizeof(struct folder));
    time(&MainFolder.dirs[0].dirTimeAccess); time(&MainFolder.dirs[0].dirTimeMod);


}

struct file* getFile(const char* path)//tested
{   
    if (path[strlen(path) - 1] == '/')//if it directory
        return NULL;

    struct folder * curFold = getCurrentFolder(path);
    char* filename = extractingName(path);//honestly idk if our path name lose first slash.
    for (int i = 0; i <= curFold->currFileIndex; i++)
    {
        if (!strcmp(curFold->files[i].fileName, filename))
            return &curFold->files[i];
    }
    free(filename);
    return NULL;
}

void addDir(const char* path)
{
    //char* nsPath = pathNoSlash(path);
    char* dirName = extractingName(path);
    struct folder* cDir = getCurrentFolder(path);
    int ind = ++(cDir->currDirIndex);
    cDir->dirs[ind].currDirIndex = -1;
    cDir->dirs[ind].currFileIndex = -1;
    cDir->dirs[ind].dirs = (struct folder*)malloc(5 * sizeof(struct folder));
    time(&cDir->dirs[ind].dirTimeAccess);
    time(&cDir->dirs[ind].dirTimeMod);
    strcpy(cDir->dirs[ind].folderName, dirName);
   // free(nsPath);
    free(dirName);

}


void addFile(const char* path)//tested
{
   // char* nsPath = pathNoSlash(path);
    struct folder* curDir = getCurrentFolder(path);
    char* fname = extractingName(path);
    int ind = ++curDir->currFileIndex;
    curDir->files[ind].fileContent[0] = '\0';
    strcpy(curDir->files[ind].fileName, fname);
    time(&curDir->files[ind].fileTimeAccess);
    time(&curDir->files[ind].fileTimeMod);
    
    //free(nsPath);
    free(fname);

}

void countIt(const char * txt)
{
    for (int i = 0; txt[i]; i++)
    {
        /* Check new line */
        if (txt[i] == '\n' || txt[i] == '\0')
            array[1]++;

        /* Check words */
        if (txt[i] == ' ' || txt[i] == '\t' || txt[i] == '\n' || txt[i] == '\0')
            array[2]++;

         /* Check digits */
        if (txt[i] <= '9' && txt[i] >= '0')
            array[4]++;
        else if ((txt[i] >= 'a' && txt[i] <= 'z') || (txt[i] >= 'A' && txt[i] <= 'Z'))
            array[3]++;
    }
    
}

void writeToStats(){
    struct file * stats = getFile("/stats");
    char buffer[5][256];
    for(int i = 0 ; i < 5;i++)
        buffer[i][0]='\0';

    itoa(array[0],buffer[0]);
    strcat(buffer[0]," files\n");
    itoa(array[1],buffer[1]);
    strcat(buffer[1]," lines\n");
    itoa(array[2],buffer[2]);
    strcat(buffer[2]," words\n");
    itoa(array[3],buffer[3]);
    strcat(buffer[3]," letters\n");
    itoa(array[4],buffer[4]);
    strcat(buffer[4]," digits\n");

    strcpy(stats->fileContent,buffer[0]);
    for(int i = 1 ; i < 5;i++)
    {
        strcat(stats->fileContent,buffer[i]);

    }

    time(&stats->fileTimeAccess);
    time(&stats->fileTimeMod);
}
void writeToFile(const char* path, const char* content)//tested
{//should be added case whe we write into /Books/*filename*

    struct file* curFile = getFile(path);
    struct folder * curDir = getCurrentFolder(path);//test it's name
    if (curFile)//file exists
    {
        if(!strcmp(curDir->folderName,"Books"))
        {
            countIt(content);
            array[0]++;
            writeToStats();
        }
        strcpy(curFile->fileContent, content);
        time(&curFile->fileTimeMod);
    }
    return;
}

static int do_getattr(const char* path, struct stat* st)
{
    //printf("\n do_getattr: %s\n", path);
    st->st_uid = getuid();
    st->st_gid = getgid();
    
    if (isFolder(path))
    {
        struct folder * curFold = getCurrentFolder(path);
        st->st_atime = curFold->dirTimeAccess;
        st->st_mtime = curFold->dirTimeMod;
        st->st_mode = S_IFDIR | 0755;
        st->st_nlink = 2;
    }
    else if (getFile(path))
    {
        struct file* cFile = getFile(path);
        st->st_atime = cFile->fileTimeAccess;
        st->st_mtime = cFile->fileTimeMod;
        if(!strcmp(cFile->fileName,"stats"))
            st->st_mode= S_IFREG | 0444;
        else
            st->st_mode = S_IFREG | 0744;
        st->st_nlink = 1;
        st->st_size = strlen(cFile->fileContent);
    }
    else
    {
        return -ENOENT;
    }

    return 0;
}



static int do_readdir(const char* path, void* buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi)
{
   //  printf("\ndo_read: %s\n", path);
    filler(buffer, ".", NULL, 0); // Current Directory
    filler(buffer, "..", NULL, 0); // Parent Directory
    char * spath = getSlash(path);
    printf("\nWith slah we made: %s\n",spath);
    struct folder* cDir = getCurrentFolder(spath);
    if (cDir)
    {
        if(!strcmp(cDir->folderName,"Books")){
            printf("\nattention:%d %d\n",cDir->currDirIndex,cDir->currFileIndex);
        }
        for (int i = 0; i <= cDir->currDirIndex; i++)
            filler(buffer, cDir->dirs[i].folderName, NULL, 0);
        
        for (int j = 0; j <= cDir->currFileIndex; j++)
            filler(buffer, cDir->files[j].fileName, NULL, 0);
    }
    free(spath);
    return 0;
}

static int do_read(const char* path, char* buffer, size_t size, off_t offset, struct fuse_file_info* fi)
{
    struct file* curFile = getFile(path);

    if (!curFile)
        return -1;
 
    time(&curFile->fileTimeAccess);
    char* content =curFile->fileContent;
    memcpy(buffer, content + offset, size);

    return strlen(content) - offset;
}

static int do_mkdir(const char* path, mode_t mode)
{
    
    addDir(path);
    return 0;
}

static int do_mknod(const char* path, mode_t mode, dev_t rdev)
{
    
    addFile(path);

    return 0;
}

static int do_write(const char* path, const char* buffer, size_t size, off_t offset, struct fuse_file_info* info)
{
    writeToFile(path, buffer);

    return size;
}

static struct fuse_operations operations = {
    .getattr = do_getattr,
    .readdir = do_readdir,
    .read = do_read,
    .mkdir = do_mkdir,
    .mknod = do_mknod,
    .write = do_write,
};

int main( int argc, char *argv[] )
{
	initSystem();
	return fuse_main( argc, argv, &operations, NULL );
}
