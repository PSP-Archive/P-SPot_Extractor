#include <pspsdk.h>
#include <pspdebug.h>
#include <pspiofilemgr.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <errno.h>
#include <sys/unistd.h> // motolegacy (22/07/2021) - include unistd to declare chdir()
#include "unzip.h"
#include "unzipToDir.h"

int usePassword = 0;

void change_file_date(filename,dosdate,tmu_date)
    const char *filename;
    uLong dosdate;
    tm_unz tmu_date;
{

}

int mymkdir(dirname)
    const char* dirname;
{
    int ret=0;
	ret = sceIoMkdir(dirname,0777);
    return ret;
}

int makedir (newdir)
    char *newdir;
{
  char *buffer ;
  char *p;
  int  len = (int)strlen(newdir);

  if (len <= 0)
    return 0;

  buffer = (char*)malloc(len+1);
  strcpy(buffer,newdir);

  if (buffer[len-1] == '/') {
    buffer[len-1] = '\0';
  }
  if (mymkdir(buffer) == 0)
    {
      free(buffer);
      return 1;
    }

  p = buffer+1;
  while (1)
    {
      char hold;

      while(*p && *p != '\\' && *p != '/')
        p++;
      hold = *p;
      *p = 0;
      if ((mymkdir(buffer) == -1) && (errno == ENOENT))
        {
          pspDebugScreenPrintf("couldn't create directory %s\n",buffer);
          free(buffer);
          return 0;
        }
      if (hold == 0)
        break;
      *p++ = hold;
    }
  free(buffer);
  return 1;
}


