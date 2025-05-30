#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#define TAILLE_HACH_IDF 100
#define TAILLE_HACH_KW 30
#define TAILLE_HACH_SEP 30


typedef struct element {
    int state;
    char name[20];
    char code[20];
    char type[20];
    int borne_inf;
    int taille;
    float val;
    struct element* next;
} element;

typedef struct elt {
    int state;
    char name[20];
    char type[20];
    struct elt* next;
} elt;


element* tab[TAILLE_HACH_IDF] = {NULL};
elt* tabm[TAILLE_HACH_KW] = {NULL};
elt* tabs[TAILLE_HACH_SEP] = {NULL};


void initialisation(void) {
    int i;
    for (i = 0; i < TAILLE_HACH_IDF; i++)
        tab[i] = NULL;
    for (i = 0; i < TAILLE_HACH_KW; i++)
        tabm[i] = NULL;
    for (i = 0; i < TAILLE_HACH_SEP; i++)
        tabs[i] = NULL;
}


unsigned int fonctionHachage(const char* cle) {
    unsigned int hash = 0;
    while (*cle) {
        hash = (hash * 31) + (*cle++);
    }
    return hash;
}


void inserer(char entite[], char code[], char type[], float val, int y) {
    unsigned int indice = fonctionHachage(entite);
    
    switch (y) {
        case 0:
            indice %= TAILLE_HACH_IDF;
            element* nouveau = (element*)malloc(sizeof(element));
            nouveau->state = 1;
            strcpy(nouveau->name, entite);
            strcpy(nouveau->code, code);
            strcpy(nouveau->type, type);
            nouveau->val = val;
            nouveau->next = tab[indice];
            tab[indice] = nouveau;
            break;
        case 1: 
            indice %= TAILLE_HACH_KW;
            elt* nouveau_kw = (elt*)malloc(sizeof(elt));
            nouveau_kw->state = 1;
            strcpy(nouveau_kw->name, entite);
            strcpy(nouveau_kw->type, code);
            nouveau_kw->next = tabm[indice];
            tabm[indice] = nouveau_kw;
            break;
        case 2: 
            indice %= TAILLE_HACH_SEP;
            elt* nouveau_sep = (elt*)malloc(sizeof(elt));
            nouveau_sep->state = 1;
            strcpy(nouveau_sep->name, entite);
            strcpy(nouveau_sep->type, code);
            nouveau_sep->next = tabs[indice];
            tabs[indice] = nouveau_sep;
            break;
    }
}


void rechercher(char entite[], char code[], char type[], float val, int y) {
    unsigned int indice = fonctionHachage(entite);
    
    switch (y) {
        case 0:
            indice %= TAILLE_HACH_IDF;
            {
                element* curr = tab[indice];
                while (curr != NULL && strcmp(entite, curr->name) != 0)
                    curr = curr->next;
                if (curr == NULL)
                    inserer(entite, code, type, val, 0);
            }
            break;

        case 1: 
            indice %= TAILLE_HACH_KW;
            {
                elt* curr = tabm[indice];
                while (curr != NULL && strcmp(entite, curr->name) != 0)
                    curr = curr->next;
                if (curr == NULL)
                    inserer(entite, code, type, val, 1);
            }
            break;

        case 2:
            indice %= TAILLE_HACH_SEP;
            {
                elt* curr = tabs[indice];
                while (curr != NULL && strcmp(entite, curr->name) != 0)
                    curr = curr->next;
                
                if (curr == NULL) {
                    inserer(entite, code, type, val, 2);
                }
            }
            break;
        
        case 3: // same as case 0
            indice %= TAILLE_HACH_IDF;
            {
                element* curr = tab[indice];
                while (curr != NULL && strcmp(entite, curr->name) != 0)
                    curr = curr->next;
                if (curr == NULL)
                    inserer(entite, code, type, val, 0);
                else
                    printf("entite existe deja\n");
            }
            break;
    }
}


