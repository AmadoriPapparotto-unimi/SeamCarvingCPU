#include <stdlib.h>
#include <iostream>

#include "image_handler.h"
#include "utils.h"
#include "seam_carving.h"

void dummyMin(seam_t* energiesArray, seam_t *output, imgProp_t* imgProp) {
    *output = energiesArray[0];
    for (int i = 0; i < imgProp->width; i++) {
        if (output->total_energy > energiesArray[i].total_energy) {
            *output = energiesArray[i];
        }
    }

}