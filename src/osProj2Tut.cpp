
#include <cstddef>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_GUESTS 5
#define NUM_ROOMS 3

sem_t check_in;
sem_t check_out;
sem_t rooms;

int totalGuests = 0;
int poolCount = 0;
int restaurantCount = 0;
int fitnessCenterCount = 0;
int businessCenterCount = 0;

// Shared data structure for passing information between threads
typedef struct {
  int guest_id;
  int room_id;
} GuestInfo;

GuestInfo check_in_info;
GuestInfo check_out_info;

int room_status[NUM_ROOMS] = {0}; // Array to keep track of room statuses

void *guest(void *arg) {
  int guest_id = *(int *)arg;
  printf("Guest %d enters the hotel.\n", guest_id);
  totalGuests++;

  sem_wait(&check_in);
  check_in_info.guest_id = guest_id; // Pass guest ID to check-in thread
  printf("Guest %d goes to the check-in reservationist.\n", guest_id);
  sem_wait(&rooms);
  // Assign an unoccupied room to the guest
  for (int i = 0; i < NUM_ROOMS; i++) {
    if (room_status[i] == 0) {
      check_in_info.room_id = i;
      room_status[i] = 1; // Mark room as occupied
      break;
    }
  }
  printf("Guest %d receives Room %d and completes check-in.\n", guest_id,
         check_in_info.room_id);
  sem_post(&check_in);

  // Hotel activity
  printf("Guest %d goes to the swimming pool.\n", guest_id);
  poolCount++;
  sleep(rand() % 3 + 1);

  sem_wait(&check_out);
  check_out_info.guest_id =
      guest_id; // Pass guest ID and room number to check-out thread
  check_out_info.room_id = check_in_info.room_id;
  printf("Guest %d goes to the check-out reservationist and returns Room %d.\n",
         guest_id, check_out_info.room_id);
  room_status[check_out_info.room_id] = 0; // Mark room as unoccupied
  sem_post(&rooms);
  printf("Guest %d receives the receipt.\n", guest_id);
  sem_post(&check_out);

  return NULL;
}

void *check_in_thread(void *arg) {
  while (totalGuests < NUM_GUESTS) {
    sem_wait(&check_in);
    printf("The check-in reservationist greets Guest %d.\n",
           check_in_info.guest_id);
    printf("Check-in reservationist assigns Room %d to Guest %d.\n",
           check_in_info.room_id, check_in_info.guest_id);
    sem_post(&check_in);
  }
}

void *check_out_thread(void *arg) {
  while (totalGuests < NUM_GUESTS) {
    sem_wait(&check_out);
    printf("The check-out reservationist greets Guest %d and receives the key "
           "from Room %d.\n",
           check_out_info.guest_id, check_out_info.room_id);
    printf("The receipt was printed.\n");
    sem_post(&check_out);
  }
}

int main() {
  pthread_t guests[NUM_GUESTS];
  pthread_t check_in_res;
  pthread_t check_out_res;
  int guest_ids[NUM_GUESTS];

  sem_init(&check_in, 0, 1);
  sem_init(&check_out, 0, 1);
  sem_init(&rooms, 0, NUM_ROOMS);

  pthread_create(&check_in_res, NULL, check_in_thread, NULL);
  pthread_create(&check_out_res, NULL, check_out_thread, NULL);

  // Create all guest threads at once
  for (int i = 0; i < NUM_GUESTS; i++) {
    guest_ids[i] = i;
    pthread_create(&guests[i], NULL, guest, &guest_ids[i]);
  }

  // Wait for all guest threads to finish
  for (int i = 0; i < NUM_GUESTS; i++) {
    pthread_join(guests[i], NULL);
  }

  sem_destroy(&check_in);
  sem_destroy(&check_out);
  sem_destroy(&rooms);

  // Print accounting information
  printf("\nAccounting:\n");
  printf("Number of Customers: %d\n", totalGuests);
  printf("Total Guests: %d\n", NUM_GUESTS);
  printf("Pool: %d\n", poolCount);
  printf("Restaurant: %d\n", restaurantCount);
  printf("Fitness Center: %d\n", fitnessCenterCount);
  printf("Business Center: %d\n", businessCenterCount);

  return 0;
}
