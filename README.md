# Serveur TCP multi-clients en C avec poll()

## Description

Ce projet implémente un serveur TCP en C capable de gérer plusieurs clients simultanément à l’aide de la fonction `poll()`.
Chaque client se connecte, choisit un pseudo, puis peut échanger des messages en temps réel avec les autres clients connectés.
Le serveur relaie (broadcast) tous les messages reçus à tous les autres clients.

Un client TCP interactif en ligne de commande est également fourni pour se connecter au serveur, envoyer un pseudo, et communiquer avec les autres clients.


## Fonctionnalités

* Gestion de plusieurs clients simultanés avec `poll()`
* Attribution d’un pseudo unique à chaque client après connexion
* Broadcast des messages avec identification par pseudo
* Gestion propre des connexions et déconnexions
* Client interactif capable d’envoyer et recevoir des messages en temps réel
* Commande `exit` côté client pour fermer proprement la connexion


## Prérequis

* Système Linux (Ubuntu recommandé)
* Compilateur C (gcc)
* Terminal pour lancer serveur et clients

## Compilation

### Serveur

```bash
gcc server.c -o serveur
```

### Client

```bash
gcc client.c -o client
```

## Utilisation

### Lancer le serveur

```bash
./serveur
```

Le serveur écoute sur le port 8084 (modifiable dans le code).

### Lancer un client

Dans un autre terminal :

```bash
./client
```

1. Le client reçoit une demande de pseudo : entrez un nom d’utilisateur.
2. Le serveur confirme la connexion.
3. Vous pouvez ensuite taper des messages qui seront envoyés à tous les autres clients connectés.
4. Tapez `exit` pour quitter proprement.


## Architecture

* **server.c** : serveur TCP multi-clients utilisant `poll()`, gestion des pseudos, diffusion des messages.
* **client.c** : client TCP interactif utilisant `poll()` pour gérer entrée utilisateur et messages serveur simultanément.


## Améliorations possibles

* Ajouter commandes spéciales (`/list` des clients, `/quit`, etc.)
* Gestion d’authentification / mots de passe
* Interface graphique
* Journalisation avancée
* Support IPv6

## Licence

Projet personnel – libre à modifier et adapter.

## Auteur

**El Hadji Abdou DRAME**

