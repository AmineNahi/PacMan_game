#include <iostream>
#include <thread>
#include <vector>
#include <ctime>
#include <cmath>
#include "mingl/mingl.h"
#include "mingl/shape/circle.h"
#include "mingl/shape/rectangle.h"
#include "mingl/shape/triangle.h"
#include "mingl/gui/text.h" // Inclure la bibliothèque de texte

#define MAX_FANTOMES 10
#define FPS_LIMIT 60
#define SPAWN_INTERVAL 2500 // Intervalle de spawn en millisecondes

using namespace std;

// Position de la boule principale (avec les canons)
nsGraphics::Vec2D mainBallPos(320, 320);
int mainBallRadius = 30;

bool isInvincible = false; // Le joueur est initialement non invincible
chrono::time_point<chrono::steady_clock> invincibilityStart;

// Liste des petites boules
vector<nsGraphics::Vec2D> smallBalls;

// Dernier spawn de petite boule
chrono::time_point<chrono::steady_clock> lastSpawn = chrono::steady_clock::now();

// Compteur de boules ramassées
int compteurBoulesRamassees = 0;
const int MAX_BOULES_RAMASSEES = 1; // Maximum de boules ramassées
//Score
unsigned score = 0; // Score du joueur

// Vecteurs de boules tirées
vector<nsGraphics::Vec2D> boulesTirees;
vector<nsGraphics::Vec2D> directionsBoulesTirees;

// Gestion des fantômes
struct Fantome {
    nsGraphics::Vec2D position;
    nsGraphics::Vec2D direction;

    Fantome(nsGraphics::Vec2D pos) : position(pos), direction(nsGraphics::Vec2D(0, 1)) {}
};

vector<Fantome> fantomes; // Liste des fantômes
int vagueEnCours = 0; // Vague actuelle

// Fonction pour gérer les déplacements via les touches ZQSD et les tirs avec IJKL
void clavier(MinGL &window)

{

    // Limiter les déplacements pour ne pas sortir de l'écran

    if (window.isPressed({'z', false}) && mainBallPos.getY() - mainBallRadius > 0) // Déplacement vers le haut

        mainBallPos.setY(mainBallPos.getY() - 3);

    if (window.isPressed({'s', false}) && mainBallPos.getY() + mainBallRadius < 640) // Déplacement vers le bas

        mainBallPos.setY(mainBallPos.getY() + 3);

    if (window.isPressed({'q', false}) && mainBallPos.getX() - mainBallRadius > 0) // Déplacement vers la gauche

        mainBallPos.setX(mainBallPos.getX() - 3);

    if (window.isPressed({'d', false}) && mainBallPos.getX() + mainBallRadius < 640) // Déplacement vers la droite

        mainBallPos.setX(mainBallPos.getX() + 3);



    if (window.isPressed({'i', false}) && compteurBoulesRamassees > 0)

    {

        boulesTirees.push_back(mainBallPos); // Ajouter la boule à la liste des boules tirées

        directionsBoulesTirees.push_back(nsGraphics::Vec2D(0, -1)); // Tirer vers le haut

        compteurBoulesRamassees--; // Décrémenter le compteur de boules ramassées

    }

    if (window.isPressed({'k', false}) && compteurBoulesRamassees > 0)

    {

        boulesTirees.push_back(mainBallPos);

        directionsBoulesTirees.push_back(nsGraphics::Vec2D(0, 1)); // Tirer vers le bas

        compteurBoulesRamassees--;

    }

    if (window.isPressed({'j', false}) && compteurBoulesRamassees > 0)

    {

        boulesTirees.push_back(mainBallPos);

        directionsBoulesTirees.push_back(nsGraphics::Vec2D(-1, 0)); //Tirer vers la gauche

        compteurBoulesRamassees--;
    }

    if (window.isPressed({'l', false}) && compteurBoulesRamassees > 0)

    {

        boulesTirees.push_back(mainBallPos);

        directionsBoulesTirees.push_back(nsGraphics::Vec2D(1, 0)); // Tirer vers la droite

        compteurBoulesRamassees--;

    }

}


