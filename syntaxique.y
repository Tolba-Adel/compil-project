%{
    #include <stdio.h>
    #include <stdbool.h>

    int nb_ligne=1, nb_colonne=1;

    char suavType[20];

    int qc=0;

    // function to generate temp variable names
    char* generer_temp() {
        static char temp[20];
        static int cpt_temp = 0;
        sprintf(temp, "temp%d", cpt_temp++);
        return temp;
    }
%}

%union{
    int entier;
    float real;
    char* str;
    char str_val[50]; // to store temporary string values
}

%token <str>idf <real>cst mc_data mc_code mc_end mc_const mc_vector mc_read mc_display mc_if mc_else <str>mc_int <str>mc_float <str>mc_char <str>mc_string <str>caractere <str>chaine mc_and mc_or mc_not mc_g mc_ge mc_l mc_le mc_eq mc_di mc_for

%left mc_or
%left mc_and
%nonassoc mc_not
%left mc_g mc_ge mc_l mc_le mc_eq mc_di
%left '+' '-'
%left '*' '/'

// declare types for non-terminals
%type <str_val> var 
%type <str_val> exp 
%type <str_val> list_exp
%type <str_val> division

%%
S: idf mc_data declaration mc_end mc_code list_inst mc_end mc_end
{printf("Programme Syntaxiquement Correcte\n"); YYACCEPT;}
;

declaration: declaration declaration_simple
           | declaration declaration_tab
           | declaration declaration_const
           | declaration_simple
           | declaration_tab
           | declaration_const
;

declaration_simple: type ':' list_var ';' 
;

type: mc_int {strcpy(suavType,$1);}
    | mc_float {strcpy(suavType,$1);}
    | mc_char {strcpy(suavType,$1);}
    | mc_string {strcpy(suavType,$1);}
;

list_var: list_var '|'  idf {
            if(double_declaration($3) == 1) inserer_type_taille($3,suavType,"1","1");
            else printf("ERREUR SEMANTIQUE: double declaration de %s, a la ligne %d, et la colonne %d\n", $3, nb_ligne, nb_colonne); 
        }
        | idf {
            if(double_declaration($1) == 1) inserer_type_taille($1,suavType,"1","1");
            else printf("ERREUR SEMANTIQUE: double declaration de %s, a la ligne %d, et la colonne %d\n", $1, nb_ligne, nb_colonne); 
        }
;

declaration_tab: mc_vector ':' idf '[' cst ',' cst ':' type ']' ';' {
                    if(double_declaration($3) == 1){
                        char bi_str[20],t_int[20];
                        sprintf(bi_str, "%d", (int) $5);
                        sprintf(t_int, "%d", (int) $7);
                        inserer_type_taille($3,suavType,bi_str,t_int);

                        // generate bounds quadruplet
                        generate_bounds($5, $7);
                        quadr("ADEC", $3, "vide", "vide");
                    } 
                    else printf("ERREUR SEMANTIQUE: double declaration de %s, a la ligne %d, et la colonne %d\n", $3, nb_ligne, nb_colonne); 
                }
;

declaration_const: mc_const ':' idf '=' cst ';' {
        char cst_str[20];
        if ((int) $5 == $5) sprintf(cst_str, "%d", (int) $5);
        else sprintf(cst_str, "%f", $5);
        update_cst($3,cst_str);
    }
;



list_inst: list_inst inst | inst
;

inst: inst_aff 
    | inst_read
    | inst_display
    | inst_if
    | inst_for
;

