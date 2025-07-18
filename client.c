#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8084
#define BUF_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in serv_addr;
    char buffer[BUF_SIZE];

    // Création du socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Erreur lors de la création du socket");
        exit(EXIT_FAILURE);
    }

    // Configuration de l'adresse du serveur
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Adresse invalide");
        exit(EXIT_FAILURE);
    }

    // Connexion au serveur
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Échec de la connexion");
        exit(EXIT_FAILURE);
    }

    // Réception de la demande de pseudo
    int n = recv(sock, buffer, BUF_SIZE - 1, 0);
    if (n <= 0) {
        perror("Erreur reception demande pseudo");
        close(sock);
        exit(EXIT_FAILURE);
    }
    buffer[n] = '\0';
    printf("%s", buffer);

    // Lire le pseudo depuis stdin et l'envoyer
    if (fgets(buffer, BUF_SIZE, stdin) == NULL) {
        printf("Erreur lecture pseudo\n");
        close(sock);
        exit(EXIT_FAILURE);
    }
    send(sock, buffer, strlen(buffer), 0);

    // Réception du message de bienvenue
    n = recv(sock, buffer, BUF_SIZE - 1, 0);
    if (n > 0) {
        buffer[n] = '\0';
        printf("%s", buffer);
    }

    // Préparer poll sur stdin et socket
    struct pollfd fds[2];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[1].fd = sock;
    fds[1].events = POLLIN;

    printf("Tapez vos messages ('exit' pour quitter) :\n");

    while (1) {
        int ret = poll(fds, 2, -1);
        if (ret < 0) {
            perror("Erreur dans poll()");
            break;
        }

        if (fds[0].revents & POLLIN) {
            if (fgets(buffer, BUF_SIZE, stdin) == NULL) break;

            if (strncmp(buffer, "exit", 4) == 0) {
                printf("Déconnexion demandée.\n");
                break;
            }

            send(sock, buffer, strlen(buffer), 0);
        }

        if (fds[1].revents & POLLIN) {
            int len = recv(sock, buffer, BUF_SIZE - 1, 0);
            if (len <= 0) {
                printf("Serveur déconnecté.\n");
                break;
            }
            buffer[len] = '\0';
            printf("%s", buffer);
        }
    }

    close(sock);
    printf("Connexion fermée proprement.\n");
    return 0;
}