// Fonction pour dessiner le joueur (main ball)
void dessiner(MinGL &window)
{
    window << nsShape::Circle(nsGraphics::Vec2D(mainBallPos.getX(), mainBallPos.getY()), mainBallRadius, nsGraphics::KYellow);

    window << nsShape::Rectangle(nsGraphics::Vec2D(mainBallPos.getX() - 5, mainBallPos.getY() - mainBallRadius - 10),
                                 nsGraphics::Vec2D(mainBallPos.getX() + 5, mainBallPos.getY() - mainBallRadius), nsGraphics::KRed);

    window << nsShape::Rectangle(nsGraphics::Vec2D(mainBallPos.getX() - 5, mainBallPos.getY() + mainBallRadius),
                                 nsGraphics::Vec2D(mainBallPos.getX() + 5, mainBallPos.getY() + mainBallRadius + 10), nsGraphics::KRed);

    window << nsShape::Rectangle(nsGraphics::Vec2D(mainBallPos.getX() - mainBallRadius - 10, mainBallPos.getY() - 5),
                                 nsGraphics::Vec2D(mainBallPos.getX() - mainBallRadius, mainBallPos.getY() + 5), nsGraphics::KRed);

    window << nsShape::Rectangle(nsGraphics::Vec2D(mainBallPos.getX() + mainBallRadius, mainBallPos.getY() - 5),
                                 nsGraphics::Vec2D(mainBallPos.getX() + mainBallRadius + 10, mainBallPos.getY() + 5), nsGraphics::KRed);
}

// Fonction pour dessiner les petites boules
void dessinerPetitesBoules(MinGL &window)
{
    for (const auto &ball : smallBalls)
    {
        window << nsShape::Circle(ball, 10, nsGraphics::KRed);
    }
}

void dessinerBoulesTirees(MinGL &window)

{

    for (size_t i = 0; i < boulesTirees.size(); i++)

    {

        window << nsShape::Circle(boulesTirees[i], 10, nsGraphics::KBlue);

    }

}

void deplacerBoulesTirees()

{

    for (size_t i = 0; i < boulesTirees.size(); i++)

    {

        boulesTirees[i].setX(boulesTirees[i].getX() + directionsBoulesTirees[i].getX() * 5);

        boulesTirees[i].setY(boulesTirees[i].getY() + directionsBoulesTirees[i].getY() * 5);

    }

}

// Fonction pour générer des boules aléatoirement
void spawnPetiteBoule()
{
    // Si 2,5 secondes se sont écoulées depuis le dernier spawn
    if (chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - lastSpawn).count() >= SPAWN_INTERVAL)
    {
        srand(static_cast<unsigned int>(chrono::steady_clock::now().time_since_epoch().count()));
        int x = rand() % 640; // Position aléatoire en X
        int y = rand() % 640; // Position aléatoire en Y
        smallBalls.push_back(nsGraphics::Vec2D(x, y)); // Ajouter la nouvelle petite boule
        lastSpawn = chrono::steady_clock::now(); // Mise à jour de la dernière fois où une boule a été spawnée
    }
}

// Fonction pour ramasser les petites boules
void ramasserPetitesBoules()
{
    // Liste temporaire pour stocker les petites boules non ramassées
    vector<nsGraphics::Vec2D> remainingBalls;

    for (const auto& ball : smallBalls)
    {
        // Calcul de la distance entre la boule principale et la petite boule
        int distance = sqrt(pow(mainBallPos.getX() - ball.getX(), 2) + pow(mainBallPos.getY() - ball.getY(), 2));

        // Si la distance est inférieure à la somme des rayons, la boule est ramassée
        if (distance <= mainBallRadius + 10 && compteurBoulesRamassees < MAX_BOULES_RAMASSEES)
        {
            compteurBoulesRamassees++; // Incrémenter le compteur de boules ramassées
        }
        else
        {
            remainingBalls.push_back(ball); // Garder la boule si elle n'est pas ramassée
        }
    }

    // Remplacer la liste des petites boules par celles non ramassées
    smallBalls = remainingBalls;
}

// Fonction pour afficher le compteur de boules ramassées
void afficherCompteur(MinGL &window)
{
    string compteurText = "Boules ramassees : " + to_string(compteurBoulesRamassees) + "/" + to_string(MAX_BOULES_RAMASSEES);

    // Afficher le texte en haut à gauche de la fenêtre
    window << nsGui::Text(nsGraphics::Vec2D(10, 10), compteurText, nsGraphics::KWhite);
}

// Fonction pour spawn une vague de fantômes
void spawnVagueDeFantomes()
{
    if (fantomes.empty()) // S'il n'y a pas de fantômes sur l'écran
    {
        for (int i = 0; i < MAX_FANTOMES; ++i)
        {
            int x = rand() % 640; // Position aléatoire en X
            int y = rand() % 640; // Position aléatoire en Y
            fantomes.push_back(Fantome(nsGraphics::Vec2D(x, y))); // Créer un fantôme aux coordonnées aléatoires
        }
        vagueEnCours++; // Augmenter le compteur de vague

        // Activer l'invincibilité pour 5 secondes
        isInvincible = true;
        invincibilityStart = chrono::steady_clock::now();
    }
}

