
#include <cstddef>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_GUESTS 5
#define NUM_ROOMS 3
int guest_checkin = 0;
int checked_out_guests = 0;
int totalGuests = 0;
int poolCount = 0;
int restaurantCount = 0;
int fitnessCenterCount = 0;
int businessCenterCount = 0;

int room_status[NUM_ROOMS] = {0}; // Array to keep track of room statuses
int guest_rooms[NUM_GUESTS];

sem_t check_in_sem;  // Semaphore for check-in
sem_t check_out_sem; // Semaphore for check-out
sem_t check_in_ready;
sem_t check_out_done;
sem_t guest_ready;
sem_t check_in_proceed;
sem_t activity;
sem_t room;

sem_t guest_done[NUM_GUESTS];

int current_guest_id;
// make small fixes for checkout
void *guest(void *arg) {
  int guest_id = *(int *)arg;

  sem_wait(&room);
  printf("Guest %d enters the hotel.\n", guest_id);
  // Mutual exclusion only one guest can check in at a time
  //
  sem_wait(&check_in_proceed);
  totalGuests++;
  printf("Guest %d goes to the check-in reservationist.\n", guest_id);
  current_guest_id = guest_id;
  sem_post(&check_in_ready);

  sem_wait(&check_in_sem);
  // If they enter the critical section go to check-in thread.
  printf("Guest %d receives Room  and completes check-in.\n", guest_id);
  sem_post(&check_in_proceed);
  // Hotel activity
  int activity_num = (rand() % 4 + 1);
  if (activity_num == 1) {
    printf("Guest %d goes to the swimming pool.\n", guest_id);
    poolCount++;
    sleep(rand() % 3 + 1);
  }
  if (activity_num == 2) {
    printf("Guest %d goes to the fitness center.\n", guest_id);
    fitnessCenterCount++;
    sleep(rand() % 3 + 1);
  }
  if (activity_num == 3) {
    printf("Guest %d goes to the restaurant.\n", guest_id);
    restaurantCount++;
    sleep(rand() % 3 + 1);
  }
  if (activity_num == 4) {
    printf("Guest %d goes to the business center.\n", guest_id);
    businessCenterCount++;
    sleep(rand() % 3 + 1);
  }
  sem_post(&activity);
  sem_post(&guest_done[guest_id]); // Signal that guest is done
  // Can't enter checkout until check-in done and activity done
  printf("Guest %d goes to the check-out reservationist and returns Room %d.\n",
         guest_id, guest_rooms[guest_id]);

  sem_wait(&check_out_sem);
  printf("Guest %d receives the receipt.\n", guest_id);
  sem_wait(&check_out_done);

  return NULL;
}

void *check_in_thread(void *arg) {
  while (guest_checkin < NUM_GUESTS) {
    sem_wait(&check_in_ready);
    printf("The check-in reservationist greets Guest %d.\n", current_guest_id);
    // Find an available room
    int room_number = -1;
    for (int i = 0; i < NUM_ROOMS; i++) {
      if (room_status[i] == 0) {
        room_status[i] = 1; // Assign room to guest
        room_number = i;
        break;
      }
    }
    guest_rooms[guest_checkin] = room_number;
    printf("The check-in reservationist assigns Room %d to guest %d.\n",
           room_number, current_guest_id);
    sem_post(&check_in_sem);
    guest_checkin++;
  }
}

void *check_out_thread(void *arg) {
  while (checked_out_guests < NUM_GUESTS) {
    sem_wait(&activity);
    sem_post(&check_out_done);
    printf("The check-out reservationist greets Guest %d and receives the key "
           "from Room %d.\n",
           current_guest_id, guest_rooms[current_guest_id]);
    for (int i = 0; i < NUM_GUESTS; i++) {
      if (sem_trywait(&guest_done[i]) ==
          0) {                            // Try to wait for guest i to be done
        int room_number = guest_rooms[i]; // Get room number for guest
        room_status[room_number] = 0;     // Unassign room
        printf("The receipt was printed.\n");
        sem_post(&check_out_sem);
        sem_post(&room);
        checked_out_guests++;
        break; // Break out of the for loop
      }
    }
  }
}

int main() {
  pthread_t guests[NUM_GUESTS];
  pthread_t check_in_res;
  pthread_t check_out_res;

  int guest_ids[NUM_GUESTS];

  // Initialize semaphores
  sem_init(&check_in_sem, 0, 0);
  sem_init(&room, 0, 3);
  sem_init(&check_out_sem, 0, 0);
  sem_init(&check_out_done, 0, 1);
  sem_init(&check_in_ready, 0, 0);
  sem_init(&check_in_proceed, 0, 1);
  sem_init(&guest_ready, 0, 0);
  for (int i = 0; i < NUM_GUESTS; i++) {
    sem_init(&guest_done[i], 0, 0);
  }
  pthread_create(&check_in_res, NULL, check_in_thread, NULL);
  pthread_create(&check_out_res, NULL, check_out_thread, NULL);

  for (int i = 0; i < NUM_GUESTS; i++) {
    guest_ids[i] = i;
    pthread_create(&guests[i], NULL, guest, &guest_ids[i]);
  }

  for (int i = 0; i < NUM_GUESTS; i++) {
    pthread_join(guests[i], NULL);
  }
  pthread_join(check_in_res, NULL);

  // Print accounting information
  printf("\nAccounting:\n");
  printf("Number of Customers: %d\n", totalGuests);
  printf("Total Guests: %d\n", NUM_GUESTS);
  printf("Pool: %d\n", poolCount);
  printf("Restaurant: %d\n", restaurantCount);
  printf("Fitness Center: %d\n", fitnessCenterCount);
  printf("Business Center: %d\n", businessCenterCount);

  // Destroy semaphores
  sem_close(&check_in_sem);
  sem_close(&check_out_sem);

  sem_close(&check_in_proceed);

  return 0;
}
