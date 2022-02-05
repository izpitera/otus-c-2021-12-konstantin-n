#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct __attribute__ ((__packed__)) {
    uint32_t signature; // 0x04034B50
    uint16_t versionNeededToExtract; // unsupported
    uint16_t generalPurposeBitFlag; // unsupported
    uint16_t compressionMethod;
    uint16_t lastModFileTime;
    uint16_t lastModFileDate;
    uint32_t crc32;
    uint32_t compressedSize;
    uint32_t uncompressedSize;
    uint16_t fileNameLength;
    uint16_t extraFieldLength; // unsupported
} localFileHeader;

typedef struct __attribute__ ((__packed__)) {
    uint32_t signature; // 0x02014B50
    uint16_t versionMadeBy; // unsupported
    uint16_t versionNeededToExtract; // unsupported
    uint16_t generalPurposeBitFlag; // unsupported
    uint16_t compressionMethod;
    uint16_t lastModFileTime;
    uint16_t lastModFileDate;
    uint32_t crc32;
    uint32_t compressedSize;
    uint32_t uncompressedSize;
    uint16_t fileNameLength;
    uint16_t extraFieldLength; // unsupported
    uint16_t fileCommentLength; // unsupported
    uint16_t diskNumberStart; // unsupported
    uint16_t internalFileAttributes; // unsupported
    uint32_t externalFileAttributes; // unsupported
    uint32_t relativeOffsetOflocalHeader;
} globalFileHeader;

typedef struct __attribute__ ((__packed__))
{
    uint32_t signature; // 0x02014b50
    uint16_t versionMadeBy;
    uint16_t versionToExtract;
    uint16_t generalPurposeBitFlag;
    uint16_t compressionMethod;
    uint16_t modificationTime;
    uint16_t modificationDate;
    uint32_t crc32;
    uint32_t compressedSize;
    uint32_t uncompressedSize;
    uint16_t filenameLength;
    uint16_t extraFieldLength;
    uint16_t fileCommentLength;
    uint16_t diskNumber;
    uint16_t internalFileAttributes;
    uint32_t externalFileAttributes;
    uint32_t localFileHeaderOffset;
    uint8_t *filename;
    uint8_t *extraField;
    uint8_t *fileComment;
} centralDirectoryFileHeader;

typedef struct __attribute__ ((__packed__)) {
    uint16_t compressionMethod;
    uint16_t lastModFileTime;
    uint16_t lastModFileDate;
    uint32_t crc32;
    uint32_t compressedSize;
    uint32_t uncompressedSize;
    uint32_t offset;
} fileHeader;

typedef struct __attribute__ ((__packed__)) {
    uint32_t signature; // 0x06054b50
    uint16_t diskNumber; // unsupported
    uint16_t centralDirectoryDiskNumber; // unsupported
    uint16_t numEntriesThisDisk; // unsupported
    uint16_t numEntries;
    uint32_t centralDirectorySize;
    uint32_t centralDirectoryOffset;
    uint16_t zipCommentLength;
    // Followed by .ZIP file comment (variable size)
} eocd;

void dump_buffer(void *buffer, int buffer_size)
{
    for (int i = 0; i < buffer_size; i++)
    {
        printf("\t%02X\n", ((char *)buffer)[i]);
    }
    printf("\n");
}

int main(int argc, char **argv)
{
    // https://www.linuxquestions.org/questions/programming-9/c-howto-read-binary-file-into-buffer-172985/
    // https://codeandlife.com/2014/01/01/unzip-library-for-c/
    // const char* filename = "non-zipjpeg.jpg";
    // const char* filename = "zipjpeg.jpg";

	char *buffer;
	eocd *e;

	char *path = (argc > 1) ? argv[1] : "non-zipjpeg.jpg";
	printf("\nFile name: %s\n", path);
	FILE *file = fopen(path, "rb");

    if (!file)
	{
		perror(path);
		return EXIT_FAILURE;
	}

	//Get file length
	fseek(file, 0, SEEK_END);
	const uint32_t fileLen = ftell(file);
	printf("\tFile size: %d\n", fileLen);
	fseek(file, 0, SEEK_SET);

	//Allocate memory
	buffer = (char *)malloc(fileLen + 1);
	if (!buffer)
	{
		perror(path);
		return EXIT_FAILURE;
	}
	else
    {
        //Read file contents into buffer
        fread(buffer, fileLen, 1, file);
	}

//    if(fclose(file))
//    {
//        perror(path);
//        return EXIT_FAILURE;
//    }

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//    int i;
//    for ( i = fileLen - sizeof(eocd); i >= 0; i--)
//    {
//        e = (eocd *)(buffer + i);
//        if(e->signature == 0x06054B50)
//        {
//            printf("\nSignature found!\n");
//            printf("\n%d", e->numEntries);
//            printf("\n%d", e->zipCommentLength);
//            printf("\n0x%08x\n", e->signature);
//            break;
//        }
//    }
//    if(i < 0) {
//        fprintf(stderr, "It is not a zip file!");
//        return EXIT_FAILURE;
//    }

    centralDirectoryFileHeader *cdfh;

	uint16_t k = 0;
	for(size_t n; n < fileLen; n++)
    {
        cdfh = (centralDirectoryFileHeader *)(buffer + n);
        if(cdfh->signature == 0x02014B50)
        {
            printf("%4d: %x %d %d ", ++k, cdfh->signature, cdfh->filenameLength, (int)n);
            if (cdfh->filenameLength)
            {
                uint8_t *bf = (uint8_t *)malloc(cdfh->filenameLength + 1);
                size_t off = sizeof(uint32_t) * 6 + sizeof(uint16_t) * 11; // number of bytes b4 filename
                for (size_t m = 0; m < cdfh->filenameLength; m++)
                {
                    *(bf + m) = *(buffer + n + off + m);
                    //putchar((uint8_t)*(buffer + n + off + m));
                }
                *(bf + cdfh->filenameLength) = '\0';
                printf("\t%s\n", bf);
                free(bf);
            }
        }
    }

	for (size_t offset = fileLen - sizeof(eocd); offset != 0; --offset)
    {
        uint32_t signature = 0;

        fseek(file, offset, SEEK_SET);
        fread((char *) &signature, 1, sizeof(signature), file);

        if (0x06054b50 == signature)
        {
            //printf("\t\t\t%x\n", signature);
            e = (eocd *)(buffer + offset);
            break;
        }
    }

    //fread((char *) &e, 1, sizeof(eocd), file);
    printf("\t%x\n",e->signature); // 0x06054b50
    printf("\t%d\n",e->numEntries);
    printf("\t%d\n",e->centralDirectoryOffset);
    printf("\t%d\n",e->diskNumber);
    printf("\t%d\n",e->centralDirectoryDiskNumber);
    printf("\t%d\n",e->numEntriesThisDisk);
    printf("\t%d\n",e->centralDirectorySize);
    printf("\t%d\n",e->zipCommentLength);


	if(fclose(file))
    {
        perror(path);
        return EXIT_FAILURE;
    }

	free(buffer);

	return EXIT_SUCCESS;
}