// Fonction pour déplacer les fantômes
void deplacerFantomes()
{
    for (auto &fantome : fantomes)
    {
        // Déplacer le fantôme dans la direction spécifiée
        fantome.position.setX(fantome.position.getX() + fantome.direction.getX() * (2 + 2* vagueEnCours)); // Déplacement horizontal
        fantome.position.setY(fantome.position.getY() + fantome.direction.getY() * (2 + 2* vagueEnCours)); // Déplacement vertical

        // Vérification des bords pour rebondir

        // Rebondir sur les bords gauche et droit
        if (fantome.position.getX() - 15 <= 0 || fantome.position.getX() + 15 >= 640)
        {
            fantome.direction.setX(-fantome.direction.getX()); // Inverser la direction horizontale
        }

        // Rebondir sur les bords supérieur et inférieur
        if (fantome.position.getY() - 15 <= 0 || fantome.position.getY() + 15 >= 640)
        {
            fantome.direction.setY(-fantome.direction.getY()); // Inverser la direction verticale
        }
    }
}


// Fonction pour avoir les fantômes
void dessinfantome(MinGL &window, const nsGraphics::Vec2D &ghostPos)
{
    window << nsShape::Circle(ghostPos, 20, nsGraphics::KCyan);
    window << nsShape::Circle(nsGraphics::Vec2D(ghostPos.getX() - 8, ghostPos.getY() - 10), 5, nsGraphics::KGreen); // Oeil gauche
    window << nsShape::Circle(nsGraphics::Vec2D(ghostPos.getX() + 8, ghostPos.getY() - 10), 5, nsGraphics::KGreen); // Oeil droit

    window << nsShape::Triangle(nsGraphics::Vec2D(ghostPos.getX() - 12, ghostPos.getY() + 10),
                                nsGraphics::Vec2D(ghostPos.getX() - 20, ghostPos.getY() + 25),
                                nsGraphics::Vec2D(ghostPos.getX(), ghostPos.getY() + 25), nsGraphics::KPurple);
    window << nsShape::Triangle(nsGraphics::Vec2D(ghostPos.getX() + 12, ghostPos.getY() + 10),
                                nsGraphics::Vec2D(ghostPos.getX() + 20, ghostPos.getY() + 25),
                                nsGraphics::Vec2D(ghostPos.getX(), ghostPos.getY() + 25), nsGraphics::KPurple);
}
// Fonction pour dessiner les fantômes
void dessinerFantomes(MinGL &window)
{
    for (auto &fantome : fantomes)
    {
        dessinfantome(window, fantome.position); // Dessiner chaque fantôme
    }
}

// Fonction pour vérifier si une boule tirée touche un fantôme
bool collisionBouleFantome(const nsGraphics::Vec2D &ballPos, const nsGraphics::Vec2D &ghostPos)
{
    // Calcul de la distance entre le centre de la boule tirée et celui du fantôme
    double distance = sqrt(pow(ballPos.getX() - ghostPos.getX(), 2) + pow(ballPos.getY() - ghostPos.getY(), 2));

    // Si la distance est inférieure ou égale à la somme des rayons, il y a collision
    return distance <= 30;  // 10 (rayon de la boule) + 20 (rayon du fantôme)
}

void mettreAJourScore(int points)
{
    score += points; // Met à jour la variable globale du score
    cout << "Score: " << score << endl; // Affichage du score pour le débogage
}



// Fonction pour gérer les collisions entre boules tirées et fantômes
// Fonction pour gérer les collisions entre boules tirées et fantômes
void gererCollisionsFantomeBoule()
{
    // Liste temporaire pour stocker les boules non touchées et leurs directions
    vector<nsGraphics::Vec2D> boulesRestantes;
    vector<nsGraphics::Vec2D> directionsRestantes;

    // Liste temporaire pour stocker les fantômes non touchés
    vector<Fantome> fantomesRestants;

    // Parcours des boules tirées
    for (size_t i = 0; i < boulesTirees.size(); i++)
    {
        bool touche = false;

        // Vérifier si cette boule touche un fantôme
        for (auto j = fantomes.begin(); j != fantomes.end();)
        {
            if (collisionBouleFantome(boulesTirees[i], j->position))
            {
                mettreAJourScore(20); // Ajoute 20 points au score
                touche = true; // Si collision, la boule est touchée
                j = fantomes.erase(j); // Supprimer le fantôme touché
                break; // Sortir de la boucle dès qu'une collision est détectée
            }
            else
            {
                ++j; // Passer au fantôme suivant
            }
        }

        // Si la boule n'a pas été touchée, elle reste dans la liste
        if (!touche)
        {
            boulesRestantes.push_back(boulesTirees[i]);
            directionsRestantes.push_back(directionsBoulesTirees[i]);
        }
    }

    // Remplacer la liste des boules tirées et des directions par celles non touchées
    boulesTirees = boulesRestantes;
    directionsBoulesTirees = directionsRestantes;
}





