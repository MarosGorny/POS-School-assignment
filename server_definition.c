//
// Created by Maros Gorny on 28. 12. 2022.
//

#include "server_definition.h"
#include "settings.h"
#include <string.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include <errno.h>
#include <stdlib.h>

char *endMsg = ":end";

void writeStateOfSharedData(DATA* pdata, int socket){
    printLog("void writeStateOfSharedData(DATA* pdata, int socket)");
    char buffer[BUFFER_LENGTH + 1];
    buffer[BUFFER_LENGTH] = '\0';
    int userNameLength = strlen(pdata->userName);
    char* action = "[Shared date state]";
    sprintf(buffer, "%s%s: %d %d %d %d %d %d %d %d %d", pdata->userName\
                                                        ,action\
                                                        ,pdata->continueSimulation\
                                                        ,pdata->columns\
                                                        ,pdata->rows\
                                                        ,pdata->numberOfAnts\
                                                        ,pdata->loadingType\
                                                        ,pdata->logicType\
                                                        ,pdata->stop\
                                                        ,pdata->ready\
                                                        ,pdata->step);
    char *pos = strchr(buffer, '\n');
    if (pos != NULL) {
        *pos = '\0';
    }
    write(socket, buffer, strlen(buffer) + 1);
    printf("State data sended\n");
};


void *data_writeData(void *data) {
    printLog("void *data_writeData(void *data)");
    DATA *pdata = (DATA *)data;
    pthread_mutex_lock(&pdata->mutex);
    int idClient = pdata->numberOfClients - 1;
    pthread_mutex_unlock(&pdata->mutex);

    //pre pripad, ze chceme poslat viac dat, ako je kapacita buffra
    fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL, 0) | O_NONBLOCK);
    printData(pdata);


    while(true) {
        pthread_mutex_lock(&pdata->mutex);
        pthread_cond_wait(&pdata->updateClients,&pdata->mutex);
        //pthread_cond_signal(&pdata->startListening[idClient]);
        writeStateOfSharedData(pdata,pdata->sockets[idClient]);
        pthread_mutex_unlock(&pdata->mutex);
    }






    //initSimulationSetting(pdata);
    //writeMsgToAllParticipants(pdata);

    fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL, 0) & ~O_NONBLOCK);

    return NULL;
}

void *data_readData(void *data) {
    printLog("void *data_readData(void *data)");
    DATA *pdata = (DATA *)data;
    pthread_mutex_lock(&pdata->mutex);
    int idClient = pdata->numberOfClients - 1;
    pthread_mutex_unlock(&pdata->mutex);
    printf("ID client %d\n",idClient);





//    pthread_mutex_lock(&pdata->mutex);
//    pthread_cond_wait(&pdata->startListening[idClient],&pdata->mutex);
//    printf("readData - after wait\n");
//    pthread_mutex_unlock(&pdata->mutex);


    char buffer[BUFFER_LENGTH + 1];
    buffer[BUFFER_LENGTH] = '\0';
    while(!data_isStopped(pdata)) {
        bzero(buffer, BUFFER_LENGTH);
        printf("%s\n",buffer);

        if (read(pdata->sockets[idClient], buffer, BUFFER_LENGTH) > 0) {

            if(checkIfQuit(buffer,pdata)) {

            } else if(checkIfPause(buffer,pdata)) {

            }  else if(checkIfContinue(buffer,pdata)) {

            } else {
                printf("%s \n", buffer);
            }


            printf("Before action\n");
            makeAction(buffer,pdata);
            pthread_cond_broadcast(&pdata->updateClients);

        }
        else {
            printf("ELSE STOP\n");
            data_stop(pdata);
        }

    }

    return NULL;
}

