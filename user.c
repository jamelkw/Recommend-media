#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "sqlite3.h" // Bibliothèque pour la gestion de la base de données SQLite

#define LONGUEUR_MAX 100        // Longueur maximale pour les chaînes de caractères
#define MAX_USERNAME 50         // Taille maximale pour le nom d'utilisateur
#define MAX_NAME 50             // Taille maximale pour le nom et prénom
#define MAX_PASSWORD 100       // Taille maximale pour le mot de passe

// Structure représentant un utilisateur avec des informations détaillées
typedef struct {
    int id;                        // Identifiant unique de l'utilisateur
    char username[MAX_USERNAME];    // Nom d'utilisateur (doit être unique)
    char nom[MAX_NAME];             // Nom de famille de l'utilisateur
    char prenom[MAX_NAME];          // Prénom de l'utilisateur
    int age;                        // Âge de l'utilisateur
    char motDePasse[MAX_PASSWORD];  // Mot de passe (doit être chiffré avant stockage)
} Utilisateur;

/**
 * Vérifier si un mot de passe respecte les critères de sécurité.
 * Le mot de passe doit contenir au moins :
 * - 8 caractères,
 * - une majuscule,
 * - une minuscule,
 * - un chiffre,
 * - un caractère spécial parmi les suivants : !@#$%^&*.
 */
int verifierMotDePasse(const char *motDePasse) {
    int i;
    int longueur = strlen(motDePasse);   // On récupère la longueur du mot de passe
    int majuscule = 0, minuscule = 0, chiffre = 0, caractereSpecial = 0;

    // Si la longueur est inférieure à 8 caractères, on retourne 0 (mot de passe invalide)
    if (longueur < 8) return 0;

    // On vérifie les différents critères du mot de passe
    for (i = 0; i < longueur; i++) {
        if (isupper(motDePasse[i])) majuscule = 1;  // Vérifie s'il y a une majuscule
        if (islower(motDePasse[i])) minuscule = 1;  // Vérifie s'il y a une minuscule
        if (isdigit(motDePasse[i])) chiffre = 1;     // Vérifie s'il y a un chiffre
        if (strchr("!@#$%^&*", motDePasse[i])) caractereSpecial = 1; // Vérifie un caractère spécial
    }

    // Si tous les critères sont remplis, retourne 1 (mot de passe valide)
    return majuscule && minuscule && chiffre && caractereSpecial;
}

/**
 * Chiffrer un mot de passe avant stockage.
 * Utilise un algorithme de chiffrement simple en appliquant un décalage de 3 caractères
 * sur chaque caractère du mot de passe.
 */
void chiffrerMotDePasse(char *motDePasse) {
    // Algorithme simple de chiffrement : décalage de 3 caractères pour chaque caractère
    int i;
    for ( i = 0; motDePasse[i] != '\0'; i++) {
        motDePasse[i] += 3;  // Décale chaque caractère du mot de passe de 3 positions
    }
}

/**
 * Créer un nouvel utilisateur en demandant ses informations et en les enregistrant
 * dans la base de données après validation.
 */
int creerUtilisateur() {
    Utilisateur user;

    // Demande à l'utilisateur son nom d'utilisateur
    printf("Nom d'utilisateur : ");
    scanf("%s", user.username);

    // Demande à l'utilisateur son nom
    printf("Nom : ");
    scanf("%s", user.nom);

    // Demande à l'utilisateur son prénom
    printf("Prénom : ");
    scanf("%s", user.prenom);

    // Demande à l'utilisateur son âge
    printf("Âge : ");
    scanf("%d", &user.age);

    // Demande à l'utilisateur son mot de passe
    printf("Mot de passe : ");
    scanf("%s", user.motDePasse);

    // Vérifie si le mot de passe respecte les critères de sécurité
    if (!verifierMotDePasse(user.motDePasse)) {
        printf("Mot de passe non sécurisé !\n");  // Si non sécurisé, on avertit l'utilisateur
        return 0;  // Retourne 0 si le mot de passe est invalide
    }

    // Chiffre le mot de passe avant de l'enregistrer
    chiffrerMotDePasse(user.motDePasse);

    // Sauvegarde l'utilisateur dans la base de données SQLite et retourne le résultat
    return sauvegarderUtilisateurSQL(user);
}

/**
 * Sauvegarder un utilisateur dans la base de données SQLite après sa création.
 * Cette fonction prépare et exécute une requête SQL pour insérer l'utilisateur dans la table.
 */
int sauvegarderUtilisateurSQL(Utilisateur utilisateur) {
    sqlite3 *db;              // Pointeur pour la base de données SQLite
    char requete[256];        // Chaîne de caractères pour la requête SQL
    int resultat = 0;         // Résultat de l'exécution de la requête

    // Ouvre la base de données
    if (sqlite3_open("utilisateurs.db", &db) == SQLITE_OK) {
        // Prépare la requête SQL pour insérer un nouvel utilisateur
        snprintf(requete, sizeof(requete),
                 "INSERT INTO utilisateurs (username, nom, prenom, age, motDePasse) VALUES ('%s', '%s', '%s', %d, '%s')",
                 utilisateur.username, utilisateur.nom, utilisateur.prenom, utilisateur.age, utilisateur.motDePasse);

        // Exécute la requête
        if (sqlite3_exec(db, requete, NULL, NULL, NULL) == SQLITE_OK) {
            resultat = 1;  // Si l'insertion réussit, retourne 1
        }
    }

    // Ferme la base de données
    sqlite3_close(db);

    return resultat;  // Retourne le résultat de l'insertion (1 pour succès, 0 pour échec)
}

