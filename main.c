#include <limits.h>

#include <stdio.h>
#include <dirent.h>

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>

#define HEADER_SIZE 5
#define FILENAME_SIZE 256

struct Point
{
    double x;
    double y;
    double z;
};

struct Color
{
    int r;
    int g;
    int b;
};

struct Mixed
{
    int r;
    int g;
    int b;
    double x;
    double y;
    double z;
};

struct FileObject
{
    int version;
    int haveColor; 	// 0: Only point, 1: With color
    int pointCount;
    int type; 		//0: Ascii, 1: Binary
    char filename[FILENAME_SIZE];

    struct Point *p;
    struct Color *c;

    int isError; //0: No Error, 1: Error
};

void displayMenu()
{
    printf("\n");
    printf("1 tusuna basildiginda : Dosya kontrolu yapılır \n");
    printf("2 tusuna basildiginda : En yakın/uzak noktaların bilgileri \n");
    printf("3 tusuna basildiginda : Tüm noktaları içine alacak bir küpün kenar bilgileri \n");
    printf("4 tusuna basildiginda : İstenilen kürenin içinde kalan noktaların bilgileri \n");
    printf("5 tusuna basildiginda : Tüm noktaların uzaklıklarının ortalaması \n");
    printf("6 tusuna basildiginde : Program sonlandırılır \n");
}

void displayStart(int rowCount, int columnCount)
{
    int i, j;

    for (i=0; i<rowCount; ++i)
    {
        for(j=0; j<columnCount; ++j)
        {
            printf("*");
        }

        printf("\n");
    }
}

void readPointsFromBinaryFile(const FILE* pFileP, const int pPointCount,
                              struct FileObject* fObjects, int size, int currentIndex)
{
    struct FileObject fObject = fObjects[currentIndex];
    int haveColor = fObject.haveColor;


    fObject.p = (struct Point*)malloc( (pPointCount) * sizeof(struct Point));
    if (haveColor)
    {
        fObject.c = (struct Color*)malloc( (pPointCount) * sizeof(struct Color));
    }


    int i;
    for (i = 0; i < pPointCount; ++i)
    {
        struct Mixed mPoint;
        struct Point pPoint;

        if (haveColor == 0)
        {
            fread(&pPoint, sizeof(struct Point), 1, pFileP);
            fObject.p[i].x = pPoint.x;
            fObject.p[i].y = pPoint.y;
            fObject.p[i].z = pPoint.z;
        }
        else
        {
            fread(&mPoint, sizeof(struct Mixed), 1, pFileP);
            fObject.p[i].x = mPoint.x;
            fObject.p[i].y = mPoint.y;
            fObject.p[i].z = mPoint.z;
            fObject.c[i].r = mPoint.r;
            fObject.c[i].g = mPoint.g;
            fObject.c[i].b = mPoint.b;
        }
    }


    fObjects[currentIndex] = fObject;

    displayStart(1,20);
}

void readPointsFromTextFile(const FILE* pFileP, const int pPointCount,
                            struct FileObject* fObjects, int size, int currentIndex)
{
    char* line = NULL;
    char * pch;

    int i, count, counter, haveColor;

    size_t len = 0;
    ssize_t read;

    struct FileObject fObject = fObjects[currentIndex];

    haveColor = fObject.haveColor;

    fObject.p = (struct Point*)malloc( (pPointCount) * sizeof(struct Point));
    if (haveColor)
    {
        printf("Renk Var...\n");
        fObject.c = (struct Color*)malloc( (pPointCount) * sizeof(struct Color));
    }

    counter = -1;

    for (i = 0; i < pPointCount; ++i)
    {
        counter += 1;
        read = getline(&line, &len, pFileP);

        if(read == -1 || read == 0)
        {
            continue;
        }

        count = 0;
        pch = strtok (line," ");

        while(pch != NULL)
        {
            if (count == 0)
            {
                fObject.p[counter].x = atof(pch);
            }
            else if (count == 1)
            {
                fObject.p[counter].y = atof(pch);
            }
            else if (count == 2)
            {
                fObject.p[counter].z = atof(pch);
            }
            else if (count == 3)
            {
                fObject.c[counter].r = atoi(pch);
            }
            else if (count == 4)
            {
                fObject.c[counter].g = atoi(pch);
            }
            else if (count == 5)
            {
                fObject.c[counter].b = atoi(pch);
            }
            else if (count == 6)
            {
                printf("Error \n");
                fObject.isError = 1;
                break;
            }

            count += 1;
            pch = strtok (NULL, " ");
        }

        if (count != 3 && count != 6)
        {
            fObject.isError = 1;
            break;
        }
    }

