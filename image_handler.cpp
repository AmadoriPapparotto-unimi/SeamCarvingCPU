#define _CRT_SECURE_NO_DEPRECATE

#include "image_handler.h"
#include "seam_carving.h"
#include "utils.h"
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>


void grayValue(energyPixel_t* energyPixel, pel_t r, pel_t g, pel_t b, int id) {
	int grayVal = (r + g + b) / 3;
	energyPixel->color = grayVal;
	energyPixel->id_pixel = id;
}

void toGrayScale(pixel_t* img, energyPixel_t* imgGray, imgProp_t* imgProp) {

	for (int i = 0; i < imgProp->imageSize; i++) {
		grayValue(&imgGray[i], img[i].R, img[i].G, img[i].B, i);
	}

}

void setupImgProp(imgProp_t* imgProp, FILE* f) {
	pel_t headerInfo[54];
	fread(headerInfo, sizeof(pel_t), 54, f);


	int width = *(int*)&headerInfo[18];
	int height = *(int*)&headerInfo[22];
	printf("#bytes: %d\n", *(int*)&headerInfo[34]);

	for (unsigned int i = 0; i < 54; i++)
		imgProp->headerInfo[i] = headerInfo[i];

	imgProp->height = height;
	imgProp->width = width;
	imgProp->imageSize = width * height;

	printf("Input BMP dimension: (%u x %u)\n", imgProp->width, imgProp->height);
	printf("IHeader[2] %d\n", *(int*)&headerInfo[2]);
}

void readBMP(FILE* f, pixel_t* img, imgProp_t* imgProp) {
	//img[0] = B
	//img[1] = G
	//img[2] = R
	//BMP LEGGE I PIXEL NEL FORMATO BGR

	for (int r = 0; r < imgProp->height; r++) {
		fread(&img[r * imgProp->width], sizeof(pel_t), imgProp->width * sizeof(pixel_t), f);

		int padding = 4 - ((imgProp->width * 3) % 4);
		//int padding = (imgProp->width * 3) % 4;
		if (padding != 0 && padding != 4) {
			fseek(f, padding, SEEK_CUR);

		}
		//printf("PADDING LETTO PER RIGA %d\n", padding % 4);
	}

}

void writeBMP_pixel(char* p, pixel_t* img, imgProp_t* ip) {
	FILE* fw = fopen(p, "wb");

	printf("FINAL HEIGHT %d\n", ip->height);
	printf("FINAL WIDTH %d\n", ip->width);
	fwrite(ip->headerInfo, 1, 54, fw);

	int padding = 0;
	for (int r = 0; r < ip->height; r++) {
		for (int c = 0; c < ip->width; c++) {
			fputc(img[c + r * ip->width].B, fw);
			fputc(img[c + r * ip->width].G, fw);
			fputc(img[c + r * ip->width].R, fw);
		}
		padding = 4 - ((ip->width * 3) % 4);
		if (padding != 0 && padding != 4) {
			for (int i = 0; i < padding; i++) {
				fputc(0, fw);
			}
		}
		//printf("PADDING AGGIUNTO PER RIGA %d\n", count_padding_per_row);

//		fwrite(&img[r * ip->width], sizeof(pixel_t), ip->width, fw);
		//560
	}
	fflush(fw);

	//printf("PADDING AGGIUNTO %d\n", count_padding);
	//Sleep(1 * 1000);

	//syncfs(fw);
	fclose(fw);
	printf("Immagine %s generata\n", p);
}
//
//void writeBMP_energy(char* p, energyPixel_t* energyImg, imgProp_t* ip) {
//	pixel_t* img;
//	img = (pixel_t*)malloc(ip->imageSize * sizeof(pixel_t));
//
//	for (int i = 0; i < ip->imageSize; i++) {
//		img[i].R = energyImg[i].energy;
//		img[i].G = energyImg[i].energy;
//		img[i].B = energyImg[i].energy;
//	}
//
//	writeBMP_pixel(p, img, ip);
//	free(img);
//}

//void writeBMP_minimumSeam(char* p, energyPixel_t* energyImg, seam_t* minSeam, imgProp_t* imgProp) {
//	for (int y = 0; y < imgProp->height; y++) {
//		printf("PATH: %d\n", minSeam[0].ids[y]);
//		energyImg[minSeam[0].ids[y]].pixel.R = 0;
//		energyImg[minSeam[0].ids[y]].pixel.G = 255;
//		energyImg[minSeam[0].ids[y]].pixel.B = 0;
//	}
//
//	pixel_t* img2convert = (pixel_t*)malloc(imgProp->imageSize * sizeof(pixel_t));
//	energy2pixel(img2convert, energyImg, imgProp);
//	writeBMP_pixel(strcat(SOURCE_PATH, "seams_map_minimum.bmp"), img2convert, imgProp);
//	free(img2convert);
//}

