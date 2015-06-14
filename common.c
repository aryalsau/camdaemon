#include "common.h"

extern struct header buildHeader();
extern void copyHeader(struct header headerStruct, uns8 *ptrFileHeader);
extern struct filepath filePathString();

int XDIM;
int YDIM;
int DAY;
int YEAR;
int XBIN;
int YBIN;
int NUMSUBARRAYS;
int LEFT;
int RIGHT;
int TOP1;
int BOTTOM1;
int NUMCOADDS;
long EXPTIMEMS;
long WAITTIMEMS;
float TEMPERATURE;
char * SITENAME;
char * FILENAME;
char * MONTH;
char * FILTER;

char * LOCATION;

extern struct header buildHeader(){
    struct header headerStruct;
    headerStruct.xdim = XDIM;
    headerStruct.ydim = YDIM;
    headerStruct.day = DAY;
    headerStruct.year = YEAR;
    headerStruct.xbin = XBIN;
    headerStruct.ybin = YBIN;
    headerStruct.numSubarrays = NUMSUBARRAYS;
    headerStruct.left = LEFT;
    headerStruct.right = RIGHT;
    headerStruct.top1 = TOP1;
    headerStruct.bottom1 = BOTTOM1;
    headerStruct.numCoadds = NUMCOADDS;
    headerStruct.expTime = EXPTIMEMS/1000;
    headerStruct.waitTime = WAITTIMEMS/1000;
    headerStruct.temperature = TEMPERATURE;
    headerStruct.siteName = SITENAME;
    headerStruct.filename = FILENAME;
    headerStruct.month = MONTH;
    headerStruct.filter = FILTER;
    return headerStruct;
}

extern void copyHeader(struct header headerStruct, uns8 *ptrFileHeader) {
    memcpy(&ptrFileHeader[0], &headerStruct.xdim, 2);
    memcpy(&ptrFileHeader[2], &headerStruct.ydim, 2);
    memcpy(&ptrFileHeader[36], &headerStruct.day, 2);
    memcpy(&ptrFileHeader[31], &headerStruct.year, 2);
    memcpy(&ptrFileHeader[45], &headerStruct.xbin, 2);
    memcpy(&ptrFileHeader[47], &headerStruct.ybin, 2);
    memcpy(&ptrFileHeader[49], &headerStruct.numSubarrays, 2);
    memcpy(&ptrFileHeader[51], &headerStruct.left, 2);
    memcpy(&ptrFileHeader[53], &headerStruct.right, 2);
    memcpy(&ptrFileHeader[55], &headerStruct.bottom1, 2);
    memcpy(&ptrFileHeader[57], &headerStruct.top1, 2);
    memcpy(&ptrFileHeader[75], &headerStruct.numCoadds, 2);
    memcpy(&ptrFileHeader[81], &headerStruct.expTime, 4);
    memcpy(&ptrFileHeader[85], &headerStruct.waitTime, 4);
    memcpy(&ptrFileHeader[89], &headerStruct.temperature, 4);
    memcpy(&ptrFileHeader[4], headerStruct.siteName, 5);
    memcpy(&ptrFileHeader[19], headerStruct.filename, 12);
    memcpy(&ptrFileHeader[33], headerStruct.month, 3);
    memcpy(&ptrFileHeader[38], headerStruct.filter, 6);
}

extern struct filepath filePathString() {
    struct filepath filePath;

    time_t rawTime;
    struct tm *timeInfo;
    time(&rawTime);
    timeInfo = localtime(&rawTime);

    char * fileNameExtPtr = (char *)malloc(14);
    strftime(fileNameExtPtr,14,"L%H%M%SA.%j", timeInfo);

    filePath.filenameptr = fileNameExtPtr;

    char * folderNamePtr = (char *)malloc(9);
    strftime(folderNamePtr,9,"%b%d%y/", timeInfo);

    char * folderPathPtr= (char *)malloc( strlen(LOCATION) + strlen(folderNamePtr));
    sprintf(folderPathPtr, "%s%s", LOCATION, folderNamePtr);
    filePath.folderpathptr = folderPathPtr;

    char * fullPathPtr = (char *)malloc( strlen(LOCATION) + strlen(folderNamePtr) + strlen(fileNameExtPtr) + 1);
    sprintf(fullPathPtr, "%s%s%s", LOCATION, folderNamePtr, fileNameExtPtr);
    filePath.fullpathptr = fullPathPtr;

    DAY = timeInfo->tm_mday;
    YEAR = 1900 + timeInfo->tm_year;
    char * monthName = (char *)malloc(4);
    strftime(monthName,4,"%^b", timeInfo);
    MONTH = monthName;
    FILENAME = fileNameExtPtr;
    
    // freeing any of these give an error
    // free(timeInfo);
    // free(fileNamePtr);
    // free(fileNameExtPtr);

    return filePath;
}


extern struct config readConfig(){
    struct config configObj;

    FILE *cfgFilePtr;
    cfgFilePtr = fopen("config.cfg" , "r");
    if(cfgFilePtr == NULL) {
        perror("Error opening file");
    }

    int lineSize = 256;
    char line[lineSize];
    char *parameterStr;
    char *valStr;
    while( fgets (line, lineSize, cfgFilePtr)!=NULL ) {
        parameterStr = strtok(line, "=");
        valStr = strtok(NULL, "\n");
        if (strcmp(parameterStr, "CAMDAEMON_SITE") == 0){
            configObj.configSite = (char *)malloc(strlen(valStr));
            strcpy(configObj.configSite, valStr);
        } else if (strcmp(parameterStr, "CAMDAEMON_CAM") == 0){
            configObj.configCam = (char *)malloc(strlen(valStr));
            strcpy(configObj.configCam, valStr);
        } else if (strcmp(parameterStr, "CAMDAEMON_PATH") == 0){
            configObj.configPath = (char *)malloc(strlen(valStr));
            strcpy(configObj.configPath, valStr);

        }
    }
    fclose(cfgFilePtr);

    return configObj;
}