void afficher_ts(void) {
    printf("\n***************Table des symboles IDF*****************\n");
    printf("______________________________________________________\n");
    printf(" Nom_Entite  | Code_Entite | Type_Entite | Val_Entite\n");
    printf("______________________________________________________\n");
    int i;
    for (i = 0; i < TAILLE_HACH_IDF; i++) {
        element* curr = tab[i];
        while (curr != NULL) {
            if (curr->state == 1) {
                printf("%12s |%12s | %11s | %10.2f\n",
                       curr->name, curr->code, curr->type, curr->val);
            }
            curr = curr->next;
        }
    }

    printf("\n**Table des symboles mots cles**\n");
    printf("_________________________________\n");
    printf("  NomEntite   |   CodeEntite \n");
    printf("_________________________________\n");
    for (i = 0; i < TAILLE_HACH_KW; i++) {
        elt* curr = tabm[i];
        while (curr != NULL) {
            if (curr->state == 1) {
                printf("%13s |%12s \n", curr->name, curr->type);
            }
            curr = curr->next;
        }
    }

    printf("\n**Table des symboles separateurs**\n");
    printf("_________________________________\n");
    printf("  NomEntite   |   CodeEntite \n");
    printf("_________________________________\n");
    for (i = 0; i < TAILLE_HACH_SEP; i++) {
        elt* curr = tabs[i];
        while (curr != NULL) {
            if (curr->state == 1) {
                printf("%13s | %12s \n", curr->name, curr->type);
            }
            curr = curr->next;
        }
    }
}



//**********************************************************************************/


/* update IDF to IDF_CONST when met with CONST declaration */
void update_cst(char entite[], char val[]) {
    unsigned int indice = fonctionHachage(entite) % TAILLE_HACH_IDF;
    element* curr = tab[indice];

    // Search for the correct element in the linked list
    while (curr != NULL && strcmp(curr->name, entite) != 0) curr = curr->next;

    // Ensure that the entity exists in the symbol table
    if (curr == NULL) {
        printf("ERREUR: Identificateur %s non trouvé dans la table des symboles!\n", entite);
        return;
    }


    // Determine the type dynamically
    char type[10];
    if (strchr(val, '.') != NULL) {
        float cst_float = strtof(val, NULL);
        curr->val = cst_float;
        strcpy(type, "FLOAT");
    } else {
        int cst_int = atoi(val);
        curr->val = cst_int;
        strcpy(type, "INTEGER");
    }


    // Change its code to IDF_CONST, update its type and its value
    strcpy(curr->code, "IDF_CONST");
    strcpy(curr->type, type);
}


/* checks if an IDF is declared twice */
int double_declaration (char entite[]){
    unsigned int indice = fonctionHachage(entite) % TAILLE_HACH_IDF;
    element* curr = tab[indice];

    
    while (curr != NULL && strcmp(curr->name, entite) != 0) curr = curr->next;

    if (curr == NULL) {
        printf("ERREUR: Identificateur %s non trouvé dans la table des symboles!\n", entite);
        return;
    }


    if (strcmp(curr->type, "") == 0) return 1; // IDF first declared
    else return 0; // IDF double declared
}


/* insert the according type to the IDF, and taille if a VECTOR */
void inserer_type_taille(char entite[],char type[],char bi[],char t[]){
    unsigned int indice = fonctionHachage(entite) % TAILLE_HACH_IDF;
    element* curr = tab[indice];
    
    
    while (curr != NULL && strcmp(curr->name, entite) != 0) curr = curr->next;

    if (curr == NULL) {
        printf("ERREUR: Identificateur %s non trouvé dans la table des symboles!\n", entite);
        return;
    }

   
    int bi_int = atoi(bi);
    int t_int = atoi(t);
    curr->borne_inf = bi_int;
    curr->taille = t_int;
   
    strcpy(curr->type, type);
}


/* check if IDF is CONST */
int is_const(char entite[]){
    unsigned int indice = fonctionHachage(entite) % TAILLE_HACH_IDF;
    element* curr = tab[indice];

    while (curr != NULL && strcmp(curr->name, entite) != 0) curr = curr->next;

    if (curr == NULL) {
        printf("ERREUR: Identificateur %s non trouvé dans la table des symboles!\n", entite);
        return;
    }


    if (strcmp(curr->code, "IDF_CONST") == 0) return 1; // IDF is CONST
    else return 0; // IDF is not CONST
}