inst_aff: idf '=' list_exp ';' {
            if(double_declaration($1) == 1) 
                printf("ERREUR SEMANTIQUE: %s non declare, a la ligne %d, et la colonne %d\n", $1, nb_ligne, nb_colonne);
            else if (is_const($1) == 1)
                printf("ERREUR SEMANTIQUE: %s est une constante a la ligne %d, et la colonne %d\n",$1,nb_ligne, nb_colonne);
            else quadr(":=", $3, "vide", $1);
        }
        | idf '[' cst ']' '=' list_exp ';' {
            if(double_declaration($1) == 1) 
                printf("ERREUR SEMANTIQUE: %s non declare, a la ligne %d, et la colonne %d\n", $1, nb_ligne, nb_colonne);
            else if (is_const($1) == 1)
                printf("ERREUR SEMANTIQUE: %s est une constante a la ligne %d, et la colonne %d\n",$1,nb_ligne, nb_colonne);
            else {
                char index_str[20];
                sprintf(index_str, "%d", (int) $3);
                if (depassement_taille($1,index_str) == 1)
                    printf("ERREUR SEMANTIQUE: Depassement de taille pour le tableau %s , a la ligne %d, t la colonne %d\n",$1,nb_ligne, nb_colonne);
                else {
                    char table_with_index[50];
                    sprintf(table_with_index, "%s[%s]", $1, index_str);
                    quadr(":=", $6, "vide", table_with_index);
                } 
            } 
        }
        | idf '=' caractere ';' {
            if(double_declaration($1) == 1) 
                printf("ERREUR SEMANTIQUE: %s non declare, a la ligne %d, et la colonne%d\n", $1, nb_ligne, nb_colonne);
            else if (is_const($1) == 1)
                printf("ERREUR SEMANTIQUE: %s est une constante a la ligne %d, et la colonne %d\n",$1,nb_ligne, nb_colonne);
            else if (check_type_char($1) == 0)
                printf("ERREUR SEMANTIQUE: %s doit etre de type CHAR, a la ligne %d, et la colonne %d\n", $1, nb_ligne, nb_colonne);
            else quadr(":=", $3, "vide", $1);                
        }
        | idf '[' cst ']' '=' caractere ';' {
            if(double_declaration($1) == 1) 
                printf("ERREUR SEMANTIQUE: %s non declare, a la ligne %d, et la colonne%d\n", $1, nb_ligne, nb_colonne);
            else if (is_const($1) == 1)
                printf("ERREUR SEMANTIQUE: %s est une constante a la ligne %d, et la colonne %d\n",$1,nb_ligne, nb_colonne);
            else if (check_type_char($1) == 0)
                printf("ERREUR SEMANTIQUE: %s doit etre de type CHAR, a la ligne %d, et la colonne %d\n", $1, nb_ligne, nb_colonne);
            else {
                char index_str[20];
                sprintf(index_str, "%d", (int) $3);
                char table_with_index[50];
                sprintf(table_with_index, "%s[%s]", $1, index_str);
                if (depassement_taille($1,index_str) == 1)
                    printf("ERREUR SEMANTIQUE: Depassement de taille pour le tableau %s , a la ligne %d, t la colonne %d\n",$1,nb_ligne, nb_colonne);
                else quadr(":=", $6, "vide", table_with_index);    
            } 
        }
        | idf '=' chaine ';' {
            if(double_declaration($1) == 1) 
                printf("ERREUR SEMANTIQUE: %s non declare, a la ligne %d, et la colonne%d\n", $1, nb_ligne, nb_colonne);
            else if (is_const($1) == 1)
                printf("ERREUR SEMANTIQUE: %s est une constante a la ligne %d, et la colonne %d\n",$1,nb_ligne, nb_colonne);
            else if (check_type_string($1) == 0)
                printf("ERREUR SEMANTIQUE: %s doit etre de type STRING, a la ligne %d, et la colonne %d\n", $1, nb_ligne, nb_colonne);
            else quadr(":=", $3, "vide", $1);
        }
;

var: idf {
        if(double_declaration($1) == 1) 
            printf("ERREUR SEMANTIQUE: %s non declare, a la ligne %d, et la colonne %d\n", $1, nb_ligne, nb_colonne);
        else {strcpy($$, $1);}
    } 
    | idf '[' cst ']' {
        if(double_declaration($1) == 1) 
            printf("ERREUR SEMANTIQUE: %s non declare, a la ligne %d, et la colonne %d\n", $1, nb_ligne, nb_colonne);
        else if (depassement_taille($1,$3) == 1)
                printf("ERREUR SEMANTIQUE: Depassement de taille pour le tableau %s , a la ligne %d, t la colonne %d\n",$1,nb_ligne, nb_colonne);
        else {
            char arr_elem[50];
            sprintf(arr_elem, "%s[%d]", $1, $3);
            strcpy($$, arr_elem);
        }
    }
    | cst {
        char cst_str[20];
        if ((int) $1 == $1) sprintf(cst_str, "%d", (int) $1);
        else sprintf(cst_str, "%f", $1);
        strcpy($$, cst_str);
    }
