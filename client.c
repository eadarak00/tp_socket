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

    // 1. Création du socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Erreur lors de la création du socket");
        exit(EXIT_FAILURE);
    }

    // 2. Configuration de l'adresse du serveur
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // 3. Convertir l'adresse IP et la définir (ici: localhost)
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Adresse invalide");
        exit(EXIT_FAILURE);
    }

    // 4. Connexion au serveur
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Échec de la connexion");
        exit(EXIT_FAILURE);
    }

    // 5. Réception et affichage du message de bienvenue
    int n = recv(sock, buffer, BUF_SIZE - 1, 0);
    if (n > 0) {
        buffer[n] = '\0';
        printf("Message du serveur : %s\n", buffer);
    }

    // 6. Utilisation de poll() pour stdin et socket
    struct pollfd fds[2];
    fds[0].fd = STDIN_FILENO; // Entrée clavier
    fds[0].events = POLLIN;

    fds[1].fd = sock;         // Socket réseau
    fds[1].events = POLLIN;

    printf("Tapez vos messages ('exit' pour quitter) :\n");

    while (1) {
        int ret = poll(fds, 2, -1);
        if (ret < 0) {
            perror("Erreur dans poll()");
            break;
        }

        // Saisie utilisateur
        if (fds[0].revents & POLLIN) {
            if (fgets(buffer, BUF_SIZE, stdin) == NULL) {
                break;
            }

            if (strncmp(buffer, "exit", 4) == 0) {
                printf("Déconnexion demandée.\n");
                break;
            }

            send(sock, buffer, strlen(buffer), 0);
        }

        // Message du serveur
        if (fds[1].revents & POLLIN) {
            int len = recv(sock, buffer, BUF_SIZE - 1, 0);
            if (len <= 0) {
                printf("Serveur déconnecté.\n");
                break;
            }
            buffer[len] = '\0';
            printf("Réponse : %s", buffer);
        }
    }

    // Fermeture propre
    close(sock);
    printf("Connexion fermée proprement.\n");

    return 0;
}
