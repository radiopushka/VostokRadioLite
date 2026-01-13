#include <stdlib.h>
#include <string.h>
#include "aliasing.h"


struct anti_aliasing* anti_aliasing_init(double* conv_frame,int frame_size){
    struct anti_aliasing* aa = malloc(sizeof(struct anti_aliasing));
    aa->conv_buffer = malloc(frame_size*sizeof(double));
    aa->conv_buffer_s = aa->conv_buffer;
    aa->conv_buffer_e = aa->conv_buffer+frame_size;

    aa->conv_buffer_vhod = malloc(frame_size*sizeof(double));
    aa->conv_buffer_vhod_s = aa->conv_buffer_vhod;
    aa->conv_buffer_vhod_e = aa->conv_buffer_vhod+frame_size;

    memcpy(aa->conv_buffer, conv_frame, sizeof(double)*frame_size);

    memset(aa->conv_buffer_vhod, 0, sizeof(double)*frame_size);

    return aa;

}
double _prem_okno(struct anti_aliasing* aa){

    double* koltso = aa->conv_buffer_vhod;
    double znachenie = 0;
    for(double* ittr = aa->conv_buffer;ittr<aa->conv_buffer_e;ittr++){
        znachenie += (*ittr)*(*koltso);
        koltso++;
        if(koltso>=aa->conv_buffer_vhod_e){
            koltso=aa->conv_buffer_vhod_s;
        }
    }
    return znachenie;
}

double aliasing(struct anti_aliasing* aa,double vhod){

    double* sledushii = aa->conv_buffer_vhod;
    sledushii++;
    if(sledushii>=aa->conv_buffer_vhod_e){
        sledushii=aa->conv_buffer_vhod_s;
    }
    *sledushii = vhod;
    aa->conv_buffer_vhod = sledushii;
    return _prem_okno(aa);

}
void free_aliasing(struct anti_aliasing* aa){
    free(aa->conv_buffer);
    free(aa->conv_buffer_vhod);
    free(aa);
}