;

list_exp: list_exp exp {
            strcpy($$, $2);
        }
        | exp {
            strcpy($$, $1);
        }
;

exp: exp '+' var {
        char* temp = generer_temp();

        int val1 = atoi($1);
        int val2 = atoi($3);
        int result = val1 + val2;

        if (result == (int)result) sprintf(temp, "%d", (int)result);
        else sprintf(temp, "%.2f", result);
        quadr("+", $1, $3, temp);
        strcpy($$, temp);
    }
    | exp '-' var {
        char* temp = generer_temp();

        int val1 = atoi($1);
        int val2 = atoi($3);
        int result = val1 - val2;

        if (result == (int)result) sprintf(temp, "%d", (int)result);
        else sprintf(temp, "%.2f", result);
        quadr("-", $1, $3, temp);
        strcpy($$, temp);
    }
    | exp '*' var {
        char* temp = generer_temp();

        int val1 = atoi($1);
        int val2 = atoi($3);
        int result = val1 * val2;

        if (result == (int)result) sprintf(temp, "%d", (int)result);
        else sprintf(temp, "%.2f", result);
        quadr("*", $1, $3, temp);
        strcpy($$, temp);
    }
    | division {strcpy($$, $1);}
    | var {strcpy($$, $1);}
;


division: exp '/' idf {
            if(double_declaration($3) == 1) 
                printf("ERREUR SEMANTIQUE: %s non declare, a la ligne %d, et la colonne %d\n", $3, nb_ligne, nb_colonne);
            else {
                char* temp = generer_temp();
                quadr("/", $1, $3, temp);
                strcpy($$, temp);
            }
        } 
        | exp '/' cst {
        if ($3==0) 
            printf("ERREUR SEMANTIQUE: Division par zero a la ligne %d\n",nb_ligne);  
        else {
            char cst_str[20];
            if ((int) $3 == $3) sprintf(cst_str, "%d", (int) $3);
            else sprintf(cst_str, "%f", $3);

            char* temp = generer_temp();

            int val1 = atoi($1);
            int result = val1 / $3;

            if (result == (int)result) sprintf(temp, "%d", (int)result);
            else sprintf(temp, "%.2f", result);
            quadr("/", $1, cst_str, temp);
            strcpy($$, temp);
        }
}
;


inst_read: mc_read '(' chaine ':' '@' idf ')' ';' {
            if(double_declaration($6) == 1) 
                printf("ERREUR SEMANTIQUE: %s non declare, a la ligne %d, et la colonne %d\n", $6, nb_ligne, nb_colonne);
            else if(check_type_match_read($3,$6) == 0)
                printf("ERREUR SEMANTIQUE: type incompatible, a la ligne %d, et la colonne %d\n", nb_ligne, nb_colonne);
        } 
;


inst_display: mc_display '(' chaine ':' idf ')' ';' {
                if(double_declaration($5) == 1) 
                    printf("ERREUR SEMANTIQUE: %s non declare, a la ligne %d, et la colonne %d\n", $5, nb_ligne, nb_colonne);
                else if(check_type_match_display($3,$5) == 0)
                    printf("ERREUR SEMANTIQUE: type incompatible, a la ligne %d, et la colonne %d\n", nb_ligne, nb_colonne);
            } 
;


inst_if: mc_if '(' condition ')' ':' list_inst mc_else ':' list_inst mc_end
;

condition: list_exp comparaison list_exp
        | condition mc_and condition
        | condition mc_or condition
        | mc_not condition
        | '(' condition ')'
;

comparaison: mc_g | mc_ge | mc_l | mc_le | mc_eq | mc_di
;


inst_for: mc_for '(' idf ':' list_exp ':' list_exp ')' list_inst mc_end
;



%%
main(){
    yyparse();

    afficher_ts();
    afficher_qdr();
}

yywrap()
{}

int yyerror(char* msg){
    printf("ERREUR SYNTAXIQUE a la ligne %d, et la colonne %d\n",nb_ligne,nb_colonne);
}