/**
 * Charger les utilisateurs depuis la base de données SQLite.
 * Cette fonction se contente d'ouvrir la base de données et de la fermer pour l'instant.
 */
int chargerUtilisateursSQL() {
    sqlite3 *db;

    // Ouvre la base de données
    if (sqlite3_open("utilisateurs.db", &db) != SQLITE_OK) {
        return -1;  // Retourne -1 en cas d'erreur d'ouverture
    }

    // Ferme la base de données
    sqlite3_close(db);

    return 0;  // Retourne 0 si la base de données est correctement ouverte et fermée
}

/**
 * Authentifier un utilisateur en vérifiant son nom d'utilisateur et son mot de passe.
 * La fonction vérifie dans la base de données si les identifiants sont corrects.
 */
int authentifierUtilisateur(const char *username, const char *motDePasse) {
    sqlite3 *db;              // Pointeur pour la base de données SQLite
    sqlite3_stmt *stmt;       // Pointeur pour l'instruction préparée (requête)
    char requete[256];        // Chaîne de caractères pour la requête SQL
    int resultat = -1;        // Résultat de l'authentification (-1 signifie échec)

    // Ouvre la base de données
    if (sqlite3_open("utilisateurs.db", &db) == SQLITE_OK) {
        // Prépare une requête SQL pour rechercher le mot de passe de l'utilisateur dans la base de données
        snprintf(requete, sizeof(requete), "SELECT motDePasse FROM utilisateurs WHERE username = '%s'", username);

        // Prépare l'instruction SQL
        if (sqlite3_prepare_v2(db, requete, -1, &stmt, NULL) == SQLITE_OK) {
            // Exécute la requête
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                // Récupère le mot de passe stocké dans la base de données
                const char *motDePasseStocke = (const char *)sqlite3_column_text(stmt, 0);

                // Compare le mot de passe saisi avec celui stocké
                if (strcmp(motDePasse, motDePasseStocke) == 0) {
                    resultat = 1;  // Si les mots de passe sont identiques, l'utilisateur est authentifié
                }
            }
        }

        // Finalise l'instruction préparée
        sqlite3_finalize(stmt);
    }

    // Ferme la base de données
    sqlite3_close(db);

    return resultat;  // Retourne 1 si authentification réussie, sinon -1
}

/**
 * Supprimer un utilisateur de la base de données de façon définitive.
 * Cette fonction supprime un utilisateur en utilisant son identifiant unique.
 */
int supprimerUtilisateur(int id) {
    sqlite3 *db;              // Pointeur pour la base de données SQLite
    char requete[256];        // Chaîne de caractères pour la requête SQL
    int resultat = 0;         // Résultat de l'exécution de la requête (0 pour échec, 1 pour succès)

    // Ouvre la base de données
    if (sqlite3_open("utilisateurs.db", &db) == SQLITE_OK) {
        // Prépare la requête SQL pour supprimer un utilisateur par son identifiant
        snprintf(requete, sizeof(requete), "DELETE FROM utilisateurs WHERE id = %d", id);

        // Exécute la requête de suppression
        if (sqlite3_exec(db, requete, NULL, NULL, NULL) == SQLITE_OK) {
            resultat = 1;  // Si la suppression réussit, retourne 1
        }
    }

    // Ferme la base de données
    sqlite3_close(db);

    return resultat;  // Retourne le résultat de la suppression (1 pour succès, 0 pour échec)
}

/**
 * Récupérer le mot de passe d'un utilisateur via une question secrète.
 * L'utilisateur doit répondre correctement à la question secrète pour pouvoir récupérer son mot de passe.
 */
int recupererMotDePasse(const char *username) {
    sqlite3 *db;              // Pointeur pour la base de données SQLite
    sqlite3_stmt *stmt;       // Pointeur pour l'instruction préparée
    char requete[256];        // Chaîne de caractères pour la requête SQL
    int resultat = 0;         // Résultat de la récupération (0 pour échec, 1 pour succès)
    char reponse[LONGUEUR_MAX];  // Réponse de l'utilisateur à la question secrète

    // Ouvre la base de données
    if (sqlite3_open("utilisateurs.db", &db) == SQLITE_OK) {
        // Prépare la requête SQL pour obtenir la question secrète et la réponse secrète
        snprintf(requete, sizeof(requete), "SELECT questionSecrete, reponseSecrete FROM utilisateurs WHERE username = '%s'", username);

        // Prépare l'instruction SQL
        if (sqlite3_prepare_v2(db, requete, -1, &stmt, NULL) == SQLITE_OK) {
            // Exécute la requête
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                // Récupère la question secrète et la réponse secrète depuis la base de données
                const char *question = (const char *)sqlite3_column_text(stmt, 0);
                const char *reponseStockee = (const char *)sqlite3_column_text(stmt, 1);

                // Demande à l'utilisateur de répondre à la question secrète
                printf("%s\nVotre réponse : ", question);
                scanf("%s", reponse);

                // Compare la réponse de l'utilisateur avec la réponse secrète stockée
                if (strcmp(reponse, reponseStockee) == 0) {
                    resultat = 1;  // Si la réponse est correcte, la récupération réussit
                    printf("Récupération réussie. Veuillez modifier votre mot de passe.\n");
                } else {
                    printf("Réponse incorrecte.\n");  // Si la réponse est incorrecte, échec
                }
            }
        }

        // Finalise l'instruction préparée
        sqlite3_finalize(stmt);
    }

    // Ferme la base de données
    sqlite3_close(db);

    return resultat;  // Retourne 1 si la récupération est réussie, 0 sinon
}
