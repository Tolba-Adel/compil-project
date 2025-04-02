#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>


typedef struct 
{
    char oper[1000];
    char op1[2000];
    char op2[2000];
    char res[2000];

}qdr;

qdr quad[1000];
extern int qc;


/* insert a quad in the matrix */
void quadr(char opr[],char op1[],char op2[],char res[]) {

    strcpy(quad[qc].oper, opr);
    strcpy(quad[qc].op1, op1);
    strcpy(quad[qc].op2, op2);
    strcpy(quad[qc].res, res);

    qc++;
}


/* update a column in a quad */
void ajour_quad(int num_quad,int colon_quad,char val[]) {

    if(colon_quad == 0) strcpy(quad[num_quad].oper, val);
    else if(colon_quad == 1) strcpy(quad[num_quad].op1, val);
    else if(colon_quad == 2) strcpy(quad[num_quad].op2, val);
    else if(colon_quad == 3) strcpy(quad[num_quad].res, val);    
}


/* display the quad matrix */
void afficher_qdr() {

    printf("\n\n*********************LES QUADRUPLETS*************************\n");
    int i;
    for(i=0;i<qc;i++) {
        printf("\n %d-( %s ,%s , %s , %s )\n",i,quad[i].oper,quad[i].op1,quad[i].op2,quad[i].res);
        printf("-------------------------------------------------------------\n");
    }
}



//**********************************************************************************/


/* generate BOUNDS quadruplet for array declaration */
void generate_bounds(char start[], char size[]) {
    int start_int = atoi(start);
    int size_int = atoi(size);

    int end_int = start_int + size_int-1;

    char end_str[20];
    sprintf(end_str, "%d", end_int);
    
    quadr("BOUNDS", start, end_str, "vide");
}





