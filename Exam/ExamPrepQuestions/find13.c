#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>


// Node structure for circular doubly linked list
typedef struct Node {
    int data;
    struct Node *prev;
    struct Node *next;
} Node;

// Linked list structure
typedef struct {
    Node *head;
} LinkedList;

// Global variables
LinkedList list;
pthread_mutex_t mutex;
int value_found = 0;

// Function to create a new node
Node *create_node(int data) {
    Node *new_node = (Node *)malloc(sizeof(Node));
    new_node->data = data;
    new_node->prev = NULL;
    new_node->next = NULL;
    return new_node;
}

// Function to insert a node at the end of the linked list
void insert_node(int data) {
    Node *new_node = create_node(data);

    if (list.head == NULL) {
        list.head = new_node;
        new_node->next = new_node;
        new_node->prev = new_node;
    } else {
        Node *last = list.head->prev;
        new_node->next = list.head;
        list.head->prev = new_node;
        new_node->prev = last;
        last->next = new_node;
    }
}

void print_list(){
    Node *start = list.head;
    printf("%d", start->data);
    Node *current = list.head->next;
    while (current != start){
        printf(" -> %d", current->data);
        current = current->next;
    }
    printf("\n");
}

// Function to search for a value in the linked list
void *search_value(void *arg) {
    int value_to_find = *((int *)arg);

    pthread_mutex_lock(&mutex);

    Node *current = list.head;
    while (current != NULL && current->data != value_to_find && !value_found) {
        current = current->next;
    }

    if (current != NULL && current->data == value_to_find) {
        printf("Found value %d at address %p\n", value_to_find, (void *)current);
        value_found = 1;
    } else {
        printf("Value %d not found\n", value_to_find);
    }

    pthread_mutex_unlock(&mutex);

    return NULL;
}

// Function to increment all values in the linked list by a given value
void *increment_values(void *arg) {
    int increment_by = *((int *)arg);

    pthread_mutex_lock(&mutex);

    Node *current = list.head;
    while (current != NULL && current != list.head && !value_found) {
        current->data += increment_by;
        current = current->next;
    }

    pthread_mutex_unlock(&mutex);
    printf("POST\n");
    print_list();

    return NULL;
}



int main() {
    pthread_t thread1, thread2;
    int search_value_thread = 25;
    int increment_value_thread = 5;

    // Initialize the linked list and mutex
    list.head = NULL;
    pthread_mutex_init(&mutex, NULL);

    // Insert nodes into the linked list
    for (int i = 1; i <= 20; i++) {
        insert_node(i);
    }

    print_list();

    // Create and run the search value thread
    pthread_create(&thread1, NULL, search_value, (void *)&search_value_thread);

    // Create and run the increment values thread
    pthread_create(&thread2, NULL, increment_values, (void *)&increment_value_thread);

    // Wait for the search value thread to finish
    pthread_join(thread1, NULL);

    // Wait for the increment values thread to finish
    pthread_join(thread2, NULL);

    // Destroy the mutex and exit
    pthread_mutex_destroy(&mutex);

    

    return 0;
}