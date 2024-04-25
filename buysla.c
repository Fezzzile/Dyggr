#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

void Usage(char* progName)
{
    fprintf(stdout, "\nRecover deleted files (currently WAVE only).\n\n");
    fprintf(stdout, "Usage: %s -s source\n\n", progName);
    fprintf(stdout, "Copyright (c) 2024 Fezile Nkuna.\n\n");
    fprintf(stdout, "%s is distributed under the terms of the Apache 2.0 license.", progName);
    exit(EXIT_FAILURE);
}
int main(int argc, char *argv[])
{
    int carve = 0;
    if (argc < 2)
    {
        Usage(argv[0]);
    }

    int numFilesRecovered = 0;
    while (carve != EOF)
    {
        char riff[12];
        char tempriff[12];
        char cmpriff[] = "RIFF";
        char cmpwave[] = "WAVE";
        char cmpdata[] = "data";
        char data[4];
        uint32_t rawSizeInBytes;
        int numBytesBeforeDataChunk = 0;
        char *source = "";
        char destName[100]; //"Recovered";
        char tempName[1000]; // Used to merge "Recovered" and numFilesRecovered

        for (int i = 1; i < argc; i++)
        {
            if (strcmp(argv[i], "-s") == 0 /*|| strcmp(argv[i], "--source=<source>") == 0*/)
            {
                if ((i + 1) == argc)
                {
                    fprintf(stderr, "Enter source from which to carve files.\n");
                    Usage(argv[0]);
                }
                source = argv[i + 1];
                break;
            }
        }
        if (strlen(source) == 0)
        {
            fprintf(stderr, "\nError: No source entered.\n");
            Usage(argv[0]);
        }
        sprintf(destName, "%d", numFilesRecovered);
        FILE *DeviceOrFile = fopen(source, "r");
        if (DeviceOrFile == NULL)
        {
            fprintf(stderr, "\nError: Could not open %s.\n", source);
            Usage(argv[0]);
        }

        while(1)
        {
            carve = getc(DeviceOrFile);
            if (carve == EOF)
            {
                fprintf(stdout, "End of %s reached.\n", source);
                exit(0);
            }
            riff[11] = carve;
            if (memcmp(riff, cmpriff, 4) == 0 && memcmp(&riff[8], cmpwave, 4) == 0)  /* WAVE header found. */
            {
                /* Update restoredfile name here */
                numFilesRecovered++;
                fprintf(stdout, "Filename before: %s\n", destName);
                sprintf(destName, "Recovered ");
                sprintf(tempName, "%d", numFilesRecovered);
                strcat(destName, tempName);
                strcat(destName, ".wav");

                /* Write to the new file */
                FILE *restoredfile = fopen(destName, "a");

                /* Write "RIFF....WAVE" first  */
                for (int v = 0; v < 12; v++)
                {
                    fprintf(restoredfile, "%c", riff[v]);
                }
                while(1)
                {
                    carve = getc(DeviceOrFile);
                    if (carve == EOF)
                    {
                        fprintf(stdout, "End of %s reached.\n", source);
                        exit(0);
                    }
                    /* Keep writing bytes until the "data" chunk is reached */
                    putc(carve, restoredfile);
                    data[3] = carve;
                    if (memcmp(data, cmpdata, 4) == 0)  /* "data" chunk found */
                    {
                        fread(&rawSizeInBytes, 1, 4, DeviceOrFile);
                        fprintf(stdout, "%d MiB WAVE file found.\n", (rawSizeInBytes/1024)/1024);

                        /* Write raw data size
                         * By shifting to the right because of little-endianness
                         * Surely there is a libC function that does this more efficiently.
                         */
                        fputc(rawSizeInBytes & 0x000000ff, restoredfile);
                        fputc((rawSizeInBytes & 0x0000ff00) >> 8, restoredfile);
                        fputc((rawSizeInBytes & 0x00ff0000) >> 16, restoredfile);
                        fputc((rawSizeInBytes & 0xff000000) >> 24, restoredfile);

                        for(int h = 0; h < rawSizeInBytes; h++)
                        {
                            putc(getc(DeviceOrFile), restoredfile); // Append the raw audio data
                        }

                        break; /* Break loop to avoid trailing bytes */
                        fclose(restoredfile);
                    }
                    /* Search: shift array contents leftward
                     * to avoid overriding, since bytes are written on the the last position.
                     */
                    for (int m = 0; m < 3; m++)
                    {
                        data[m] = data[m + 1];
                    }
                }
            }
            /* Shift array contents leftward */
            for (int b = 0; b < 11; b++)
            {
                riff[b] = riff[b + 1];
            }
        }

    }
    return EXIT_SUCCESS;
}