void makeAction(char* buffer, DATA *pdata) {
    printf("STEP:%d\n",pdata->step);
    printLog("CLIENT: void makeAction(char* buffer, DATA *pdata)");
    char *target = NULL;
    char *posActionStart = strchr(buffer, '[');
    char *posActionEnd;

    pthread_mutex_lock(&pdata->mutex);
    if (posActionStart != NULL) {
        //posActionStart += 1;
        posActionEnd = strchr(buffer, ']');

        target = ( char * )malloc( posActionEnd - posActionStart + 2 );
        memcpy( target, posActionStart, posActionEnd - posActionStart +1);
        target[posActionEnd - posActionStart + 1] = '\0';

        if (strcmp(target,"[Shared date state]") == 0) {
            //LOGIC_TYPE logicType = (LOGIC_TYPE) atoi(posActionEnd + 2) -1;
            //pdata->logicType = logicType;
            //printf("Changed logic type: %d\n",pdata->logicType);


            // Extract the first token
            char* token = strtok(posActionEnd+2," ");
            // loop through the string to extract all other tokens
            for (int i = 0; i < 6; i++) {
                int tempNumber = atoi(token);
                if(i == 0) {
                    pdata->continueSimulation = tempNumber;
                } else if(i == 1) {
                    pdata->columns = tempNumber;
                } else if(i == 2) {
                    pdata->rows = tempNumber;
                } else if(i ==3) {
                    pdata->numberOfAnts = tempNumber;
                } else if (i ==4) {
                    pdata->loadingType = tempNumber;
                } else if (i == 5) {
                    pdata->logicType = tempNumber;
                }
                token = strtok(NULL, " ");
            }
        }
        printData(pdata);

        if (strcmp(target,"[Number of ants]") == 0) {
            pdata->numberOfAnts = atoi(posActionEnd + 2);
            printf("Changed number of ants: %d\n",pdata->numberOfAnts);
            data_written(pdata);
            pdata->step++;
        }

        if (strcmp(target,"[Loading type]") == 0) {
            LOADING_TYPE loadingType = (LOADING_TYPE) atoi(posActionEnd + 2) - 1;
            pdata->loadingType = loadingType;
            printf("Changed loading type: %d\n",pdata->loadingType);
            data_written(pdata);
            pdata->step++;
        }

        if (strcmp(target,"[Logic type]") == 0) {
            LOGIC_TYPE logicType = (LOGIC_TYPE) atoi(posActionEnd + 2) -1;
            pdata->logicType = logicType;
            printf("Changed logic type: %d\n",pdata->logicType);
            data_written(pdata);
            pdata->step++;
        }

        if (strcmp(target,"[Dimensions]") == 0) {
            char *columnsCharPointer = strchr(posActionEnd+3,' ');
            if ( columnsCharPointer ) printf( "%s\n", columnsCharPointer );
            if(columnsCharPointer == NULL) {
                pdata->columns = atoi(posActionEnd+2);
            } else {
                pdata->columns = atoi(columnsCharPointer);
            }
            if ( posActionEnd+2 ) printf( "%s\n", posActionEnd+2 );
            pdata->rows = atoi(posActionEnd+2);

            printf("Changed rows: %d columns: %d\n",pdata->rows,pdata->columns);
            data_written(pdata);
            pdata->step++;
        }

        if (strcmp(target,"[READY]") == 0) {
            char *columnsCharPointer = strchr(posActionEnd,' ');
            int temp = atoi(columnsCharPointer);
            if (temp == 1) {
                printf("ANTS ARE READY\n");
                pthread_cond_signal(&pdata->startAntSimulation);
                data_written(pdata);
                pdata->step++;
            } else {
                printf("ANTS ARE NOT READY\n");
            }
        }
//        printLog("bool checkIfQuit(char* buffer,DATA *pdata)");
//        char *posSemi = strchr(buffer, ':');
//        char *pos = strstr(posSemi + 1, endMsg);
//        if (pos != NULL && pos - posSemi == 2 && *(pos + strlen(endMsg)) == '\0') {
//            *(pos - 2) = '\0';
//            printf("Pouzivatel %s ukoncil komunikaciu.\n", buffer);
//            data_stop(pdata);
//        }
        //pdata->step++;
    }

    //if ( target ) printf( "%s\n", target );

    pthread_mutex_unlock(&pdata->mutex);
    printf("After mutex unlock\n");


    free( target );
}