    fObjects[currentIndex] = fObject;

    if (line != NULL)
    {
        free(line);
    }
    displayStart(1,20);
}


char* get_file_ext(const char *filename)
{
    const char *ext = strrchr(filename, '.');
    return (ext && ext != filename) ? ext : (filename + strlen(filename));
}


int findFileCountInFolder(char* folderName)
{
    struct dirent *pDirent;
    DIR *pDir;
    int fileCount = 0;

    pDir = opendir ("files");
    if (pDir == NULL)
    {
        printf ("Cannot open directory '%s'\n", folderName);
        return 1;
    }

    while ((pDirent = readdir(pDir)) != NULL)
    {

        if (strcmp(get_file_ext(pDirent->d_name),".nkt") == 0)
        {
            ++fileCount;
        }
    }

    closedir (pDir);
    return fileCount;
}


void readFiles(struct FileObject* files, int fileCount)
{
    // Folder
    struct dirent *pDirent;
    DIR *pDir;

    // File
    FILE * fp;
    size_t len = 0;
    ssize_t read;

    // Open Folder
    pDir = opendir ("files");
    if (pDir == NULL)
    {
        printf ("Cannot open directory '%s'\n", "files");
        return;
    }

    int index = 0;

    while ((pDirent = readdir(pDir)) != NULL)
    {
        if (strcmp(get_file_ext(pDirent->d_name),".nkt") == 0)
        {
            char* fileName = NULL;
            char * line = NULL;

            fileName = (char*)malloc( (strlen(pDirent->d_name) + 6) * sizeof(char));
            strcpy(fileName, "files/");
            strcat(fileName, pDirent->d_name);
            printf("\nDosya adı: \t [%s]\n", fileName);

            fp = fopen(fileName, "rb");
            if (fp == NULL)
                exit(EXIT_FAILURE);

            int counter = 0;
            struct FileObject fileObject = files[index];
            strcpy(fileObject.filename,fileName);

            int i;
            for (i =0; i< HEADER_SIZE; ++i)
            {
                if ((read = getline(&line, &len, fp)) != -1)
                {
                    char * pch;

                    // Counter 0 is ignored which started with #

                    if (counter == 1)	//Check Version
                    {
                        pch = strtok (line," ");
                        if (pch != NULL)
                        {
                            pch = strtok (NULL, " ");
                            fileObject.version = atoi(pch);
                            if(fileObject.version !=1)
                            {
                                fileObject.isError =1;
                            }
                        }
                    }
                    else if (counter == 2) //Check there is a color or only point
                    {
                        pch = strtok (line," ");
                        int splitCount = -1;
                        while (pch != NULL)
                        {
                            splitCount += 1;
                            pch = strtok (NULL, " ");
                        }

                        if (splitCount == 3)
                        {
                            fileObject.haveColor = 0;
                        }
                        else if (splitCount == 6)
                        {
                            fileObject.haveColor = 1;
                        }
                        else
                        {
                            fileObject.haveColor = -1;
                        }
                    }
                    else if (counter == 3)
                    {
                        pch = strtok (line," ");
                        if (pch != NULL)
                        {
                            pch = strtok (NULL, " ");
                            fileObject.pointCount = atoi(pch);
                        }
                    }
                    else if (counter == 4)
                    {
                        pch = strtok (line," ");
                        if (pch != NULL)
                        {
                            pch = strtok (NULL, " ");
                            if(pch[0] == 'a') // Ascii
                            {
                                fileObject.type = 0;
                            }
                            else if (pch[0] == 'b') // Binary
                            {
                                fileObject.type = 1;
                            }
                            else
                            {
                                fileObject.type = -1;
                            }
                        }
                    }

                    counter += 1;
                }
            }

            files[index] = fileObject;

            if (fileObject.type == 0)
            {
                readPointsFromTextFile(fp, fileObject.pointCount, files, fileCount, index);
            }
            else if(fileObject.type == 1)
            {
                readPointsFromBinaryFile(fp,fileObject.pointCount, files, fileCount, index);
            }

            // Go to another file, change file index
            index += 1;

            fclose(fp);

            if(line != NULL)
                free(line);

            if(fileName != NULL)
                free(fileName);
        }
    }

    closedir (pDir);
}

