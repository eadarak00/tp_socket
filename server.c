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
    // -> creation du serveur
    int serv_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serv_socket < 0)
    {
        perror("Echec de la creation du serveur de socket avec socket()\n");
        exit(EXIT_FAILURE);
    }

    // -> creation de l'adresse du serveur socket = addresse IP et Port TCP
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);

    // // -> associer l'adresse IP + Port TCP au socket
    // if (bind(serv_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    // {
    //     perror("Échec du bind()");
    //     exit(EXIT_FAILURE);
    // }

    // // -> Demander l'autorisation au systeme d'ecouter des connexions
    // int MAXCONNEXION = 10;
    // if(listen(serv_socket, MAXCONNEXION) < 0){
    //     perror("Erreur listen()\n");
    //     exit(EXIT_FAILURE);
    // }
    
}