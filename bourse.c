#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#define NON_TROUVE -1
#define TAILLE_CODE_ISIN 20
#define TAILLE_NOM_SOCIETE 100
#define TAILLE_SYMBOLE 20
#define NOMBRE_MAX_ACTIONS 1000
#define NOMBRE_MAX_OPERATIONS 1000
#define TAILLE_DATE 20
#define TAILLE_HEURE 10
#define COURS_DE_BOURSE_FILE_NAME "COURS_DE_BOURSE.csv"
#define OPERATIONS_EN_ATTENTE_FILE_NAME "OPERATIONS_EN_ATTENTE.csv"

/* Déclation de la structure globale */
struct struct_action
{
    char code_isin[TAILLE_CODE_ISIN];
    char nom_societe[TAILLE_NOM_SOCIETE];
    char symbole[TAILLE_SYMBOLE];
    float prix_achat_unit;
    int quantite;
};
struct operation
{
    struct struct_action action;
    char date[TAILLE_DATE]; // Format : JJ/MM/AAAA
    char heure[TAILLE_HEURE]; // Format : HH:MM
    char proprietaire_portefeuille[100];
    char type_operation; // A pour Achat, V pour Vente
};

/*---------- Déclaration préliminaires ----------*/
void MenuPortefeuille();
void chargement();
void Achat();
void Vente();
void OrdreAuMarche();
void OrdreACoursLimite();
void affichage();
void GetDateDuJour();
void GetHeureCourante(char *dest);
void GetCoursDeBourseFromCSV();
int RechercheAction(char *mot_cle, struct struct_action *tableau, char *nom_tab);
void conv_maj(char *ch);
void MettreOperationEnAttente(struct struct_action action, float prix, int quantite, char type_operation);
void ChargerOperationEnAttente();

/* Déclaration des variables globales */
int nb_actions_portefeuille = 0;
int nb_actions_cours_bourse = 0;
int nb_operations_en_attente  = 0;
char date_du_jour[TAILLE_DATE]; // Format : JJ/MM/AAAA
char NomProprietaire[100];
struct struct_action portefeuille[NOMBRE_MAX_ACTIONS];
struct struct_action cours_bourse[NOMBRE_MAX_ACTIONS];
struct operation operations_en_attente[NOMBRE_MAX_OPERATIONS];

/*---------- Programme principale ----------*/
int main()
{   
    // Recupérer le cours de bourse
    GetCoursDeBourseFromCSV();

    // Chargement de portefeuille à partir d'un fichier CSV
    chargement();

    // Chargement des opérations en attente
    ChargerOperationEnAttente();

    // Récupérer la date du jour
    GetDateDuJour();
    printf("La date du jour est : %s\n", date_du_jour);

    // Init choix
    int choix = -1;

    // Boucle du Menu Principal
    while(choix != 0)
    {
        printf("================ MENU PRINCIPAL ================\n");
        printf("-1- Gestion de portefeuille\n");
        printf("-2- Achat d'action\n");
        printf("-3- Vente d'action\n");
        printf("-4- Vente à découvert\n");
        printf("-0- Quitter l'application\n");
        printf("Choix : ");
        scanf("%d", &choix);
        printf("==============================================\n");

        switch (choix)
        {
            case 1:
                MenuPortefeuille();
                break; 
            case 2:
                Achat();
            case 3:
                Vente();  
            case 0:
                printf("Au revoir !\n");
                break;
            default:
                printf("Choix inconnu ! Veuillez réessayer...\n");
                break;
        } // Fin du switch
    } // Fin du while
}

