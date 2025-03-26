%{
    #include "syntaxique.tab.h"
    #include "ts.h"
    #include "quad.h"

    extern nb_ligne;
    extern nb_colonne;
    extern YYSTYPE yylval;
%}

idf  [a-zA-Z](_?[a-zA-Z0-9]+)*
cst  [+-]?([1-9][0-9]*|0)(\.[0-9]+)?
char_lit  \'[^\\]\'
str_lit \"[^\"]*\"



%%
DATA {rechercher(yytext,"Mot cle","",0,1); nb_colonne = nb_colonne + yyleng; return mc_data;}
END {rechercher(yytext,"Mot cle","",0,1); nb_colonne = nb_colonne + yyleng; return mc_end;}
CODE {rechercher(yytext,"Mot cle","",0,1); nb_colonne = nb_colonne + yyleng; return mc_code;}
READ {rechercher(yytext,"Mot cle","",0,1); nb_colonne = nb_colonne + yyleng; return mc_read;}
DISPLAY {rechercher(yytext,"Mot cle","",0,1); nb_colonne = nb_colonne + yyleng; return mc_display;}
IF {rechercher(yytext,"Mot cle","",0,1); nb_colonne = nb_colonne + yyleng; return mc_if;}
ELSE {rechercher(yytext,"Mot cle","",0,1); nb_colonne = nb_colonne + yyleng; return mc_else;}
FOR {rechercher(yytext,"Mot cle","",0,1); nb_colonne = nb_colonne + yyleng; return mc_for;}


INTEGER {rechercher(yytext,"Mot cle","",0,1); nb_colonne = nb_colonne + yyleng; yylval.str=strdup(yytext); return mc_int;}
FLOAT {rechercher(yytext,"Mot cle","",0,1); nb_colonne = nb_colonne + yyleng; yylval.str=strdup(yytext); return mc_float;}
CHAR {rechercher(yytext,"Mot cle","",0,1); nb_colonne = nb_colonne + yyleng; yylval.str=strdup(yytext); return mc_char;}
STRING {rechercher(yytext,"Mot cle","",0,1); nb_colonne = nb_colonne + yyleng; yylval.str=strdup(yytext); return mc_string;}

VECTOR {rechercher(yytext,"Mot cle","",0,1); nb_colonne = nb_colonne + yyleng; return mc_vector;}

CONST {rechercher(yytext,"Mot cle","",0,1); nb_colonne = nb_colonne + yyleng; return mc_const;}


[\-+*/';,:@=|()[\]\.] {rechercher(yytext,"Separateur","",0,2); nb_colonne = nb_colonne + yyleng; return yytext[0];}

".AND." {rechercher(yytext,"Separateur","",0,2); nb_colonne = nb_colonne + yyleng; return mc_and;}
".OR." {rechercher(yytext,"Separateur","",0,2); nb_colonne = nb_colonne + yyleng; return mc_or;}
NOT {rechercher(yytext,"Separateur","",0,2); nb_colonne = nb_colonne + yyleng; return mc_not;}

".G." {rechercher(yytext,"Separateur","",0,2); nb_colonne = nb_colonne + yyleng; return mc_g;}
".GE." {rechercher(yytext,"Separateur","",0,2); nb_colonne = nb_colonne + yyleng; return mc_ge;}
".L." {rechercher(yytext,"Separateur","",0,2); nb_colonne = nb_colonne + yyleng; return mc_l;}
".LE." {rechercher(yytext,"Separateur","",0,2); nb_colonne = nb_colonne + yyleng; return mc_le;}
".EQ." {rechercher(yytext,"Separateur","",0,2); nb_colonne = nb_colonne + yyleng; return mc_eq;}
".DI." {rechercher(yytext,"Separateur","",0,2); nb_colonne = nb_colonne + yyleng; return mc_di;}


{char_lit} {
    yylval.str=strdup(yytext); 
    // rechercher(yytext,"","CHAR",0,0);  
    nb_colonne = nb_colonne + strlen(yytext); 
    return caractere;
}


{str_lit} {
    yylval.str=strdup(yytext);
    // rechercher(yytext,"","STRING",0,0);  
    nb_colonne = nb_colonne + strlen(yytext);
    return chaine;
}

{idf} {
    if (yyleng <= 8) {
        yylval.str=strdup(yytext);
        rechercher(yytext,"IDF","",0,0);
        nb_colonne = nb_colonne + strlen(yytext);
        return idf;
    } 
    else printf("ERREUR LEXICALE a la ligne %d, et la colonne %d sur l'entite %s (IDF trop long)\n",nb_ligne,nb_colonne,yytext);
}

{cst} {
    if (atoi(yytext) >= -32768 && atoi(yytext) <= 32767) {
        yylval.real=atof(yytext); 
        rechercher(yytext,"CST","",atof(yytext),0); 
        nb_colonne = nb_colonne + yyleng;
        return cst;
    }
    else printf("ERREUR LEXICALE a la ligne %d, et la colonne %d sur l'entite %s (CST non valide)\n",nb_ligne,nb_colonne,yytext);
}

[ \t] nb_colonne = nb_colonne + yyleng;

\n {nb_colonne=1; nb_ligne++;}

§.* {nb_colonne=1; nb_ligne++;}

. printf("ERREUR LEXICALE a la ligne %d, et la colonne %d\n",nb_ligne,nb_colonne);
