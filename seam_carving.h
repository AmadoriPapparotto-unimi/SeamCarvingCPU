#pragma once
#include "image_handler.h";

typedef struct seamStruct {
	float total_energy;
	int* ids;
} seam_t;

void energyMap(energyPixel_t* energyImg, imgProp_t* imgProp);
void findSeams(energyPixel_t* energyImg, pixel_t* imgSrc, imgProp_t* imgProp, seam_t* minSeam, seam_t* seams);
void removeSeam(energyPixel_t* imgGray, energyPixel_t* imgWithoutSeamGray, seam_t* idsToRemove, imgProp_t* imgProp);
void removePixelsFromSrc(pixel_t* imgSrc, pixel_t* newImgSrc, energyPixel_t* imgGray, imgProp_t* imgProp);