//
//  main.c
//  OS_Assignment2
//
//  Created by Nehir Poyraz on 18.03.2018.
//  Copyright © 2018 Nehir Poyraz. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "main.h"
#define NUM_SEAT 3
#define SLEEP_MAX 5
#define SEM_TA "sem_ta"
#define SEM_STU "sem_stu"
#define NUM_STU 15


// semaphores
sem_t *sem_student;
sem_t *sem_ta;

// mutex
pthread_mutex_t mutex;

int freeSeats = NUM_SEAT;   // number of available seats in the waiting area
int seat[NUM_SEAT];         // elements of the array are studentIDs
int *next_seat, *next_teach;

int main(int argc, const char * argv[]) {
    next_seat = seat;
    next_teach = seat;
    
    //threads
    pthread_t *students;
    pthread_t TA;

    int numstu;
    int *id;
    if(argc != 2) {
        fprintf(stderr,"Number of students is not specified. Using default (%d)\n", NUM_STU);
        numstu = NUM_STU;
    }
    else if (atoi(argv[1]) > 0){
        numstu = atoi(argv[1]);
    }
    else{
        fprintf(stderr,"Argument must be a positive integer. Using default (%d)\n", NUM_STU);
        numstu = NUM_STU;
    }
    
    students = (pthread_t*)malloc(sizeof(pthread_t) * numstu);
    id = (int*)malloc(sizeof(int) * numstu);
    
//  sem_open - initialize and open a named semaphore
    sem_student = sem_open(SEM_STU, O_CREAT, 0644, 0);
    sem_ta = sem_open(SEM_TA, O_CREAT, 0644, 1);
    
//  initialize mutex
    pthread_mutex_init(&mutex,NULL);
    
//  create threads
    if(pthread_create(&TA, NULL, TA_runner, NULL) != 0)
        fprintf(stderr,"Thread could not be created.\n");
    for(int i = 0; i < numstu; i++){
        id[i] = i+1;
        if(pthread_create(&students[i], NULL, student_runner, (void*) &id[i]) != 0)
            fprintf(stderr,"Thread could not be created.\n");
    }
//   wait for the threads to finish
    pthread_join(TA, NULL);
    for(int i = 0; i < numstu; i++)
        pthread_join(students[i], NULL);
    
//  free the memory allocated for the pointer
    free(students);

    return 0;
}

void *TA_runner() {
    /* this function is run by the TA */
    while(1) {
        sem_wait(sem_student);          // waiting for student semaphore to be 1 (studentReady)
        pthread_mutex_lock(&mutex);     // mutex on the chairs
        int studentID = *next_teach;    // next student to teach
        if(studentID != 0){             // 0 is an invalid studentID, indicating that the seat is empty
            printf("TA is teaching student %d\n", studentID);
            *next_teach = 0;            // empty the seat
            freeSeats++;                // increase the number of available seats
            printf("Waiting students: [%d] [%d] [%d]\n", seat[0], seat[1], seat[2]);
            if (next_teach == &seat[NUM_SEAT-1])
                next_teach = seat;
            else
                next_teach++;
            printf("TA finished teaching student %d\n", studentID);
        }
        pthread_mutex_unlock(&mutex);   // unlock the mutex
        sem_post(sem_ta);               // signal TA ready to teach other students, increase sem_ta
    }
}

void *student_runner(void *id) {
    /* this function is run by the student threads */
    int studentID = *(int*) id;
    printf("Student %d is programming\n", studentID);
    while(1) {
        rand_sleep();
        pthread_mutex_lock(&mutex);     // mutex on the chairs
//            int waiting = isWaiting(studentID);
        if(freeSeats > 0) {
//                if(!waiting) {
            *next_seat = studentID;     // sit on the next seat
            freeSeats--;                // decrease the number of available seats
            printf("Student %d is waiting\n", studentID);
            printf("Waiting students: [%d] [%d] [%d]\n", seat[0], seat[1], seat[2]);
            if (next_seat == &seat[NUM_SEAT-1])
                next_seat = seat;
            else
                next_seat++;
//                }
            pthread_mutex_unlock(&mutex);   // unlock the mutex
            sem_post(sem_student);          // signal student ready, increase sem_ta
            sem_wait(sem_ta);               // wait for sem_ta
        }
        else{
            pthread_mutex_unlock(&mutex);
//                if(!waiting)
                printf("No available seats, student %d will come back another time\n", studentID);
            rand_sleep();
        }
    }
}


void rand_sleep(void){
    int time = rand() % SLEEP_MAX + 1;
    sleep(time);
}

int isWaiting(int id) {
    for (int i = 0; i < NUM_SEAT; i++){
        if (seat[i] == id)
            return 1;
    }
    return 0;
}