// function to print distance
float distance(float x1, float y1, float z1, float x2,  float y2, float z2)
{
    float d = sqrt(pow(x2 - x1, 2) +
                   pow(y2 - y1, 2) +
                   pow(z2 - z1, 2) * 1.0);
    return d;
}

void findMinimumDistanceBetweenPointsInFile(struct FileObject fObject,FILE *fp)
{
    int i, j;
    double dist = 0;
    int pointCount = fObject.pointCount;

    int minDistance = INT_MAX;
    int maxDistance = INT_MIN;

    int minIndex1, minIndex2, maxIndex1, maxIndex2;
    for (i = 0; i < pointCount - 1; ++i)
    {
        struct Point fPoint = fObject.p[i];

        for (j = i + 1; j < pointCount; ++j)
        {
            struct Point sPoint = fObject.p[j];

            dist =  distance(fPoint.x, fPoint.y, fPoint.z, sPoint.x, sPoint.y, sPoint.z);
            if (dist < minDistance)
            {
                minIndex1 = i;
                minIndex2 = j;

                minDistance = dist;
            }

            if (dist > maxDistance)
            {
                maxIndex1 = i;
                maxIndex2 = j;
                maxDistance = dist;
            }
        }
    }
    if(fObject.haveColor)
    {
        displayStart(1,50);

        printf("Filename [%s] \t Total Point Count: [%8d] \n", fObject.filename, fObject.pointCount);
        printf("Minimum Distance: [%5.5f] \t Index 1: [%6d] \t Index 2: [%6d] \t Point:[%6.2f, %6.2f, %6.2f] Color: [%5d, %5d, %5d]  \t Point: [%6.2f, %6.2f, %6.2f] Color: [%5d, %5d, %5d]\n",
               minDistance, minIndex1, minIndex2,  fObject.p[minIndex1].x, fObject.p[minIndex1].y, fObject.p[minIndex1].z,fObject.c[minIndex1].r,fObject.c[minIndex1].g,fObject.c[minIndex1].b,
               fObject.p[minIndex2].x, fObject.p[minIndex2].y, fObject.p[minIndex2].z,fObject.c[minIndex2].r,fObject.c[minIndex2].g,fObject.c[minIndex2].b);

        displayStart(1,50);

        fprintf(fp,"Filename [%s] \t Total Point Count: [%8d] \n", fObject.filename, fObject.pointCount);
        fprintf(fp,"Minimum Distance: [%5.5f] \t Index 1: [%6d] \t Index 2: [%6d] \t Point:[%6.2f, %6.2f, %6.2f] Color: [%5d, %5d, %5d]  \t Point: [%6.2f, %6.2f, %6.2f] Color: [%5d, %5d, %5d]\n\n",
                minDistance, minIndex1, minIndex2,  fObject.p[minIndex1].x, fObject.p[minIndex1].y, fObject.p[minIndex1].z,fObject.c[minIndex1].r,fObject.c[minIndex1].g,fObject.c[minIndex1].b,
                fObject.p[minIndex2].x, fObject.p[minIndex2].y, fObject.p[minIndex2].z,fObject.c[minIndex2].r,fObject.c[minIndex2].g,fObject.c[minIndex2].b);

        printf("\n");

        displayStart(1,50);

        printf("Filename [%s] \t Total Point Count: [%8d] \n", fObject.filename, fObject.pointCount);
        printf("Maximum Distance: [%5.5f] \t Index 1: [%6d] \t Index 2: [%6d] \t Point:[%6.2f, %6.2f, %6.2f]  Color: [%5d, %5d, %5d] \t Point:[%6.2f, %6.2f, %6.2f]  Color: [%5d, %5d, %5d]\n",
               maxDistance, maxIndex1, maxIndex2,  fObject.p[maxIndex1].x, fObject.p[maxIndex1].y, fObject.p[maxIndex1].z,fObject.c[maxIndex1].r,fObject.c[maxIndex1].g,fObject.c[maxIndex1].b,
               fObject.p[maxIndex2].x, fObject.p[maxIndex2].y, fObject.p[maxIndex2].z,fObject.c[maxIndex2].r,fObject.c[maxIndex2].g,fObject.c[maxIndex2].b);

        displayStart(1,50);

        fprintf(fp,"Filename [%s] \t Total Point Count: [%8d] \n", fObject.filename, fObject.pointCount);
        fprintf(fp,"Maximum Distance: [%5.5f] \t Index 1: [%6d] \t Index 2: [%6d] \t Point:[%6.2f, %6.2f, %6.2f]  Color: [%5d, %5d, %5d] \t Point:[%6.2f, %6.2f, %6.2f]  Color: [%5d, %5d, %5d]\n\n",
                maxDistance, maxIndex1, maxIndex2,  fObject.p[maxIndex1].x, fObject.p[maxIndex1].y, fObject.p[maxIndex1].z,fObject.c[maxIndex1].r,fObject.c[maxIndex1].g,fObject.c[maxIndex1].b,
                fObject.p[maxIndex2].x, fObject.p[maxIndex2].y, fObject.p[maxIndex2].z,fObject.c[maxIndex2].r,fObject.c[maxIndex2].g,fObject.c[maxIndex2].b);

    }
    else
    {
        displayStart(1,50);

        printf("Filename [%s] \t Total Point Count: [%8d] \n", fObject.filename, fObject.pointCount);
        printf("Minimum Distance: [%5.5f] \t Index 1: [%6d] \t Index 2: [%6d] \t Point:[%6.2f, %6.2f, %6.2f] \t Point:[%6.2f, %6.2f, %6.2f]\n",
               minDistance, minIndex1, minIndex2,  fObject.p[minIndex1].x, fObject.p[minIndex1].y, fObject.p[minIndex1].y,
               fObject.p[minIndex2].x, fObject.p[minIndex2].y, fObject.p[minIndex2].y);

        displayStart(1,50);

        fprintf(fp,"Filename [%s] \t Total Point Count: [%8d] \n", fObject.filename, fObject.pointCount);
        fprintf(fp,"Minimum Distance: [%5.5f] \t Index 1: [%6d] \t Index 2: [%6d] \t Point:[%6.2f, %6.2f, %6.2f] \t Point:[%6.2f, %6.2f, %6.2f]\n\n",
                minDistance, minIndex1, minIndex2,  fObject.p[minIndex1].x, fObject.p[minIndex1].y, fObject.p[minIndex1].y,
                fObject.p[minIndex2].x, fObject.p[minIndex2].y, fObject.p[minIndex2].y);
        printf("\n");

        displayStart(1,50);

        printf("Filename [%s] \t Total Point Count: [%8d] \n", fObject.filename, fObject.pointCount);
        printf("Maximum Distance: [%5.5f] \t Index 1: [%6d] \t Index 2: [%6d] \t Point:[%6.2f, %6.2f, %6.2f] \t Point:[%6.2f, %6.2f, %6.2f]\n",
               maxDistance, maxIndex1, maxIndex2,  fObject.p[maxIndex1].x, fObject.p[maxIndex1].y, fObject.p[maxIndex1].y,
               fObject.p[maxIndex2].x, fObject.p[maxIndex2].y, fObject.p[maxIndex2].y);

        displayStart(1,50);

        fprintf(fp,"Filename [%s] \t Total Point Count: [%8d] \n", fObject.filename, fObject.pointCount);
        fprintf(fp,"Maximum Distance: [%5.5f] \t Index 1: [%6d] \t Index 2: [%6d] \t Point:[%6.2f, %6.2f, %6.2f] \t Point:[%6.2f, %6.2f, %6.2f]\n\n",
                maxDistance, maxIndex1, maxIndex2,  fObject.p[maxIndex1].x, fObject.p[maxIndex1].y, fObject.p[maxIndex1].y,
                fObject.p[maxIndex2].x, fObject.p[maxIndex2].y, fObject.p[maxIndex2].y);
    }
}

