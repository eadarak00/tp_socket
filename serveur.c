#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>

#define PORT 8084
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // -> 1. Créer socket serveur
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    // -> 2. Adresse serveur
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    // -> 3. bind()
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind()");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // -> 4. listen()
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen()");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Serveur en écoute sur le port %d...\n", PORT);

    // -> 5. Init pollfd
    struct pollfd fds[MAX_CLIENTS + 1];
    for (int i = 0; i <= MAX_CLIENTS; i++) {
        fds[i].fd = -1;
        fds[i].events = POLLIN;
    }
    fds[0].fd = server_fd;
    int nfds = 1;

    // -> 6. Boucle principale
    while (1) {
        int activity = poll(fds, nfds, -1);
        if (activity < 0) {
            perror("poll()");
            break;
        }

        // -> 7. Connexion entrante ?
        if (fds[0].revents & POLLIN) {
            client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
            if (client_fd < 0) {
                perror("accept()");
                continue;
            }

            int ajouté = 0;
            for (int i = 1; i <= MAX_CLIENTS; i++) {
                if (fds[i].fd == -1) {
                    fds[i].fd = client_fd;
                    if (i >= nfds) nfds = i + 1;
                    ajouté = 1;
                    break;
                }
            }

            if (!ajouté) {
                printf("Trop de clients. Connexion refusée.\n");
                close(client_fd);
            } else {
                char *welcome = "Bienvenue sur le serveur !\n";
                send(client_fd, welcome, strlen(welcome), 0);
                printf("Nouveau client connecté (fd %d)\n", client_fd);
            }
        }

        // -> 8. Lecture + Echo + Déconnexion
        for (int i = 1; i < nfds; i++) {
            int fd = fds[i].fd;
            if (fd == -1) continue;

            if (fds[i].revents & POLLIN) {
                int bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);

                if (bytes <= 0) {
                    // -> Client déconnecté
                    printf("Client (fd %d) déconnecté\n", fd);
                    close(fd);
                    fds[i].fd = -1;
                } else {
                    buffer[bytes] = '\0';
                    printf("Reçu de fd %d : %s", fd, buffer);

                    // -> Echo
                    send(fd, buffer, strlen(buffer), 0);
                }
            }
        }
    }

    close(server_fd);
    return 0;
}
