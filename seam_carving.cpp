#include "image_handler.h"
#include "seam_carving.h"
#include "utils.h"

#include <math.h>

char getPosition(int id, imgProp_t* imgProp) {
	int width = imgProp->width;
	int imageSize = imgProp->imageSize;

	//ANGOLO BASSO SX:									[0]
	//ANGOLO ALTO SX									[1]
	//COLONNA SX: id % imgProp->width == 0				[2]
	//ANGOLO BASSO DX									[3]
	//ANGOLO ALTO DX									[4]
	//COLONNA DX: id + 1 %  imgProp->width == 0			[5]
	//RIGA DOWN: id < imgProp->width					[6]
	//RIGA UP: id > imgProp.imageSize - imgProp->width	[7]
	//IN MEZZO											[-1]
	/*
		08 09 10 11
		04 05 06 07
		00 01 02 03
	*/

	if (id % width == 0) {
		if (id == 0)
			return 0;
		if (id == imageSize - width)
			return 1;
		return 2;
	}
	else if (id % width == width - 1) {
		if (id == width - 1)
			return 3;
		if (id == imageSize - 1)
			return 4;
		return 5;
	}
	else if (id < width)
		return 6;
	else if (id > imageSize - width)
		return 7;
	return -1;
}

void calculateEnergy(energyPixel_t* energyPixel, energyPixel_t* pixel, int id, imgProp_t* imgProp) {
	int dx2, dy2;
	//ANGOLO BASSO SX:									[0]
	//ANGOLO ALTO SX									[1]
	//COLONNA SX: id % imgProp->width == 0				[2]
	//ANGOLO BASSO DX									[3]
	//ANGOLO ALTO DX									[4]
	//COLONNA DX: id + 1 %  imgProp->width == 0			[5]
	//RIGA DOWN: id < imgProp->width					[6]
	//RIGA UP: id > imgProp.imageSize - imgProp->width	[7]
	//IN MEZZO											[-1]
	/*
		678
		345
		012
	*/
	char pos = getPosition(id, imgProp);
	switch (pos)
	{
	case 0:
		dx2 = energyPixel[id + 1].color;
		dy2 = energyPixel[id + imgProp->width].color;
		break;
	case 1:
		dx2 = energyPixel[id + 1].color;
		dy2 = energyPixel[id - imgProp->width].color;
		break;
	case 2:
		dx2 = energyPixel[id + 1].color;
		dy2 = energyPixel[id + imgProp->width].color - energyPixel[id - imgProp->width].color;
		break;
	case 3:
		dx2 = energyPixel[id - 1].color;
		dy2 = energyPixel[id + imgProp->width].color;
		break;
	case 4:
		dx2 = energyPixel[id - 1].color;
		dy2 = energyPixel[id - imgProp->width].color;
		break;
	case 5:
		dx2 = energyPixel[id - 1].color;
		dy2 = energyPixel[id + imgProp->width].color - energyPixel[id - imgProp->width].color;
		break;
	case 6:
		dx2 = energyPixel[id - 1].color - energyPixel[id + 1].color;
		dy2 = energyPixel[id + imgProp->width].color;
		break;
	case 7:
		dx2 = energyPixel[id - 1].color - energyPixel[id + 1].color;
		dy2 = energyPixel[id - imgProp->width].color;
		break;
	case -1:
		dx2 = energyPixel[id - 1].color - energyPixel[id + 1].color;
		dy2 = energyPixel[id + imgProp->width].color - energyPixel[id - imgProp->width].color;
		break;
	}

	pixel->energy = sqrt((dx2 * dx2) + (dy2 * dy2));
}

void energyMap(energyPixel_t* energyImg, imgProp_t* imgProp) {
	for (int i = 0; i < imgProp->imageSize; i++) {
		calculateEnergy(energyImg, &energyImg[i], i, imgProp);
	}
}

int min(int id1, int id2, energyPixel_t* energyImg)
{
	return (energyImg[id1].energy < energyImg[id2].energy) ? id1 : id2;
}