// Fonction pour vérifier si le joueur (la boule principale) touche un fantôme
bool collisionJoueurFantome(const nsGraphics::Vec2D &playerPos, const nsGraphics::Vec2D &ghostPos)
{
    // Calcul de la distance entre la boule principale et le fantôme
    double distance = sqrt(pow(playerPos.getX() - ghostPos.getX(), 2) + pow(playerPos.getY() - ghostPos.getY(), 2));

    // Si la distance est inférieure ou égale à la somme des rayons, il y a collision
    return distance <= mainBallRadius + 20;  // 30 (rayon de la boule principale) + 20 (rayon du fantôme)
}


// Fonction pour vérifier si un fantôme touche le joueur
int gererCollisionJoueurFantome(MinGL &window, const nsGraphics::Vec2D &mainBallPos, const vector<Fantome> &fantomes)
{
    if (isInvincible) // Ne pas vérifier les collisions si le joueur est invincible
    {
        return 0; // Continuer le jeu normalement
    }

    // Parcourir tous les fantômes
    for (const auto &fantome : fantomes)
    {
        // Vérifier la collision avec le joueur
        if (collisionJoueurFantome(mainBallPos, fantome.position))
        {
            window.clearScreen();  // Effacer l'écran
            return 1; // Ferme le jeu
        }
    }
    return 0; // Aucune collision
}


// Fonction pour vérifier et désactiver l'invincibilité si nécessaire
void verifierInvincibilite()
{
    if (isInvincible)
    {
        // Vérifier si 5 secondes se sont écoulées
        if (chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - invincibilityStart).count() >= 5)
        {
            isInvincible = false; // Désactiver l'invincibilité
        }
    }
}

void afficherTimerInvincibilite(MinGL &window, const int DUREE_INVINCIBILITE)
{
    if (isInvincible)
    {
        // Temps écoulé depuis le début de l'invincibilité
        int tempsEcoule = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - invincibilityStart).count();

        // Temps restant en millisecondes
        int tempsRestant = max(0, DUREE_INVINCIBILITE - tempsEcoule);

        if (tempsRestant > 0)
        {
            // Afficher le timer en haut à droite
            string timerText = "Invincible : " + to_string(tempsRestant / 1000) + "s";
            window << nsGui::Text(nsGraphics::Vec2D(540, 10), timerText, nsGraphics::KGreen);
        }
        else
        {
            // Fin de l'invincibilité
            isInvincible = false;
        }
    }
}

void afficherScore(MinGL &window) {
    window << nsGui::Text(nsGraphics::Vec2D(10, 10), "Score: " + std::to_string(score), nsGraphics::KWhite);
}


int main()
{
    MinGL window("Jeu", nsGraphics::Vec2D(640, 640), nsGraphics::Vec2D(128, 128), nsGraphics::KBlack);
    window.initGlut();
    window.initGraphic();

    chrono::microseconds frameTime = chrono::microseconds::zero();
    const int DUREE_INVINCIBILITE = 5000; // Durée de l'invincibilité en millisecondes

    int score = 0; // Initialisation du score

    while (window.isOpen())
    {
        chrono::time_point<chrono::steady_clock> start = chrono::steady_clock::now();

        window.clearScreen();
        clavier(window);
        ramasserPetitesBoules();
        deplacerBoulesTirees();
        deplacerFantomes();

        spawnVagueDeFantomes();
        gererCollisionsFantomeBoule();

        verifierInvincibilite(); // Vérifier si l'invincibilité doit être désactivée

        // Mise à jour du score lorsque le joueur touche un fantôme
        if (gererCollisionJoueurFantome(window, mainBallPos, fantomes))
        {
            return 0; // Quitter le jeu si une collision est détectée
        }

        dessiner(window);
        dessinerPetitesBoules(window);
        dessinerBoulesTirees(window);
        dessinerFantomes(window);

        spawnPetiteBoule();
        afficherCompteur(window);

        // Affichage du score
        afficherScore(window); // Affichage du score

        window.finishFrame();
        window.getEventManager().clearEvents();

        this_thread::sleep_for(chrono::milliseconds(1000 / FPS_LIMIT) - chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - start));

        frameTime = chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - start);
    }

    return 0;
}
