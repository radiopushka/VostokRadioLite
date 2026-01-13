#include "alsa_pipe/alsa_pipe.h"
#include "./FFT/FFT.h"
#include "./EQ_max_algorithm/maxim.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "./anti-alias/aliasing.h"

//the Raspberry Pi will have no cli control, purely config file based
//CPU is limited
#include "./config_file/config.h"


char recording[32];
char playback[32];

const float int_value = 2147483600.0;
const float int_value_attack = int_value*0.8;
const float int_range = int_value-int_value_attack;
//rates fixed to 48khz and 192 khz
//
//user settings:
float pilot_amp = 0.15;
float neg_mod = 0.0;
int c1_MPX = 1;
int c2_MPX = 1;
float bass_cut = 80.0f;
//post AGC limiter amp and image spectral limiter
float pre_amp = 0.7;
float post_amp =  1;
float limit_value = 2.3e9;
float* l_release;
float harmonic_diff = 0.6;
int lookahead = 3;
//the raspberry pi zero can do 9 bins max
int bins = 9;//valid values: 5,9,15,30,45
//pre limiter equalization
float* pre_eq;
//gain controller
float attack = 0.03;
float release = 0.003;
float target = 6e9;//3e9 for pi zero and 6e9 for normal setups
float noise_th = 2e6;


//mono to st_ratio
float stereo_ratio = 0.3;


//high pass filter
float alpha = (400.0)/48000.0;
float nalpha;//10hz
float hpv_l = 0;
float hpv_r = 0;

//bass boost
float bhpv_l = 0;
float bhpv_r = 0;
float bass_boost = 0.4;
float nbass_boost;

//anti aliasing for composite signals
int mpx_anti_alias = 1;
double cv_frame[] = {0.1,0.8,0.1};
double cv_frame_clipper[] = {0.05,0.15,0.3,0.3,0.15,0.05};

