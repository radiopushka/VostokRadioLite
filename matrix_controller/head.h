#ifndef MATRIX_CTRL

#define MATRIX_CTRL
//управлает композитном сигналом и делает так чтобы махимальное значение сигнала не превышало ограничения ЦАП

struct Matrix_st{
    float* LmR_orig;
    float* LmR_orig_e;
    float* LmR_orig_s;

    float* LpR_orig;
    float* LpR_orig_e;
    float* LpR_orig_s;

    float* LmR_ogr;
    float* LmR_ogr_e;
    float* LmR_ogr_s;

    float* LpR_ogr;
    float* LpR_ogr_e;
    float* LpR_ogr_s;

    int size;

    float g_LmR;
    float g_LpR;

    float release;

};

struct Matrix_st* Matrix_st_init(int lookahead,float release);
void Matrix_st_free(struct Matrix_st* matrix);

void Matrix_st_update(struct Matrix_st* matrix,float* LmR,float* LpR,float limit);



#endif
