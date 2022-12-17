//
// Created by Maroš Gorný on 17. 12. 2022.
//

#ifndef LANGTONSANTPOS_DISPLAY_H
#define LANGTONSANTPOS_DISPLAY_H

#include <stdbool.h>
#include "pthread.h"

typedef enum backgroundColor {
    WHITE = 0,
    BLACK = 1,
}BACKGROUND_COLOR;

typedef struct box{
    BACKGROUND_COLOR color;
    int x;
    int y;

    pthread_mutex_t* mut;
}BOX;

typedef struct display{
    int width;
    int height;

    bool directLogic;
    BOX*** box;
}DISPLAY;

//void printBackground(const int *display, int rows,int columns);
void printBackground(const BOX ***boxes, int rows,int columns);

#endif //LANGTONSANTPOS_DISPLAY_H