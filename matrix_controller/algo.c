#include "head.h"
#include <stdlib.h>
#include <string.h>


struct Matrix_st *Matrix_st_init(int lookahead, float release)
{
    struct Matrix_st *matrix = (struct Matrix_st *)malloc(sizeof(struct Matrix_st));

    matrix->size = lookahead;
    matrix->release = release;

    matrix->LmR_orig = malloc(sizeof(float)*lookahead);
    matrix->LmR_orig_s = matrix->LmR_orig;
    matrix->LmR_orig_e = matrix->LmR_orig + lookahead;

    matrix->LpR_orig = malloc(sizeof(float)*lookahead);
    matrix->LpR_orig_s = matrix->LpR_orig;
    matrix->LpR_orig_e = matrix->LpR_orig + lookahead;

    matrix->LmR_ogr = malloc(sizeof(float)*lookahead);
    matrix->LmR_ogr_s = matrix->LmR_ogr;
    matrix->LmR_ogr_e = matrix->LmR_ogr + lookahead;


    matrix->LpR_ogr = malloc(sizeof(float)*lookahead);
    matrix->LpR_ogr_s = matrix->LpR_ogr;
    matrix->LpR_ogr_e = matrix->LpR_ogr + lookahead;

    memset(matrix->LmR_orig, 0, sizeof(float)*lookahead);
    memset(matrix->LpR_orig, 0, sizeof(float)*lookahead);

    for(int i = 0;i<lookahead;i++){
        matrix->LmR_ogr[i] = 1.0;
        matrix->LpR_ogr[i] = 1.0;
    }

    matrix->g_LmR = 1.0;
    matrix->g_LpR = 1.0;

    return matrix;

}

float dabs(float a){

    if(a<0)
        return -a;
    return a;
}

void naiti_min_mults(struct Matrix_st *matrix,float* LmR, float* LpR, int* dlmr,int* dlpr){

    float* LmR_ogr = matrix->LmR_ogr;
    float* LpR_ogr = matrix->LpR_ogr;

    *LpR = 1.0;
    *LmR = 1.0;

    for(int i = 0;i<matrix->size;i++){

        if(*LmR_ogr < *LmR){
            *LmR = *LmR_ogr;
            *dlmr = matrix->size - i;
        }

        if(*LpR_ogr < *LpR){
            *LpR = *LpR_ogr;
            *dlpr = matrix->size - i;
        }

        LmR_ogr++;
        LpR_ogr++;
        if(LmR_ogr == matrix->LmR_ogr_e){
            LmR_ogr = matrix->LmR_ogr_s;
            LpR_ogr = matrix->LpR_ogr_s;
        }
    }
}


void Matrix_st_update(struct Matrix_st *matrix, float *LmR, float *LpR,float limit){
    float* LmR_orig = matrix->LmR_orig;
    float* LpR_orig = matrix->LpR_orig;

    float* LmR_ogr = matrix->LmR_ogr;
    float* LpR_ogr = matrix->LpR_ogr;


    LmR_orig = LmR_orig + 1;
    LpR_orig = LpR_orig + 1;
    LmR_ogr = LmR_ogr + 1;
    LpR_ogr = LpR_ogr + 1;
    if(LmR_orig >= matrix->LmR_orig_e){
        LmR_orig = matrix->LmR_orig_s;
        LpR_orig = matrix->LpR_orig_s;
        LmR_ogr = matrix->LmR_ogr_s;
        LpR_ogr = matrix->LpR_ogr_s;
    }

    float lmR_r = *LmR_orig;
    float lpR_r = *LpR_orig;

    float vglmr = *LmR_ogr;
    float vglpr = *LpR_ogr;
    if(matrix->g_LmR < vglmr){
        vglmr = matrix->g_LmR;
    }
    if(matrix->g_LpR < vglpr){
        vglpr = matrix->g_LpR;
    }

    float rlmr = *LmR;
    float rlpr = *LpR;
    *LmR_orig = rlmr;
    *LpR_orig = rlpr;
    *LmR = lmR_r*vglmr;
    *LpR = lpR_r*vglpr;

    float abs_lmr = dabs(rlmr);
    float abs_lpr = dabs(rlpr);

    float sum = abs_lmr + abs_lpr;

    if(sum > limit){
        float ratiod = abs_lmr/(abs_lmr + abs_lpr);
        float ratios = 1-ratiod;

        float lim_d = ratiod*limit;
        float lim_s = ratios*limit;

        float atten_s = lim_s / abs_lpr;
        float atten_d = lim_d / abs_lmr;
        *LmR_ogr = atten_d;
        *LpR_ogr = atten_s;

    }else{
        *LmR_ogr = 1.0;
        *LpR_ogr = 1.0;
    }
    matrix->LmR_orig = LmR_orig;
    matrix->LpR_orig = LpR_orig;
    matrix->LmR_ogr = LmR_ogr;
    matrix->LpR_ogr = LpR_ogr;

//найти растояние до минималного умножителя
    float mogrlpr = 1;
    float mogrlmr = 1;
    int ilmr = 0;
    int ilpr = 0;
    naiti_min_mults(matrix,&mogrlmr,&mogrlpr,&ilmr,&ilpr);


    if(mogrlmr > matrix->g_LmR){
        matrix->g_LmR = matrix->release + matrix->g_LmR;
        if(matrix->g_LmR > 1.0){
            matrix->g_LmR = 1.0;
        }
    }else{
        matrix->g_LmR = matrix->g_LmR - ((matrix->g_LmR - mogrlmr)/((float)ilmr));
    }

    if(mogrlpr > matrix->g_LpR){
        matrix->g_LpR = matrix->release + matrix->g_LpR;
        if(matrix->g_LpR > 1.0){
            matrix->g_LpR = 1.0;
        }
    }else{
        matrix->g_LpR = matrix->g_LpR - ((matrix->g_LpR - mogrlpr)/((float)ilpr));
    }



}

void Matrix_st_free(struct Matrix_st *matrix){
    free(matrix->LmR_orig);
    free(matrix->LpR_orig);
    free(matrix->LmR_ogr);
    free(matrix->LpR_ogr);
    free(matrix);
}
