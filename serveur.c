#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>
#include <netinet/in.h>

#define PORT 8084
#define MAX_CLIENTS 10
#define BUF_SIZE 1024
#define PSEUDO_LEN 32

typedef struct {
    int fd;
    char pseudo[PSEUDO_LEN];
} Client;

int main() {
    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUF_SIZE];

    Client clients[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].fd = -1;
        clients[i].pseudo[0] = '\0';
    }

    // ->  Création socket serveur
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    // ->  Configuration adresse serveur
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    // ->  bind()
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind()");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // ->  listen()
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen()");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Serveur en écoute sur le port %d...\n", PORT);

    struct pollfd fds[MAX_CLIENTS + 1];
    for (int i = 0; i <= MAX_CLIENTS; i++) {
        fds[i].fd = -1;
        fds[i].events = POLLIN;
    }
    fds[0].fd = server_fd;
    int nfds = 1;

    while (1) {
        int activity = poll(fds, nfds, -1);
        if (activity < 0) {
            perror("poll()");
            break;
        }

        // ->  Nouvelle connexion entrante
        if (fds[0].revents & POLLIN) {
            int new_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
            if (new_fd < 0) {
                perror("accept()");
                continue;
            }

            // ->  Trouver une place libre
            int slot = -1;
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].fd == -1) {
                    slot = i;
                    break;
                }
            }

            if (slot == -1) {
                printf("Trop de clients, connexion refusée.\n");
                close(new_fd);
            } else {
                clients[slot].fd = new_fd;
                clients[slot].pseudo[0] = '\0'; // ->  pseudo pas encore reçu

                if (slot + 1 >= nfds)
                    nfds = slot + 2; // ->  +1 pour serveur, +1 pour indice

                fds[slot + 1].fd = new_fd;
                fds[slot + 1].events = POLLIN;

                printf("[+] Nouveau client connecté: fd=%d, IP=%s, port=%d\n",
                       new_fd,
                       inet_ntoa(client_addr.sin_addr),
                       ntohs(client_addr.sin_port));

                // ->  Demander le pseudo au client
                const char *ask_pseudo = "Entrez votre pseudo (max 31 caractères) : ";
                send(new_fd, ask_pseudo, strlen(ask_pseudo), 0);
            }
        }

        // ->  Gérer les messages clients
        for (int i = 1; i < nfds; i++) {
            int fd = fds[i].fd;
            if (fd == -1)
                continue;

            if (fds[i].revents & POLLIN) {
                int bytes = recv(fd, buffer, BUF_SIZE - 1, 0);
                if (bytes <= 0) {
                    // ->  Client déconnecté
                    printf("[-] Client %s (fd=%d) déconnecté.\n",
                           clients[i-1].pseudo[0] ? clients[i-1].pseudo : "(sans pseudo)",
                           fd);
                    close(fd);
                    fds[i].fd = -1;
                    clients[i-1].fd = -1;
                    clients[i-1].pseudo[0] = '\0';
                } else {
                    buffer[bytes] = '\0';

                    // ->  Si pseudo non défini, on le reçoit ici
                    if (clients[i-1].pseudo[0] == '\0') {
                        // ->  Retirer le '\n' s'il existe
                        char *newline = strchr(buffer, '\n');
                        if (newline) *newline = '\0';

                        strncpy(clients[i-1].pseudo, buffer, PSEUDO_LEN - 1);
                        clients[i-1].pseudo[PSEUDO_LEN - 1] = '\0';

                        char welcome_msg[BUF_SIZE];
                        snprintf(welcome_msg, sizeof(welcome_msg),
                                 "Bienvenue %s ! Vous pouvez maintenant envoyer des messages.\n",
                                 clients[i-1].pseudo);
                        send(fd, welcome_msg, strlen(welcome_msg), 0);

                        printf("[*] Client fd=%d a choisi le pseudo : %s\n", fd, clients[i-1].pseudo);
                    } else {
                        // ->  Message normal : ajout du pseudo puis broadcast avec saut de ligne
                        char msg_with_pseudo[BUF_SIZE + PSEUDO_LEN + 4];
                        snprintf(msg_with_pseudo, sizeof(msg_with_pseudo),
                                 "%s : %s\n", clients[i-1].pseudo, buffer);

                        printf("Message de %s (fd=%d) : %s",
                               clients[i-1].pseudo, fd, buffer);

                        // ->  Broadcast aux autres clients
                        for (int j = 1; j < nfds; j++) {
                            int other_fd = fds[j].fd;
                            if (other_fd != -1 && other_fd != fd) {
                                send(other_fd, msg_with_pseudo, strlen(msg_with_pseudo), 0);
                            }
                        }
                    }
                }
            }
        }
    }

    close(server_fd);
    return 0;
}
