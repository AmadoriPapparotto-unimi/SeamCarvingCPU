#define _CRT_SECURE_NO_DEPRECATE

#include "image_handler.h"
#include "seam_carving.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys\timeb.h> 

char* src_path;

void applySeamCarving(char* p, int iterations) {

	imgProp_t* imgProp;

	pixel_t* imgSrc;
	pixel_t* imgWithoutSeamSrc;

	energyPixel_t* imgGray;
	energyPixel_t* imgWithoutSeamGray;

	seam_t* seams;
	seam_t* minSeamsPerBlock;
	seam_t* minSeam;

	FILE* f = fopen(p, "rb");
	if (f == NULL) {
		printf("*** FILE NOT FOUND %s ***\n", p);
		exit(1);
	}

	imgProp = (imgProp_t*) malloc(sizeof(imgProp_t));
	setupImgProp(imgProp, f);

	imgSrc = (pixel_t*) malloc(imgProp->imageSize * sizeof(pixel_t));
	imgGray = (energyPixel_t*) malloc(imgProp->imageSize * sizeof(energyPixel_t));
	imgWithoutSeamSrc = (pixel_t*) malloc((imgProp->imageSize - (imgProp->height * iterations)) * sizeof(pixel_t));
	imgWithoutSeamGray = (energyPixel_t*) malloc(imgProp->imageSize * sizeof(energyPixel_t));

	minSeam = (seam_t*)malloc(sizeof(seam_t));
	minSeam->ids = (int*)malloc(imgProp->height * sizeof(int));

	seams = (seam_t*)malloc(imgProp->width * sizeof(seam_t));
	for (int i = 0; i < imgProp->width; i++)
		seams[i].ids = (int*)malloc(imgProp->height * sizeof(int));

	int numBlocks = imgProp->width / 1024 + 1;
	minSeamsPerBlock = (seam_t*) malloc(numBlocks * sizeof(seam_t));
	for (int i = 0; i < numBlocks; i++)
		minSeamsPerBlock[i].ids = (int*) malloc(imgProp->height * sizeof(int));

	readBMP(f, imgSrc, imgProp);

	struct timeb start, end;
	ftime(&start);
	toGrayScale(imgSrc, imgGray, imgProp);

	for (int i = 0; i < iterations; i++) {
		energyMap(imgGray, imgProp);		
		findSeams(imgGray, imgSrc, imgProp, minSeam, seams);
		removeSeam(imgGray, imgWithoutSeamGray, minSeam, imgProp);
		//printf("ITERAZIONE %d COMPLETATA\n", i);
	}

	removePixelsFromSrc(imgSrc, imgWithoutSeamSrc, imgGray, imgProp);
	ftime(&end);
	int diff = (int)(1000.0 * (end.time - start.time)
		+ (end.millitm - start.millitm));;
	printf("\nOperation took %u milliseconds\n", diff);
	setBMP_header(imgProp, 0, imgProp->width);
	writeBMP_pixel((char*)"C:\\aa\\reducedcpu.bmp", imgWithoutSeamSrc, imgProp);

	fclose(f);
}

int main(int argc, char** argv) {

	char* path = argv[1];
	int iterations = atoi(argv[2]);

	applySeamCarving(path, iterations);

	return 0;
}
