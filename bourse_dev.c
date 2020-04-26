#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#define NON_TROUVE -1
#define PAS_DE_SEUIL_DECLENCHEMNT 999
#define TAILLE_CODE_ISIN 20
#define TAILLE_NOM_SOCIETE 100
#define TAILLE_SYMBOLE 20
#define NOMBRE_MAX_ACTIONS 1000
#define NOMBRE_MAX_OPERATIONS 1000
#define NOMBRE_MAX_PORTEFEUILLE 100
#define TAILLE_DATE 20
#define TAILLE_HEURE 10
#define TAILLE_STATUT_OPERATION 20
#define TAILLE_LIGNE 1000
#define TYPE_OPERATION_NON_DEFINI '\0'
#define QUANTITE_NON_DEFINI 0
#define COURS_DE_BOURSE_FILE_NAME "COURS_DE_BOURSE.csv"
#define OPERATIONS_EN_ATTENTE_FILE_NAME "OPERATIONS_EN_ATTENTE.csv"
#define HISTORIQUE_FILE_NAME "HISTORIQUE.csv"
#define VALORISATION_FILE_NAME "VALO_PORTEFEUILLE.csv"

/* Déclation de la structure globale */
struct struct_action
{
    char code_isin[TAILLE_CODE_ISIN];
    char nom_societe[TAILLE_NOM_SOCIETE];
    char symbole[TAILLE_SYMBOLE];
    float prix_achat_unit;
    int quantite;
    float seuil_declenchement; // en pourcentage; 999 => pas de seuil de déclement
};
struct operation
{
    struct struct_action action;
    char date[TAILLE_DATE]; // Format : JJ/MM/AAAA
    char heure[TAILLE_HEURE]; // Format : HH:MM
    char proprietaire_portefeuille[100];
    char type_operation; // A pour Achat, V pour Vente
    char statut[TAILLE_STATUT_OPERATION];
};
struct valorisation
{
    char proprietaire_portefeuille[100];
    float montant_investissement;
    int frais_ouverture;
    float somme_titres_detenus;
    float frais_courtage;
    float solde;
    char statut[30];
};
struct date
{
    int jour;
    int mois;
    int annee;
};

/*---------- Déclaration préliminaires ----------*/
void MenuPortefeuille();
void MenuPrincipal();
void chargement();
void MenuOperation();
void OrdreAuMarche();
void OrdreACoursLimite();
void AlerteSeuilDeclenchement();
void affichage();
void GetDateDuJour();
void GetHeureCourante(char *dest);
void GetCoursDeBourseFromCSV();
int RechercheAction(char *mot_cle, struct struct_action *tableau, char *nom_tab);
void conv_maj(char *ch);
void MettreOperationEnAttente(struct struct_action action, float prix, int quantite, char type_operation);
void ChargerOperationEnAttente();
void AffichageCoursDeBourse();
void ModificationCoursBourse();
void CloturePortefeuille();
void ChargerHistorique();
void Sauvegarde();
void EnregistrerDansLHistorique(struct struct_action action, float prix, int quantite, char type_operation, char *statut);
void AffichageHisotrique();
float CaculNouveauPrix(float prix_1, int quantite_1, float prix_2, int quantite_2);
int Egal(float f1, float f2);
void CalculerSommeOperation(char *statut, char type_operation, float prix, int quantite);
void AchatVente(char *type_ordre, char symbole_input[], char type_operation_argument, float quantite_argument);
void verif_sauvegarde();
void AfficherOperationsEnAttente();
void MettreAJourDesOperationsEnAttente();
void VerificationOperationEnAttente();
void ChargerValorisationPortefeuille();
void MettreAJourValorisationPortefeuille();
void AffichageValorisationPortefeuille();
float CalculerNouvelleSolde(char type_operation, float prix, int quantite);
void BienvenueMessage();
int DateValide(struct date ma_date);
int PortfeuilleDeCinqAns(struct date date_ouverture, struct date date_cloture);
void VenteToutesActions();
float CaclulerSommeVenteToutesActions();

/* Déclaration des variables globales */
int nb_actions_portefeuille = 0;
int nb_actions_cours_bourse = 0;
int nb_operations_en_attente  = 0;
int nb_operation_en_attente_session = 0;
int nb_operation_effectuee_total = 0;
int nb_portefeuilles = 0;
int index_portefeuille_tab_valorisation;
int a_sauvegarder = 0; // pas d'operation effectue
char NomFichierPortefeuille[200]; 
char date_du_jour[TAILLE_DATE]; // Format : JJ/MM/AAAA
char NomProprietaire[100];
char PortefeuilleType[20];
struct struct_action portefeuille[NOMBRE_MAX_ACTIONS];
struct struct_action cours_bourse[NOMBRE_MAX_ACTIONS];
struct operation operations_en_attente[NOMBRE_MAX_OPERATIONS];
struct operation historique[NOMBRE_MAX_OPERATIONS];
struct valorisation tab_valorisation[NOMBRE_MAX_PORTEFEUILLE];
struct valorisation valorisation_portefeuille;