bool checkIfQuit(char* buffer,DATA *pdata) {
    printLog("bool checkIfQuit(char* buffer,DATA *pdata)");
    char *posSemi = strchr(buffer, ':');
    char *pos = strstr(posSemi + 1, endMsg);
    if (pos != NULL && pos - posSemi == 2 && *(pos + strlen(endMsg)) == '\0') {
        *(pos - 2) = '\0';
        printf("Pouzivatel %s ukoncil komunikaciu.\n", buffer);
        data_stop(pdata);
        return true;


    }
    else {
        return false;
    }
}


bool checkIfPause(char* buffer,DATA *pdata) {
    printLog("bool checkIfPause(char* buffer,DATA *pdata)");
    char *posSemi = strchr(buffer, ':');
    char *pos = strstr(posSemi + 1, ":pause");
    if (pos != NULL && pos - posSemi == 2 && *(pos + strlen(":pause")) == '\0') {
        *(pos - 2) = '\0';
        printf("Pouzivatel %s pauzol simulaciu.\n", buffer);
        printf("%d\n",pthread_mutex_lock(&pdata->mutex));
        pdata->continueSimulation = 0;
        printf("Contunie simulation = 0\n");
        pthread_mutex_unlock(&pdata->mutex);
        printf("Po pauznuti\n");
        return true;
    }
    else {
        return false;
    }
}




bool checkIfContinue(char* buffer,DATA *pdata) {
    printLog("bool checkIfContinue(char* buffer,DATA *pdata)");
    char *posSemi = strchr(buffer, ':');
    char *pos = strstr(posSemi + 1, ":continue");
    if (pos != NULL && pos - posSemi == 2 && *(pos + strlen(":continue")) == '\0') {
        *(pos - 2) = '\0';
        printf("Pouzivatel %s chce pokracovat.\n", buffer);
        pthread_mutex_lock(&pdata->mutex);
        pdata->continueSimulation = 1;
        pthread_cond_broadcast(&pdata->continueAntSimulation);
        pthread_mutex_unlock(&pdata->mutex);
        return true;
    }
    else {
        return false;
    }
}

void* writeMsgToAllParticipants(DATA* pdata) {
    printLog("void* writeMsgToAllParticipantsClient(DATA* pdata)");
    char buffer[BUFFER_LENGTH + 1];
    buffer[BUFFER_LENGTH] = '\0';
    int userNameLength = strlen(pdata->userName);
    fd_set inputs;
    FD_ZERO(&inputs);
    struct timeval tv;
    tv.tv_usec = 0;
    while(!data_isStopped(pdata)) {
        tv.tv_sec = 1;
        FD_SET(STDIN_FILENO, &inputs);
        select(STDIN_FILENO + 1, &inputs, NULL, NULL, &tv);
        if (FD_ISSET(STDIN_FILENO, &inputs)) {

            sprintf(buffer, "%s: ", pdata->userName);
            char *textStart = buffer + (userNameLength + 2);
            while (fgets(textStart, BUFFER_LENGTH - (userNameLength + 2), stdin) > 0) {
                char *pos = strchr(textStart, '\n');
                if (pos != NULL) {
                    *pos = '\0';
                }
                printf("Written msg: %s\n",buffer);

                for (int i = 0; i < 2; ++i) {
                    write(pdata->sockets[i], buffer, strlen(buffer) + 1);
                }


                if (strstr(textStart, endMsg) == textStart && strlen(textStart) == strlen(endMsg)) {
                    printf("Koniec komunikacie.\n");
                    data_stop(pdata);
                }

                if (strstr(textStart, ":pause") == textStart && strlen(textStart) == strlen(":pause")) {
                    printf("Simulacia pozastavena.\n");
                    pthread_mutex_lock(&pdata->mutex);
                    pdata->continueSimulation = 0;
                    printf("Continue simulation = 0\n");
                    pthread_mutex_unlock(&pdata->mutex);
                }

                if (strstr(textStart, ":continue") == textStart && strlen(textStart) == strlen(":continue")) {
                    printf("Simulacia pokracuje.\n");
                    pthread_mutex_lock(&pdata->mutex);
                    pdata->continueSimulation = 1;
                    printf("Continue simulation = 1\n");
                    pthread_cond_broadcast(&pdata->continueAntSimulation);
                    printf("AFter broadcast\n");
                    pthread_mutex_unlock(&pdata->mutex);
                }
            }
        }
    }
}

