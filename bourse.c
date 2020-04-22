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
#define TAILLE_DATE 20
#define TAILLE_HEURE 10
#define TAILLE_STATUT_OPERATION 20
#define COURS_DE_BOURSE_FILE_NAME "COURS_DE_BOURSE.csv"
#define OPERATIONS_EN_ATTENTE_FILE_NAME "OPERATIONS_EN_ATTENTE.csv"
#define HISTORIQUE_FILE_NAME "HISTORIQUE.csv"

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

/*---------- Déclaration préliminaires ----------*/
void MenuPortefeuille();
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
void ChargerHistorique();
void EnregistrerDansLHistorique(struct struct_action action, float prix, int quantite, char type_operation, char *statut);
float CaculNouveauPrix(float prix_1, int quantite_1, float prix_2, int quantite_2);
int Egal(float f1, float f2);
void CalculerSommeOperation(char *statut, char type_operation, float prix, int quantite);

/* Déclaration des variables globales */
int nb_actions_portefeuille = 0;
int nb_actions_cours_bourse = 0;
int nb_operations_en_attente  = 0;
int nb_operation_effectuee = 0;
char date_du_jour[TAILLE_DATE]; // Format : JJ/MM/AAAA
char NomProprietaire[100];
struct struct_action portefeuille[NOMBRE_MAX_ACTIONS];
struct struct_action cours_bourse[NOMBRE_MAX_ACTIONS];
struct operation operations_en_attente[NOMBRE_MAX_OPERATIONS];
struct operation historique[NOMBRE_MAX_OPERATIONS];

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

    // Chargement de l'historique
    ChargerHistorique();

    // Verification les seuils de declenchement
    AlerteSeuilDeclenchement();

    // Afficher la date du jour
    printf("La date du jour est                   : %s\n", date_du_jour);

    // Init choix
    int choix = -1;

    // Boucle du Menu Principal
    while(choix != 0)
    {
        printf("================ MENU PRINCIPAL ================\n");
        printf("-1- Gestion de portefeuille\n");
        printf("-2- Menu des operations\n");
        printf("-3- Affichage du cours de bourse\n");
        printf("-4- Modification du cours de bourse\n");
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
/*---------- Chargement de portefeuille ----------*/
void chargement() // ask for person name not the file name
{
    int i;
    struct struct_action action;
    FILE *f1;
    char NomFichier[100], NomProprietaireInput[100], PortefeuilleType[10];
    char bidon[100]; // caractere pour consommer le retour à la ligne
    char ligne[1000];
    int retour;

    printf("Nom du propriétaire de portefeuille : ");
    scanf("%s", NomProprietaire);
    conv_maj(NomProprietaire);
    while(strcmp(PortefeuilleType, "PEA") != 0 && strcmp(PortefeuilleType, "COMPTE_TITRE") != 0)
    {
        printf("Portefeille type (PEA/COMPTE_TITRE) : ");
        scanf("%s", PortefeuilleType);
        conv_maj(PortefeuilleType);
        if(strcmp(PortefeuilleType, "PEA") != 0 && strcmp(PortefeuilleType, "COMPTE_TITRE") != 0)
        {
            printf("La réponse attendue est             : PEA/COMPTE_TITRE\n");
        }
    }

    // Nom du fichier à charger à partir de NomProprietaire et PortefeuilleType (format : NomProprietaireInput_PortefeuilleType.csv)
    strcpy(NomFichier, NomProprietaire);
    strcat(NomFichier, "_");
    strcat(NomFichier, PortefeuilleType);
    strcat(NomFichier, ".csv");

    // Init le compteur i
    i = nb_actions_portefeuille;
    
    // Ouvrir le fichier de portefeuille en mode Lecture
    f1 = fopen(NomFichier, "r");

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
        fgets(ligne, sizeof ligne, f1);
        retour = sscanf(ligne, "%[^,],%[^,],%[^,],%f,%d,%f", action.code_isin, action.symbole,action.nom_societe, &action.prix_achat_unit, &action.quantite, &action.seuil_declenchement);
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
                    printf("|%-15s |%-10s |%-35s |%-15f |%-15d |%-20f|\n", action.code_isin, action.symbole, action.nom_societe, action.prix_achat_unit, action.quantite, action.seuil_declenchement);
                }
                else
                { // Si le seuil de declenchement n'est pas defini, afficher Non_Defini à la place de 999
                    printf("|%-15s |%-10s |%-35s |%-15f |%-15d |%-20s|\n", action.code_isin, action.symbole, action.nom_societe, action.prix_achat_unit, action.quantite, "Non_Defini");
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
    char type_operation;
    char symbole_input[TAILLE_CODE_ISIN];
    char heure_courante[TAILLE_HEURE];
    char statut[TAILLE_STATUT_OPERATION];
    char ConfirmationVente, ConfirmationSeuilDeclenchement;
    int quantite_input, index_action_recherche_cours_bourse, index_action_recherche_portefeuille;
    float seuil_declenchement_input;
    struct struct_action action_portefeuille;
    struct struct_action action_cours_bourse;

    printf("================ ORDRE AU MARCHE ================\n");
    printf("Prix correspond au prix du marché.\n");

    printf("Veuillez saisir le code de la société concernée par votre opération : ");
    scanf("%s", symbole_input);
    conv_maj(symbole_input);

    // Recherche l'action dans le cours de bourse
    index_action_recherche_cours_bourse = RechercheAction(symbole_input, cours_bourse, "cours_bourse");
    action_cours_bourse = cours_bourse[index_action_recherche_cours_bourse];

    // Recherche l'action dans le portefeuill
    index_action_recherche_portefeuille = RechercheAction(symbole_input, portefeuille, "portefeuille");
    action_portefeuille = portefeuille[index_action_recherche_portefeuille];

    printf("Le prix du marché                     : %f\n", action_cours_bourse.prix_achat_unit);

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
        printf("Opération choisie                     : Achat\n");
        printf("Quantité disponbile                   : %d\n", action_cours_bourse.quantite);
    }
    else
    {
        printf("Opération choisie                     : Vente\n");
        printf("Quantité disponbile                   : %d\n", action_portefeuille.quantite);
    }
    
    printf("Veuillez saisir la quantité           : ");
    scanf("%d", &quantite_input);

    if(type_operation == 'A')
    { // ACHAT
        if (quantite_input <= action_cours_bourse.quantite)
        {
            if (index_action_recherche_portefeuille == NON_TROUVE)
            {
                // Saisie d'un seuil de déclenchement
                printf("Achat de %d actions avec succès.\n", quantite_input);
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
                strcpy(statut, "Reussi");
                affichage();
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
                affichage();
            }
        }
        else
        {
            printf("Quantité disponible insuffisante !\n");
            strcpy(statut, "Echoue");
        }
    }
    else
    { // VENTE
        if (index_action_recherche_portefeuille == NON_TROUVE)
        {
            printf("La vente à découvert est suspendue suite aux consignes de l'AMF (Autorité des marché financiers).\n");
            strcpy(statut, "Echoue");
        }
        else
        {
            if (action_portefeuille.quantite >= quantite_input)
            {
                // Si le portefeuill contient suffisamment d'actions à vendre, la quantié à vendre sera enlevée du portefeuille et ajoutée dans le cours de bourse
                action_portefeuille.quantite -= quantite_input;
                portefeuille[index_action_recherche_portefeuille] = action_portefeuille;
                cours_bourse[index_action_recherche_cours_bourse].quantite += quantite_input;
                printf("Vente de %d actions avec succès.\n", quantite_input);
                strcpy(statut, "Reussi");
                affichage();
            }
            else
            {
                // Si le portefeuill ne contient pas suffisamment d'actions à vendre, la quantié à vendre sera enlevée du portefeuille, l'action sera supprimée du portefeuille et cette quantité sera ajoutée dans le cours de bourse
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
                affichage();
            }
        }
    }
    // Calculer et annoncer la somme de l'achat et de la vente
    CalculerSommeOperation(statut, type_operation, action_cours_bourse.prix_achat_unit, quantite_input);

    // Enregistrer l'operation dans l'historique
    // Pour l'ordre au marche, le prix d'operation enregistre dans l'historique est aussi le prix du marche
    EnregistrerDansLHistorique(action_cours_bourse, action_cours_bourse.prix_achat_unit, quantite_input, type_operation, statut);

    // Afficher la date et l'heure d'operation
    GetHeureCourante(heure_courante);
    printf("Date de l'opération                   : %s %s\n", date_du_jour, heure_courante);
}
/*---------- Ordre à cours limite ----------*/
void OrdreACoursLimite()
{
    char type_operation;
    char symbole_input[TAILLE_CODE_ISIN];
    char heure_courante[TAILLE_HEURE];
    char statut[TAILLE_STATUT_OPERATION];
    char ConfirmationVente; // O pour Oui, N pour Non
    char ConfirmationSeuilDeclenchement; // O pour Oui, N pour Non
    float prix_input;
    float seuil_declenchement_input;
    int quantite_input, index_action_recherche_cours_bourse, index_action_recherche_portefeuille;
    struct struct_action action_portefeuille;
    struct struct_action action_cours_bourse;

    printf("================ ORDRE A COURS LIMITE ================\n");

    printf("Veuillez saisir le code de la société concernée par votre opération : ");
    scanf("%s", symbole_input);
    conv_maj(symbole_input);

    // Recherche l'action dans le cours de bourse
    index_action_recherche_cours_bourse = RechercheAction(symbole_input, cours_bourse, "cours_bourse");
    action_cours_bourse = cours_bourse[index_action_recherche_cours_bourse];

    // Recherche l'action dans le portefeuill
    index_action_recherche_portefeuille = RechercheAction(symbole_input, portefeuille, "portefeuille");
    action_portefeuille = portefeuille[index_action_recherche_portefeuille];

    printf("Le prix du marché                     : %f\n", action_cours_bourse.prix_achat_unit);

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
        printf("Opération choisie                     : Achat\n");
        printf("Quantité disponbile                   : %d\n", action_cours_bourse.quantite);
    }
    else
    {
        printf("Opération choisie                     : Vente\n");
        printf("Quantité disponbile                   : %d\n", action_portefeuille.quantite);
    }
    
    printf("Veuillez saisir la quantité           : ");
    scanf("%d", &quantite_input);

    printf("Veuillez saisir le prix               : ");
    scanf("%f", &prix_input);

    if (type_operation == 'A')
    { // ACHAT
        if (quantite_input <= action_cours_bourse.quantite)
        {
            if (Egal(prix_input, action_cours_bourse.prix_achat_unit))
            {  
                // Quand le prix correspond au prix du marché, l'opération sera exécutée
                if (index_action_recherche_portefeuille == NON_TROUVE)
                {
                    // Saisie d'un seuil de déclenchement
                    printf("Achat de %d actions avec succès.\n", quantite_input);
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
                    affichage();
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
                    affichage();
                }
            }
            else
            {
                // Si le prix ne correspond pas au prix du marché, il faut mettre cette opération en attente
                MettreOperationEnAttente(action_cours_bourse, prix_input, quantite_input, type_operation);
                strcpy(statut, "En_attente");
                printf("Le prix actuel ne correspond pas au prix souhaite, l'operation est mise en attente.\n");
            }
        }
        else
        {
            strcpy(statut, "Echoue");
            printf("Quantité disponible insuffisante !\n");
        }
    }
    else
    { // VENTE
        if (index_action_recherche_portefeuille == NON_TROUVE)
        {
            strcpy(statut, "Echoue");
            printf("La vente à découvert est suspendue suite aux consignes de l'AMF (Autorité des marché financiers).\n");
        }
        else
        {
            if (Egal(prix_input, action_cours_bourse.prix_achat_unit))
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
                    affichage();
                }
                else
                {
                    // Si le portefeuill ne contient pas suffisamment d'actions à vendre, la quantié à vendre sera enlevée du portefeuille, l'action sera supprimée du portefeuille et cette quantité sera ajoutée dans le cours de bourse
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
                    affichage();
                }
            }
            else
            {
                // Si le prix ne correspond pas au prix du marché, il faut mettre cette opération en attente
                MettreOperationEnAttente(action_cours_bourse, prix_input, quantite_input, type_operation);
                strcpy(statut, "En_attente");
                printf("Le prix actuel ne correspond pas au prix souhaite, l'operation est mise en attente.\n");
            }
        }
    }
    // Calculer et annoncer la somme de l'achat et de la vente
    CalculerSommeOperation(statut, type_operation, prix_input, quantite_input);

    // Enregistrer l'operation dans l'historique
    // Pour l'ordre a cours limite, le prix d'operation enregistre dans l'historique est le prix_input
    EnregistrerDansLHistorique(action_cours_bourse, prix_input, quantite_input, type_operation, statut);

    // Afficher la date et l'heure d'operation
    GetHeureCourante(heure_courante);
    printf("Date de l'opération                   : %s %s\n", date_du_jour, heure_courante);
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
    char ligne[1000];
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
        fgets(ligne, sizeof ligne, f1);
        retour = sscanf(ligne, "%[^,],%[^,],%[^,],%d,%f", action.code_isin, action.symbole, action.nom_societe, &action.quantite, &action.prix_achat_unit);
        if(retour != EOF)
        {
            cours_bourse[nb_actions_cours_bourse++] = action;
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
        printf("|%-15s |%-10s |%-35s |%-15f |%-15d|\n", action.code_isin, action.symbole, action.nom_societe, action.prix_achat_unit, action.quantite);
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
/*---------- Charger l'historique des opérations ----------*/
void ChargerHistorique()
{
    int i;
    struct struct_action action;
    struct operation operation;
    FILE *f1;
    char ligne[1000];
    int retour;

    /* --- Boucle de chargement --- */
    i = nb_operation_effectuee;
    
    // Ouvrir le fichier historique en mode lecture
    f1 = fopen(HISTORIQUE_FILE_NAME, "r");

    //Ignorer la première ligne réservée pour les en-têtes
    fgets(ligne, sizeof ligne, f1);

    while(!feof(f1))
    {
        fgets(ligne, sizeof ligne, f1);
        retour = sscanf(ligne, "%[^,],%[^,],%[^,],%[^,],%[^,],%f,%d,%c,%s", operation.date, operation.heure, operation.proprietaire_portefeuille, operation.action.symbole, operation.action.code_isin, &operation.action.prix_achat_unit, &operation.action.quantite, &operation.type_operation, operation.statut);
        if(retour != EOF)
        {
            historique[i++] = operation;
        }
    }
    fclose(f1);
    nb_operation_effectuee = i;
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
    historique[nb_operation_effectuee++] = operation;

    /* --- Boucle de sauvegarde --- */
    f1 = fopen(HISTORIQUE_FILE_NAME, "w");
    fprintf(f1, "Date,Heure,Proprietaire_Portefeuille,Symbole,Code_ISIN,Prix,Quantite,Type_Operation,Statut\n");
    for(i = 0; i < nb_operation_effectuee; i++)
    {
        fprintf(f1, "%s,%s,%s,%s,%s,%f,%d,%c,%s\n", historique[i].date, historique[i].heure, historique[i].proprietaire_portefeuille, historique[i].action.symbole, historique[i].action.code_isin, historique[i].action.prix_achat_unit, historique[i].action.quantite, historique[i].type_operation, historique[i].statut);
    }
    fclose(f1);
}
/*---------- Calculer la nouvelle valeur d'action dans le portefeuille (moyenne pondérée) ----------*/
float CaculNouveauPrix(float prix_1, int quantite_1, float prix_2, int quantite_2)
{
    return (prix_1*quantite_1 + prix_2*quantite_2)/(quantite_1+quantite_2);
}
/*---------- Alerte Seuil Déclenchement ----------*/
void AlerteSeuilDeclenchement()
{
    int i, index_action_cours_bourse, quantite_input;
    float prix_declenchement_superieur, prix_declenchement_inferieur;
    char type_operation, ConfirmationVente;
    char heure_courante[TAILLE_HEURE];
    char statut[TAILLE_STATUT_OPERATION];
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
            printf("Prix dans le portefeuille             : %f €\n", action_portefeuille.prix_achat_unit);
            printf("Prix du marche                        : %f €\n", action_cours_bourse.prix_achat_unit);
            while (type_operation != 'A' && type_operation != 'V')
            {
                printf("Opération (A pour Achat, V pour Vente): ");
                while (getchar() != '\n');
                scanf("%c", &type_operation);
                type_operation = toupper(type_operation);
                if (type_operation != 'A' && type_operation != 'V')
                {
                    printf("La réponse attendue est : A pour Achat ou V pour Vente\n");
                }
            }

            if (type_operation == 'A')
            {
                printf("Opération choisie                     : Achat\n");
                printf("Quantité disponbile                   : %d\n", action_cours_bourse.quantite);
            }
            else
            {
                printf("Opération choisie                     : Vente\n");
                printf("Quantité disponbile                   : %d\n", action_portefeuille.quantite);
            }

            printf("Veuillez saisir la quantité           : ");
            scanf("%d", &quantite_input);

            if (type_operation == 'A')
            { // ACHAT
                if (quantite_input <= action_cours_bourse.quantite)
                {
                    // Calculer le nouveau prix à partir du prix d'achat venant du portefeuille, de la quantité dans la portefeuille et du prix du marché, de la quantité achetée
                    // Nouveau_prix = (prix_portefeuill*quantite_portfeuille + prix_marche*quantite_achetee) / (quantite_portfeuille + quantite_achetee)
                    action_portefeuille.prix_achat_unit = CaculNouveauPrix(action_portefeuille.prix_achat_unit, action_portefeuille.quantite, action_cours_bourse.prix_achat_unit, quantite_input);
                    action_portefeuille.quantite += quantite_input;
                    portefeuille[i] = action_portefeuille;
                    action_cours_bourse.quantite -= quantite_input;
                    cours_bourse[index_action_cours_bourse] = action_cours_bourse;
                    printf("Achat de %d actions avec succès.\n", quantite_input);
                    strcpy(statut, "Reussi");
                    affichage();
                }
                else
                {
                    printf("Quantité disponible insuffisante !\n");
                    strcpy(statut, "Echoue");
                }
            }
            else
            { // VENTE
                if (action_portefeuille.quantite >= quantite_input)
                {
                    // Si le portefeuill contient suffisamment d'actions à vendre, la quantié à vendre sera enlevée du portefeuille et ajoutée dans le cours de bourse
                    action_portefeuille.quantite -= quantite_input;
                    portefeuille[i] = action_portefeuille;
                    action_cours_bourse.quantite += quantite_input;
                    cours_bourse[index_action_cours_bourse] = action_cours_bourse;
                    printf("Vente de %d actions avec succès.\n", quantite_input);
                    strcpy(statut, "Reussi");
                    affichage();
                }
                else
                {
                    // Si le portefeuill ne contient pas suffisamment d'actions à vendre, la quantié à vendre sera enlevée du portefeuille, l'action sera supprimée du portefeuille et cette quantité sera ajoutée dans le cours de bourse
                    while (ConfirmationVente != 'O' && ConfirmationVente != 'N')
                    {
                        printf("Vous disposez de quantité insuffisante, souhaitez-vous continuer la vente ? (O pour Oui, N pour Non) : "); // Si oui, la vente porte sur la quantité disponible. Si non, on ne fais rien.
                        while (getchar() != '\n')
                            ;
                        scanf("%c", &ConfirmationVente);
                        ConfirmationVente = toupper(ConfirmationVente);
                        if (ConfirmationVente != 'O' && ConfirmationVente != 'N')
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
                            else
                            {
                                action_cours_bourse.quantite += action_portefeuille.quantite;
                                printf("Vente de %d actions avec succès.\n", action_portefeuille.quantite);
                                action_portefeuille.quantite = 0;
                                portefeuille[i] = action_portefeuille;
                                cours_bourse[index_action_cours_bourse] = action_cours_bourse;
                                strcpy(statut, "Reussi");
                                affichage();
                            }
                        }
                    }
                }
            }
            // Calculer et annoncer la somme de l'achat et de la vente
            CalculerSommeOperation(statut, type_operation, action_cours_bourse.prix_achat_unit, quantite_input);

            // Enregistrer l'operation dans l'historique
            // Pour l'ordre au seuil de declenchement, le prix d'operation enregistre dans l'historique est aussi le prix du marche 
            EnregistrerDansLHistorique(action_cours_bourse, action_cours_bourse.prix_achat_unit, quantite_input, type_operation, statut);

            // Afficher la date et l'heure d'operation
            GetHeureCourante(heure_courante);
            printf("Date de l'opération                   : %s %s\n", date_du_jour, heure_courante);
        }
    }
}
/*---------- Modification le prix du cours de bourse ----------*/
void ModificationCoursBourse()
{
    char symbole_input[TAILLE_CODE_ISIN];
    char heure_courante[TAILLE_HEURE];
    int  index_action_recherche_cours_bourse;
    float prix_input;
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
        printf("Le prix actuel                        : %f\n", action_cours_bourse.prix_achat_unit);
        printf("Veuillez saisir le nouveau prix       : ");
        scanf("%f", &prix_input);
        action_cours_bourse.prix_achat_unit = prix_input;
        cours_bourse[index_action_recherche_cours_bourse] = action_cours_bourse;
        printf("Modification réussie.\n");

        // Verification les seuils de declenchement
        AlerteSeuilDeclenchement();
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
    float somme_operation;

    if(strcmp(statut, "Reussi") == 0)
    {
        // Uniquement quand l'operation est reussie
        somme_operation = prix * quantite;
        if(type_operation == 'A')
        {
            // Somme de l'achat
            printf("Somme à payer                         : %.2f\n", somme_operation);
        }
        else
        {
            // Somme de la vente
            printf("Somme à verser                        : %.2f\n", somme_operation);
        }
    }
}