void cubeEdgePoints(struct FileObject fObject, FILE *fp)
{
    int xMaxIndex, yMaxIndex, zMaxIndex;
    int xMinIndex, yMinIndex, zMinIndex;
    xMaxIndex = yMaxIndex = zMaxIndex = 0;
    xMinIndex = yMinIndex = zMinIndex = 0;
    int i;
    int pointCount;
    pointCount = fObject.pointCount;
    for(i=1; i<pointCount; i++)
    {
        if(fObject.p[xMaxIndex].x<fObject.p[i].x)
            xMaxIndex = i;
        if(fObject.p[xMinIndex].x>fObject.p[i].x)
            xMinIndex = i;
        if(fObject.p[yMaxIndex].y<fObject.p[i].y)
            yMaxIndex = i;
        if(fObject.p[yMinIndex].y>fObject.p[i].y)
            yMinIndex = i;
        if(fObject.p[zMaxIndex].z<fObject.p[i].z)
            zMaxIndex = i;
        if(fObject.p[zMinIndex].z>fObject.p[i].z)
            zMinIndex = i;

    }
    displayStart(1,20);
    printf("Edge points of the smallest cube with all points : \n");
    printf("Filename [%s] \t Total Point Count: [%8d] \n", fObject.filename, fObject.pointCount);
    printf("edge 1 : x[%5.5f] y[%5.5f] z[%5.5f] \n",fObject.p[xMaxIndex].x,fObject.p[xMaxIndex].y,fObject.p[xMaxIndex].z);
    printf("edge 2 : x[%5.5f] y[%5.5f] z[%5.5f] \n",fObject.p[xMaxIndex].x,fObject.p[xMaxIndex].y,fObject.p[xMinIndex].z);
    printf("edge 3 : x[%5.5f] y[%5.5f] z[%5.5f] \n",fObject.p[xMaxIndex].x,fObject.p[xMinIndex].y,fObject.p[xMaxIndex].z);
    printf("edge 4 : x[%5.5f] y[%5.5f] z[%5.5f] \n",fObject.p[xMaxIndex].x,fObject.p[xMinIndex].y,fObject.p[xMinIndex].z);
    printf("edge 5 : x[%5.5f] y[%5.5f] z[%5.5f] \n",fObject.p[xMinIndex].x,fObject.p[xMaxIndex].y,fObject.p[xMaxIndex].z);
    printf("edge 6 : x[%5.5f] y[%5.5f] z[%5.5f] \n",fObject.p[xMinIndex].x,fObject.p[xMaxIndex].y,fObject.p[xMinIndex].z);
    printf("edge 7 : x[%5.5f] y[%5.5f] z[%5.5f] \n",fObject.p[xMinIndex].x,fObject.p[xMinIndex].y,fObject.p[xMaxIndex].z);
    printf("edge 8 : x[%5.5f] y[%5.5f] z[%5.5f] \n",fObject.p[xMinIndex].x,fObject.p[xMinIndex].y,fObject.p[xMinIndex].z);

    displayStart(1,20);

    fprintf(fp,"\nEdge points of the smallest cube with all points : \n");
    fprintf(fp,"\nFilename [%s] \t Total Point Count: [%8d] \n", fObject.filename, fObject.pointCount);
    fprintf(fp,"edge 1 : x[%5.5f] y[%5.5f] z[%5.5f] \n",fObject.p[xMaxIndex].x,fObject.p[xMaxIndex].y,fObject.p[xMaxIndex].z);
    fprintf(fp,"edge 2 : x[%5.5f] y[%5.5f] z[%5.5f] \n",fObject.p[xMaxIndex].x,fObject.p[xMaxIndex].y,fObject.p[xMinIndex].z);
    fprintf(fp,"edge 3 : x[%5.5f] y[%5.5f] z[%5.5f] \n",fObject.p[xMaxIndex].x,fObject.p[xMinIndex].y,fObject.p[xMaxIndex].z);
    fprintf(fp,"edge 4 : x[%5.5f] y[%5.5f] z[%5.5f] \n",fObject.p[xMaxIndex].x,fObject.p[xMinIndex].y,fObject.p[xMinIndex].z);
    fprintf(fp,"edge 5 : x[%5.5f] y[%5.5f] z[%5.5f] \n",fObject.p[xMinIndex].x,fObject.p[xMaxIndex].y,fObject.p[xMaxIndex].z);
    fprintf(fp,"edge 6 : x[%5.5f] y[%5.5f] z[%5.5f] \n",fObject.p[xMinIndex].x,fObject.p[xMaxIndex].y,fObject.p[xMinIndex].z);
    fprintf(fp,"edge 7 : x[%5.5f] y[%5.5f] z[%5.5f] \n",fObject.p[xMinIndex].x,fObject.p[xMinIndex].y,fObject.p[xMaxIndex].z);
    fprintf(fp,"edge 8 : x[%5.5f] y[%5.5f] z[%5.5f] \n",fObject.p[xMinIndex].x,fObject.p[xMinIndex].y,fObject.p[xMinIndex].z);

}