void computeSeams(int currentColumn, energyPixel_t* energyImg, pixel_t* imgSrc, seam_t* seams, imgProp_t* imgProp) {

	//ANGOLO BASSO SX:									[0]
	//ANGOLO ALTO SX									[1]
	//COLONNA SX: id % imgProp->width == 0				[2]
	//ANGOLO BASSO DX									[3]
	//ANGOLO ALTO DX									[4]
	//COLONNA DX: id + 1 %  imgProp->width == 0			[5]
	//RIGA DOWN: id < imgProp->width					[6]
	//RIGA UP: id > imgProp.imageSize - imgProp->width	[7]
	//IN MEZZO											[-1]
	/*
		678
		345
		012
	*/


	int nextIdMin = currentColumn;
	int currentId = currentColumn;
	seams[currentColumn].total_energy = 0;

	for (int i = 0; i < imgProp->height; i++) {

		currentId = nextIdMin;

		seams[currentColumn].total_energy += energyImg[currentId].energy;
		seams[currentColumn].ids[i] = currentId;
		
		int pos = getPosition(currentId, imgProp);
		switch (pos)
		{
		case 0:
		case 2:
			nextIdMin = min(currentId + imgProp->width, currentId + 1 + imgProp->width, energyImg);
			break;
		case 3:
		case 5:
			nextIdMin = min(currentId + imgProp->width, currentId - 1 + imgProp->width, energyImg);
			break;
		case 1:
		case 7:
			break;
		default:
			nextIdMin = min(min(currentId + imgProp->width, currentId - 1 + imgProp->width, energyImg),
				currentId + 1 + imgProp->width, energyImg);
			break;
		}

	}
}


void findSeams(energyPixel_t* energyImg, pixel_t* imgSrc, imgProp_t* imgProp, seam_t* minSeam, seam_t* seams) {

	for (int i = 0; i < imgProp->width; i++) {
		computeSeams(i, energyImg, imgSrc, seams, imgProp);
	}

	dummyMin (seams, minSeam, imgProp);
}

void removePixel(int idPixel, energyPixel_t* energyImg, int* idsToRemove, imgProp_t* imgProp, energyPixel_t* newImageGray) {

	int idRow = idPixel / imgProp->width;
	int idToRemove = idsToRemove[idRow];
	int shift = idPixel < idToRemove ? idRow : idRow + 1;

	int newPosition = idPixel - shift;

	newImageGray[newPosition].energy = energyImg[idPixel].energy;
	newImageGray[newPosition].color = energyImg[idPixel].color;
	newImageGray[newPosition].idPixel = energyImg[idPixel].idPixel;

}

void updateImageGray(int idPixel, energyPixel_t* imgGray, energyPixel_t* imgWithoutSeamGray, imgProp_t* imgProp) {
	imgGray[idPixel].color = imgWithoutSeamGray[idPixel].color;
	imgGray[idPixel].idPixel = imgWithoutSeamGray[idPixel].idPixel;
}

void removePixelsFromSrc(pixel_t* imgSrc, pixel_t* newImgSrc, energyPixel_t* imgGray, imgProp_t* imgProp) {
	for (int i = 0; i < imgProp->imageSize; i++) {
		newImgSrc[i].R = imgSrc[imgGray[i].idPixel].R;
		newImgSrc[i].G = imgSrc[imgGray[i].idPixel].G;
		newImgSrc[i].B = imgSrc[imgGray[i].idPixel].B;
	}
}


void removeSeam(energyPixel_t* imgGray, energyPixel_t* imgWithoutSeamGray, seam_t* idsToRemove, imgProp_t* imgProp) {
	int newImgSizePixel = imgProp->imageSize - imgProp->height;
	for(int i = 0; i < imgProp->imageSize; i++) 
		removePixel(i, imgGray, idsToRemove->ids, imgProp, imgWithoutSeamGray);

	imgProp->imageSize = newImgSizePixel;
	imgProp->width -= 1;

	for (int i = 0; i < imgProp->imageSize; i++)
		updateImageGray(i, imgGray, imgWithoutSeamGray, imgProp);

}