/*---------- Gestion de portefeuille ----------*/
void MenuPortefeuille()
{
    int choix = -1;

    while(choix != 0)
    {
        printf("================ GESTION DE PORTEFEUILLE ================\n");
        printf("-1- Affichage\n");
        printf("-2- Sauvegarde\n");
        printf("-3- Clôture\n");
        printf("-4- Exporter le portefeuille en CSV\n");
        printf("-0- Revenir au menu principal\n");
        printf("Choix : ");
        scanf("%d", &choix);
        printf("==============================================\n");

        switch (choix)
        {
            case 0:
                break;
            case 1:
                affichage();
                break;
            default:
                printf("Choix inconnu ! Veuillez réessayer...\n");
                break;
        }
    }
}
/*---------- Affichage de portefeuille ----------*/
void chargement() // ask for person name not the file name
{
    int i;
    struct struct_action action;
    FILE *f1;
    char NomFichier[100], NomProprietaireInput[100], PortefeuilleType[10];
    int retour;

    printf("Nom du propriétaire de portefeuille : ");
    scanf("%s", NomProprietaireInput);
    strcpy(NomProprietaire, NomProprietaireInput); // Enregistrer le nom du propriétaire de la session active
    printf("Type de portefeille : ");
    scanf("%s", PortefeuilleType);

    // Nom du fichier à charger à partir de NomProprietaire et PortefeuilleType (format : NomProprietaireInput_PortefeuilleType.csv)
    strcpy(NomFichier, NomProprietaireInput);
    strcat(NomFichier, "_");
    strcat(NomFichier, PortefeuilleType);
    strcat(NomFichier, ".csv");

    /* --- Boucle de chargement --- */
    i = nb_actions_portefeuille;
    
    f1 = fopen(NomFichier, "r");
    while(!feof(f1))
    {
        retour = fscanf(f1, "%[^,],%[^,],%[^,],%f,%d", action.code_isin, action.symbole,action.nom_societe, &action.prix_achat_unit, &action.quantite);
        if(retour != EOF)
        {
            portefeuille[i++] = action;
        }
    }
    fclose(f1);
    nb_actions_portefeuille = i;

    printf("%d lignes chargée(s) !\n", nb_actions_portefeuille); // une ligne = une action (vocab finance)
}
/*---------- Affichage de portefeuille ----------*/
void affichage()
{
    int i;
    struct struct_action action;

    if(nb_actions_portefeuille == 0)
    {
        printf("Aucune action à afficher !\n");
    }
    else
    {
        /* --- Boucle d'affichage --- */
        for(i = 0; i < nb_actions_portefeuille; i++)
        {
            action = portefeuille[i];
            if(action.quantite > 0)
            {
                printf("%s,%s,%s,%f,%d", action.code_isin, action.symbole, action.nom_societe, action.prix_achat_unit, action.quantite);
                printf("\n");
            }
        }
        printf("\n");
    }
}
/*---------- Achat ----------*/
void Achat()
{
    int choix = -1;

    while(choix != 0)
    {
        printf("================ MENU ACHAT ================\n");
        printf("-1- Ordre au marche\n");
        printf("-2- Ordre à cours limite\n");
        printf("-3- Ordre avec seuil de declenchement\n");
        printf("-0- Revenir au menu principal\n");
        printf("Choix : ");
        scanf("%d", &choix);
        printf("==============================================\n");

        switch (choix)
        {
            case 0:
                break;
            case 1:
                OrdreAuMarche();
                break;
            case 2:
                OrdreACoursLimite();
                break;
            default:
                printf("Choix inconnu ! Veuillez réessayer...\n");
                break;
        } // Fin de Switch
    } // Fin de Whille
}
/*---------- Vente ----------*/
void Vente()
{

}
/*---------- Ordre au marche ----------*/
void OrdreAuMarche()
{
    char type_operation;
    char symbole_input[TAILLE_CODE_ISIN];
    char heure_courante[TAILLE_HEURE];
    int quantite_input, index_action_recherche_cours_bourse, index_action_recherche_portefeuille;
    struct struct_action action;

    printf("================ ORDRE AU MARCHE ================\n");
    printf("Prix correspond au prix du marché.\n");

    printf("Veuillez saisir le code de la société concernée par votre opération : ");
    scanf("%s", symbole_input);
    conv_maj(symbole_input);

    index_action_recherche_cours_bourse = RechercheAction(symbole_input, cours_bourse, "cours_bourse");
    action = cours_bourse[index_action_recherche_cours_bourse];
    printf("Le prix du marché : %f\n", action.prix_achat_unit);

    while (type_operation != 'A' && type_operation != 'V')
    {
        printf("Opération (A pour Achat, V pour Vente): ");
        while (getchar() != '\n');
        scanf("%c", &type_operation);
        type_operation = toupper(type_operation);
        if(type_operation != 'A' && type_operation != 'V')
        {
            printf("La réponse attendue est : A pour Achat ou V pour Vente\n");
        }
    }

    if(type_operation == 'A')
    {
        printf("Opération choisie : Achat\n");
        printf("Quantité disponbile : %d\n", action.quantite);
    }
    else
    {
        printf("Opération choisie : Vente\n");
        index_action_recherche_portefeuille = RechercheAction(symbole_input, portefeuille, "portefeuille");
        printf("Quantité disponbile : %d\n", portefeuille[index_action_recherche_portefeuille].quantite);
    }
    
    printf("Veuillez saisir la quantité : ");
    scanf("%d", &quantite_input);

    if(type_operation == 'A')
    { // ACHAT
        if (quantite_input <= action.quantite)
        {
            index_action_recherche_portefeuille = RechercheAction(symbole_input, portefeuille, "portefeuille");
            if (index_action_recherche_portefeuille == NON_TROUVE)
            {
                action.quantite = quantite_input;
                portefeuille[nb_actions_portefeuille++] = action;
                cours_bourse[index_action_recherche_cours_bourse].quantite -= quantite_input;
                printf("Achat de %d actions avec succès.\n", quantite_input);
                affichage();
            }
            else
            {
                action = portefeuille[index_action_recherche_portefeuille];
                action.quantite += quantite_input;
                portefeuille[index_action_recherche_portefeuille] = action;
                cours_bourse[index_action_recherche_cours_bourse].quantite -= quantite_input;
                printf("Achat de %d actions avec succès.\n", quantite_input);
                affichage();
            }
        }
        else
        {
            printf("Quantité disponible insuffisante !\n");
        }
    }
    else
    { // VENTE
        index_action_recherche_portefeuille = RechercheAction(symbole_input, portefeuille, "portefeuille");
        if (index_action_recherche_portefeuille == NON_TROUVE)
        {
            printf("La vente à découvert est suspendue suite aux consignes de l'AMF (Autorité des marché financiers).\n");
        }
        else
        {
            if (portefeuille[index_action_recherche_portefeuille].quantite >= quantite_input)
            {
                // Si le portefeuill contient suffisamment d'actions à vendre, la quantié à vendre sera enlevée du portefeuille et ajoutée dans le cours de bourse
                action = portefeuille[index_action_recherche_portefeuille];
                action.quantite -= quantite_input;
                portefeuille[index_action_recherche_portefeuille] = action;
                cours_bourse[index_action_recherche_cours_bourse].quantite += quantite_input;
                printf("Vente de %d actions avec succès.\n", quantite_input);
                affichage();
            }
            else
            {
                // Si le portefeuill ne contient pas suffisamment d'actions à vendre, la quantié à vendre sera enlevée du portefeuille, l'action sera supprimée du portefeuille et cette quantité sera ajoutée dans le cours de bourse
                action = portefeuille[index_action_recherche_portefeuille];
                cours_bourse[index_action_recherche_cours_bourse].quantite += action.quantite;
                printf("Vente de %d actions avec succès.\n", action.quantite);
                action.quantite = 0;
                portefeuille[index_action_recherche_portefeuille] = action;
                affichage();
            }
        }
    }

    GetHeureCourante(heure_courante);
    printf("Date de l'opération : %s %s\n", date_du_jour, heure_courante);
}
/*---------- Ordre à cours limite ----------*/
void OrdreACoursLimite()
{
    char type_operation;
    char symbole_input[TAILLE_CODE_ISIN];
    char heure_courante[TAILLE_HEURE];
    float prix_input;
    int quantite_input, index_action_recherche_cours_bourse, index_action_recherche_portefeuille;
    struct struct_action action;

    printf("================ ORDRE A COURS LIMITE ================\n");

    printf("Veuillez saisir le code de la société concernée par votre opération : ");
    scanf("%s", symbole_input);
    conv_maj(symbole_input);

    index_action_recherche_cours_bourse = RechercheAction(symbole_input, cours_bourse, "cours_bourse");
    action = cours_bourse[index_action_recherche_cours_bourse];
    printf("Le prix du marché : %f\n", action.prix_achat_unit);

    while (type_operation != 'A' && type_operation != 'V')
    {
        printf("Opération (A pour Achat, V pour Vente): ");
        while (getchar() != '\n');
        scanf("%c", &type_operation);
        type_operation = toupper(type_operation);
        if(type_operation != 'A' && type_operation != 'V')
        {
            printf("La réponse attendue est : A pour Achat ou V pour Vente\n");
        }
    }

    if(type_operation == 'A')
    {
        printf("Opération choisie : Achat\n");
        printf("Quantité disponbile : %d\n", action.quantite);
    }
    else
    {
        printf("Opération choisie : Vente\n");
        index_action_recherche_portefeuille = RechercheAction(symbole_input, portefeuille, "portefeuille");
        printf("Quantité disponbile : %d\n", portefeuille[index_action_recherche_portefeuille].quantite);
    }

    printf("Veuillez saisir la quantité : ");
    scanf("%d", &quantite_input);

    printf("Veuillez saisir le prix : ");
    scanf("%f", &prix_input);

    if (type_operation == 'A')
    { // ACHAT
        if (quantite_input <= action.quantite)
        {
            index_action_recherche_portefeuille = RechercheAction(symbole_input, portefeuille, "portefeuille");
            if (prix_input == portefeuille[index_action_recherche_cours_bourse].prix_achat_unit)
            {   
                // Quand le prix correspond au prix du marché, l'opération sera exécutée
                if (index_action_recherche_portefeuille == NON_TROUVE)
                {
                    action.quantite = quantite_input;
                    portefeuille[nb_actions_portefeuille++] = action;
                    cours_bourse[index_action_recherche_cours_bourse].quantite -= quantite_input;
                    printf("Achat de %d actions avec succès.\n", quantite_input);
                    affichage();
                }
                else
                {
                    action = portefeuille[index_action_recherche_portefeuille];
                    action.quantite += quantite_input;
                    portefeuille[index_action_recherche_portefeuille] = action;
                    cours_bourse[index_action_recherche_cours_bourse].quantite -= quantite_input;
                    printf("Achat de %d actions avec succès.\n", quantite_input);
                    affichage();
                }
            }
            else
            {
                // Si le prix ne correspond pas au prix du marché, il faut mettre cette opération en attente
                MettreOperationEnAttente(action, prix_input, quantite_input, type_operation);
            }
        }
        else
        {
            printf("Quantité disponible insuffisante !\n");
        }
    }
    else
    { // VENTE
        index_action_recherche_portefeuille = RechercheAction(symbole_input, portefeuille, "portefeuille");
        if (index_action_recherche_portefeuille == NON_TROUVE)
        {
            printf("La vente à découvert est suspendue suite aux consignes de l'AMF (Autorité des marché financiers).\n");
        }
        else
        {
            if (prix_input == portefeuille[index_action_recherche_cours_bourse].prix_achat_unit)
            {
                // Quand le prix correspond au prix du marché, l'opération sera exécutée
                if (portefeuille[index_action_recherche_portefeuille].quantite >= quantite_input)
                {
                    // Si le portefeuill contient suffisamment d'actions à vendre, la quantié à vendre sera enlevée du portefeuille et ajoutée dans le cours de bourse
                    action = portefeuille[index_action_recherche_portefeuille];
                    action.quantite -= quantite_input;
                    portefeuille[index_action_recherche_portefeuille] = action;
                    cours_bourse[index_action_recherche_cours_bourse].quantite += quantite_input;
                    printf("Vente de %d actions avec succès.\n", quantite_input);
                    affichage();
                }
                else
                {
                    // Si le portefeuill ne contient pas suffisamment d'actions à vendre, la quantié à vendre sera enlevée du portefeuille, l'action sera supprimée du portefeuille et cette quantité sera ajoutée dans le cours de bourse
                    action = portefeuille[index_action_recherche_portefeuille];
                    cours_bourse[index_action_recherche_cours_bourse].quantite += action.quantite;
                    printf("Vente de %d actions avec succès.\n", action.quantite);
                    action.quantite = 0;
                    portefeuille[index_action_recherche_portefeuille] = action;
                    affichage();
                }
            }
            else
            {
                // Si le prix ne correspond pas au prix du marché, il faut mettre cette opération en attente
                MettreOperationEnAttente(action, prix_input, quantite_input, type_operation);
            }
        }
    }
}
/*---------- Get Date du Jour ----------*/
void GetDateDuJour()
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    strftime(date_du_jour, sizeof(date_du_jour), "%d/%m/%Y", t);
}
/*---------- Get Heure Courante ----------*/
void GetHeureCourante(char *dest)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    strftime(dest, sizeof(date_du_jour), "%H:%M", t);
}
/*---------- Recuperer le cours de bourse ----------*/
void GetCoursDeBourseFromCSV()
{
    int j;
    struct struct_action action;
    FILE *f1;
    int retour;

    /* --- Boucle de chargement --- */
    f1 = fopen(COURS_DE_BOURSE_FILE_NAME, "r");
    while(!feof(f1))
    {
        retour = fscanf(f1, "%[^,],%[^,],%[^,],%d,%f\n", action.code_isin, action.symbole, action.nom_societe, &action.quantite, &action.prix_achat_unit);
        if(retour != EOF)
        {
            cours_bourse[nb_actions_cours_bourse++] = action;
        }
    }
    fclose(f1);

    printf("%d indices chargée(s) !\n", nb_actions_cours_bourse); // une ligne = une action (vocab finance)

    for(j = 0; j < nb_actions_cours_bourse; j++)
    { // boucle affichage
        action = cours_bourse[j];
        printf("%s,%s,%s,%d,%f \n", action.code_isin, action.symbole, action.nom_societe, action.quantite, action.prix_achat_unit);
    }
}
/*---------- Recherche d'une action dans le cours de bourse ----------*/
int RechercheAction(char *mot_cle, struct struct_action *tableau, char *nom_tab)
{
    int i, nb_action, numero_r = NON_TROUVE;
    struct struct_action action;

    if(strcmp(nom_tab, "portefeuille") == 0)
    {
        nb_action = nb_actions_portefeuille;
    }
    else
    {
        nb_action = nb_actions_cours_bourse;
    }
    
    for(i = 0; i < nb_action; i++)
    {
        action = tableau[i];
        if(strcmp(action.symbole, mot_cle) == 0)
        {
            numero_r = i;
        }
    }
    return numero_r;
}
/*---------- Conversion en majuscule ----------*/
void conv_maj(char *ch)
{
    int i, taille;

    taille = strlen(ch);

    for(i = 0; i < taille; i++)
    {
        ch[i] = toupper(ch[i]);
    }
}
/*---------- Mettre une opération en attente ----------*/
void ChargerOperationEnAttente()
{
    int i;
    struct struct_action action;
    struct operation operation;
    FILE *f1;
    int retour;

    /* --- Boucle de chargement --- */
    i = nb_operations_en_attente;
    
    f1 = fopen(OPERATIONS_EN_ATTENTE_FILE_NAME, "r");
    while(!feof(f1))
    {
        retour = fscanf(f1, "%[^,],%[^,],%[^,],%[^,],%[^,],%f,%d,%c\n", operation.date, operation.heure, operation.proprietaire_portefeuille, operation.action.symbole, operation.action.code_isin, &operation.action.prix_achat_unit, &operation.action.quantite, &operation.type_operation);
        if(retour != EOF)
        {
            operations_en_attente[i++] = operation;
        }
    }
    fclose(f1);
    nb_operations_en_attente = i;

    printf("%d opérations chargée(s) !\n", nb_operations_en_attente); // Pour le test - A supprimer
}
/*---------- Mettre une opération en attente ----------*/
void MettreOperationEnAttente(struct struct_action action, float prix, int quantite, char type_operation)
{
    int i;
    FILE *f1;
    char NomFichier[100];
    char heure_courante[TAILLE_HEURE];
    struct operation operation;
    int retour;

    // Ajouter l'opération dans le tableau operations_en_attente
    operation.action = action;
    operation.action.prix_achat_unit = prix;
    operation.action.quantite = quantite;
    strcpy(operation.date, date_du_jour);
    GetHeureCourante(heure_courante);
    strcpy(operation.heure, heure_courante);
    operation.type_operation = type_operation;
    strcpy(operation.proprietaire_portefeuille, NomProprietaire);
    operations_en_attente[nb_operations_en_attente++] = operation;

    /* --- Boucle de sauvegarde --- */
    f1 = fopen(OPERATIONS_EN_ATTENTE_FILE_NAME, "w");
    for(i = 0; i < nb_operations_en_attente; i++)
    {
        fprintf(f1, "%s,%s,%s,%s,%s,%f,%d,%c\n", operations_en_attente[i].date, operations_en_attente[i].heure, operations_en_attente[i].proprietaire_portefeuille, operations_en_attente[i].action.symbole, operations_en_attente[i].action.code_isin, operations_en_attente[i].action.prix_achat_unit, operations_en_attente[i].action.quantite, operations_en_attente[i].type_operation);
    }
    fclose(f1);
}