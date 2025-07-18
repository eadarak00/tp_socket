#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 11000
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char *message = "Bienvenue sur le serveur TCP !\n";

    // 1. Créer le socket serveur
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket() a échoué");
        exit(EXIT_FAILURE);
    }

    // 2. Préparer l'adresse serveur
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;                // IPv4
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Toutes interfaces
    server_addr.sin_port = htons(PORT);              // Port TCP

    // 3. Lier le socket au port
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind() a échoué");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 4. Écouter les connexions entrantes
    if (listen(server_fd, 1) < 0) {
        perror("listen() a échoué");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Serveur en écoute sur le port %d...\n", PORT);

    // 5. Accepter une seule connexion client
    client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd < 0) {
        perror("accept() a échoué");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Client connecté\n");

    // 6. Envoyer un message de bienvenue
    send(client_fd, message, strlen(message), 0);
    printf("Message de bienvenue envoyé au client.\n");

    // 7. Fermer les connexions
    close(client_fd);
    close(server_fd);
    printf("Connexion fermée proprement.\n");

    return 0;
}
