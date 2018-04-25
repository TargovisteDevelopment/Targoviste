
/*
MIT License

Copyright (c) 2018 Krzysztof Szewczyk

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <targoviste.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int writeArchive(targoviste_archive archive, char * file) {
    FILE * f = fopen(file, "wb");
    int i = 0, totalSize = 0;
    if(!f) return 1;
    
    fprintf(f, "TAR%d%c", archive.amount, 0);
    
    for(i = 0; i < archive.amount; i++, totalSize += archive.files[i].size) {
        fwrite(archive.files[i].name, 1, strlen(archive.files[i].name)+1, f);
        fprintf(f, "%d%c%d%c", totalSize, 0, archive.files[i].size, 0);
    }
    
    for(i = 0; i < archive.amount; i++)
        fwrite(archive.files[i].buffer, 1, archive.files[i].size, f);
    
    fclose(f);
    return 0;
}

static int readi(FILE * f) {
	char s[16];
	fscanf(f, "%[^\0]s", s);
	fgetc(f);
	return atoi(s);
}

targoviste_archive readArchive(char * file, int *error) {
	FILE * f;
	targoviste_archive archive;
	int size, i, j, hst, * startOffsets;
	char * nbuf, * fbuf;
	
	f = fopen(file, "rb");
	if(!f)
        return (*error = 1, archive);
    
    if(fgetc(f)!='T' || fgetc(f)!='A' || fgetc(f)!='R')
        return (*error = 3, archive);
	
	size = readi(f);
	archive.amount = size;

	#ifndef CAST_MALLOC
        archive.files = calloc(sizeof(targoviste_file), size);
    	startOffsets = calloc(sizeof(int), size);
	#else
        archive.files = (targoviste_file *) calloc(sizeof(targoviste_file), size);
		startOffsets = calloc(sizeof(int), size);
	#endif
	
	for(i = 0; i < size; i++) {
		#ifndef CAST_MALLOC
			nbuf = malloc(MAX_FILENAME_LEN);
		#else
			nbuf = (char *) malloc(MAX_FILENAME_LEN);
		#endif
		if(!nbuf)
            return (*error = 2, archive);
		fscanf(f, "%[^\0]s", nbuf);
		fgetc(f);
		archive.files[i].name = nbuf;
		startOffsets[i] = readi(f)+3;
		archive.files[i].size = readi(f);
	}
	
	hst = ftell(f);
	
	for(j = 0; j < size; j++) {
		#ifndef CAST_MALLOC
			fbuf = malloc(archive.files[j].size);
		#else
			fbuf = (char *) malloc(archive.files[j].size);
		#endif
		if(!fbuf) {
			*error = 3;
			return archive;
		}
		fseek(f, hst+startOffsets[j], SEEK_SET);
		fread(fbuf, archive.files[j].size, 1, f);
		archive.files[j].buffer = fbuf;
	}
	
	fclose(f);
	return archive;
}

void cleanArchive(targoviste_archive archive) {
    int i;
    for(i = 0; i < archive.amount; i++)
        free(archive.files[i].buffer),free(archive.files[i].name);
    free(archive.files);
}