/* check if chaine is [$%#&] and if its compatible with IDF's type */
int check_type_match_read(char chaine[], char entite[]){
    unsigned int indice = fonctionHachage(entite) % TAILLE_HACH_IDF;
    element* curr = tab[indice];
    
    while (curr != NULL && strcmp(curr->name, entite) != 0) curr = curr->next;

    if (curr == NULL) {
        printf("ERREUR: Identificateur %s non trouvé dans la table des symboles!\n", entite);
        return;
    }


    if( (chaine[0] == '"') && (chaine[2] == '"') && 

        ( chaine[1]=='$'  &&  strcmp(curr->type, "INTEGER")==0 ) ||
        ( chaine[1]=='%'  &&  strcmp(curr->type, "FLOAT")==0 ) ||
        ( chaine[1]=='#'  &&  strcmp(curr->type, "STRING")==0 ) ||
        ( chaine[1]=='&'  &&  strcmp(curr->type, "CHAR")==0 )
    ) return 1; // Valid match

    else return 0; // Type mismatch       
}


/* check if chaine contains [$%#&] and if its compatible with IDF's type */
int check_type_match_display(char chaine[], char entite[]){
    unsigned int indice = fonctionHachage(entite) % TAILLE_HACH_IDF;
    element* curr = tab[indice];

    while (curr != NULL && strcmp(curr->name, entite) != 0) curr = curr->next;

    if (curr == NULL) {
        printf("ERREUR: Identificateur %s non trouvé dans la table des symboles!\n", entite);
        return;
    }


    int l = strlen(chaine);
    l = l - 1; //sauter guillemet du fin
    int i = 1; //sauter guillemet du debut 
    
    while (i<l){
        if(
            ( chaine[i]=='$'  &&  strcmp(curr->type, "INTEGER")==0 ) ||
            ( chaine[i]=='%'  &&  strcmp(curr->type, "FLOAT")==0 ) ||
            ( chaine[i]=='#'  &&  strcmp(curr->type, "STRING")==0 ) ||
            ( chaine[i]=='&'  &&  strcmp(curr->type, "CHAR")==0 )
        ) return 1; // Valid match

        else i++;       
    }
    return 0; // Type mismatch
}


/* check if idf's type is CHAR */
int check_type_char(char entite[]){
    unsigned int indice = fonctionHachage(entite) % TAILLE_HACH_IDF;
    element* curr = tab[indice];
    
    while (curr != NULL && strcmp(curr->name, entite) != 0) curr = curr->next;

    if (curr == NULL) {
        printf("ERREUR: Identificateur %s non trouvé dans la table des symboles!\n", entite);
        return;
    }


    if( strcmp(curr->type,"CHAR")==0 ) return 1; // is char
    else return 0; // is not char
}


/* check if idf's type is STRING */
int check_type_string(char entite[]){
    unsigned int indice = fonctionHachage(entite) % TAILLE_HACH_IDF;
    element* curr = tab[indice];
    
    while (curr != NULL && strcmp(curr->name, entite) != 0) curr = curr->next;

    if (curr == NULL) {
        printf("ERREUR: Identificateur %s non trouvé dans la table des symboles!\n", entite);
        return;
    }


    if( strcmp(curr->type,"STRING")==0 ) return 1; // is string
    else return 0; // is not string
}


/* check if index of VECTOR is out of bounds */
int depassement_taille(char entite[],char index[]){
    unsigned int indice = fonctionHachage(entite) % TAILLE_HACH_IDF;
    element* curr = tab[indice];

    while (curr != NULL && strcmp(curr->name, entite) != 0) curr = curr->next;

    if (curr == NULL) {
        printf("ERREUR: Identificateur %s non trouvé dans la table des symboles!\n", entite);
        return;
    }


    int borne_inf = (curr->borne_inf);
    int taille = (curr->taille);
    int borne_sup = borne_inf + taille-1;

    int index_int = atoi(index);
    
    if( (borne_inf<=index_int) && (index_int<=(borne_sup)) ) return 0; // pas de depassement
    else return 1; // index out of bounds
}


/* get the IDF's type from the symbol table */
// unused
char* get_type(char entite[]) {
    unsigned int indice = fonctionHachage(entite) % TAILLE_HACH_IDF;
    element* curr = tab[indice];

    while (curr != NULL && strcmp(curr->name, entite) != 0) curr = curr->next;

    if (curr == NULL) {
        printf("ERREUR: Identificateur %s non trouvé dans la table des symboles!\n", entite);
        return;
    }


    return curr->type;  // Return the stored type
}






