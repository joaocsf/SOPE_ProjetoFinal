/** Version: 1.0
  * Author: Grupo T1G09
  *
*/

#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "viatura.h"

#define DIRECTORY_LENGTH 4096
#define FILE_LENGTH 255

int viatura_ID = 1;

int fileLog = 0;
int tempoInicial;
/**
*
*
*/
void * viatura_thread(void * arg){

  clock_t tInicial = clock();

  Viatura* viatura = (Viatura*)arg;

  char fifoName[DIRECTORY_LENGTH + FILE_LENGTH] ;

  sprintf(fifoName, "/tmp/viatura%d", viatura->numeroID);

  if( mkfifo(fifoName, S_IRWXU) != 0){
    perror(fifoName);
    exit(5);
  }

  sprintf(fifoName, "/tmp/fifo%c", viatura->portaEntrada);

  int fifoDestino = 0;
  if( (fifoDestino = open(fifoName, O_WRONLY)) == -1){
    perror(fifoName);
    exit (4);
  }

  if( write( fifoDestino, viatura, sizeof(Viatura) ) == -1 ){
    printf("Error Writing to FIFO Dest\n");
    exit(6);
  }

  sprintf(fifoName, "/tmp/viatura%d", viatura->numeroID);


  int fifoOrigem = 0;
  if( (fifoOrigem = open(fifoName, O_RDONLY)) == -1 ){
    perror(fifoName);
    exit(7);
  }

  char info;
  int res = 0;
  while ( (res = read(fifoOrigem, &info, sizeof(char) )) == 0);
  if(res == -1){
    printf("Error Reading fifo!");
    exit(8);
  }


  if(info == RES_ENTRADA){

    sprintf(fifoName, "%d ; %d ; %c ; %d ; ? ; entrada\n" , (int)(tempoInicial - clock()) , viatura->numeroID , viatura->portaEntrada, viatura->tempoEstacionamento);

    write(fileLog, fifoName , strlen(fifoName) );

    while ( (res = read(fifoOrigem, &info, sizeof(char) )) == 0);
    if(res == -1){
      printf("Error Reading fifo!");
      exit(8);
    }

  }else if(info == RES_CHEIO){
    sprintf(fifoName, "%d ; %d ; %c ; %d ; ? ; cheio!\n" , (int)(tempoInicial - clock()) , viatura->numeroID , viatura->portaEntrada, viatura->tempoEstacionamento);

    write(fileLog, fifoName , strlen(fifoName) );

  }else if(info == RES_ENCERRADO){
    sprintf(fifoName, "%d ; %d ; %c ; %d ; ? ; \n" , (int)(tempoInicial - clock()) , viatura->numeroID , viatura->portaEntrada, viatura->tempoEstacionamento);

    write(fileLog, fifoName , strlen(fifoName) );

  }

  if(info == RES_SAIDA){
    sprintf(fifoName, "%d ; %d ; %c ; %d ; %d ; saida\n" , (int)(tempoInicial - clock()) , viatura->numeroID , viatura->portaEntrada, viatura->tempoEstacionamento,(int)(tInicial-clock() ));

    write(fileLog, fifoName , strlen(fifoName) );

  }

  return viatura ;
}


int main(int argn, char *argv[]){

  if(argn != 3){
    printf("Error <Usage>: %s <T_GERACAO> <U_RELOGIO>\n",  argv[0]);
    return 1;
  }
  char path[DIRECTORY_LENGTH + FILE_LENGTH];
  realpath(".", path);
  sprintf(path, "%s/%s", path, "gerador.log");

  if( (fileLog = open(path, O_CREAT | O_WRONLY | O_TRUNC , S_IRWXU)) == -1){
    perror(path);
    exit(3);
  }

  write(fileLog, "t(ticks) ; id_viat ; destin ; t_estacion ; t_vida ; observ\n" ,60);



  unsigned int u_relogio = 10;
  //clock_t c_inicio = clock();

  //int t_geracao = atoi(argv[1]);

  tempoInicial = clock();

  u_relogio = atoi(argv[2]);

  int local = rand()%4;

  do{

    Viatura* v = (Viatura*)malloc(sizeof(Viatura));

    switch (local) {
      case 0:
        v->portaEntrada = 'N';
      break;
      case 1:
        v->portaEntrada = 'S';
      break;
      case 2:
        v->portaEntrada = 'E';
      break;
      case 3:
        v->portaEntrada = 'O';
      break;
    }
    v->numeroID = viatura_ID++;
    v->fifoID = v->numeroID;

    local = rand() % 100;

    if(local < 50){
      local = 0;
    }else if(local < 80){
      local = 1;
    }else if(local < 100){
      local = 2;
    }

    v->tempoEstacionamento = (rand()%10 + 1 )* u_relogio;
    pthread_t tid;
    if(pthread_create(&tid, NULL , viatura_thread , v)){
      printf("Error Creating Thread!\n");
      exit(2);
    }
    pthread_detach(tid);

  }while(0);


  return 0;
}