void pointsInSphere(struct FileObject fObject,double userX, double userY, double userZ, double userR,FILE *fp)
{
    int pointCount = fObject.pointCount;
    int i;
    double dist ;
    int controlFlag = 0;
    displayStart(1,20);
    printf("\n\n");
    printf("Filename [%s] \t Total Point Count: [%8d] \n", fObject.filename, fObject.pointCount);
    fprintf(fp,"Filename [%s] \t Total Point Count: [%8d] \n", fObject.filename, fObject.pointCount);
    fprintf(fp,"center point and radius of user-entered sphere : x[%5.5lf] \t y[%5.5lf] \t z[%5.5lf] \t r[%5.5lf] \n \n",userX,userY,userZ,userR);
    printf("Points inside of sphere : \n");
    fprintf(fp,"Points inside of sphere : \n");
    for(i=0; i<pointCount; i++)
    {
        dist = 0;
        dist = distance(userX, userY, userZ, fObject.p[i].x, fObject.p[i].y, fObject.p[i].z);
        if(userR > dist)
        {
            controlFlag++;
            if(fObject.haveColor)
            {
                printf("x[%5.5f] y[%5.5f] z[%5.5f] r[%5.5d] g[%5.5d] b[%5.5d]\n",fObject.p[i].x,fObject.p[i].y,fObject.p[i].z,fObject.c[i].r,
                       fObject.c[i].g,fObject.c[i].b);
                fprintf(fp,"x[%5.5f] y[%5.5f] z[%5.5f] r[%5.5d] g[%5.5d] b[%5.5d]\n",fObject.p[i].x,fObject.p[i].y,fObject.p[i].z,fObject.c[i].r,
                        fObject.c[i].g,fObject.c[i].b);
            }
            else
            {
                printf("x[%5.5f] y[%5.5f] z[%5.5f] \n",fObject.p[i].x,fObject.p[i].y,fObject.p[i].z);
                fprintf(fp,"x[%5.5f] y[%5.5f] z[%5.5f] \n",fObject.p[i].x,fObject.p[i].y,fObject.p[i].z);
            }
        }

    }
    if(controlFlag == 0)
    {
        printf("There is no points in sphere !! ");
        fprintf(fp,"There is no points in sphere !!\n");
    }
    printf("\n\n");
    displayStart(1,20);
}

