struct configFile{
    char zipFile[262];
    char appName[257];
    char destDir[262];
    double bytesNeeded;
    int autoDelete;
};
struct configFile config;

int readConfig();

