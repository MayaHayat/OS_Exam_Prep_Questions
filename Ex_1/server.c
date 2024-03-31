#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <math.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 

#define PORT "8080"
#define MAX_MSG_SIZE 100
#define MAX_CLIENTS 10

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int client_sockets[MAX_CLIENTS];
int num_clients = 0;

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void send_percentage_to_clients(double percentage) {
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < num_clients; i++) {
        send(client_sockets[i], &percentage, sizeof(double), 0);
    }
    pthread_mutex_unlock(&mutex);
}

void handle_client(int client_socket, FILE *fp) {
    
    char buffer[MAX_MSG_SIZE];
    int total_messages = 0;
    int total_coverage = 0;

    while (1) {
        
        int valread = recv(client_socket, buffer, MAX_MSG_SIZE,0);
        if (valread <= 0) {
            perror("Couldn't get info from client");
            break;  // Client disconnected
        }
        printf("Got a connection");
        printf("\n");
        double x, y, r;
        sscanf(buffer, "%lf %lf %lf", &x, &y, &r);

        // Calculate coverage (assuming r is the radius of the ellipse)
        double coverage = r * r * M_PI;
        total_messages++;
        total_coverage += coverage;

        printf("Received ellipse with center (%.2f, %.2f) and radius %.2f\n", x, y, r);
        printf("Total messages received: %d\n", total_messages);
        printf("Total coverage so far: %d\n", total_coverage);
        fflush(stdout);

        // Write to file after each ellipse
        fprintf(fp, "Ellipse %d: Center (%.2f, %.2f), Radius %.2f, Coverage %.2f\n", total_messages, x, y, r, coverage);
        fflush(fp); // Flush the file stream to ensure data is written to the file
        if (fprintf(fp, "...") < 0) {
            perror("Error writing to file");
            // Handle the error (e.g., close file, exit)
        }


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

    close(client_socket);
}


int main(void)
{
    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    FILE* fp = fopen("server_log.txt", "a");

    int server_socket;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    char buf[256];    // buffer for client data
    int nbytes;

    char remoteIP[INET6_ADDRSTRLEN];

    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, rv;

    struct addrinfo hints, *ai, *p;

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }
    
    for(p = ai; p != NULL; p = p->ai_next) {
        server_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (server_socket < 0) { 
            continue;
        }
        
        // lose the pesky "address already in use" error message
        setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(server_socket, p->ai_addr, p->ai_addrlen) < 0) {
            close(server_socket);
            continue;
        }

        break;
    }

    // if we got here, it means we didn't get bound
    if (p == NULL) {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai); // all done with this

    // listen
    if (listen(server_socket, 10) == -1) {
        perror("listen");
        exit(3);
    }
    printf("Listening on port 8080\n");

    // add the server_socket to the master set
    FD_SET(server_socket, &master);

    // keep track of the biggest file descriptor
    fdmax = server_socket; // so far, it's this one

    // main loop
    for (;;) {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        // run through the existing connections looking for data to read
        for (i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == server_socket) {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
                    newfd = accept(server_socket,
                        (struct sockaddr *)&remoteaddr,
                        &addrlen);

                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }
                        printf("selectserver: new connection from %s on "
                            "socket %d\n",
                            inet_ntop(remoteaddr.ss_family,
                                get_in_addr((struct sockaddr*)&remoteaddr),
                                remoteIP, INET6_ADDRSTRLEN),
                            newfd);
                    }
                } else {
                    handle_client(i, fp);
                    // handle data from a client
                    // if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
                    //     // got error or connection closed by client
                    //     if (nbytes == 0) {
                    //         // connection closed
                    //         printf("selectserver: socket %d hung up\n", i);
                    //     } else {
                    //         perror("recv");
                    //     }
                    //     close(i); // bye!
                    //     FD_CLR(i, &master); // remove from master set
                    // } else {
                    //     handle_client(i, fp); // Pass the client socket and the file pointer to handle_client using the function pointer
                    // }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!

    fclose(fp);  // Close the file
    return 0;
}