/*---------- Programme principale ----------*/
int main()
{   
    // Afficher le message de bienvenue
    BienvenueMessage();

    // Recupérer le cours de bourse
    GetCoursDeBourseFromCSV();

    // Chargement de portefeuille à partir d'un fichier CSV
    chargement();

    // Chargement des opérations en attente
    ChargerOperationEnAttente();

    // Récupérer la date du jour
    GetDateDuJour();

    // Chargement de l'historique
    ChargerHistorique();

    // Charger la valorisation du portefeuille
    ChargerValorisationPortefeuille();

    // Verification les seuils de declenchement
    AlerteSeuilDeclenchement();

    // Afficher la date du jour
    printf("La date du jour est                   : %s\n\n", date_du_jour);

    // Afficher le menu principal selon le statut du portefeuille
    MenuPrincipal();
}
/*---------- MenuPrincipal ----------*/
void MenuPrincipal()
{
    // Init choix
    int choix = -1;

    if(strcmp(valorisation_portefeuille.statut, "Actif") == 0)
    {
        // Boucle du Menu Principal
        while(choix != 0)
        {
            printf("================ MENU PRINCIPAL ================\n");
            printf("-1- Gestion de portefeuille\n");
            printf("-2- Menu des operations\n");
            printf("-3- Affichage du cours de bourse\n");
            printf("-4- Modification du cours de bourse\n");
            printf("-5- Affichage des operation en attente\n");
            printf("-6- Affichage de l'historique\n");
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
                    MenuOperation();
                    break;
                case 3:
                    AffichageCoursDeBourse();
                    break;
                case 4:
                    ModificationCoursBourse();
                    break;
                case 5:
                    AfficherOperationsEnAttente();
                    break;
                case 6:
                    AffichageHisotrique();
                    break;
                case 0:
                    verif_sauvegarde();
                    printf("Au revoir !\n");
                    break;
                default:
                    printf("Choix inconnu ! Veuillez réessayer...\n");
                    break;
            } // Fin du switch
        } // Fin du while
    }
    else
    {
        // Boucle du Menu Principal
        while(choix != 0)
        {
            printf("================ MENU PRINCIPAL - PORTEFEUILLE CLOTURE ================\n");
            printf("-1- Affichage de l'historique\n");
            printf("-0- Quitter l'application\n");
            printf("Choix : ");
            scanf("%d", &choix);
            printf("==============================================\n");

            switch (choix)
            {
                case 1:
                    AffichageHisotrique();
                    break;
                case 0:
                    verif_sauvegarde();
                    printf("Au revoir !\n");
                    break;
                default:
                    printf("Choix inconnu ! Veuillez réessayer...\n");
                    break;
            } // Fin du switch
        } // Fin du while
    }
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
        printf("-4- Valorisation\n");
        printf("-0- Revenir au menu principal\n");
        printf("Choix : ");
        scanf("%d", &choix);
        printf("==============================================\n");

        switch (choix)
        {
            case 1:
                affichage();
                break;
            case 2:
                Sauvegarde();
                break;
            case 3:
                CloturePortefeuille();
                break;
            case 4:
                AffichageValorisationPortefeuille();
                break;
            case 0:
                break;
            default:
                printf("Choix inconnu ! Veuillez réessayer...\n");
                break;
        }
    }
}
/*---------- Chargement de portefeuille ----------*/
void chargement() // ask for person name not the file name
{
    int i;
    struct struct_action action;
    FILE *f1;
    char bidon[100]; // caractere pour consommer le retour à la ligne
    char ligne[TAILLE_LIGNE];
    int retour;

    printf("Nom du propriétaire de portefeuille   : ");
    scanf("%s", NomProprietaire);
    conv_maj(NomProprietaire);
    while(strcmp(PortefeuilleType, "PEA") != 0 && strcmp(PortefeuilleType, "COMPTE_TITRE") != 0)
    {
        printf("Portefeille type (PEA/COMPTE_TITRE)   : ");
        scanf("%s", PortefeuilleType);
        conv_maj(PortefeuilleType);
        if(strcmp(PortefeuilleType, "PEA") != 0 && strcmp(PortefeuilleType, "COMPTE_TITRE") != 0)
        {
            printf("La réponse attendue est               : PEA/COMPTE_TITRE\n");
        }
    }

    // Nom du fichier à charger à partir de NomProprietaire et PortefeuilleType (format : NomProprietaireInput_PortefeuilleType.csv)
    strcpy(NomFichierPortefeuille, NomProprietaire);
    strcat(NomFichierPortefeuille, "_");
    strcat(NomFichierPortefeuille, PortefeuilleType);
    strcat(NomFichierPortefeuille, ".csv");

    // Init le compteur i
    i = nb_actions_portefeuille;
    
    // Ouvrir le fichier de portefeuille en mode Lecture
    f1 = fopen(NomFichierPortefeuille, "r");

    // Ignorer la premiere ligne qui est réservée pour les en-têtes
    fgets(ligne, sizeof ligne, f1);

    // Boucle de chatgement utilisant fscanf() <= Problème avec le retour à la ligne
    // while(!feof(f1))
    // {
    //     retour = fscanf(f1, "%[^,],%[^,],%[^,],%f,%d,%f%[^\n]\n", action.code_isin, action.symbole,action.nom_societe, &action.prix_achat_unit, &action.quantite, &action.seuil_declenchement, bidon);
    //     if(retour != EOF)
    //     {
    //         portefeuille[i++] = action;
    //     }
    // }

    // Boucle de chargement utilisant fgets() & sscanf()
    while(!feof(f1))
    {
        if(fgets(ligne, sizeof ligne, f1))
        {
            retour = sscanf(ligne, "%[^,],%[^,],%[^,],%f,%d,%f", action.code_isin, action.symbole,action.nom_societe, &action.prix_achat_unit, &action.quantite, &action.seuil_declenchement);
            if(retour != EOF && ligne[0] != '\0' && ligne[0] != '\n')
            {
                portefeuille[i++] = action;
            }
        }
    }

    fclose(f1);
    nb_actions_portefeuille = i;

    printf("Nombre de lignes chargée(s)           : %d\n", nb_actions_portefeuille); // une ligne = une action (vocab finance)
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
        printf("==========================================================================================================================\n");
        printf("|%-15s |%-10s |%-35s |%-15s |%-15s |%-20s|\n", "Code_ISIN", "Symbole", "Nom_Societe", "Prix", "Quantite", "Seuil_Declenchement");
        printf("==========================================================================================================================\n");
        for(i = 0; i < nb_actions_portefeuille; i++)
        {
            action = portefeuille[i];
            if(action.quantite > 0)
            {
                if(action.seuil_declenchement != PAS_DE_SEUIL_DECLENCHEMNT)
                {
                    printf("|%-15s |%-10s |%-35s |%-15.2f |%-15d |%-20.2f|\n", action.code_isin, action.symbole, action.nom_societe, action.prix_achat_unit, action.quantite, action.seuil_declenchement);
                }
                else
                { // Si le seuil de declenchement n'est pas defini, afficher Non_Defini à la place de 999
                    printf("|%-15s |%-10s |%-35s |%-15.2f |%-15d |%-20s|\n", action.code_isin, action.symbole, action.nom_societe, action.prix_achat_unit, action.quantite, "Non_Defini");
                }
            }
        }
        printf("==========================================================================================================================\n");
    }
}
/*---------- MENU OPERATION ----------*/
void MenuOperation()
{
    int choix = -1;

    while(choix != 0)
    {
        printf("================ MENU OPERATION ================\n");
        printf("-1- Ordre au marche\n");
        printf("-2- Ordre à cours limite\n");
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
/*---------- Ordre au marche ----------*/
void OrdreAuMarche()
{
    char symbole_input[TAILLE_CODE_ISIN];

    printf("================ ORDRE AU MARCHE ================\n");
    printf("Prix correspond au prix du marché.\n");

    printf("Veuillez saisir le code de la société concernée par votre opération : ");
    scanf("%s", symbole_input);
    conv_maj(symbole_input);

    AchatVente("ordre_au_marche", symbole_input, TYPE_OPERATION_NON_DEFINI, QUANTITE_NON_DEFINI);
}
/*---------- Ordre à cours limite ----------*/
void OrdreACoursLimite()
{
    char symbole_input[TAILLE_CODE_ISIN];

    printf("================ ORDRE A COURS LIMITE ================\n");

    printf("Veuillez saisir le code de la société concernée par votre opération : ");
    scanf("%s", symbole_input);
    conv_maj(symbole_input);

    AchatVente("ordre_a_cours_limite", symbole_input, TYPE_OPERATION_NON_DEFINI, QUANTITE_NON_DEFINI);
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
    char bidon[100]; // caractere pour consommer le retour à la ligne
    char ligne[TAILLE_LIGNE];
    FILE *f1;
    int retour;

    // Ouvrir le fichier de cours de bourse en mode Lecture
    f1 = fopen(COURS_DE_BOURSE_FILE_NAME, "r");

    // Ignorer la premiere ligne qui est réservée pour les en-têtes
    fgets(ligne, sizeof ligne, f1);

    // Boucle de chatgement utilisant fscanf() <= Problème avec le retour à la ligne
    // while(!feof(f1))
    // {
    //     retour = fscanf(f1, "%[^,],%[^,],%[^,],%d,%f%[^\n]\n", action.code_isin, action.symbole, action.nom_societe, &action.quantite, &action.prix_achat_unit, bidon);
    //     if(retour != EOF)
    //     {
    //         cours_bourse[nb_actions_cours_bourse++] = action;
    //     }
    // }

    // Boucle de chargement utilisant fgets() & sscanf()
    while(!feof(f1))
    {
        if (fgets(ligne, sizeof ligne, f1))
        {
            retour = sscanf(ligne, "%[^,],%[^,],%[^,],%d,%f", action.code_isin, action.symbole, action.nom_societe, &action.quantite, &action.prix_achat_unit);
            if (retour != EOF && ligne[0] != '\0' && ligne[0] != '\n')
            {
                cours_bourse[nb_actions_cours_bourse++] = action;
            }
        }
    }
        
    fclose(f1); 
}
/*---------- Afficher le cours de bourse ----------*/
void AffichageCoursDeBourse()
{
    int i;
    struct struct_action action;

    printf("========================================= COURS DE BOURSE ==========================================\n");
    printf("====================================================================================================\n");
    printf("|%-15s |%-10s |%-35s |%-15s |%-15s|\n", "Code_ISIN", "Symbole", "Nom_Societe", "Prix", "Quantite");
    printf("====================================================================================================\n");
    for(i = 0; i < nb_actions_cours_bourse; i++)
    { // boucle affichage
        action = cours_bourse[i];
        printf("|%-15s |%-10s |%-35s |%-15.2f |%-15d|\n", action.code_isin, action.symbole, action.nom_societe, action.prix_achat_unit, action.quantite);
    }
    printf("====================================================================================================\n");
    printf("%d indices chargée(s) !\n", nb_actions_cours_bourse);
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
/*---------- Charger les opérations en attente ----------*/
void ChargerOperationEnAttente()
{
    struct struct_action action;
    struct operation operation;
    char ligne[1000];
    FILE *f1;
    int retour;

    /* --- Boucle de chargement --- */
    
    f1 = fopen(OPERATIONS_EN_ATTENTE_FILE_NAME, "r");
    while(!feof(f1))
    {
        if(fgets(ligne, sizeof ligne, f1))
        {
            retour = sscanf(ligne, "%[^,],%[^,],%[^,],%[^,],%[^,],%f,%d,%c", operation.date, operation.heure, operation.proprietaire_portefeuille, operation.action.symbole, operation.action.code_isin, &operation.action.prix_achat_unit, &operation.action.quantite, &operation.type_operation);
            if(retour != EOF && ligne[0] != '\0' && ligne[0] != '\n')
            {
                strcpy(operation.statut, "En_attente");
                operations_en_attente[nb_operations_en_attente++] = operation;
            }
        }
    }
    fclose(f1);
}
/*---------- Mettre une opération en attente ----------*/
void MettreOperationEnAttente(struct struct_action action, float prix, int quantite, char type_operation)
{
    int i;
    char heure_courante[TAILLE_HEURE];
    struct operation operation;

    // Ajouter l'opération dans le tableau operations_en_attente
    operation.action = action;
    operation.action.prix_achat_unit = prix;
    operation.action.quantite = quantite;
    strcpy(operation.date, date_du_jour);
    GetHeureCourante(heure_courante);
    strcpy(operation.heure, heure_courante);
    operation.type_operation = type_operation;
    strcpy(operation.proprietaire_portefeuille, NomProprietaire);
    strcpy(operation.statut, "En_attente");
    operations_en_attente[nb_operations_en_attente++] = operation;
    nb_operation_en_attente_session++;

    /* --- Boucle de sauvegarde --- */
    MettreAJourDesOperationsEnAttente();
}
/*---------- Mettre une opération en attente ----------*/
void MettreAJourDesOperationsEnAttente()
{
    int i;
    FILE *f1;
    struct operation operation;

    /* --- Boucle de sauvegarde --- */
    f1 = fopen(OPERATIONS_EN_ATTENTE_FILE_NAME, "w");
    for(i = 0; i < nb_operations_en_attente; i++)
    {
        operation = operations_en_attente[i];
        if(strcmp(operation.statut, "Resolu") != 0)
        {
            fprintf(f1, "%s,%s,%s,%s,%s,%.2f,%d,%c\n", operation.date, operation.heure, operation.proprietaire_portefeuille, operation.action.symbole, operation.action.code_isin, operation.action.prix_achat_unit, operation.action.quantite, operation.type_operation);
        }
    }
    fclose(f1);
}
/*---------- Afficher des opérations en attente ----------*/
void AfficherOperationsEnAttente()
{
    int i;
    char type_operation_libelle[20];
    struct operation operation;

    /* --- Boucle d'affichage --- */
    printf("========================================================== OPERATIONS EN ATTENTE ==================================================================\n");
    printf("===================================================================================================================================================\n");
    printf("|%-15s |%-10s |%-30s |%-10s |%-15s |%-15s |%-20s |%-15s |\n", "Date", "Heure", "Proprietaire_Portefeuille", "Symbole", "Code_ISIN", "Prix", "Quantite", "Type_Operation");
    printf("===================================================================================================================================================\n");
    for(i = 0; i < nb_operations_en_attente; i++)
    {
        operation = operations_en_attente[i];
        if(strcmp(operation.proprietaire_portefeuille, NomProprietaire) == 0)
        {
            if(operation.type_operation == 'A')
            {
                strcpy(type_operation_libelle, "ACHAT");
            }
            else
            {
                strcpy(type_operation_libelle, "VENTE");
            }

            printf("|%-15s |%-10s |%-30s |%-10s |%-15s |%-15.2f |%-20d |%-15s |\n", operation.date, operation.heure, operation.proprietaire_portefeuille, operation.action.symbole, operation.action.code_isin, operation.action.prix_achat_unit, operation.action.quantite, type_operation_libelle);
        }
        
    }
    printf("===================================================================================================================================================\n");
}
/*---------- Charger l'historique des opérations ----------*/
void ChargerHistorique()
{
    struct struct_action action;
    struct operation operation;
    FILE *f1;
    char ligne[TAILLE_LIGNE];
    int retour;

    // Ouvrir le fichier historique en mode lecture
    f1 = fopen(HISTORIQUE_FILE_NAME, "r");

    //Ignorer la première ligne réservée pour les en-têtes
    fgets(ligne, sizeof ligne, f1);

    /* --- Boucle de chargement --- */
    while(!feof(f1))
    {
        if(fgets(ligne, sizeof ligne, f1))
        {
            retour = sscanf(ligne, "%[^,],%[^,],%[^,],%[^,],%[^,],%f,%d,%c,%s", operation.date, operation.heure, operation.proprietaire_portefeuille, operation.action.symbole, operation.action.code_isin, &operation.action.prix_achat_unit, &operation.action.quantite, &operation.type_operation, operation.statut);
            if(retour != EOF && ligne[0] != '\0' && ligne[0] != '\n')
            {
                historique[nb_operation_effectuee_total++] = operation;
            }
        }
    }
    fclose(f1);
}
/*---------- Enregistrer une opération dans l'historique ----------*/
void EnregistrerDansLHistorique(struct struct_action action, float prix, int quantite, char type_operation, char *statut)
{
    int i;
    FILE *f1;
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
    strcpy(operation.statut, statut);
    historique[nb_operation_effectuee_total++] = operation;

    /* --- Boucle de sauvegarde --- */
    f1 = fopen(HISTORIQUE_FILE_NAME, "w");
    fprintf(f1, "Date,Heure,Proprietaire_Portefeuille,Symbole,Code_ISIN,Prix,Quantite,Type_Operation,Statut\n");
    for(i = 0; i < nb_operation_effectuee_total; i++)
    {
        fprintf(f1, "%s,%s,%s,%s,%s,%.2f,%d,%c,%s\n", historique[i].date, historique[i].heure, historique[i].proprietaire_portefeuille, historique[i].action.symbole, historique[i].action.code_isin, historique[i].action.prix_achat_unit, historique[i].action.quantite, historique[i].type_operation, historique[i].statut);
    }
    fclose(f1);
}
/*---------- Afficher l'historique des operations ----------*/
void AffichageHisotrique()
{
    int i;
    struct operation operation;
    char type_operation_libelle[20];


    printf("===================================================================== HISTORIQUE ===================================================================================\n");
    printf("====================================================================================================================================================================\n");
    printf("|%-15s |%-10s |%-30s |%-10s |%-15s |%-15s |%-20s |%-15s |%-15s |\n", "Date", "Heure", "Proprietaire_Portefeuille", "Symbole", "Code_ISIN", "Prix", "Quantite", "Type_Operation", "Statut");
    printf("====================================================================================================================================================================\n");
    for(i = 0; i < nb_operation_effectuee_total; i++)
    {
        operation = historique[i];
        if(strcmp(operation.proprietaire_portefeuille, NomProprietaire) == 0)
        {
            operation = historique[i];

            if(operation.type_operation == 'A')
            {
                strcpy(type_operation_libelle, "ACHAT");
            }
            else
            {
                strcpy(type_operation_libelle, "VENTE");
            }

            printf("|%-15s |%-10s |%-30s |%-10s |%-15s |%-15.2f |%-20d |%-15s |%-15s |\n", operation.date, operation.heure, operation.proprietaire_portefeuille, operation.action.symbole, operation.action.code_isin, operation.action.prix_achat_unit, operation.action.quantite, type_operation_libelle, operation.statut);
        }
    }
    printf("====================================================================================================================================================================\n");
    
}
/*---------- Calculer la nouvelle valeur d'action dans le portefeuille (moyenne pondérée) ----------*/
float CaculNouveauPrix(float prix_1, int quantite_1, float prix_2, int quantite_2)
{
    return (prix_1*quantite_1 + prix_2*quantite_2)/(quantite_1+quantite_2);
}
/*---------- Alerte Seuil Déclenchement ----------*/
void AlerteSeuilDeclenchement()
{
    int i, index_action_cours_bourse;
    char ConfirmationSeuilDeclenchement;
    float prix_declenchement_superieur, prix_declenchement_inferieur;
    struct struct_action action_portefeuille, action_cours_bourse;

    for (i = 0; i < nb_actions_portefeuille; i++)
    {
        // Notre action pour un tour de boucle
        action_portefeuille = portefeuille[i];

        // La même action venant du cours de bourse
        index_action_cours_bourse = RechercheAction(action_portefeuille.symbole, cours_bourse, "cours_bourse");
        action_cours_bourse = cours_bourse[index_action_cours_bourse];

        // Les prix de declenchement
        prix_declenchement_superieur = action_portefeuille.prix_achat_unit + (action_portefeuille.prix_achat_unit * (action_portefeuille.seuil_declenchement / 100));
        prix_declenchement_inferieur = action_portefeuille.prix_achat_unit - (action_portefeuille.prix_achat_unit * (action_portefeuille.seuil_declenchement / 100));

        if (Egal(prix_declenchement_superieur, action_cours_bourse.prix_achat_unit) || Egal(prix_declenchement_inferieur, action_cours_bourse.prix_achat_unit))
        {
            // Le cas où le seuil de déclenchement est atteint
            printf("Le seuil de déclenchement pour l'action %s est atteint\n", action_cours_bourse.symbole);
            printf("Prix dans le portefeuille             : %.2f €\n", action_portefeuille.prix_achat_unit);
            printf("Prix du marche                        : %.2f €\n", action_cours_bourse.prix_achat_unit);

            ConfirmationSeuilDeclenchement = '\0';
            while (ConfirmationSeuilDeclenchement != 'O' && ConfirmationSeuilDeclenchement != 'N')
            {
                printf("Souhaitez-vous effectuer une operation ? (O pour Oui, N pour Non)    : ");
                while (getchar() != '\n');
                scanf("%c", &ConfirmationSeuilDeclenchement);
                ConfirmationSeuilDeclenchement = toupper(ConfirmationSeuilDeclenchement);

                if (ConfirmationSeuilDeclenchement != 'O' && ConfirmationSeuilDeclenchement != 'N')
                {
                    printf("La réponse attendue est : O pour Oui, N pour Non\n");
                }
            }

            // Si le gestionaire ne veut pas poursuivre avec le seuil de declenchement de l'action
            // Operation 
            // On continue la boucle pour verifier le seuil des autres actions
            if (ConfirmationSeuilDeclenchement == 'N')
            {
                EnregistrerDansLHistorique(action_cours_bourse, action_cours_bourse.prix_achat_unit, 0, '\0', "Abandonnee");
            }
            else
            {
                AchatVente("ordre_seuil_declenchement", action_portefeuille.symbole, TYPE_OPERATION_NON_DEFINI, QUANTITE_NON_DEFINI);
            }

        }
    }
}
/*---------- Modification le prix du cours de bourse ----------*/
void ModificationCoursBourse()
{
    char symbole_input[TAILLE_CODE_ISIN];
    char heure_courante[TAILLE_HEURE];
    int  i, index_action_recherche_cours_bourse;
    float prix_input;
    FILE *f1;
    struct struct_action action_cours_bourse;

    printf("================ MODIFICATION DU COURS DE BOURSE ================\n");
    printf("Veuillez saisir le code de la société concernée par votre opération : ");
    scanf("%s", symbole_input);
    conv_maj(symbole_input);

    // Recherche l'action dans le cours de bourse
    index_action_recherche_cours_bourse = RechercheAction(symbole_input, cours_bourse, "cours_bourse");
    if(index_action_recherche_cours_bourse == NON_TROUVE)
    {
        printf("L'action %s est introuvable. Veuillez réessayer...\n", symbole_input);
    }
    else
    {
        action_cours_bourse = cours_bourse[index_action_recherche_cours_bourse];

        // Modification du prix du cours de bourse
        printf("Le prix actuel                        : %.2f\n", action_cours_bourse.prix_achat_unit);
        printf("Veuillez saisir le nouveau prix       : ");
        scanf("%f", &prix_input);
        action_cours_bourse.prix_achat_unit = prix_input;
        cours_bourse[index_action_recherche_cours_bourse] = action_cours_bourse;

        // Sauvegarder la modfication
        f1 = fopen(COURS_DE_BOURSE_FILE_NAME, "w");
        fprintf(f1, "Code_ISIN,Symbole,Nom_Societe,Quantite,Prix\n");
        for(i = 0; i < nb_actions_cours_bourse; i++)
        {
            fprintf(f1, "%s,%s,%s,%d,%.2f\n", cours_bourse[i].code_isin, cours_bourse[i].symbole, cours_bourse[i].nom_societe, cours_bourse[i].quantite, cours_bourse[i].prix_achat_unit);
        }
        fclose(f1);

        printf("Modification réussie.\n");

        // Verification les seuils de declenchement
        AlerteSeuilDeclenchement();

        // Verification des operation en attente
        VerificationOperationEnAttente();
    }
}
/*---------- Comparaison de 2 chiffres décimaux ----------*/
int Egal(float f1, float f2)
{
    // Fonction pour comparer 2 chiffres décimaux avec une précision
    // Retourne 1 si les 2 chiffres sont "égaux"
    // Retourne 0 si ils sont "différents"
    float precision = 0.01;

    if (((f1 - precision) < f2) && ((f1 + precision) > f2))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
/*---------- Calculer et annoncer la somme d'operation (achat/vente) ----------*/
void CalculerSommeOperation(char *statut, char type_operation, float prix, int quantite)
{
    float somme_operation, frais_courtage_operation, frais_operation_pourcentage;

    if(strcmp(statut, "Reussi") == 0)
    {
        // Uniquement quand l'operation est reussie
        somme_operation = prix * quantite;
        
        if(strcmp(PortefeuilleType, "PEA") == 0)
        {
            // 0.5% pour PEA
            frais_operation_pourcentage = 0.005;
        }
        else
        {
            // 0.4% pour PEA
            frais_operation_pourcentage = 0.004;
        }

        frais_courtage_operation = somme_operation * frais_operation_pourcentage;
        valorisation_portefeuille.frais_courtage += frais_courtage_operation;

        if(type_operation == 'A')
        {
            // Somme de l'achat
            printf("Somme à payer                         : %.2f\n", somme_operation);
            printf("Frais d'operation                     : %.2f\n", frais_courtage_operation);
            valorisation_portefeuille.somme_titres_detenus += somme_operation;
            valorisation_portefeuille.solde -= (somme_operation + frais_courtage_operation); 
        }
        else
        {
            // Somme de la vente
            printf("Somme à collecter                     : %.2f\n", somme_operation);
            printf("Frais d'operation                     : %.2f\n", frais_courtage_operation);
            valorisation_portefeuille.somme_titres_detenus -= somme_operation;
            valorisation_portefeuille.solde += (somme_operation - frais_courtage_operation); 
        }
        MettreAJourValorisationPortefeuille();
    }
}
/*---------- Calculer la nouvelle solde si l'operation serait acceptée ----------*/
float CalculerNouvelleSolde(char type_operation, float prix, int quantite)
{
    float somme_operation, frais_courtage_operation, frais_operation_pourcentage, nouveau_solde;

    somme_operation = prix * quantite;
    
    if(strcmp(PortefeuilleType, "PEA") == 0)
    {
        // 0.5% pour PEA
        frais_operation_pourcentage = 0.005;
    }
    else
    {
        // 0.4% pour PEA
        frais_operation_pourcentage = 0.004;
    }

    frais_courtage_operation = somme_operation * frais_operation_pourcentage;

    if(type_operation == 'A')
    {
        // ACHAT
        nouveau_solde = valorisation_portefeuille.solde - (somme_operation + frais_courtage_operation); 
    }
    else
    {
        // VENTE
        nouveau_solde = valorisation_portefeuille.solde + (somme_operation - frais_courtage_operation); 
    }
    
    return nouveau_solde;
}
/*---------- Operation Achat/Vente ----------*/
void AchatVente(char *type_ordre, char symbole_input[], char type_operation_argument, float quantite_argument)
{
    char type_operation;
    char heure_courante[TAILLE_HEURE];
    char statut[TAILLE_STATUT_OPERATION];
    char ConfirmationVente; // O pour Oui, N pour Non
    char ConfirmationSeuilDeclenchement; // O pour Oui, N pour Non
    float prix_input;
    float nouveau_solde;
    float seuil_declenchement_input;
    int quantite_input, index_action_recherche_cours_bourse, index_action_recherche_portefeuille;
    struct struct_action action_portefeuille;
    struct struct_action action_cours_bourse;

    // Recherche l'action dans le cours de bourse
    index_action_recherche_cours_bourse = RechercheAction(symbole_input, cours_bourse, "cours_bourse");
    action_cours_bourse = cours_bourse[index_action_recherche_cours_bourse];

    // Recherche l'action dans le portefeuill
    index_action_recherche_portefeuille = RechercheAction(symbole_input, portefeuille, "portefeuille");
    action_portefeuille = portefeuille[index_action_recherche_portefeuille];

    // Si l'utilisateur saisie un symbole d'action qui n'esxiste pas dans le cours de bourse
    // Il faut afficher un message d'erreur et revenir au menu d'operation
    if(index_action_recherche_cours_bourse == NON_TROUVE)
    {
        printf("L'action que vous recherchez n'esxiste pas dans le cours de bourse. Veuillez réessayer...\n");
        return;
    }

    // On n'affiche pas le prix pour l'ordre au seuil de declenchement car il est deja affiche dans la module de l'ordre
    if(strcmp(type_ordre, "ordre_seuil_declenchement") != 0)
    {
        printf("Prix du marche                        : %.2f €\n", action_cours_bourse.prix_achat_unit);
    }

    // Quand il s'agit d'une opération en attente, il faut prendre l'argument quantite_argument comme quantite_input
    if(strcmp(type_ordre, "operation_en_attente") == 0 || strcmp(type_ordre, "cloture") == 0)
    {
        // Le cas ou l'argument type_operation_argument est defini
        // Usage principal : pour effectuer une operation_en_attente
        type_operation = type_operation_argument;
    }
    else
    {
        // Si l'argument type_operation_argument n'est pas défini, il faut demander l'utilisateur à saisir type_operation
        type_operation = '\0';
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
    }

    if(type_operation == 'A')
    {
        printf("Opération choisie                     : Achat\n");
        printf("Quantité disponbile                   : %d\n", action_cours_bourse.quantite);
    }
    else
    {
        if(index_action_recherche_portefeuille != NON_TROUVE)
        {
            printf("Opération choisie                     : Vente\n");
            printf("Quantité disponbile                   : %d\n", action_portefeuille.quantite);
        }
        else
        {
            // Le cas de la Vente a decouvert
            printf("Opération choisie                     : Vente\n");
            printf("Quantité disponbile                   : 0\n");
        }
    }
    
    // Quand il s'agit de la vente a decouvert (type_operation == 'V' && action_portefeuille.quantite == NON_TROUVE), on s'arrete l'operation et s'affiche toute suite le message de l'AMF
    if(type_operation != 'V' || index_action_recherche_portefeuille != NON_TROUVE)
    {
        // Quand il s'agit d'une opération en attente, il faut prendre l'argument quantite_argument comme quantite_input
        if(strcmp(type_ordre, "operation_en_attente") == 0 || strcmp(type_ordre, "cloture") == 0)
        {
            // Le cas ou l'argument quantite_argument est defini
            // Usage principal : pour effectuer une operation_en_attente
            quantite_input = quantite_argument;
        }
        else
        {
            printf("Veuillez saisir la quantité           : ");
            scanf("%d", &quantite_input);
        }

        // Quand il s'agit de l'ordre a cours limite, il faut saisir le prix d'achat/vente souhaite
        if(strcmp(type_ordre, "ordre_a_cours_limite") == 0)
        {
            printf("Veuillez saisir le prix               : ");
            scanf("%f", &prix_input);
        }   
    }
    else
    {
        quantite_input = 0;
    }

    // Calculer la nouvelle solde si l'operation serait acceptee
    if(strcmp(type_ordre, "ordre_a_cours_limite") == 0)
    {
        nouveau_solde = CalculerNouvelleSolde(type_operation, prix_input, quantite_input);
    }
    else
    {
        nouveau_solde = CalculerNouvelleSolde(type_operation, action_cours_bourse.prix_achat_unit, quantite_input);
    }

    // Quand il s'agit de l'achat et la somme à payer va faire dépasser le montant de l'investissement
    // On s'arrete l'operation et retourne au menu des operation
    if(type_operation == 'A' && nouveau_solde <= 0)
    {
        printf("Votre montant d'investissement sera depasse de %.2f €. La liquidité disponible sur votre portefeuille ne permet pas de conclure l'achat\n", -nouveau_solde);
        return;
    }
    
    if (type_operation == 'A')
    { // ACHAT
        if (quantite_input <= action_cours_bourse.quantite)
        {
            // On se rend compte que l'achat et la vente est le même entre les 3 ordres (uniquement dans le cadre de notre programme) quand
            // 1, Pour l'ordre a cour limité : le prix_input = prix du marché
            // 2, Pour l'ordre au seuil de declenchement : quand le seuil est atteint
            // 3, Pour l'ordre au cours de marche : pas de condition, l'operation est toujours acceptée
            // 4, Pour l'operation en attente (une fois qu'elle est passé ici): pas de condition, l'operation est toujours acceptée
            if (Egal(prix_input, action_cours_bourse.prix_achat_unit) || strcmp(type_ordre, "ordre_au_marche") == 0 || strcmp(type_ordre, "ordre_seuil_declenchement") == 0 || strcmp(type_ordre, "operation_en_attente") == 0 || strcmp(type_ordre, "cloture") == 0)
            {  
                // Quand le prix correspond au prix du marché, l'opération sera exécutée
                if (index_action_recherche_portefeuille == NON_TROUVE)
                {
                    // Saisie d'un seuil de déclenchement
                    printf("Achat de %d actions avec succès.\n", quantite_input);
                    ConfirmationSeuilDeclenchement = '\0';
                    while (ConfirmationSeuilDeclenchement != 'O' && ConfirmationSeuilDeclenchement != 'N')
                    {
                        printf("Voulez-vous définir un seuil de déclenchement : (O pour Oui, N pour Non) : ");
                        while (getchar() != '\n');
                        scanf("%c", &ConfirmationSeuilDeclenchement);
                        ConfirmationSeuilDeclenchement = toupper(ConfirmationSeuilDeclenchement);
                        if(ConfirmationSeuilDeclenchement == 'O')
                        {
                            printf("Saisissez le seuil de declenchement souhiate (Ex : 12.4 pour 12.4%%) : ");
                            scanf("%f", &seuil_declenchement_input);
                            action_cours_bourse.seuil_declenchement = seuil_declenchement_input;
                        }
                        else
                        {
                            action_cours_bourse.seuil_declenchement = PAS_DE_SEUIL_DECLENCHEMNT;
                        }
                        
                        if(ConfirmationSeuilDeclenchement != 'O' && ConfirmationSeuilDeclenchement != 'N')
                        {
                            printf("La réponse attendue est : O pour Oui, N pour Non\n");
                        }
                    }
                    action_cours_bourse.quantite = quantite_input;
                    portefeuille[nb_actions_portefeuille++] = action_cours_bourse;
                    cours_bourse[index_action_recherche_cours_bourse].quantite -= quantite_input;
                    printf("Achat de %d actions avec succès.\n", quantite_input);
                    strcpy(statut, "Reussi");
                }
                else
                {
                    // Calculer le nouveau prix à partir du prix d'achat venant du portefeuille, de la quantité dans la portefeuille et du prix du marché, de la quantité achetée
                    // Nouveau_prix = (prix_portefeuill*quantite_portfeuille + prix_marche*quantite_achetee) / (quantite_portfeuille + quantite_achetee)
                    action_portefeuille.prix_achat_unit = CaculNouveauPrix(action_portefeuille.prix_achat_unit, action_portefeuille.quantite, action_cours_bourse.prix_achat_unit, quantite_input);
                    action_portefeuille.quantite += quantite_input;
                    portefeuille[index_action_recherche_portefeuille] = action_portefeuille;
                    cours_bourse[index_action_recherche_cours_bourse].quantite -= quantite_input;
                    printf("Achat de %d actions avec succès.\n", quantite_input);
                    strcpy(statut, "Reussi");
                }
            }
            else
            {
                // Si le prix ne correspond pas au prix du marché, il faut mettre cette opération en attente
                MettreOperationEnAttente(action_cours_bourse, prix_input, quantite_input, type_operation);
                strcpy(statut, "En_attente");
                printf("Le prix du marché est different du prix souhaite, l'operation ordre au limite est mise en attente.\n");
            }
        }
        else
        {
            strcpy(statut, "Echec");
            printf("Quantité disponible insuffisante !\n");
        }
    }
    else
    { // VENTE
        if (index_action_recherche_portefeuille == NON_TROUVE)
        {
            strcpy(statut, "Echec");
            printf("Depuis Mardi 17 Mars 2020, l'AMF (Autorité des marché financiers) a interdit La vente à découvert sur 92 titres à la bourse de PARIS.\n");
        }
        else
        {
            // On se rend compte que l'achat et la vente est le même entre les 3 ordres (uniquement dans le cadre de notre programme) quand
            // 1, Pour l'ordre a cour limité : le prix_input = prix du marché
            // 2, Pour l'ordre au seuil de declenchement : quand le seuil est atteint
            // 3, Pour l'ordre au cours de marche : pas de condition, l'operation est toujours acceptée
            // 4, Pour l'operation en attente (une fois qu'elle est passé ici): pas de condition, l'operation est toujours acceptée
            if (Egal(prix_input, action_cours_bourse.prix_achat_unit) || strcmp(type_ordre, "ordre_au_marche") == 0 || strcmp(type_ordre, "ordre_seuil_declenchement") == 0 || strcmp(type_ordre, "operation_en_attente") == 0 || strcmp(type_ordre, "cloture") == 0)
            {
                // Quand le prix correspond au prix du marché, l'opération sera exécutée
                if (action_portefeuille.quantite >= quantite_input)
                {
                    // Si le portefeuill contient suffisamment d'actions à vendre, la quantié à vendre sera enlevée du portefeuille et ajoutée dans le cours de bourse
                    action_portefeuille.quantite -= quantite_input;
                    portefeuille[index_action_recherche_portefeuille] = action_portefeuille;
                    cours_bourse[index_action_recherche_cours_bourse].quantite += quantite_input;
                    printf("Vente de %d actions avec succès.\n", quantite_input);
                    strcpy(statut, "Reussi");
                }
                else
                {
                    // Si le portefeuill ne contient pas suffisamment d'actions à vendre, la quantié à vendre sera enlevée du portefeuille, l'action sera supprimée du portefeuille et cette quantité sera ajoutée dans le cours de bourse
                    ConfirmationVente = '\0';
                    while(ConfirmationVente != 'O' && ConfirmationVente != 'N')
                    {
                        printf("Vous disposez de quantité insuffisante, souhaitez-vous continuer la vente ? (O pour Oui, N pour Non) : "); // Si oui, la vente porte sur la quantité disponible. Si non, on ne fais rien.
                        while (getchar() != '\n');
                        scanf("%c", &ConfirmationVente);
                        ConfirmationVente = toupper(ConfirmationVente);
                        if(ConfirmationVente != 'O' && ConfirmationVente != 'N')
                        {
                            printf("La réponse attendue est : O pour Oui, N pour Non\n");
                        }
                        else
                        {
                            if (ConfirmationVente == 'N')
                            {
                                printf("Opération abandonnée...\n");
                                EnregistrerDansLHistorique(action_cours_bourse, action_cours_bourse.prix_achat_unit, quantite_input, type_operation, "Abandonnee");
                                return; // Annuler la vente et revenir au menu précédent
                            }
                        }
                    }
                    cours_bourse[index_action_recherche_cours_bourse].quantite += action_portefeuille.quantite;
                    printf("Vente de %d actions avec succès.\n", action_portefeuille.quantite);
                    action_portefeuille.quantite = 0;
                    portefeuille[index_action_recherche_portefeuille] = action_portefeuille;
                    strcpy(statut, "Reussi");
                }
            }
            else
            {
                // Si le prix ne correspond pas au prix du marché, il faut mettre cette opération en attente
                MettreOperationEnAttente(action_cours_bourse, prix_input, quantite_input, type_operation);
                strcpy(statut, "En_attente");
                printf("LLe prix du marché est different du prix souhaite, l'operation ordre au limite est mise en attente.\n");
            }
        }
    }
    // Calculer et annoncer la somme de l'achat et de la vente
    CalculerSommeOperation(statut, type_operation, action_cours_bourse.prix_achat_unit, quantite_input);

    // Enregistrer l'operation dans l'historique
    if(strcmp(type_ordre, "ordre_a_cours_limite") == 0)
    {
        // Pour l'ordre a cours limite, le prix d'operation enregistre dans l'historique est le prix_input
        EnregistrerDansLHistorique(action_cours_bourse, prix_input, quantite_input, type_operation, statut);
    }
    else
    {
        // Pour l'ordre au marche, le prix d'operation enregistre dans l'historique est aussi le prix du marche
        EnregistrerDansLHistorique(action_cours_bourse, action_cours_bourse.prix_achat_unit, quantite_input, type_operation, statut);
    }
    
    // Afficher la date et l'heure d'operation
    GetHeureCourante(heure_courante);
    printf("Date de l'opération                   : %s %s\n", date_du_jour, heure_courante);

    // Modification de portefeuille => à sauvegarder
    if(strcmp(statut, "Reussi") == 0)
    {
        affichage();
        a_sauvegarder = 1;
    }
}
/*-----------------------------------------------------------*/
void verif_sauvegarde()
{
    char ConfirmationSauvegarde;

    if (a_sauvegarder)
    {
        printf("Votre portefeuille a été modifié.\n");

        ConfirmationSauvegarde = '\0';

        while (ConfirmationSauvegarde != 'O' && ConfirmationSauvegarde != 'N')
        {
            printf("Voulez-vous faire une sauvegarde ? (O pour Oui/N pour Non) : ");
            while (getchar() != '\n');
            scanf("%c", &ConfirmationSauvegarde);
            ConfirmationSauvegarde = toupper(ConfirmationSauvegarde);
            if (ConfirmationSauvegarde != 'O' && ConfirmationSauvegarde != 'N')
            {
                printf("La réponse attendue est : O pour Oui, N pour Non\n");
            }
        }

        if(ConfirmationSauvegarde == 'O')
        {
            Sauvegarde();
        }
    }
}
/*---------- Sauvegarder l'etat du portefeuille au cas ou il y avait des operations effectues ----------*/
void Sauvegarde()
{
    int i;
    FILE *f1;
    struct struct_action action;

    // Ouvrir le fichier en mode "Ecriture"
    f1 = fopen(NomFichierPortefeuille, "w");

    // Ecrire l'en-tete
    fprintf(f1, "Code_ISIN,Symbole,Nom_Société,Prix_ou_Valeur,Quantité,Seuil_Declenchement\n");

    // Boucle de sauvegarde
    for(i = 0; i < nb_actions_portefeuille; i++)
    {
        action = portefeuille[i];
        if(action.quantite != 0)
        {
            fprintf(f1, "%s,%s,%s,%.2f,%d,%.2f\n", action.code_isin, action.symbole,action.nom_societe, action.prix_achat_unit, action.quantite, action.seuil_declenchement);
        }
    }
    fclose(f1);

    // Confirmation
    printf("%d lignes modifiées !\n", i);
}
/*---------- Verification des operation en attente ----------*/
void VerificationOperationEnAttente()
{
    int index_action_portefeuille, index_action_cours_bourse;
    char type_operation_libelle[20];
    char ConfirmationOperatioEnAttente;
    struct operation operation;
    struct struct_action action_cours_bourse;
    struct struct_action action_portefeuille;
    int i;

    for(i = 0; i < nb_operations_en_attente; i++)
    {
        operation = operations_en_attente[i];

        if(strcmp(operation.proprietaire_portefeuille, NomProprietaire) == 0 && strcmp(operation.statut, "En_attente") == 0)
        {
            // Recherche d'action dans le portefeille
            index_action_portefeuille = RechercheAction(operation.action.symbole, portefeuille, "portefeuille");
            action_portefeuille = portefeuille[index_action_portefeuille];

            // Recherche d'action dans le cours de bourse
            index_action_cours_bourse = RechercheAction(operation.action.symbole, cours_bourse, "cours_bourse");
            action_cours_bourse = cours_bourse[index_action_cours_bourse];

            // Si le prix du marché est matche avec le prix souhaite de l'operation en attente
            if (Egal(action_cours_bourse.prix_achat_unit, operation.action.prix_achat_unit))
            {
                // Afficher l'operation en attente 
                printf("Le prix souhaite d'une opération en attente est atteint\n");
                printf("========================================================== OPERATION EN ATTENTE ===================================================================\n");
                printf("===================================================================================================================================================\n");
                printf("|%-15s |%-10s |%-30s |%-10s |%-15s |%-15s |%-20s |%-15s |\n", "Date", "Heure", "Proprietaire_Portefeuille", "Symbole", "Code_ISIN", "Prix", "Quantite", "Type_Operation");
                printf("===================================================================================================================================================\n");
                if (operation.type_operation == 'A')
                {
                    strcpy(type_operation_libelle, "ACHAT");
                }
                else
                {
                    strcpy(type_operation_libelle, "VENTE");
                }
                printf("|%-15s |%-10s |%-30s |%-10s |%-15s |%-15.2f |%-20d |%-15s |\n", operation.date, operation.heure, operation.proprietaire_portefeuille, operation.action.symbole, operation.action.code_isin, operation.action.prix_achat_unit, operation.action.quantite, type_operation_libelle);
                printf("===================================================================================================================================================\n");

                // Demande de confirmation
                ConfirmationOperatioEnAttente = '\0';
                while (ConfirmationOperatioEnAttente != 'O' && ConfirmationOperatioEnAttente != 'N')
                {
                    printf("Souhaitez-vous effectuer cette operation ? (O pour Oui, N pour Non)  : ");
                    while (getchar() != '\n');
                    scanf("%c", &ConfirmationOperatioEnAttente);
                    ConfirmationOperatioEnAttente = toupper(ConfirmationOperatioEnAttente);

                    // Si le gestionaire ne veut pas poursuivre avec l'operation en attente
                    // On continue la boucle pour verifier le seuil des autres actions
                    if (ConfirmationOperatioEnAttente == 'N')
                    {
                        EnregistrerDansLHistorique(action_cours_bourse, action_cours_bourse.prix_achat_unit, 0, '\0', "Abandonnee");
                        strcpy(operation.statut, "Resolu");
                        operations_en_attente[i] = operation;
                        MettreAJourDesOperationsEnAttente();
                    }
                    else
                    {
                        AchatVente("operation_en_attente", action_cours_bourse.symbole, operation.type_operation, operation.action.quantite);
                        strcpy(operation.statut, "Resolu");
                        operations_en_attente[i] = operation;
                        MettreAJourDesOperationsEnAttente();
                    }

                    if (ConfirmationOperatioEnAttente != 'O' && ConfirmationOperatioEnAttente != 'N')
                    {
                        printf("La réponse attendue est : O pour Oui, N pour Non\n");
                    }
                }
            }
        }
    }
}
/*-----------------------------------------------------------*/
void ChargerValorisationPortefeuille()
{
    int i, retour;
    char ligne[1000];
    char NomPortefeuille[100];
    struct valorisation valorisation;
    FILE *f1;

    strcpy(NomPortefeuille, NomProprietaire);
    strcat(NomPortefeuille, "_");
    strcat(NomPortefeuille, PortefeuilleType);

    f1 = fopen(VALORISATION_FILE_NAME, "r");

    // Boucle de chargement utilisant fgets() & sscanf()
    i = 0;
    fgets(ligne, sizeof ligne, f1);
    while(!feof(f1))
    {
        if (fgets(ligne, sizeof ligne, f1))
        {
            retour = sscanf(ligne, "%[^,],%f,%d,%f,%f,%f,%s", valorisation.proprietaire_portefeuille, &valorisation.montant_investissement, &valorisation.frais_ouverture, &valorisation.somme_titres_detenus, &valorisation.frais_courtage, &valorisation.solde, valorisation.statut);
            if (retour != EOF && ligne[0] != '\0' && ligne[0] != '\n')
            {
                if (strcmp(valorisation.proprietaire_portefeuille, NomPortefeuille) == 0)
                {
                    valorisation_portefeuille = valorisation;
                    index_portefeuille_tab_valorisation = i;
                }
                tab_valorisation[i++] = valorisation;
            }
        }
    }
    fclose(f1);

    nb_portefeuilles = i;
}
/*-----------------------------------------------------------*/
void MettreAJourValorisationPortefeuille() 
{
    int i;
    struct valorisation valorisation;
    FILE *f1;

    tab_valorisation[index_portefeuille_tab_valorisation] = valorisation_portefeuille;

    f1 = fopen(VALORISATION_FILE_NAME, "w");
    fprintf(f1, "Portefeuille,Invesstissement,Frais d'ouverture,Titre detenue,Frais de courtage,Solde,Staut\n");
    for(i = 0; i < nb_portefeuilles; i++)
    {
        valorisation = tab_valorisation[i];
        fprintf(f1, "%s,%.2f,%d,%.2f,%.2f,%.2f,%s\n", valorisation.proprietaire_portefeuille, valorisation.montant_investissement, valorisation.frais_ouverture, valorisation.somme_titres_detenus, valorisation.frais_courtage, valorisation.solde, valorisation.statut);
    }
    fclose(f1);
}
/*-----------------------------------------------------------*/
void AffichageValorisationPortefeuille() 
{
    printf("============================================================== VALORISATION ============================================================================\n");
    printf("========================================================================================================================================================\n");
    printf("|%-30s |%-15s |%-20s |%-20s |%-20s |%-20s |%-12s |\n", "Portefeuille","Invesstissement","Frais d'ouverture","Titre detenue","Frais de courtage","Solde","Staut");
    printf("========================================================================================================================================================\n");
    printf("|%-30s |%-15.2f |%-20d |%-20.2f |%-20.2f |%-20.2f |%-12s |\n", valorisation_portefeuille.proprietaire_portefeuille, valorisation_portefeuille.montant_investissement, valorisation_portefeuille.frais_ouverture, valorisation_portefeuille.somme_titres_detenus, valorisation_portefeuille.frais_courtage, valorisation_portefeuille.solde, valorisation_portefeuille.statut);
    printf("========================================================================================================================================================\n");
}
/*-----------------------------------------------------------*/
void BienvenueMessage()
{
    printf("\n");
    printf("======================================================================\n");
    printf("========== Bonjour et Bienvenu sur le programme FrontEquity ==========\n");
    printf("======================================================================\n");
    printf("\n");
}
/*-----------------------------------------------------------*/
int DateValide(struct date ma_date)
{
    // Retourner 1 si la date est valide
    // Retourner 0 si la date n'est pas valide

    int date_valide = 0;
    int nb_jours_dans_1_mois;

    // Verifier si le mois et annee est valide et le jour est superieur ou egale 1
    if((ma_date.annee >= 0 && ma_date.annee <= 9999) && (ma_date.mois >=1 && ma_date.mois <= 12) && (ma_date.jour >= 1))
    {
        // Mois fevrier
        if(ma_date.mois == 2)
        {
            // Annee bissextile
            if((ma_date.annee%4 == 0 && ma_date.annee%100 != 0) || (ma_date.annee%400 == 0))
            {
                nb_jours_dans_1_mois = 29;
            }
            else
            {
                nb_jours_dans_1_mois = 28;
            }
        }
        else
        {
            if (ma_date.mois == 1 || ma_date.mois == 3 || ma_date.mois == 5 || ma_date.mois == 7 || ma_date.mois == 8 || ma_date.mois == 10 || ma_date.mois == 12)
            {
                nb_jours_dans_1_mois = 31;
            }
            else
            {
                nb_jours_dans_1_mois = 30;
            }
        }

        if(ma_date.jour <= nb_jours_dans_1_mois)
        {
            date_valide = 1;
        }
        else
        {
            date_valide = 0;
        }
    }
    else
    {
        date_valide = 0;
    }
    
    return date_valide;
}
/*------------- Verifier si le portefuille a plus de 5 ans ------------------*/
int PortfeuilleDeCinqAns(struct date date_ouverture, struct date date_cloture)
{
    int portefeuille_de_5_ans = 0;

    if(date_cloture.annee - date_ouverture.annee < 5)
    {
        portefeuille_de_5_ans = 0;
    }
    else
    {
        if(date_cloture.annee - date_ouverture.annee == 5)
        {
            // Portfeuille a 5 ans

            if(date_cloture.mois >= date_ouverture.mois)
            {
                if(date_cloture.mois == date_ouverture.mois)
                {
                    if(date_cloture.jour >= date_ouverture.jour)
                    {
                        portefeuille_de_5_ans = 1;
                    }
                    else
                    {
                        portefeuille_de_5_ans = 0;
                    }
                }
                else
                {
                    // date_cloture.mois > date_ouverture.mois
                    portefeuille_de_5_ans = 1;
                }
            }
            else
            {
                // Si le mois de la date de cloture est inferireur de celui de la date d'ouverture
                portefeuille_de_5_ans = 0;
            }
        }
        else
        {
            // Portfeuille de 5 ans et plus
            portefeuille_de_5_ans = 1;
        }
    }

    return portefeuille_de_5_ans;
}
/*------------- Vente de toutes les actions------------------*/
void VenteToutesActions()
{
    int i, index_recherche_cours_bourse;
    struct struct_action action_portefeuille;
    struct struct_action action_cours_bourse;

    for(i = 0; i < nb_actions_portefeuille; i++)
    {
        action_portefeuille = portefeuille[i];

        printf("Action                                : %s\n", action_portefeuille.symbole);
        AchatVente("cloture", action_portefeuille.symbole, 'V', action_portefeuille.quantite);
    }
}
/*------------- Calculer la somme de la vente de toutes les actions du portefeuilles ------------------*/
float CaclulerSommeVenteToutesActions()
{
    // Somme avec le frais de courtage inclu 

    int i, index_recherche_cours_bourse;
    float somme_des_ventes = 0;
    float somme_frais_courtage = 0;
    float frais_courtage_pourcentage = 0;
    struct struct_action action_portefeuille;
    struct struct_action action_cours_bourse;

    if(strcmp(PortefeuilleType, "PEA") == 0)
    {
        // 0.5% pour PEA
        frais_courtage_pourcentage = 0.005;
    }
    else
    {
        // 0.4% pour PEA
        frais_courtage_pourcentage = 0.004;
    }

    for(i = 0; i < nb_actions_portefeuille; i++)
    {
        action_portefeuille = portefeuille[i];

        index_recherche_cours_bourse = RechercheAction(action_portefeuille.symbole, cours_bourse, "cours_bourse");
        action_cours_bourse = cours_bourse[index_recherche_cours_bourse];

        somme_des_ventes += action_cours_bourse.prix_achat_unit * action_portefeuille.quantite;

        somme_frais_courtage += (action_cours_bourse.prix_achat_unit * action_portefeuille.quantite) * frais_courtage_pourcentage;
    }
    return (somme_des_ventes - somme_frais_courtage);
}
/*------------- Cloture du portefeuille ------------------*/
void CloturePortefeuille()
{
    int retour = 0;
    int portefeuille_de_5_ans = 0;
    float nouveau_solde = 0;
    float taux_impot = 0;
    float montant_impot = 0;
    float gain_net = 0;
    char ConfirmationCloture;
    struct date date_cloture;
    struct date date_ouverture;

    // Init date du jour
    sscanf(date_du_jour, "%d/%d/%d", &date_cloture.jour, &date_cloture.mois, &date_cloture.annee);

    // Inir date d'ouverture
    date_ouverture.jour = 0;
    date_ouverture.mois = 0;
    date_ouverture.annee = 0;
    
    printf("================ CLOTURE DU PORTEFEUILLE ================\n");
    printf("Date du jour                                  : %s\n", date_du_jour);
    while(retour != 3 || !DateValide(date_ouverture))
    {
        printf("Date d'ouverture du portefeuille (JJ/MM/AAAA) : ");
        while(getchar() != '\n');
        retour = scanf("%d/%d/%d", &date_ouverture.jour, &date_ouverture.mois, &date_ouverture.annee);
        // Si retour != 3, ça veut dire qu'on n'arrive pas à lire correctement la date
        // retour = 3 => Reussi à affecter 3 variables : date_ouverture.jour, date_ouverture.mois, date_ouverture.annee
        if(retour != 3 || !DateValide(date_ouverture))
        {
            printf("La date saisie n'est pas valide. Veuillez réessayer...\n");
        }
    }

    // Afficher la somme de la vente de toutes les actions
    printf("Investissement                      : %.2f €\n", valorisation_portefeuille.montant_investissement);
    nouveau_solde = valorisation_portefeuille.solde + CaclulerSommeVenteToutesActions();
    printf("Nouveau solde apres la cloture      : %.2f €\n", nouveau_solde);

    // Calculer le gain net
    gain_net = nouveau_solde - valorisation_portefeuille.montant_investissement;
    printf("Gain brut                           : %.2f €\n", gain_net);

    // Si le gain_net > 0, les impôts s'appliquent
    if(gain_net > 0)
    {
        // Calculer le taux et le montant d'imposition
        portefeuille_de_5_ans = PortfeuilleDeCinqAns(date_ouverture, date_cloture);
        if((strcmp(PortefeuilleType, "PEA") == 0 && !portefeuille_de_5_ans) || (strcmp(PortefeuilleType, "COMPTE_TITRE") == 0))
        {
            // Taux d'impot est 30% sur le gain net si :
            // 1, Il s'agit d'un PEA et le portefeuille a moins de 5 ans
            // 2, Il s'agit d'un compte titre
            taux_impot = 0.3;
        }
        else
        {
            taux_impot = 0.172;
        }
        printf("Taux d'imposition sur le gain net   : %.2f %%\n", taux_impot*100);

        // Calculer le montant d'imposition
        montant_impot = gain_net * taux_impot;
        printf("Montant d'imposition                : %.2f €\n", montant_impot);
    }
    else
    {   
        printf("Montant d'imposition                : 0 €\n");
    }
    
    // Demande de confirmation
    ConfirmationCloture = '\0';
    while(ConfirmationCloture != 'O' && ConfirmationCloture != 'N')
    {
        printf("Souhaitez-vous poursuivre avec la cloture ? (O pour Oui, N pour Non) : ");
        while (getchar() != '\n');
        scanf("%c", &ConfirmationCloture);
        ConfirmationCloture = toupper(ConfirmationCloture);
        if(ConfirmationCloture != 'O' && ConfirmationCloture != 'N')
        {
            printf("La réponse attendue est : O pour Oui, N pour Non\n");
        }
    }

    if (ConfirmationCloture == 'N')
    {
        printf("Opération abandonnée...\n");
        return;
    }
    
    VenteToutesActions();
    strcpy(valorisation_portefeuille.statut, "Cloture");
    valorisation_portefeuille.somme_titres_detenus = 0;
    valorisation_portefeuille.frais_courtage = 0;
    valorisation_portefeuille.solde = 0;
    MettreAJourValorisationPortefeuille();
}