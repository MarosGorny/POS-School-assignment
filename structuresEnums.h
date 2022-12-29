//
// Created by Maroš Gorný on 23. 12. 2022.
//

#ifndef LANGTONSANTPOS_STRUCTURESENUMS_H
#define LANGTONSANTPOS_STRUCTURESENUMS_H

#define BUFSIZE 4096
#define SERVER_BACKLOG 2

#include "pthread.h"
#include <arpa/inet.h>
//#include "client_server_definitions.h"

#define USER_LENGTH 10
#define BUFFER_LENGTH 300
extern char *endMsg;


typedef enum loadingType {
    NOT_SELECTED_LOADING_TYPE = -1,
    ALL_WHITE = 0,
    RANDOM_COLOR = 1,
    TERMINAL_INPUT = 2,
    FILE_INPUT = 3,
}LOADING_TYPE;

typedef enum logicType {
    NOT_SELECTED_ANTS_LOGIC = -1,
    DIRECT = 0,
    INVERT = 1,
}LOGIC_TYPE;

typedef enum direction {
    NORTH = 0,
    EAST = 1,
    SOUTH = 2,
    WEST = 3,
}ANT_DIRECTION;

typedef enum backgroundColor {
    WHITE = 0,
    BLACK = 1,
}BACKGROUND_COLOR;

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

typedef struct acceptData {
    int serverSocket;
    SA_IN* client_addr;
    int* addr_size;
}ACCEPT_DATA;

typedef struct data {
    char userName[USER_LENGTH + 1];
    pthread_mutex_t mutex;
    int numberOfClients;
    int* sockets;
    int socket;
    int stop;
    int continueSimulation;

    int columns;
    int rows;
    int numberOfAnts;

    LOADING_TYPE loadingType;
    LOGIC_TYPE logicType;
    pthread_mutex_t writtenMutex;



    int written;
    pthread_cond_t startAntSimulation;
    pthread_cond_t continueAntSimulation;
    pthread_cond_t* startListening;
    pthread_cond_t updateClients;
    ACCEPT_DATA* acceptDataForServer;
    int step;
    int ready;
} DATA;


typedef  struct box{
    BACKGROUND_COLOR color;
    int x;
    int y;
    //TODO LINKED LIST NA ANTS??

    pthread_mutex_t* mut;
}BOX;

typedef enum actionCode {
    UKNOWN_ACTION = -1,
    NUMBER_OF_ANTS_ACTION = 1,
    LOADING_TYPE_ACTION = 2,
    LOGIC_TYPE_ACTION = 3,
    DIMENSION_ACTION = 4,
    READY_ACTION = 5,
}ACTION_CODE;

typedef struct display{
    int width;
    int height;
    int actualNumberOfAnts;

    pthread_barrier_t* barriers;
    pthread_barrier_t* mainBarrier;
    pthread_mutex_t* mut;


    LOGIC_TYPE logicType;
    BOX*** box;
    DATA* dataSocket;
}DISPLAY;

typedef struct ant {
    int id;
    int x;
    int y;
    ANT_DIRECTION direction;


    DISPLAY* display;
}ANT;


#endif //LANGTONSANTPOS_STRUCTURESENUMS_H
