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

// semaphores
sem_t *sem_student;
sem_t *sem_ta;

// mutex
pthread_mutex_t mutex;

int freeSeats = NUM_SEAT;
int seat[NUM_SEAT];
int next_seat = 0;
int next_teach = 0;

int main(int argc, const char * argv[]) {
    
    //threads
    pthread_t *students;
    pthread_t TA;

    int numstu;
    int *id;
    
    printf("How many students?\t");
    scanf("%d", &numstu);
    
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
        sem_wait(sem_student);
        pthread_mutex_lock(&mutex);
        int studentID = seat[next_teach];
        if(studentID != 0){
            printf("TA is teaching student %d\n", studentID);
            seat[next_teach]=0;
            freeSeats++;
            printf("Waiting students: [%d] [%d] [%d]\n", seat[0], seat[1], seat[2]);
            next_teach = (next_teach + 1) % NUM_SEAT;
            rand_sleep();
            printf("TA finished teaching student %d\n", studentID);
        }
        pthread_mutex_unlock(&mutex);
        sem_post(sem_ta);
    }
}

void *student_runner(void *id) {
    /* this function is run by the student threads */
    int studentID = *(int*) id;
    printf("Student %d is programming\n", studentID);
    while(1) {
        rand_sleep();
        pthread_mutex_lock(&mutex);
        if(freeSeats > 0) {
            seat[next_seat] = studentID;
            freeSeats--;
            printf("Student %d is waiting\n", studentID);
            printf("Waiting students: [%d] [%d] [%d]\n", seat[0], seat[1], seat[2]);
            next_seat = (next_seat+1) % NUM_SEAT;
            pthread_mutex_unlock(&mutex);
            sem_post(sem_student);
            sem_wait(sem_ta);
        }
        else{
            pthread_mutex_unlock(&mutex);
            printf("No available seats, student %d will come back another time\n", studentID);
        }
    }
}

void rand_sleep(void){
    int time = rand() % SLEEP_MAX + 1;
    sleep(time);
}





