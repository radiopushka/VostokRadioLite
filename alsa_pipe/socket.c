
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 8469

 struct sockaddr_in serv_addr;
 int sock_id;

 int setup_connection(){
     //setup connection to the player's server
      if ((sock_id = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
       }
       serv_addr.sin_family = AF_INET;
       serv_addr.sin_port = htons(PORT);

       if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {

            return -1;
        }
       if (connect(sock_id, (struct sockaddr*)&serv_addr,sizeof(serv_addr))< 0) {
        return -1;
    }
       return 1;
 }

int snd_audio_rx(int* data, unsigned int size){
  int status;
  if((status=send(sock_id,&size,sizeof(unsigned int),0))<0){
    return -1;
  }

  return read(sock_id,data,sizeof(int)*size);
}
void close_sock(){
    close(sock_id);
}