//sample rate tunning
int discard_samples = 0;
//so that we can call this later from a gui
void pre_set_settings(){
    nalpha = 1-alpha;
    nbass_boost = 1-bass_boost;
}
void setup_globals_from_config(char* file){
    CfgRaster cfg = read_cfg_file(file);
    if(!cfg){
        printf("config file not found\n");
        return;
    }

    //audio playback recording
    char* recording_f = get_value_by(cfg,"audio","iface");
    char* playback_f = get_value_by(cfg,"audio","oface");
    if(recording_f)
        sprintf(recording,"%s",recording_f);
    if(playback_f)
        sprintf(playback,"%s",playback_f);
    printf("read config\n");
    printf("playback: %s, rec: %s\n",playback,recording);

    //clock tunning
    char* sample_f = get_value_by(cfg,"audio","discard_samples");
    if(sample_f){
        discard_samples = atoi(sample_f)*2;
        printf("sample discard: %d\n",discard_samples/2);
    }
    //MPX settings
    char* pilot_amp_f = get_value_by(cfg,"MPX","pilot_amp");
    if(pilot_amp_f){
        pilot_amp = atof(pilot_amp_f);
        printf("pilot amp: %f\n",pilot_amp);
    }

    char* stereo_ratio_f = get_value_by(cfg,"MPX","stereo_ratio");
    if(stereo_ratio_f){
        stereo_ratio = atof(stereo_ratio_f);
        printf("stereo ratio: %f\n",stereo_ratio);
    }
    char* negmod_f = get_value_by(cfg,"MPX","negmod");
    if(negmod_f){
        neg_mod = atof(negmod_f);
        printf("pilot tone negative modulation: %f\n",neg_mod);
    }

    char* c1_mpx_f = get_value_by(cfg,"MPX","c1_mpx");
    if(c1_mpx_f){
        c1_MPX = atof(c1_mpx_f);
        printf("channel 1 MPX: %d\n",c1_MPX);
    }
    char* c2_mpx_f = get_value_by(cfg,"MPX","c2_mpx");
    if(c2_mpx_f){
        c2_MPX = atof(c2_mpx_f);
        printf("channel 1 MPX: %d\n",c2_MPX);
    }
    char* alias_f = get_value_by(cfg,"MPX","antialias");
    if(alias_f){
        mpx_anti_alias = atoi(alias_f);
        printf("composite anti-aliasing %d\n",mpx_anti_alias);
    }
    char* aliasw1_f = get_value_by(cfg,"MPX","a0");
    if(aliasw1_f){
        cv_frame[0] = atof(aliasw1_f);
        printf("aliasing buffer 0:%f\n",cv_frame[0]);
    }
    char* aliasw2_f = get_value_by(cfg,"MPX","a1");
    if(aliasw2_f){
        cv_frame[1] = atof(aliasw2_f);
        printf("aliasing buffer 1:%f\n",cv_frame[1]);
    }
    char* aliasw3_f = get_value_by(cfg,"MPX","a2");
    if(aliasw3_f){
        cv_frame[2] = atof(aliasw3_f);
        printf("aliasing buffer 2:%f\n",cv_frame[2]);
    }







    //limiter
    char* pre_amp_f = get_value_by(cfg,"limiter","pre_amp");
    if(pre_amp_f){
        pre_amp = atof(pre_amp_f);
        printf("pre amp: %f\n",pre_amp);
    }
    char* bass_cut_f = get_value_by(cfg,"limiter","basscut");
    if(bass_cut_f){
        bass_cut = atof(bass_cut_f);
        printf("pre limiter bass cut(Hz): %f\n",bass_cut);
    }

    char* lookahead_f = get_value_by(cfg,"limiter","lookahead");
    if(lookahead_f){
        lookahead = atoi(lookahead_f);
        printf("lookahead: %d\n",lookahead);
    }
    char* post_amp_f = get_value_by(cfg,"limiter","post_amp");
    if(post_amp_f){
        post_amp = atof(post_amp_f);
        printf("post amp: %f\n",post_amp);
    }
    char* limit_value_f = get_value_by(cfg,"limiter","limit");
    if(limit_value_f){
        limit_value = atof(limit_value_f);
        printf("limit: %f\n",limit_value);
    }
    char* harmonic_diff_f = get_value_by(cfg,"limiter","save_harmonics");
    if(harmonic_diff_f){
        harmonic_diff = atof(harmonic_diff_f);
        printf("harmonic diff: %f\n",harmonic_diff);
    }

    char* bins_f = get_value_by(cfg,"limiter","bins");
    if(bins_f){
        int bin_r = atoi(bins_f);
        if(bin_r == 5 || bin_r == 9 || bin_r == 15 || bin_r == 30 || bin_r == 45){
            bins = bin_r;
        }
        printf("bins: %d\n",bins);
    }

    //gain controller
    char* attack_f = get_value_by(cfg,"agc","attack");
    if(attack_f){
        attack = atof(attack_f);
        printf("attack: %f\n",attack);
    }
    char* release_f = get_value_by(cfg,"agc","release");
    if(release_f){
        release = atof(release_f);
        printf("release: %f\n",release);
    }
    char* target_f = get_value_by(cfg,"agc","target");
    if(target_f){
        target = atof(target_f);
        printf("target: %f\n",target);
    }
    char* noise_th_f = get_value_by(cfg,"agc","noise_th");
    if(noise_th_f){
        noise_th = atof(noise_th_f);
        printf("noise th: %f\n",noise_th);
    }

    //high pass filter
    char* alpha_f = get_value_by(cfg,"highpass","alpha");
    if(alpha_f){
        alpha = atof(alpha_f);
        printf("alpha: %f\n",alpha);
    }
    //bass boost
    char* bass_boost_f = get_value_by(cfg,"bassboost","boost");
    if(bass_boost_f){
        bass_boost = atof(bass_boost_f);
        printf("bass boost: %f\n",bass_boost);
    }

    free_cfg_mem(cfg);

}
//setup the eq and limiter release
void setup_from_config(float* eq,float* release,char* file){

    CfgRaster cfg = read_cfg_file(file);
    if(!cfg)
        return;
    char val[13];
    for(int i = 0;i<bins;i++){
        sprintf(val,"%d",i);

        char* e_val = get_value_by(cfg,"eq",val);
        char* r_val = get_value_by(cfg,"release",val);
        if(e_val){
            eq[i] = atof(e_val);
            printf("eq %d: %f\n",i,eq[i]);
        }
        if(r_val){
            release[i] = atof(r_val);
            printf("release %d: %.10f\n",i,release[i]);
        }

    }

    free_cfg_mem(cfg);

}
int main(int argn,char* argv[]){

    if(argn >= 2){
            setup_globals_from_config(argv[1]);
    }else{
        sprintf(recording,"default");
        sprintf(playback,"default");
    }
    pre_set_settings();

    int mpx_b = 189999;

    int ch1 = 2;
    int ch2 = 2;
    int rate1 = 48000;// mandatory
    int rate2 = 192000;// mandatory
    int input_buffer_prop = rate2/rate1;
    int i_buffer_size = rate1/input_buffer_prop;
    int half_b = i_buffer_size/2;

    float ratio = ((float)((i_buffer_size+discard_samples)/2.0f))/((float)(i_buffer_size/2.0f));


    int* sample_buff = malloc(sizeof(int)*(i_buffer_size+discard_samples));
    memset(sample_buff,0, sizeof(int)*(i_buffer_size+discard_samples));
    int samp_size = i_buffer_size+discard_samples;

    int* recbuff = malloc(sizeof(int)*(i_buffer_size));
    int* recbuff_end = recbuff+i_buffer_size;
    memset(recbuff,0, sizeof(int)*(i_buffer_size));
    float* midbuff_m = malloc(sizeof(float)*half_b);
    float* midbuff_s = malloc(sizeof(float)*half_b);
    memset(midbuff_m,0,sizeof(float)*half_b);
    memset(midbuff_s,0,sizeof(float)*half_b);
    int* output = malloc(sizeof(int)*rate1);


    //equalizer, programatically controlled
    float eq[bins];
    float eq_s[bins];
    float eq_helper[bins];
    float eq_helper_s[bins];
    pre_eq = malloc(sizeof(float)*bins);
    l_release = malloc(sizeof(float)*bins);
    for(int i = 0;i<bins;i++){
        eq[i] = 1;
        eq_s[i] = 1;
        pre_eq[i] = 1;
        l_release[i] = 0.5;
    }
    //user setting eq

    //default setting eq 45 bins
    /*
    pre_eq[0]=0.0001;
    pre_eq[1]=0.001;
    pre_eq[2]=0.001;
    pre_eq[3]=0.001;
    pre_eq[4]=0.001;
    pre_eq[5]=0.1;
    pre_eq[6]=0.6;
    pre_eq[7]=0.8;
    pre_eq[8]=0.9;

    pre_eq[40]=1.5;
    pre_eq[41]=1.5;
    pre_eq[43]=1.5;
    pre_eq[44]=1.5;
    */

    //default setting for eq 9 bins
    /*if(bins == 9){
        pre_eq[0]=0.005;
        pre_eq[1]=0.2;
        pre_eq[2]=0.7;
        pre_eq[3]=0.75;
        pre_eq[4]=0.78;
        pre_eq[5]=0.8;
        pre_eq[6]=1;
        pre_eq[7]=1.1;
        pre_eq[8]=1.1;
        //release settings:
        l_release[0]=0.0000001;
        l_release[1]=0.0001;
    }*/
    //user setting eq and release
    if(argn >= 2){
        setup_from_config(eq,l_release,argv[1]);
    }

    //FFT resampling mono
    struct FFT_rsmp *rsmp = FFT_resample_init(bins,lookahead, 1000, 16000,bass_cut, rate1);
    struct FFT_rsmp *rsmp_st = FFT_resample_init(bins,lookahead, 1000, 16000,bass_cut, rate1);
    //gain controller
    struct Gain_Control *gc = gain_control_init(attack,release,target,noise_th);
    //anti aliasing to remove nasty waveforms
    struct anti_aliasing* aa_m = anti_aliasing_init(cv_frame,3);
    struct anti_aliasing* aa_m_s = anti_aliasing_init(cv_frame_clipper,6);
    struct anti_aliasing* aa_s = anti_aliasing_init(cv_frame,3);
    struct anti_aliasing* aa_s_s = anti_aliasing_init(cv_frame_clipper,6);



    //mpx syntheizer

    long double shifter_19 = ((19000.0) / (rate2*4.0))*(2*M_PI);
    long double shifter_38 = shifter_19*2;

    float* synth_19 = malloc(sizeof(float)*mpx_b);
    float* synth_38 = malloc(sizeof(float)*mpx_b);

    float* synth_38_i = synth_38;
    float* synth_19_i = synth_19;
    long double count = 0;
    for(int i = 0;i<mpx_b;i++){
        long double a19 = 0;
        long double a38 = 0;
        for(int i = 0;i<4;i++){
            a19 = a19 + sinl(shifter_19*count);
            a38 = a38 + sinl(shifter_38*count);
            count = count + 1.0;
        }
        *synth_19_i = a19/4.0;
        *synth_38_i = a38/4.0;
        synth_19_i++;
        synth_38_i++;

    }


    if(setup_alsa_pipe(recording, playback, &ch1, &ch2,&rate1, &rate2, rate1)== -1){
        printf("alsa error\n");
        goto exit;
    }else{
        printf("started main thread\n");
    }

    printf("rate1: %d rate2: %d\n",rate1,rate2);


    int mpx_count = 0;
    while(1){

        int status;
        if(!discard_samples)
            status = get_audio(recbuff,i_buffer_size);
        else
            status = get_audio(sample_buff,samp_size);

        if(status <0){
            continue;
        }


        if(discard_samples){
           int samp_size_s = samp_size>>1;
           int scount = 0;
           for(int i = 0;i<i_buffer_size;i = i+2){
                float exact_index = scount*ratio;
                int base1 = (int)exact_index;
                if(base1>=samp_size_s-1)
                    base1 = samp_size_s-1;
                float fraction = exact_index - base1;
                int top = base1+1;
                if(top>=samp_size_s)
                    top = base1;
                int index1 = base1*2;
                int index_e = top*2;
                recbuff[i] = (sample_buff[index1]*(1-fraction)) + fraction*(sample_buff[index_e]);
                recbuff[i+1] = (sample_buff[index1+1]*(1-fraction)) + fraction*(sample_buff[index_e+1]);
                scount++;

           }
        }
        //break the stereo signal into the L+R and L-R buffers
        float* i_mb = midbuff_m;
        float* i_sb = midbuff_s;
        set_gain_control(gc,target,attack,release,noise_th);
        for(int* sp = recbuff;sp<recbuff_end;sp = sp+2){

            float l = ((float)(*sp))*pre_amp;
            float r = ((float)(*(sp+1)))*pre_amp;
            hpv_r = hpv_r*nalpha+r*alpha;
            hpv_l = hpv_l*nalpha+l*alpha;
            r = r - hpv_r;
            l = l - hpv_l;
            gain_control(gc,&l,&r);
            bhpv_r = bhpv_r*nalpha+r*alpha;
            bhpv_l = bhpv_l*nalpha+l*alpha;
            l = l*nbass_boost+bhpv_l*bass_boost;
            r = r*nbass_boost+bhpv_r*bass_boost;
            float sum = l+r;
            float diff = l-r;
            if(mpx_anti_alias){
                sum = aliasing(aa_m,sum);
                diff = aliasing(aa_s,diff);

            }

            *i_mb = sum;
            *i_sb = diff;
            i_mb++;
            i_sb++;

        }
        //calculate the i and r FFT coefficients for each
        i_mb = midbuff_m;
        i_sb = midbuff_s;
        float suma;

        for(int i = 0;i<half_b;i++){

            float* amplitude = resamp_pre_process(rsmp, *i_mb,pre_eq);
            suma = find_amp(amplitude,bins);

            adjust_eq(eq,amplitude,rsmp->rastoyanee,bins,limit_value,suma,eq_helper,l_release,harmonic_diff);
            float val = resamp_get_signal(rsmp,eq);


            amplitude = resamp_pre_process(rsmp_st, *i_sb,pre_eq);
            suma = find_amp(amplitude,bins);

            adjust_eq(eq_s,amplitude,rsmp_st->rastoyanee,bins,limit_value,suma,eq_helper_s,l_release,harmonic_diff);
            float stval = resamp_get_signal(rsmp_st,eq_s);

            *i_mb = val*post_amp;
            *i_sb = stval*post_amp;
            i_mb++;
            i_sb++;

        }
        //sample up to 192khz and add the MPX signals

        int* oi = output;
        i_mb = midbuff_m;
        i_sb = midbuff_s;

        float limit_audio = (1-(pilot_amp+fabs(neg_mod)));
        float pilot_v = pilot_amp*int_value;
        for(int i = 0;i<half_b;i++){
            float mono_i = *i_mb;
            float stereo = *i_sb;
            i_mb++;i_sb++;


            float st_abs = fabs(stereo);
            float m_abs = fabs(mono_i);
            float sum = m_abs+st_abs;

            float ratios = 1;
            float ratiom = 1;

            float mono =mono_i;

            if(sum > int_value_attack){
                float overflow = sum - int_value_attack;
                float app_ratio = overflow/int_range;
                if(app_ratio>1)
                    app_ratio = 1;
                float orig_ratio = 1- app_ratio;
                ratios = st_abs/(st_abs+fabs(m_abs));
                ratiom = 1-ratios;

                float lim_st = int_value*ratios;
                float lim_m = int_value*ratiom;

                float n_st = stereo*ratios;
                if(n_st>lim_st){
                    n_st = lim_st;
                }else if(n_st<-lim_st){
                    n_st = -lim_st;
                }

                float nmon = mono_i*ratiom;
                if(nmon>lim_m){//when there is clipping mirror it back :)
                    nmon = lim_m;
                }else if(nmon<-lim_m){
                    nmon = -lim_m;
                }
                if(mpx_anti_alias){
                    n_st = aliasing(aa_m_s,n_st);
                    nmon = aliasing(aa_s_s,nmon);
                }

                stereo = (n_st)*app_ratio + stereo*orig_ratio;
                mono_i = (nmon)*app_ratio + mono_i*orig_ratio;
            }else if(mpx_anti_alias){
                aliasing(aa_m_s,stereo);
                aliasing(aa_s_s,mono_i);
            }


            float nmod = mono*neg_mod;
            float p_amp = (pilot_v-nmod);

            //put 8 samples for 192 khz
            for(int i2 = 0;i2<4;i2++){
                float w38 = synth_38[mpx_count];
                float w19 = synth_19[mpx_count];

                float sample = w19*p_amp+(w38*stereo+mono)*limit_audio;
                //float sample = stereo+mono;
                if(c1_MPX)
                    *oi = sample;
                else
                    *oi = mono_i;
                oi++;
                if(c2_MPX)
                    *oi = sample;
                else
                    *oi = mono_i;
                oi++;
                mpx_count++;
                if(mpx_count >= mpx_b)
                    mpx_count = 0;
            }
        }
        status = queue_audio(output);



    }

exit:
    free_resamp(rsmp);
    free_resamp(rsmp_st);

    free(sample_buff);

    free_gain_control(gc);

    free_aliasing(aa_m);
    free_aliasing(aa_m_s);
    free_aliasing(aa_s);
    free_aliasing(aa_s_s);

    free(pre_eq);
    free(recbuff);
    free(midbuff_m);
    free(midbuff_s);
    free(output);
    free(synth_19);
    free(synth_38);
    alsa_pipe_exit();
}
