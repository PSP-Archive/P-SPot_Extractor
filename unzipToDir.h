#define MAXFILENAME 262
#define WRITEBUFFERSIZE 8192

#define ERROR_CANNOT_OPEN -1
#define ERROR_CANNOT_CHDIR -2

int unzipToDir(const char *zippath, const char *destpath, const char *pass);