int do_extract_currentfile(uf,popt_extract_without_path,popt_overwrite,password)
    unzFile uf;
    const int* popt_extract_without_path;
    int* popt_overwrite;
    const char* password;
{
    char filename_inzip[256];
    char* filename_withoutpath;
    char* p;
    int err=UNZ_OK;

	FILE *fot=NULL;
    SceUID fout; //=NULL;
    u8* buf;
    uInt size_buf;

    unz_file_info file_info;
    //uLong ratio=0;
    err = unzGetCurrentFileInfo(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);

    if (err!=UNZ_OK)
    {
        pspDebugScreenPrintf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        return err;
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (void*)memalign(64,size_buf);
    if (buf==NULL)
    {
        pspDebugScreenPrintf("Error allocating memory\n");
        return UNZ_INTERNALERROR;
    }

    p = filename_withoutpath = filename_inzip;
    while ((*p) != '\0')
    {
        if (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }

    if ((*filename_withoutpath)=='\0')
    {
        if ((*popt_extract_without_path)==0)
        {
            pspDebugScreenSetXY(0 , 14);
            pspDebugScreenPrintf("Creating directory: %-45.45s\n",filename_inzip);
            mymkdir(filename_inzip);
        }
    }
    else
    {
        const char* write_filename;
        int skip=0;

        if ((*popt_extract_without_path)==0)
            write_filename = filename_inzip;
        else
            write_filename = filename_withoutpath;


		if(usePassword == 1){
			err = unzOpenCurrentFilePassword(uf,password);}else{err = unzOpenCurrentFilePassword(uf,NULL);}
        if (err!=UNZ_OK)
        {
            pspDebugScreenPrintf("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
        }

        if (((*popt_overwrite)==0) && (err==UNZ_OK))
        {
            char rep=0;
            FILE* ftestexist;
            ftestexist = fopen(write_filename,"rb");
            if (ftestexist!=NULL)
            {
                fclose(ftestexist);
                do
                {
                    char answer[128];
                    int ret;

                    pspDebugScreenPrintf("The file %s will be overwritten ",write_filename);
                    ret = 'y';//scanf("%1s",answer);
                    if (ret != 1)
                    {
                       exit(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    if ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                while ((rep!='Y') && (rep!='N') && (rep!='A'));
            }

            if (rep == 'N')
                skip = 1;

            if (rep == 'A')
                *popt_overwrite=1;
        }

        if ((skip==0) && (err==UNZ_OK))
        {
            fot=fopen(write_filename,"wb");

            /* some zipfile don't contain directory alone before file */
            if ((fot==NULL) && ((*popt_extract_without_path)==0) &&
                                (filename_withoutpath!=(char*)filename_inzip))
            {
                char c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                makedir(write_filename);
                *(filename_withoutpath-1)=c;
                fot=fopen(write_filename,"wb");
            }

            if (fot==NULL)
            {
                pspDebugScreenPrintf("error opening %s\n",write_filename);
            }
        }

		fclose(fot);

		fout = sceIoOpen(write_filename, PSP_O_RDWR | PSP_O_CREAT,0777);
		int counter;counter = 0;
        if (fout>=0)
        {
            //pspDebugScreenPrintf(" extracting: %s\n",write_filename);

            do
            {
				pspDebugScreenSetXY(0 , 14);
				pspDebugScreenPrintf("Extracting: %-55.55s\n",write_filename);

				counter++;
                err = unzReadCurrentFile(uf,buf,size_buf);
                if (err<0)
                {
                    pspDebugScreenPrintf("error %d with zipfile in unzReadCurrentFile\n",err);
                    break;
                }//sceIoWrite(fout,buf,err);
                if (err>0)
                    if (sceIoWrite(fout,buf,err)<1)
                    {//fwrite(buf,err,1,fout)
                        pspDebugScreenPrintf("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        break;
                    }
            }
            while (err>0);
            if (fout)
                    sceIoClose(fout);

            if (err==0)
                change_file_date(write_filename,file_info.dosDate,
                                 file_info.tmu_date);
        }

        if (err==UNZ_OK)
        {
            err = unzCloseCurrentFile (uf);
            if (err!=UNZ_OK)
            {
                pspDebugScreenPrintf("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        else
            unzCloseCurrentFile(uf); /* don't lose the error */
    }

    free(buf);
    return err;
}

int do_extract(uf,opt_extract_without_path,opt_overwrite,password)
    unzFile uf;
    int opt_extract_without_path;
    int opt_overwrite;
    const char* password;
{
    uLong i;
    unz_global_info gi;
    int err;

    err = unzGetGlobalInfo (uf,&gi);
    if (err!=UNZ_OK)
        pspDebugScreenPrintf("error %d with zipfile in unzGetGlobalInfo \n",err);

    for (i=0;i<gi.number_entry;i++)
    {
        if (do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password) != UNZ_OK)
            break;

        if ((i+1)<gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            if (err!=UNZ_OK)
            {
                pspDebugScreenPrintf("error %d with zipfile in unzGoToNextFile\n",err);
                break;
            }
        }
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////
//Public API
////////////////////////////////////////////////////////////////////////////////////////////
int unzipToDir(const char *zippath, const char *destpath, const char *pass){
    const char *zipfilename=NULL;
    const char *password=NULL;
    char filename_try[MAXFILENAME+16] = "";
    int opt_do_extract=1;
    int opt_do_extract_withoutpath=0;
    int opt_overwrite=1;
    int opt_extractdir=1;
    const char *dirname=NULL;
    unzFile uf=NULL;

	zipfilename = zippath;
	dirname = destpath;
	password = pass;

	//make destpath, so we don't try to extract to nonexistant
	makedir(dirname);

    if (zipfilename!=NULL)
    {

        strncpy(filename_try, zipfilename,MAXFILENAME-1);
        /* strncpy doesnt append the trailing NULL, of the string is too long. */
        filename_try[ MAXFILENAME ] = '\0';

        uf = unzOpen(zipfilename);
        if (uf==NULL)
        {
            strcat(filename_try,".zip");
            uf = unzOpen(filename_try);
        }
    }

    if (uf==NULL)
    {
        pspDebugScreenPrintf("Cannot open %s or %s.zip\n",zipfilename,zipfilename);
        return ERROR_CANNOT_OPEN;
    }
    pspDebugScreenPrintf("%s opened\n",filename_try);

    if (opt_do_extract==1)
    {
        if (opt_extractdir && chdir(dirname))
        {
          pspDebugScreenPrintf("Error changing into %s, aborting\n", dirname);
          return ERROR_CANNOT_CHDIR;
        }

        int retValue = do_extract(uf,opt_do_extract_withoutpath,opt_overwrite,password);
        unzCloseCurrentFile(uf);
        int err = unzClose(uf);
        if (err!=UNZ_OK)
            pspDebugScreenPrintf("error %d with zipfile in unzClose\n",err);
        else{
            pspDebugScreenSetXY(0 , 14);
            pspDebugScreenPrintf("%-65.65s\n", "Done!");
        }
        return retValue;
    }
    unzCloseCurrentFile(uf);
    unzClose(uf);
    

    return 0;
}