//!******************************************************************/
/* Get the type ID (1=INT, 2=FLOAT, 3=CHAR, 4=STRING) for a given entity */
int get_type_id(char entite[]) {
    // Handle constants first (numeric literals)
    if (entite[0] >= '0' && entite[0] <= '9' || 
        (entite[0] == '-' && entite[1] >= '0' && entite[1] <= '9')) {
        // Check if it contains a decimal point
        if (strchr(entite, '.') != NULL)
            return 2; // FLOAT
        else
            return 1; // INTEGER
    }
    
    // Handle array elements
    char base_name[50];
    if (strchr(entite, '[') != NULL) {
        int i = 0;
        while (entite[i] != '[' && i < strlen(entite)) {
            base_name[i] = entite[i];
            i++;
        }
        base_name[i] = '\0';
        entite = base_name;
    }

    // Lookup in symbol table
    unsigned int indice = fonctionHachage(entite) % TAILLE_HACH_IDF;
    element* curr = tab[indice];
    
    while (curr != NULL && strcmp(curr->name, entite) != 0) 
        curr = curr->next;

    if (curr == NULL) {
        printf("ERREUR: Identificateur %s non trouvé dans la table des symboles!\n", entite);
        return 0; // Unknown type
    }

    // Map type strings to type IDs
    if (strcmp(curr->type, "INTEGER") == 0) return 1;
    if (strcmp(curr->type, "FLOAT") == 0) return 2;
    if (strcmp(curr->type, "CHAR") == 0) return 3;
    if (strcmp(curr->type, "STRING") == 0) return 4;
    
    return 0; // Unknown type
}

/* Check if two types are compatible for arithmetic operations */
int are_compatible_for_arithmetic(char op1[], char op2[]) {
    int type1 = get_type_id(op1);
    int type2 = get_type_id(op2);
    
    // Only numeric types (INT and FLOAT) are compatible for arithmetic
    if ((type1 == 1 || type1 == 2) && (type2 == 1 || type2 == 2))
        return 1; // Compatible
    return 0; // Incompatible
}

/* Determine the resulting type of an arithmetic operation */
int resulting_type(char op1[], char op2[]) {
    int type1 = get_type_id(op1);
    int type2 = get_type_id(op2);
    
    // If either operand is FLOAT, result is FLOAT
    if (type1 == 2 || type2 == 2)
        return 2; // FLOAT
    else
        return 1; // INTEGER
}

/* Check if target can accept the source type in assignment */
int check_assignment_compatibility(char target[], char source[]) {
    int target_type = get_type_id(target);
    int source_type = get_type_id(source);
    
    // Same types are always compatible
    if (target_type == source_type)
        return 1;
    
    // INT can accept INT, but not FLOAT
    if (target_type == 1 && source_type == 2)
        return 0;
    
    // FLOAT can accept INT or FLOAT
    if (target_type == 2 && (source_type == 1 || source_type == 2))
        return 1;
    
    // CHAR and STRING are only compatible with their own types
    if ((target_type == 3 && source_type != 3) || 
        (target_type == 4 && source_type != 4))
        return 0;
    
    return 0; // Default: incompatible
}

/* Get string representation of a type ID */
char* type_id_to_string(int type_id) {
    switch(type_id) {
        case 1: return "INTEGER";
        case 2: return "FLOAT";
        case 3: return "CHAR";
        case 4: return "STRING";
        default: return "UNKNOWN";
    }
}



/* Update the value of an identifier in the symbol table */
void update_value(char entite[], char value[]) {
    unsigned int indice = fonctionHachage(entite) % TAILLE_HACH_IDF;
    element* curr = tab[indice];

    // Search for the identifier in the linked list
    while (curr != NULL && strcmp(curr->name, entite) != 0) {
        curr = curr->next;
    }

    // If identifier not found, print error
    if (curr == NULL) {
        printf("ERREUR: Identificateur %s non trouve dans la table des symboles!\n", entite);
        return;
    }

    // Check if the variable is a constant
    if (strcmp(curr->code, "IDF_CONST") == 0) {
        printf("ERREUR SEMANTIQUE: modification de la constante %s \n",entite);
        return;
    }

    // Convert value to float (since val is float in element struct)
    float val_float;
    if (strchr(value, '.') != NULL) {
        // Value is a float
        val_float = strtof(value, NULL);
    } else {
        // Value is an integer
        val_float = (float)atoi(value);
    }

    // Update the value
    curr->val = val_float;
}