//void energy2pixel(pixel_t* img2convert, energyPixel_t* energyImg, imgProp_t* ip) {
//	//pixel_t* img;
//	//img = (pixel_t*)malloc(ip->imageSize * sizeof(pixel_t));
//
//	for (int i = 0; i < ip->imageSize; i++) {
//		img2convert[i] = energyImg[i].pixel;
//	}
//
//	//return img;
//}

void setBMP_header(imgProp_t* imgProp, int fileSize, int width) {
	imgProp->headerInfo[2] = (unsigned char)(fileSize >> 0) & 0xff;
	imgProp->headerInfo[3] = (unsigned char)(fileSize >> 8) & 0xff;
	imgProp->headerInfo[4] = (unsigned char)(fileSize >> 16) & 0xff;
	imgProp->headerInfo[5] = (unsigned char)(fileSize >> 24) & 0xff;

	imgProp->headerInfo[18] = (unsigned char)(width >> 0) & 0xff;
	imgProp->headerInfo[19] = (unsigned char)(width >> 8) & 0xff;
	imgProp->headerInfo[20] = (unsigned char)(width >> 16) & 0xff;
	imgProp->headerInfo[21] = (unsigned char)(width >> 24) & 0xff;
}


//void writeBMPHeader(char* p, energyPixel_t* energyImg, imgProp_t* ip, int newSize) {
//
//	//printf("Original image size = %d\n", ip->imageSize);
//	//printf("new size byte= %d\n", newSize);
//	pixel_t* img;
//
//	//printf("new image size = %d\n", (newSize -54 )/3);
//	//printf("new image size 2 = %d\n", (ip->imageSize - ip->height));
//
//	
//	ip->headerInfo[2] = (unsigned char)(newSize >> 0) & 0xff;
//	ip->headerInfo[3] = (unsigned char)(newSize >> 8) & 0xff;
//	ip->headerInfo[4] = (unsigned char)(newSize >> 16) & 0xff;
//	ip->headerInfo[5] = (unsigned char)(newSize >> 24) & 0xff;
//
//
//	
//	//printf("#bytes: %x\n", *(int*)&(ip->headerInfo[2]));
//
//	int newWidth = ip->width - 1;
//
//	ip->imageSize =(newSize- 54) /3;
//	ip->width = newWidth;
//
//	//ip->headerInfo[18] = newWidth;
//
//	ip->headerInfo[18] = (unsigned char)(newWidth >> 0) & 0xff;
//	ip->headerInfo[19] = (unsigned char)(newWidth >> 8) & 0xff;
//	ip->headerInfo[20] = (unsigned char)(newWidth >> 16) & 0xff;
//	ip->headerInfo[21] = (unsigned char)(newWidth >> 24) & 0xff;
//	
//	//printf("newWidth  = %d\n", *(int*)&(ip->headerInfo[18]));
//	img = (pixel_t*)malloc(ip->imageSize * sizeof(pixel_t));
//
//	for (int i = 0; i < ip->imageSize; i++) {
//		img[i].R = energyImg[i].energy;
//		img[i].G = energyImg[i].energy;
//		img[i].B = energyImg[i].energy;
//	}
//
//	writeBMP_pixel(p, img, ip);
//	free(img);
//}

//void writeBMP_pel(char* p, imgProp imgProp, pel* img) {
//	FILE* fw = fopen(p, "wb");
//
//	//0000 0000 0001 0101 0001 0111 1010 0000
//	imgProp.headerInfo[34] = ip.imageSize >> 0;//0xa0;
//	imgProp.headerInfo[35] = ip.imageSize >> 8;//0x17;
//	imgProp.headerInfo[36] = ip.imageSize >> 16;//0x15;
//	imgProp.headerInfo[37] = ip.imageSize >> 24;//0x0;
//	
//	printf("%ld; %d", imgProp.headerInfo[34], imgProp.height * imgProp.width);
//	fwrite(imgProp.headerInfo, sizeof(pel), 54, fw);
//	fwrite(img, sizeof(pel), imgProp.height * imgProp.width, fw);
//
//	fclose(fw);
//}