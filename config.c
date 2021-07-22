#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

int readConfig(){
    strcpy(config.zipFile, "");
    strcpy(config.destDir, "");
    config.bytesNeeded = 0;

    char lineText[256];
	FILE *f = fopen("./unzip.cfg", "rt");
	if (f == NULL)
		return -1;

    int element = 0;
	while(fgets(lineText, 256, f) != NULL){
        char name[257] = "";
        
		if (strlen(lineText) > 0){
            if ((int)lineText[strlen(lineText) - 1] == 10 || (int)lineText[strlen(lineText) - 1] == 13)
                lineText[strlen(lineText) - 1] = '\0';
            if ((int)lineText[strlen(lineText) - 1] == 10 || (int)lineText[strlen(lineText) - 1] == 13)
                lineText[strlen(lineText) - 1] = '\0';

			//Split line:
			element = 0;
			char *result = NULL;
			result = strtok(lineText, "=");
			while(result != NULL){
				if (strlen(result) > 0){
					if (element == 0)
						strcpy(name, result);
					else if (element == 1){
                        if (!strcmp(name, "ZIPFILE"))
    						strcpy(config.zipFile, result);
                        else if (!strcmp(name, "APPNAME"))
    						strcpy(config.appName, result);
                        else if (!strcmp(name, "DESTDIR"))
    						strcpy(config.destDir, result);
                        else if (!strcmp(name, "BYTESNEEDED"))
    						config.bytesNeeded = atoi(result);
                        else if (!strcmp(name, "AUTODELETE"))
    						config.autoDelete = atoi(result);
                    }
					element++;
				}
				result = strtok(NULL, "=");
			}
        }
    }
    fclose(f);
    return 0;
}