int data_isStopped(DATA *data) {
    //printLog("int data_isStopped(DATA *data)");
    int stop;
    pthread_mutex_lock(&data->mutex);
    stop = data->stop;
    pthread_mutex_unlock(&data->mutex);
    return stop;
}


void initSimulationSetting(DATA* pdata) {
    printLog("void initSimulationSetting(DATA* pdata)");


    if(pdata->numberOfAnts <= 0) {
        writeToSocketByAction(pdata,NUMBER_OF_ANTS_ACTION);
    }

    printLog("after: writeToSocketByAction(pdata,NUMBER_OF_ANTS_ACTION)");
    if(pdata->loadingType == NOT_SELECTED_LOADING_TYPE) {
        writeToSocketByAction(pdata,LOADING_TYPE_ACTION);
    }

    printLog("after: writeToSocketByAction(pdata,LOADING_TYPE_ACTION)");
    if(pdata->logicType == NOT_SELECTED_ANTS_LOGIC) {
        writeToSocketByAction(pdata,LOGIC_TYPE_ACTION);
    }

    printLog("after: writeToSocketByAction(pdata,LOGIC_TYPE_ACTION)");
    if(pdata->columns <= 0 || pdata->rows <= 0) {
        writeToSocketByAction(pdata,DIMENSION_ACTION);
    }

    printLog("after: writeToSocketByAction(pdata,DIMENSION_ACTION)");
    writeToSocketByAction(pdata,READY_ACTION);
}
char* printActionQuestionByCode(ACTION_CODE actionCode) {

    if(actionCode == NUMBER_OF_ANTS_ACTION) {
        printf("\nHow many ants do you want in simulation? [write number and press enter]\n");
        return "[Number of ants]";
    } else if (actionCode == LOADING_TYPE_ACTION) {
        printf("\nHow do you want start simulation? [write number and press enter]\n");
        printf("1: All squares are white.\n");
        printf("2: Square color is random.\n");
        printf("3: Select black squares through the terminal input.\n");
        printf("4: Load dimension and squares colors from file.\n");
        printf("Q: Quit simulation\n");
        return "[Loading type]";
    } else if (actionCode == LOGIC_TYPE_ACTION) {
        printf("\nWhich logic of ants do you want? [write number and press enter]\n");
        printf("1: Direct logic.\n");
        printf("2: Inverted logic.\n");
        printf("Q: Quit simulation\n");
        return "[Logic type]";
    } else if (actionCode == DIMENSION_ACTION) {
        printf("\nSelect dimensions of ground, [write rows and columns, divided by space\n");
        return "[Dimensions]";
    } else if (actionCode == READY_ACTION) {
        printf("\nAre you ready for start? [write 1 for yes, 2 for no and press enter]\n");
        return "[READY]";
    }

    return "";
}


