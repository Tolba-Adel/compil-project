%{
    #include <stdio.h>
    #include <stdbool.h>

    int nb_ligne=1, nb_colonne=1;

    char suavType[20];

    int qc=0;
    int deb_else, fin_if, and_qc, or_qc;
    char tmp[20];

    // function to generate temp variable names
    char* generer_temp() {
        static char temp[20];
        static int cpt_temp = 0;
        sprintf(temp, "temp%d", cpt_temp++);
        return temp;
    }


    // stack to manage nested loops
    typedef struct ForLoopContext {
        int sauv_deb;  // start position of the loop
        int sauv_bz;   // position of the BZ quad
    } ForLoopContext;

    #define MAX_LOOP_DEPTH 20
    ForLoopContext loop_stack[MAX_LOOP_DEPTH];
    int loop_stack_top = -1;  // stack pointer
    int loop_sauv_deb = 0;

    void push_loop_context(int deb, int bz) {
        if (loop_stack_top < MAX_LOOP_DEPTH - 1) {
            loop_stack_top++;
            loop_stack[loop_stack_top].sauv_deb = deb;
            loop_stack[loop_stack_top].sauv_bz = bz;
        } 
    }

    ForLoopContext pop_loop_context() {
        ForLoopContext ctx;
        if (loop_stack_top >= 0) {
            ctx = loop_stack[loop_stack_top];
            loop_stack_top--;
            return ctx;
        } 
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
%type <str_val> condition comparaison


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
                        generate_bounds(bi_str, t_int);
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
                printf("ERREUR SEMANTIQUE: modification de la constane %s a la ligne %d, et la colonne %d\n",$1,nb_ligne, nb_colonne);
            else quadr(":=", $3, "vide", $1);
        }
        | idf '[' cst ']' '=' list_exp ';' {
            if(double_declaration($1) == 1) 
                printf("ERREUR SEMANTIQUE: %s non declare, a la ligne %d, et la colonne %d\n", $1, nb_ligne, nb_colonne);
            else if (is_const($1) == 1)
                printf("ERREUR SEMANTIQUE: modification de la constane %s a la ligne %d, et la colonne %d\n",$1,nb_ligne, nb_colonne);
            else {
                char index_str[20];
                sprintf(index_str, "%d", (int) $3);
                if (depassement_taille($1,index_str) == 1)
                    printf("ERREUR SEMANTIQUE: Depassement de taille pour le tableau %s , a la ligne %d, t la colonne %d\n",$1,nb_ligne, nb_colonne);
                else {
                    char arr_elem[50];
                    sprintf(arr_elem, "%s[%s]", $1, index_str);
                    quadr(":=", $6, "vide", arr_elem);
                } 
            } 
        }
        | idf '=' caractere ';' {
            if(double_declaration($1) == 1) 
                printf("ERREUR SEMANTIQUE: %s non declare, a la ligne %d, et la colonne%d\n", $1, nb_ligne, nb_colonne);
            else if (is_const($1) == 1)
                printf("ERREUR SEMANTIQUE: modification de la constane %s a la ligne %d, et la colonne %d\n",$1,nb_ligne, nb_colonne);
            else if (check_type_char($1) == 0)
                printf("ERREUR SEMANTIQUE: %s doit etre de type CHAR, a la ligne %d, et la colonne %d\n", $1, nb_ligne, nb_colonne);
            else quadr(":=", $3, "vide", $1);                
        }
        | idf '[' cst ']' '=' caractere ';' {
            if(double_declaration($1) == 1) 
                printf("ERREUR SEMANTIQUE: %s non declare, a la ligne %d, et la colonne%d\n", $1, nb_ligne, nb_colonne);
            else if (is_const($1) == 1)
                printf("ERREUR SEMANTIQUE: modification de la constane %s a la ligne %d, et la colonne %d\n",$1,nb_ligne, nb_colonne);
            else if (check_type_char($1) == 0)
                printf("ERREUR SEMANTIQUE: %s doit etre de type CHAR, a la ligne %d, et la colonne %d\n", $1, nb_ligne, nb_colonne);
            else {
                char index_str[20];
                sprintf(index_str, "%d", (int) $3);
                char arr_elem[50];
                sprintf(arr_elem, "%s[%s]", $1, index_str);
                if (depassement_taille($1,index_str) == 1)
                    printf("ERREUR SEMANTIQUE: Depassement de taille pour le tableau %s , a la ligne %d, t la colonne %d\n",$1,nb_ligne, nb_colonne);
                else quadr(":=", $6, "vide", arr_elem);    
            } 
        }
        | idf '=' chaine ';' {
            if(double_declaration($1) == 1) 
                printf("ERREUR SEMANTIQUE: %s non declare, a la ligne %d, et la colonne%d\n", $1, nb_ligne, nb_colonne);
            else if (is_const($1) == 1)
                printf("ERREUR SEMANTIQUE: modification de la constane %s a la ligne %d, et la colonne %d\n",$1,nb_ligne, nb_colonne);
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
        else {
            char index_str[20];
            sprintf(index_str, "%d", (int) $3);
            if (depassement_taille($1,index_str) == 1)
                printf("ERREUR SEMANTIQUE: Depassement de taille pour le tableau %s , a la ligne %d, t la colonne %d\n",$1,nb_ligne, nb_colonne);
            else {
                char arr_elem[50];
                sprintf(arr_elem, "%s[%s]", $1, index_str);
                strcpy($$, arr_elem);
            }
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
        | exp '/' idf '[' cst ']' {
            if(double_declaration($3) == 1) 
                printf("ERREUR SEMANTIQUE: %s non declare, a la ligne %d, et la colonne %d\n", $3, nb_ligne, nb_colonne);
            else {
                char index_str[20];
                sprintf(index_str, "%d", (int) $5);
                char arr_elem[50];
                sprintf(arr_elem, "%s[%s]", $3, index_str);
                if (depassement_taille($3,index_str) == 1)
                    printf("ERREUR SEMANTIQUE: Depassement de taille pour le tableau %s , a la ligne %d, t la colonne %d\n",$3,nb_ligne, nb_colonne);
                else {
                    char* temp = generer_temp();
                    quadr("/", $1, arr_elem, temp);
                    strcpy($$, temp);
                }
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


inst_if: deb_inst_if mc_else ':' list_inst mc_end { 
            sprintf(tmp, "%d", qc);
            ajour_quad(fin_if,1,tmp);
        }
;

deb_inst_if: debut_if list_inst { 
            fin_if = qc;
            quadr("BR", "", "vide", "vide");
            sprintf(tmp, "%d", qc);
            ajour_quad(deb_else,1,tmp);
            ajour_quad(and_qc,1,tmp);
            ajour_quad(or_qc,1,tmp);
        }
;

debut_if: mc_if '(' condition ')' ':'
;

condition: list_exp comparaison list_exp {
            char* temp = generer_temp();
            deb_else = qc;
            quadr($2, "", $1, $3);
            strcpy($$, temp);
        }
        | condition mc_and condition {
            char* temp = generer_temp();
            quadr("AND", $1, $3, temp);
            and_qc = qc;
            quadr("BZ", "", $1, "vide");  // 1er condition fausse on saute a ELSE
            strcpy($$, temp);
        }
        | condition mc_or condition {
            char* temp = generer_temp();
            quadr("OR", $1, $3, temp);
            or_qc = qc;
            quadr("BZ", "", $3, "vide");  // 2 conditions fausses on saute a ELSE
            strcpy($$, temp);
        }
        | mc_not condition {
            char* temp = generer_temp();
            quadr("NOT", $2, "vide", temp);
            strcpy($$, temp);
        }
        | '(' condition ')' { strcpy($$, $2); }
;

comparaison: mc_g  { strcpy($$, "BLE"); }
        | mc_ge { strcpy($$, "BL"); }
        | mc_l  { strcpy($$, "BGE"); }
        | mc_le { strcpy($$, "BG"); }
        | mc_eq { strcpy($$, "BNE"); }
        | mc_di { strcpy($$, "BE"); }
;


inst_for: deb_inst_for list_inst mc_end {
    ForLoopContext ctx = pop_loop_context();
    char sauv_deb_str[20];
    sprintf(sauv_deb_str, "%d", ctx.sauv_deb);
    quadr("BR", sauv_deb_str, "vide", "vide");
    sprintf(tmp, "%d", qc);
    ajour_quad(ctx.sauv_bz, 1, tmp);
}
;

deb_inst_for: debut_for '(' idf ':' list_exp ':' list_exp ')' {
    char* temp = generer_temp();
    int current_bz = qc;
    quadr("BZ", "", temp, "vide");
    push_loop_context(loop_sauv_deb, current_bz);
}
;

debut_for: mc_for {
    int current_deb = qc;
    loop_sauv_deb = current_deb;  // using a global variable to pass to deb_inst_for
}
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
