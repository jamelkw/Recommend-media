#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <math.h>
#include "recommendation.h"

#ifndef RECOMMENDATION_H
#define RECOMMENDATION_H

#define MAX_GENRES 5
#define MAX_MUSIC 5
#define MAX_RECOMMANDATIONS 10

// Structure pour stocker les préférences utilisateur
typedef struct {
    int idUtilisateur;
    char genresFavoris[MAX_GENRES][30];
    char musiquesFavorites[MAX_MUSIC][30];
} PreferencesUtilisateur;

// Structure pour stocker une recommandation
typedef struct {
    int idUtilisateur;
    int idMedia;
    float score;
} Recommandation;

// Simule une base de données d'utilisateurs en mémoire (peut être remplacée par une vraie BD)
PreferencesUtilisateur basePreferences[100]; // Tableau de préférences pour 100 utilisateurs max
int nombreUtilisateurs = 0; // Compteur du nombre d'utilisateurs enregistrés

/**
 * Analyse les préférences d'un utilisateur.
 * Ici on remplit simplement les genres/musiques avec des exemples simulés.
 */
int analyserPreferencesUtilisateur(int idUtilisateur) {
    int i;
    // Cherche si l'utilisateur existe déjà
    for ( i = 0; i < nombreUtilisateurs; i++) {
        if (basePreferences[i].idUtilisateur == idUtilisateur) {
            // Si trouvé, on met à jour ses préférences (exemples en dur)
            strcpy(basePreferences[i].genresFavoris[0], "Action");
            strcpy(basePreferences[i].genresFavoris[1], "Science-fiction");
            strcpy(basePreferences[i].musiquesFavorites[0], "Rock");
            return 1;
        }
    }

    // Sinon, on ajoute un nouvel utilisateur avec des préférences par défaut
    PreferencesUtilisateur p;
    p.idUtilisateur = idUtilisateur;
    strcpy(p.genresFavoris[0], "Action");
    strcpy(p.genresFavoris[1], "Science-fiction");
    strcpy(p.musiquesFavorites[0], "Rock");
    basePreferences[nombreUtilisateurs++] = p; // Ajout à la base simulée

    return 1;
}

/**
 * Calcule un score de similarité simple entre deux utilisateurs
 * basé sur leurs genres préférés.
 */
float calculerSimilariteUtilisateurs(int idUtilisateur1, int idUtilisateur2) {
    int i;
    PreferencesUtilisateur *u1 = NULL, *u2 = NULL;

    // Recherche des deux utilisateurs dans la base
    for ( i = 0; i < nombreUtilisateurs; i++) {
        if (basePreferences[i].idUtilisateur == idUtilisateur1) u1 = &basePreferences[i];
        if (basePreferences[i].idUtilisateur == idUtilisateur2) u2 = &basePreferences[i];
    }

    if (!u1 || !u2) return 0.0f; // Un des utilisateurs n'existe pas

    int similarGenres = 0;

    // Comparaison des genres préférés
    for ( i = 0; i < MAX_GENRES; i++) {
        for (int j = 0; j < MAX_GENRES; j++) {
            if (strcmp(u1->genresFavoris[i], u2->genresFavoris[j]) == 0) {
                similarGenres++;
            }
        }
    }

    // Score de similarité normalisé entre 0 et 1
    return (float)similarGenres / MAX_GENRES;
}

/**
 * Analyse globale des tendances de visionnage (fonction simulée).
 */
int analyserTendancesVisionnage() {
    printf("Analyse des tendances de visionnage en cours...\n");
    // À développer avec des statistiques sur les médias vus
    return 1;
}

/**
 * Génère une liste simple de recommandations simulées
 * pour un utilisateur, avec des scores décroissants.
 */
int genererRecommandations(int idUtilisateur, Recommandation recommandations[MAX_RECOMMANDATIONS]) {
    // S'assurer que les préférences sont connues
    analyserPreferencesUtilisateur(idUtilisateur);

    // Création de recommandations fictives avec des ID médias simulés
    int i;
    for ( i = 0; i < MAX_RECOMMANDATIONS; i++) {
        recommandations[i].idUtilisateur = idUtilisateur;
        recommandations[i].idMedia = 100 + i;  // Ex : médias ID de 100 à 109
        recommandations[i].score = 1.0f / (i + 1);  // Scores décroissants
    }

    return MAX_RECOMMANDATIONS;
}

/**
 * Sauvegarde des recommandations d'un utilisateur dans une base SQLite.
 */
int sauvegarderRecommandationsSQL(int idUtilisateur, Recommandation recommandations[MAX_RECOMMANDATIONS], int nombreRecommandations) {
    int i;
    sqlite3 *db;
    char *errMsg = NULL;

    // Ouvrir la base de données SQLite
    if (sqlite3_open("recommandations.db", &db)) {
        fprintf(stderr, "Erreur ouverture DB: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    // Créer la table si elle n'existe pas encore
    const char *sqlCreate = "CREATE TABLE IF NOT EXISTS recommandations ("
                            "idUtilisateur INTEGER, idMedia INTEGER, score REAL);";
    sqlite3_exec(db, sqlCreate, 0, 0, &errMsg);

    // Préparer la requête d'insertion
    const char *sqlInsert = "INSERT INTO recommandations (idUtilisateur, idMedia, score) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt;

    // Insérer chaque recommandation dans la base
    for ( i = 0; i < nombreRecommandations; i++) {
        sqlite3_prepare_v2(db, sqlInsert, -1, &stmt, NULL);
        sqlite3_bind_int(stmt, 1, recommandations[i].idUtilisateur);
        sqlite3_bind_int(stmt, 2, recommandations[i].idMedia);
        sqlite3_bind_double(stmt, 3, recommandations[i].score);
        sqlite3_step(stmt);         // Exécuter l’insertion
        sqlite3_finalize(stmt);     // Nettoyer la requête préparée
    }

    sqlite3_close(db); // Fermer la base
    return 1;
}

/**
 * Charge les recommandations depuis SQLite pour un utilisateur donné.
 */
int chargerRecommandationsSQL(int idUtilisateur, Recommandation recommandations[MAX_RECOMMANDATIONS]) {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int count = 0;

    // Ouvre la base de données
    if (sqlite3_open("recommandations.db", &db)) {
        fprintf(stderr, "Erreur ouverture DB: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    // Requête pour récupérer les recommandations de l'utilisateur
    const char *sqlSelect = "SELECT idMedia, score FROM recommandations WHERE idUtilisateur = ? LIMIT ?;";
    sqlite3_prepare_v2(db, sqlSelect, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, idUtilisateur);              // Paramètre 1 : ID utilisateur
    sqlite3_bind_int(stmt, 2, MAX_RECOMMANDATIONS);        // Paramètre 2 : limite

    // Lire chaque ligne retournée
    while (sqlite3_step(stmt) == SQLITE_ROW && count < MAX_RECOMMANDATIONS) {
        recommandations[count].idUtilisateur = idUtilisateur;
        recommandations[count].idMedia = sqlite3_column_int(stmt, 0);        // Colonne 0 = idMedia
        recommandations[count].score = (float)sqlite3_column_double(stmt, 1);// Colonne 1 = score
        count++;
    }

    sqlite3_finalize(stmt); // Nettoyer
    sqlite3_close(db);      // Fermer la base

    return count; // Nombre de recommandations chargées
}