void writeToSocketByAction(DATA* pdata,ACTION_CODE actionCode) {
    printLog("void writeToSocketByAction(DATA* pdata,ACTION_CODE actionCode)");
    char buffer[BUFFER_LENGTH + 1];
    buffer[BUFFER_LENGTH] = '\0';
    int userNameLength = strlen(pdata->userName);
    char* action;

    action = printActionQuestionByCode(actionCode);

    fd_set inputs;
    FD_ZERO(&inputs);
    struct timeval tv;
    tv.tv_usec = 0;
    while(!data_isStopped(pdata)) {
        if(data_isWritten(pdata)) {
            break;
        }

        tv.tv_sec = 1;
        FD_SET(STDIN_FILENO, &inputs);
        select(STDIN_FILENO + 1, &inputs, NULL, NULL, &tv);
        int countCharAfterName = 2 + strlen(action);
        if (FD_ISSET(STDIN_FILENO, &inputs)) {
            sprintf(buffer, "%s%s: ", pdata->userName,action);
            char *textStart = buffer + (userNameLength + countCharAfterName);
            while (fgets(textStart, BUFFER_LENGTH - (userNameLength + countCharAfterName), stdin) > 0) {
                char *pos = strchr(textStart, '\n');
                if (pos != NULL) {
                    *pos = '\0';
                }

                if (strstr(textStart, endMsg) == textStart && strlen(textStart) == strlen(endMsg)) {
                    printf("Written msg before simulation: %s\n",buffer);
                    for (int i = 0; i < 2; i++) {
                        write(pdata->sockets[i], buffer, strlen(buffer) + 1);
                    }

                    printf("End of communication.\n");
                    data_stop(pdata);
                } else {
                    if(writeToSocketAndSetSharedAntsData(pdata,actionCode,buffer,textStart)) {
                        data_written(pdata);
                        goto exit;
                    }

                }
            }
        }
    }
    exit:
    reset_written(pdata);
}

void data_stop(DATA *data) {
    printLog("CLIENT: void data_stop(DATA *data)");
    pthread_mutex_lock(&data->mutex);
    data->stop = 1;
    printf("Stop -> 1\n");
    pthread_mutex_unlock(&data->mutex);
}


bool writeToSocketAndSetSharedAntsData(DATA* pdata,ACTION_CODE actionCode, char* buffer, char* textStart) {
    printLog("writeToSocketAndSetSharedAntsData(DATA* pdata,ACTION_CODE actionCode, char* buffer, char* textStart)");
    if(actionCode == NUMBER_OF_ANTS_ACTION) {
        int temp = atoi(textStart);
        if(temp > 0) {

            write(pdata->socket, buffer, strlen(buffer) + 1);
            //data_written(pdata);
            pdata->numberOfAnts = temp;
            return true;
        }
    } else if (actionCode == LOADING_TYPE_ACTION) {
        LOADING_TYPE tempLoadType = 100;
        switch (textStart[0]) {
            case '1':
                printf("All squares are white.\n");
                tempLoadType = ALL_WHITE;
                break;
            case'2':
                printf("Square color is random.\n");
                tempLoadType = RANDOM_COLOR;
                break;
            case'3':
                printf("Select black square through the terminal input.\n");
                tempLoadType = TERMINAL_INPUT;
                break;
            case'4':
                printf("Load dimension and squares color from file.\n");
                tempLoadType = FILE_INPUT;
                break;
            case'Q':
            case'q':
                printf("Closing simulation...\n");
                tempLoadType = NOT_SELECTED_LOADING_TYPE;
                break;
            default:
                printf("Selection is unknown. Try again please.\n");
                break;
        }
        if (tempLoadType != 100) {
            write(pdata->socket, buffer, strlen(buffer) + 1);
            //data_written(pdata);
            pdata->loadingType = tempLoadType;
            //reset_written(pdata);
            return true;
        }
    } else if (actionCode == LOGIC_TYPE_ACTION) {
        LOGIC_TYPE tempLogicType = 100;
        switch (textStart[0]) {
            case '1':
                printf("Logic of ants is direct.\n");
                tempLogicType = DIRECT;
                break;
            case '2':
                printf("Logic of ants is inverted.\n");
                tempLogicType =  INVERT;
                break;
            case 'q':
            case 'Q':
                printf("Closing simulation...\n");
                tempLogicType =  NOT_SELECTED_ANTS_LOGIC;
                break;
            default:
                printf("Selection is unknown. Try again please.\n");
                break;
        }
        if (tempLogicType != 100) {
            write(pdata->socket, buffer, strlen(buffer) + 1);
            //data_written(pdata);
            pdata->logicType = tempLogicType;
            //reset_written(pdata);
            return true;
        }
    } else if (actionCode == DIMENSION_ACTION) {
        int rows;
        int columns;
        char *columnsCharPointer = strchr(textStart,' ');
        rows = atoi(textStart);
        if(columnsCharPointer == NULL) {
            columns = atoi(textStart);
        } else {
            columns = atoi(columnsCharPointer);
        }
        if(columns > 0 && rows > 0) {
            write(pdata->socket, buffer, strlen(buffer) + 1);
            //data_written(pdata);
            pdata->rows = rows;
            pdata->columns = columns;
            //reset_written(pdata);
            return true;
        }
    } else if (actionCode == READY_ACTION) {
        int temp = atoi(textStart);
        if(temp == 2 || temp == 1) {
            write(pdata->socket, buffer, strlen(buffer) + 1);
            //data_written(pdata);
            //reset_written(pdata);
            return true;
        }
    } else {
        write(pdata->socket, buffer, strlen(buffer) + 1);
        return false;
    }
    return false;

}


