%{
    #include <stdio.h>
    #include <stdbool.h>

    int nb_ligne=1, nb_colonne=1;

    char suavType [20];

    int deb_else=0, qc=0, fin_if=0;

    // function to generate temporary variable names
    int cpt_temp = 0;
    int c=0;
    char* generer_temp() {
        static char temp[20];
        sprintf(temp, "temp%d", cpt_temp++);
        return temp;
    }
%}

%union{
    int entier;
    float real;
    char* str;
}

%token <str>idf <entier>cst mc_data mc_code mc_end mc_const mc_vector mc_read mc_display mc_if mc_else <str>mc_int <str>mc_float <str>mc_char <str>mc_string <str>caractere <str>chaine mc_and mc_or mc_not mc_g mc_ge mc_l mc_le mc_eq mc_di mc_for

%left mc_or
%left mc_and
%nonassoc mc_not
%left mc_g mc_ge mc_l mc_le mc_eq mc_di
%left '+' '-'
%left '*' '/'


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
            if(double_declaration($3) == 1) inserer_type_taille($3,suavType,1,1);
            else printf("ERREUR SEMANTIQUE: double declaration de %s, a la ligne %d, et la colonne %d\n", $3, nb_ligne, nb_colonne); 
        }
        | idf {
            if(double_declaration($1) == 1) inserer_type_taille($1,suavType,1,1);
            else printf("ERREUR SEMANTIQUE: double declaration de %s, a la ligne %d, et la colonne %d\n", $1, nb_ligne, nb_colonne); 
        }
;

declaration_tab: mc_vector ':' idf '[' cst ',' cst ':' type ']' ';' {
                    if(double_declaration($3) == 1){
                        inserer_type_taille($3,suavType,$5,$7);

                        // generate bounds quadruplet
                        generate_bounds($5, $7);
                        quadr("ADEC", $3, "vide", "vide");
                    } 
                    else printf("ERREUR SEMANTIQUE: double declaration de %s, a la ligne %d, et la colonne %d\n", $3, nb_ligne, nb_colonne); 
                }
;

declaration_const: mc_const ':' idf '=' cst ';' {update_cst($3,$5);}
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

        }
        | idf '[' cst ']' '=' list_exp ';' {
            if(double_declaration($1) == 1) 
                printf("ERREUR SEMANTIQUE: %s non declare, a la ligne %d, et la colonne %d\n", $1, nb_ligne, nb_colonne);
            else if (is_const($1) == 1)
                printf("ERREUR SEMANTIQUE: %s est une constante a la ligne %d, et la colonne %d\n",$1,nb_ligne, nb_colonne);
            else if (depassement_taille($1,$3) == 1)
                printf("ERREUR SEMANTIQUE: Depassement de taille pour le tableau %s , a la ligne %d, t la colonne %d\n",$1,nb_ligne, nb_colonne);

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
            else if (depassement_taille($1,$3) == 1)
                printf("ERREUR SEMANTIQUE: Depassement de taille pour le tableau %s , a la ligne %d, t la colonne %d\n",$1,nb_ligne, nb_colonne);
            else quadr(":=", $6, "vide", $1);    
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
    } 
    | idf '[' cst ']' {
        if(double_declaration($1) == 1) 
            printf("ERREUR SEMANTIQUE: %s non declare, a la ligne %d, et la colonne %d\n", $1, nb_ligne, nb_colonne);
        else if (depassement_taille($1,$3) == 1)
                printf("ERREUR SEMANTIQUE: Depassement de taille pour le tableau %s , a la ligne %d, t la colonne %d\n",$1,nb_ligne, nb_colonne);
    }
    | cst
;

list_exp: list_exp exp
        | exp
;

exp: exp operateur var
    | division
    | var
;

operateur: '+' | '-' | '*'
;


division: exp '/' idf {
            if(double_declaration($3) == 1) 
                printf("ERREUR SEMANTIQUE: %s non declare, a la ligne %d, et la colonne %d\n", $3, nb_ligne, nb_colonne);
        } 
        | exp '/' cst {
        if ($3==0) 
            printf("ERREUR SEMANTIQUE: Division par zero a la ligne %d\n",nb_ligne);  
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