void averageDistanceOfAllPoints(struct FileObject fObject, FILE *fp )
{
    double sum = 0.0;
    double average = 0.0;
    double dist = 0.0;
    int pointCount = fObject.pointCount;
    int i,j;
    for (i = 0; i < pointCount - 1; ++i)
    {
        struct Point fPoint = fObject.p[i];

        for (j = i + 1; j < pointCount; ++j)
        {
            struct Point sPoint = fObject.p[j];
            dist =  distance(fPoint.x, fPoint.y, fPoint.z, sPoint.x, sPoint.y, sPoint.z);
            sum+= dist;

        }
    }
    double totalTransaction;
    totalTransaction = (((long double)pointCount*((long double)pointCount-1)))/2;
    average = sum/totalTransaction;
    printf("Filename [%s] \t Total Point Count: [%8d] \n", fObject.filename, fObject.pointCount);
    fprintf(fp,"Filename [%s] \t Total Point Count: [%8d] \n", fObject.filename, fObject.pointCount);
    printf("Average distance of all points : [%5.5f] \n",average);
    fprintf(fp,"Average distance of all points : [%5.5f] \n",average);


}


int main (int c, char *v[])
{

    // All Objects
    struct FileObject* fObjects = NULL;

    // Index
    int i;

    // File count
    int fileCount = 0;
    fileCount = findFileCountInFolder("files");
    printf("File count : [%d] \n",fileCount);

    // Dynamic allocation with point count
    //struct FileObject* files = NULL;
    fObjects = (struct FileObject*)malloc( (fileCount) * sizeof(struct FileObject));

    int isRunning = 1;
    char key;

    //Pointer to output.nkt
    FILE *outfp;
    outfp = fopen("output.nkt","w");

    if(outfp == NULL)
    {
        printf("Could not create output.nkt file\n");
        exit(1);
    }

    while(isRunning)
    {
        displayMenu();

        //read a single character
        key=fgetc(stdin);

        if(key==0x0A)
        {
            printf("ENTER KEY is pressed.\n");
            break;
        }
        else
        {
            if (key == '1')
            {
                readFiles(fObjects, fileCount);
                if (fObjects == NULL)
                {
                    printf("< Program did not read files in folder >\n");
                    return 0;
                }

                displayStart(1,20);
                int i, j;
                fprintf(outfp,"\n \n \n \tCHOICE 1 : \n \n \n");
                for(i = 0; i < fileCount; ++i)
                {
                    struct FileObject fObject = fObjects[i];

                    for (j = 0; j < 5; ++j)
                    {
                        if (fObject.haveColor)
                        {
                            printf("x: [%f] \t [%f] \t [%f] \t [%d] \t [%d] \t [%d] \n",
                                   fObject.p[j].x, fObject.p[j].y, fObject.p[j].z, fObject.c[j].r,
                                   fObject.c[j].g, fObject.c[j].b);
                        }
                        else
                        {
                            printf("x: [%f] \t [%f] \t [%f] \n",
                                   fObject.p[j].x, fObject.p[j].y, fObject.p[j].z);
                        }
                    }

                    printf("Hatalı mı: [%s] \t Version: [%d] \t Renk var mı: [%s] \t PointCount: [%d] \t DataType: [%d] \n \n \n",
                           fObject.isError == 1 ? "Evet" : "Hayır",
                           fObject.version, fObject.haveColor == 1 ? "Evet" : "Hayır", fObject.pointCount, fObject.type);

                    fprintf(outfp,"Dosya adı: [%s] \t Hatalı mı: [%s] \t Version: [%d] \t Renk var mı: [%s] \t Nokta sayısı: [%d] \t Data: [%d] \n \n \n",
                            fObject.filename,fObject.isError == 1 ? "Evet" : "Hayır",fObject.version,fObject.haveColor == 1 ? "Evet" : "Hayır",
                            fObject.pointCount,fObject.type);

                }
                displayStart(1,20);
            }
            else if (key == '2')
            {
                fprintf(outfp,"\n \n \n \tCHOICE 2:\n \n \n");

                for ( i = 0; i < fileCount; ++i )
                {
                    struct FileObject fObject = fObjects[i];
                    if (fObject.isError)
                    {
                        continue;
                    }

                    findMinimumDistanceBetweenPointsInFile(fObject,outfp);
                }
                printf("Pressed 2");
            }
            else if (key == '3')
            {
                fprintf(outfp,"\n \n \n \tCHOICE 3:\n \n \n");
                for ( i = 0; i < fileCount; ++i )
                {
                    struct FileObject fObject = fObjects[i];
                    if (fObject.isError)
                    {
                        continue;
                    }

                    cubeEdgePoints(fObject,outfp);

                }
                printf("Pressed 3");
            }
            else if (key == '4')
            {
                fprintf(outfp,"\n \n \n \tCHOICE 4: \n \n \n");
                double userX, userY, userZ, userR;
                printf("Enter the center point coordinate of the sphere : \n");
                printf("x : ");
                scanf("%lf",&userX);
                printf("y : ");
                scanf("%lf",&userY);
                printf("z : ");
                scanf("%lf",&userZ);
                printf("Enter the radius of the sphere : ");
                scanf("%lf",&userR);
                for ( i = 0; i < fileCount; ++i )
                {
                    struct FileObject fObject = fObjects[i];
                    if (fObject.isError)
                    {
                        continue;
                    }
                    pointsInSphere(fObject,userX,userY,userZ,userR,outfp);

                }
                printf("Pressed 4");
            }
            else if (key == '5')
            {
                fprintf(outfp,"\n \n \n \tCHOICE 5: \n \n \n");
                for ( i = 0; i < fileCount; ++i )
                {
                    struct FileObject fObject = fObjects[i];
                    if (fObject.isError)
                    {
                        continue;
                    }
                    averageDistanceOfAllPoints(fObject,outfp);

                }
                printf("Pressed 5");
            }
            else if (key == '6')
            {
                break;
            }
            else
            {
                printf("Entered value: %c", key);
            }
        }
        //read dummy character to clear
        //input buffer, which inserts after character input
        key=getchar();
    }


    // Free
    if (fObjects != NULL)
    {
        for (i=0; i<fileCount; ++i)
        {
            if (fObjects[i].p != NULL)
            {
                free(fObjects[i].p);
            }

            if (fObjects[i].c != NULL)
            {
                fObjects[i].c;
            }
        }

        free(fObjects);
    }
    fclose(outfp);
    return 0;
}
