#ifndef ANTI_ALIASING


#define ANTI_ALIASING

struct anti_aliasing {
    double* conv_buffer;
    double* conv_buffer_s;
    double* conv_buffer_e;

    double* conv_buffer_vhod;
    double* conv_buffer_vhod_s;
    double* conv_buffer_vhod_e;
};

struct anti_aliasing* anti_aliasing_init(double* conv_frame,int frame_size);

double aliasing(struct anti_aliasing* aa,double vhod);

void free_aliasing(struct anti_aliasing* aa);


#endif
