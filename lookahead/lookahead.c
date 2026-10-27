#include "lookahead.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

struct Lookahead* init_Lookahead(int size){
  if(size < 1)
    return NULL;

  struct Lookahead* strt = malloc(sizeof(struct Lookahead));
  
  //init
  strt->gain=1.0f;
  strt->buff_size=size;
  strt->l_index=0;
  strt->ring_buffer=malloc(sizeof(float)*size);

  //create buffer
  memset(strt->ring_buffer,0,sizeof(float)*size);
  
  
  return strt;
}
float run_Lookahead(struct Lookahead* obj,float input,float limit,float release){
  //pull out the current sample
  int index_tracking=obj->l_index;
  int bsize=obj->buff_size;
  float sample_tek=obj->ring_buffer[index_tracking];
  index_tracking++;
  if(index_tracking>=bsize)
      index_tracking=0;

  float max_val=fabsf(sample_tek);
  int distance=0;

  //loop through the ring buffer and find the max value
  //find it's distance from the current sample
  for(int i=1;i<bsize;i++){
    float cval=fabsf(obj->ring_buffer[index_tracking]);
    index_tracking++;
    if(index_tracking>=bsize)
      index_tracking=0;
    if(cval>max_val){
      max_val=cval;
      distance=i;
    }
  }
  //place our value at the start of the buffer, increment,check overflow;
  obj->ring_buffer[index_tracking]=input;
  index_tracking++;
  if(index_tracking>=bsize)
      index_tracking=0;

  obj->l_index=index_tracking;
  //get out the gain and scale the max value
  float gain=obj->gain;
  float old_mval=max_val;
  max_val=max_val*gain;

  //limiting/clipping algorithm
  if(max_val>limit){
    //calculate the slope of the attack
    float t_gain=limit/old_mval;
    float gain_delta=gain-t_gain;
    float ngi=gain_delta/(distance+1.0f);
    gain=gain-ngi;

  }else if(max_val<limit){
    gain += release;
    float cap = limit / old_mval;
    if (gain > cap) gain = cap;
    if (gain > 1.0f) gain = 1.0f;
  }


  obj->gain=gain;


  //scale the sample
  float rval=sample_tek*gain;
  //clip
  if(rval > limit)
    rval=limit;
  if(rval < -limit)
    rval=-limit;
  return rval;


}
float run_Lookahead_delay(struct Lookahead* obj,float input){
  //pull out the current sample
  int index_tracking=obj->l_index;
  int bsize=obj->buff_size;
  float sample_tek=obj->ring_buffer[index_tracking];
  obj->ring_buffer[index_tracking]=input;
  index_tracking++;
  if(index_tracking>=bsize)
      index_tracking=0;

  obj->l_index=index_tracking;
 
  float rval=sample_tek;

  return rval;


}

void free_Lookahead(struct Lookahead* obj){
  free(obj->ring_buffer);
  free(obj);
}