void data_written(DATA *data) {
    printLog("CLIENT: void data_written(DATA *data)");
    pthread_mutex_lock(&data->writtenMutex);
    data->written = 1;
    printf("Written -> 1\n");
    pthread_mutex_unlock(&data->writtenMutex);
}


void reset_written(DATA *data) {
    pthread_mutex_lock(&data->writtenMutex);
    data->written = 0;
    pthread_mutex_unlock(&data->writtenMutex);
}

int data_isWritten(DATA *data) {
    int written;
    pthread_mutex_lock(&data->writtenMutex);
    written = data->written;
    //printf("%d written\n",written);
    pthread_mutex_unlock(&data->writtenMutex);
    return written;
}


void printError(char *str) {
    if (errno != 0) {
        perror(str);
    }
    else {
        fprintf(stderr, "%s\n", str);
    }
    exit(EXIT_FAILURE);
}

void data_destroy(DATA *data) {
    pthread_mutex_destroy(&data->mutex);
    pthread_mutex_destroy(&data->writtenMutex);
    pthread_cond_destroy(&data->startAntSimulation);
    pthread_cond_destroy(&data->continueAntSimulation);
}

void data_init(DATA *data, const char* userName) {
    printLog("void data_init(DATA *data, const char* userName, const int socket)");;
    data->stop = 0;
    data->continueSimulation = 1;
    data->written = 0;
    data->userName[USER_LENGTH] = '\0';
    data->rows = 0;
    data->columns = 0;
    data->numberOfAnts = 0;
    data->loadingType = NOT_SELECTED_LOADING_TYPE;
    data->logicType = NOT_SELECTED_ANTS_LOGIC;

    strncpy(data->userName, userName, USER_LENGTH);
    pthread_mutex_init(&data->mutex, NULL);
    pthread_mutex_init(&data->writtenMutex, NULL);
    pthread_cond_init(&data->startAntSimulation, NULL);
    printf("PREDDD\n");
    pthread_cond_init(&data->continueAntSimulation, NULL);
    printf("PREDDD\n");
}





void printData(DATA* pdata) {
    printLog("printData(DATA* pdata)");
    printf( "%s: con:%d col:%d row:%d ants:%d loaTyp%d logTyp%d\n", pdata->userName\
                                                        ,pdata->continueSimulation\
                                                        ,pdata->columns\
                                                        ,pdata->rows\
                                                        ,pdata->numberOfAnts\
                                                        ,pdata->loadingType\
                                                        ,pdata->logicType);
}
