#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define MAX_REQUESTS 10

void generate_random_ellipse(double *x, double *y, double *r) {
    *x = (double)(rand() % 100);
    *y = (double)(rand() % 100);
    *r = (double)(rand() % 50);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <seed>\n", argv[0]);
        return 1;
    }

    unsigned int seed = atoi(argv[1]);
    srand(seed);

    double total_coverage = 0.0;

    // Create a socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Failed to create socket");
        return 1;
    }

    // Connect to the server
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to connect to server");
        return 1;
    }

    //for (int i = 0; i < MAX_REQUESTS; i++) {
        double x, y, r;
        generate_random_ellipse(&x, &y, &r);

        // Send request to server
        char request[100];
        snprintf(request, sizeof(request), "%.2f %.2f %.2f", x, y, r);
        send(sockfd, request, strlen(request), 0);

        // Receive response from server
        char response[100];
        recv(sockfd, response, sizeof(response), 0);
        double coverage = atof(response);

        total_coverage += coverage;

        printf("Request %d: Ellipse center (%.2f, %.2f), Radius %.2f, Coverage %.2f\n", i + 1, x, y, r, coverage);
        printf("Total coverage so far: %.2f\n", total_coverage);

        // Sleep for a short period to simulate processing time
        usleep(100000);
    //}

    // Close the socket
    close(sockfd);

    return 0;
}