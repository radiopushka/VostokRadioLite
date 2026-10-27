//simple Lookahead limiter
#ifndef Lookahead_LIM
#define Lookahead_LIM

struct Lookahead{
  int buff_size;
  float* ring_buffer;
  int l_index;
  float gain;
};

struct Lookahead* init_Lookahead(int size);
float run_Lookahead(struct Lookahead* obj,float input,float limit,float release);
float run_Lookahead_delay(struct Lookahead* obj,float input);
void free_Lookahead(struct Lookahead* obj);

#endif // !Lookahead_LIM
