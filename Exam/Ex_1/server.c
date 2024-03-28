#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <math.h>
#include <pthread.h>

#define PORT 8080
#define MAX_MSG_SIZE 100
#define MAX_CLIENTS 10

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int client_sockets[MAX_CLIENTS];
int num_clients = 0;

void send_percentage_to_clients(double percentage) {
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < num_clients; i++) {
        send(client_sockets[i], &percentage, sizeof(double), 0);
    }
    pthread_mutex_unlock(&mutex);
}

void handle_client(int client_socket) {
    char buffer[MAX_MSG_SIZE] = {0};
    int total_messages = 0;
    int total_coverage = 0;

    FILE *fp = fopen("server_log.txt", "a");  // Open file for appending

    while (1) {
        int valread = read(client_socket, buffer, MAX_MSG_SIZE);
        if (valread == 0) {
            break;  // Client disconnected
        }

        double x, y, r;
        sscanf(buffer, "%lf %lf %lf", &x, &y, &r);

        // Calculate coverage (assuming r is the radius of the ellipse)
        double coverage = r * r * M_PI;
        total_messages++;
        total_coverage += coverage;

        printf("Received ellipse with center (%.2f, %.2f) and radius %.2f\n", x, y, r);
        printf("Total messages received: %d\n", total_messages);
        printf("Total coverage so far: %d\n", total_coverage);

        // Write to file after each ellipse
        fprintf(fp, "Ellipse %d: Center (%.2f, %.2f), Radius %.2f, Coverage %.2f\n", total_messages, x, y, r, coverage);
        fflush(fp); // Flush the file stream to ensure data is written to the file

        // Calculate and send total percentage of points covered to all clients
        double percentage_covered = ((double)total_coverage / (100 * 100)) * 100;
        send_percentage_to_clients(percentage_covered);
    }

    // Calculate percentage of points covered
    double percentage_covered = ((double)total_coverage / (100 * 100)) * 100;
    printf("Percentage of points covered by ellipses: %.2f%%\n", percentage_covered);

    // Write total messages and coverage percentage to file
    fprintf(fp, "Messages received: %d, Percentage covered: %.2f%%\n", total_messages, percentage_covered);
    fflush(fp); // Flush the file stream to ensure data is written to the file

    fclose(fp);  // Close the file
    close(client_socket);
}

void *client_thread(void *arg) {
    int client_socket = *((int *)arg);
    handle_client(client_socket);
    return NULL;
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket to port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        // Accept incoming connection
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        pthread_t tid;
        pthread_create(&tid, NULL, client_thread, &client_socket);

        pthread_mutex_lock(&mutex);
        if (num_clients < MAX_CLIENTS) {
            client_sockets[num_clients++] = client_socket;
        } else {
            close(client_socket);
        }
        pthread_mutex_unlock(&mutex);
    }

    return 0;
}
