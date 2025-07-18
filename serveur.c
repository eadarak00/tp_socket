#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>

#define PORT 11000
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // -> 1.Créer le socket serveur
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket() a échoué");
        exit(EXIT_FAILURE);
    }

    // -> 2. Préparer l'adresse
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    // -> 3. Lier l'adresse
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind() a échoué");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // -> 4. Écouter
    if (listen(server_fd, MAX_CLIENTS) < 0)
    {
        perror("listen() a échoué");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Serveur en écoute sur le port %d...\n", PORT);

    // -> 5. Initialiser le tableau pollfd
    struct pollfd fds[MAX_CLIENTS + 1];
    for (int i = 0; i <= MAX_CLIENTS; i++)
    {
        fds[i].fd = -1;
        fds[i].events = POLLIN;
    }
    fds[0].fd = server_fd;

    int nfds = 1;

   // -> 6. Boucle principale
    while (1) {
        int activity = poll(fds, nfds, -1);
        if (activity < 0) {
            perror("poll() a échoué");
            break;
        }

        // -> 7. Nouvelle connexion
        if (fds[0].revents & POLLIN) {
            client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
            if (client_fd < 0) {
                perror("accept() a échoué");
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
                printf("Nouveau client connecté (fd %d)\n", client_fd);
                send(client_fd, "Bienvenue sur le serveur multi-client !\n", 40, 0);
            }
        }

        // -> 8. Messages des clients
        for (int i = 1; i < nfds; i++) {
            if (fds[i].fd == -1) continue;

            if (fds[i].revents & POLLIN) {
                int bytes = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);

                if (bytes <= 0) {
                    printf("Client (fd %d) déconnecté\n", fds[i].fd);
                    close(fds[i].fd);
                    fds[i].fd = -1;
                } else {
                    buffer[bytes] = '\0';
                    printf("Client %d a dit : %s", fds[i].fd, buffer);
                    send(fds[i].fd, "Reçu.\n", 7, 0);
                }
            }
        }
    }

    close(server_fd);
    return 0;
}
