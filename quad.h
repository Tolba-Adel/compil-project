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




//* 1. PROPAGATION DE COPIE */
void propagation_copie() {
    int i, j;
    int optimized = 0;
    
    // Structure to keep track of copy operations
    typedef struct {
        char target[1000];  // The target variable (a in a := b)
        char source[1000];  // The source variable (b in a := b)
        int active;         // Is this copy still valid?
        int quad_index;     // Index of the copy operation in the quad array
    } CopyInfo;
    
    // We'll use an array to store all copy operations
    CopyInfo copies[1000];
    int copy_count = 0;
    
    printf("\n--- Performing Copy Propagation ---\n");
    
    // First pass: identify all copy operations
    for (i = 0; i < qc; i++) {
        // Check if this is a copy operation (a := b) where both are variables
        if (strcmp(quad[i].oper, ":=") == 0 && 
            strcmp(quad[i].op2, "vide") == 0 &&
            // Make sure source is a variable (not a constant)
            (!isdigit(quad[i].op1[0]) && 
             quad[i].op1[0] != '\"' && 
             quad[i].op1[0] != '\'' &&
             strcmp(quad[i].op1, "0") != 0 &&
             strcmp(quad[i].op1, "1") != 0)) {
            
            // Store this copy operation
            strcpy(copies[copy_count].target, quad[i].res);
            strcpy(copies[copy_count].source, quad[i].op1);
            copies[copy_count].active = 1;
            copies[copy_count].quad_index = i;
            copy_count++;
            
            printf("Found copy: %s := %s at quad %d\n", 
                   copies[copy_count-1].target, 
                   copies[copy_count-1].source, 
                   copies[copy_count-1].quad_index);
        }
    }
    
    if (copy_count == 0) {
        printf("No copy operations found to propagate.\n");
        return;
    }
    
    // Second pass: propagate copies
    for (i = 0; i < qc; i++) {
        // Skip the copy operations themselves
        int is_copy_op = 0;
        for (j = 0; j < copy_count; j++) {
            if (copies[j].quad_index == i) {
                is_copy_op = 1;
                break;
            }
        }
        if (is_copy_op) continue;
        
        // Check if operands can be replaced
        for (j = 0; j < copy_count; j++) {
            if (!copies[j].active) continue;
            
            // If this quad modifies the source variable of a copy,
            // deactivate that copy since the source value might change
            if (strcmp(quad[i].res, copies[j].source) == 0) {
                // printf("Deactivating copy %s := %s (source modified at quad %d)\n",
                //        copies[j].target, copies[j].source, i);
                copies[j].active = 0;
                continue;
            }
            
            // If this quad modifies the target variable of a copy,
            // deactivate that copy since the target is being reassigned
            if (strcmp(quad[i].res, copies[j].target) == 0) {
                // printf("Deactivating copy %s := %s (target modified at quad %d)\n",
                //        copies[j].target, copies[j].source, i);
                copies[j].active = 0;
                continue;
            }
            
            // Replace source with target in operand 1
            if (strcmp(quad[i].op1, copies[j].source) == 0) {
                // printf("Replacing %s with %s in quad %d (op1)\n", 
                //        copies[j].source, copies[j].target, i);
                strcpy(quad[i].op1, copies[j].target);
                optimized++;
            }
            
            // Replace source with target in operand 2
            if (strcmp(quad[i].op2, copies[j].source) == 0 && 
                strcmp(quad[i].op2, "vide") != 0) {
                // printf("Replacing %s with %s in quad %d (op2)\n", 
                //        copies[j].source, copies[j].target, i);
                strcpy(quad[i].op2, copies[j].target);
                optimized++;
            }
            
            // Replace source with target in result (rare case)
            if (strcmp(quad[i].res, copies[j].source) == 0 && 
                strcmp(quad[i].oper, ":=") != 0) {
                // printf("Replacing %s with %s in quad %d (res)\n", 
                //        copies[j].source, copies[j].target, i);
                strcpy(quad[i].res, copies[j].target);
                optimized++;
            }
        }
    }
    
    // Third pass: remove unnecessary copy operations
    int removed = 0;
    for (j = 0; j < copy_count; j++) {
        // Only consider active copies for removal
        if (!copies[j].active) continue;
        
        // Check if the source has been fully replaced by target
        int source_still_used = 0;
        for (i = copies[j].quad_index + 1; i < qc; i++) {
            if (strcmp(quad[i].op1, copies[j].source) == 0 ||
                strcmp(quad[i].op2, copies[j].source) == 0 ||
                strcmp(quad[i].res, copies[j].source) == 0) {
                source_still_used = 1;
                // printf("Source %s still used in quad %d - cannot remove copy operation\n", 
                //        copies[j].source, i);
                break;
            }
        }
        
        // If the source is no longer used, we can remove the copy operation
        if (!source_still_used) {
            // printf("Removing copy operation %s := %s at quad %d\n", 
            //        copies[j].target, copies[j].source, copies[j].quad_index);
            strcpy(quad[copies[j].quad_index].oper, "NOP");
            removed++;
        }
    }
    
    // Final pass: remove NOPs by shifting the array
    if (removed > 0) {
        int write_idx = 0;
        for (i = 0; i < qc; i++) {
            if (strcmp(quad[i].oper, "NOP") != 0) {
                if (i != write_idx) {
                    // Move this quad to the write position
                    strcpy(quad[write_idx].oper, quad[i].oper);
                    strcpy(quad[write_idx].op1, quad[i].op1);
                    strcpy(quad[write_idx].op2, quad[i].op2);
                    strcpy(quad[write_idx].res, quad[i].res);
                }
                write_idx++;
            }
        }
        qc = write_idx; // Update the quad count
    }
    
    printf("Copy propagation complete: %d replacements, %d copies removed\n", optimized, removed);
}


//* 2. PROPAGATION D'EXPRESSION */
void propagation_expr() {
    int i, j;
    int replacements = 0;
    int removed = 0;
    
    printf("\n--- Performing Expression Propagation ---\n");
    
    // First pass: identify all temporary variables that are used in assignments to other variables
    // These are likely CSE results and should be preserved
    char cse_temps[500][100];
    int cse_count = 0;
    
    for (i = 0; i < qc; i++) {
        if (strcmp(quad[i].oper, ":=") == 0) {
            // Check if we're assigning from one temp to another
            if ((quad[i].op1[0] == 't' || strncmp(quad[i].op1, "temp", 4) == 0) &&
                (quad[i].res[0] == 't' || strncmp(quad[i].res, "temp", 4) == 0)) {
                
                // This is a temp-to-temp assignment, likely from CSE
                // Mark the source temp as used in CSE
                int already_recorded = 0;
                for (j = 0; j < cse_count; j++) {
                    if (strcmp(cse_temps[j], quad[i].op1) == 0) {
                        already_recorded = 1;
                        break;
                    }
                }
                
                if (!already_recorded) {
                    strcpy(cse_temps[cse_count], quad[i].op1);
                    cse_count++;
                }
            }
        }
    }
    
    // Build a map of numeric constants that appear in operation results
    typedef struct {
        char const_val[100];  // The constant value (like "8")
        char temp_var[100];   // The temp variable that holds it (like "t6")
        int quad_idx;         // Index of the assignment quad
    } ConstantMapping;
    
    ConstantMapping const_map[500];
    int const_count = 0;
    
    // Array to mark quads for removal
    int to_remove[500] = {0};
    
    // Second pass: find operation results that get assigned to temps
    for (i = 0; i < qc - 1; i++) {
        // Check if this is an operation quad with numeric operands
        if ((strcmp(quad[i].oper, "+") == 0 || 
             strcmp(quad[i].oper, "-") == 0 ||
             strcmp(quad[i].oper, "*") == 0 ||
             strcmp(quad[i].oper, "/") == 0)) {
            
            int op1_val, op2_val;
            if (sscanf(quad[i].op1, "%d", &op1_val) == 1 && 
                sscanf(quad[i].op2, "%d", &op2_val) == 1) {
                
                // This operation has constant operands
                if (i + 1 < qc && 
                    strcmp(quad[i+1].oper, ":=") == 0 && 
                    strcmp(quad[i+1].op1, quad[i].res) == 0) {
                    
                    // Record this constant and its associated temp variable
                    strcpy(const_map[const_count].const_val, quad[i].res);
                    strcpy(const_map[const_count].temp_var, quad[i+1].res);
                    const_map[const_count].quad_idx = i+1;
                    const_count++;
                }
            }
        }
    }
    
    // Third pass: handle the specific pattern where an operation result is assigned to a temporary
    // Example: 7-( + ,6 , 5 , 11 ) followed by 8-( := ,11 , vide , t4 )
    for (i = 0; i < qc - 1; i++) {
        // Check if this is an operation quad
        if ((strcmp(quad[i].oper, "+") == 0 || 
             strcmp(quad[i].oper, "-") == 0 ||
             strcmp(quad[i].oper, "*") == 0 ||
             strcmp(quad[i].oper, "/") == 0)) {
            
            // Check if the next quad assigns this result to a temporary
            if (strcmp(quad[i+1].oper, ":=") == 0 &&
                strcmp(quad[i].res, quad[i+1].op1) == 0 &&
                (quad[i+1].res[0] == 't' || strncmp(quad[i+1].res, "temp", 4) == 0)) {
                
                char temp_var[100];
                strcpy(temp_var, quad[i+1].res);
                
                // Check if this temp is used in a CSE assignment
                int used_for_cse = 0;
                for (j = 0; j < cse_count; j++) {
                    if (strcmp(cse_temps[j], temp_var) == 0) {
                        used_for_cse = 1;
                        break;
                    }
                }
                
                // If this temp is used for CSE, do not propagate or remove it
                if (used_for_cse) {
                    continue;
                }
                
                // Look for uses of this temporary in operations (not assignments)
                int used_in_op = 0;
                for (j = i + 2; j < qc; j++) {
                    // Only replace in operations, not in assignments
                    // to avoid breaking CSE patterns
                    if (strcmp(quad[j].oper, ":=") != 0) {
                        if (strcmp(quad[j].op1, temp_var) == 0) {
                            strcpy(quad[j].op1, quad[i].res);
                            used_in_op = 1;
                            replacements++;
                        }
                        if (strcmp(quad[j].op2, temp_var) == 0) {
                            strcpy(quad[j].op2, quad[i].res);
                            used_in_op = 1;
                            replacements++;
                        }
                    }
                    
                    // If the temporary is redefined, stop propagation
                    if (strcmp(quad[j].res, temp_var) == 0) {
                        break;
                    }
                }
                
                // If the temporary was used and replaced, mark the assignment for removal
                if (used_in_op) {
                    to_remove[i+1] = 1;
                    removed++;
                }
            }
        }
    }
    
    // Apply removals (replace with NOP)
    for (i = 0; i < qc; i++) {
        if (to_remove[i]) {
            strcpy(quad[i].oper, "NOP");
        }
    }
    
    // Final pass: remove NOPs and compact the quadruple array
    if (removed > 0) {
        int write_idx = 0;
        for (i = 0; i < qc; i++) {
            if (strcmp(quad[i].oper, "NOP") != 0) {
                if (i != write_idx) {
                    strcpy(quad[write_idx].oper, quad[i].oper);
                    strcpy(quad[write_idx].op1, quad[i].op1);
                    strcpy(quad[write_idx].op2, quad[i].op2);
                    strcpy(quad[write_idx].res, quad[i].res);
                }
                write_idx++;
            }
        }
        qc = write_idx;
    }
    
    printf("Expression propagation complete: %d replacements, %d assignments removed\n", replacements, removed);
}


//* 3. ELIMINATION D'EXPRESSIONS REDONDNATES */
// Structure to track expressions
typedef struct {
    char op1[20];      // First operand
    char op2[20];      // Second operand
    char oper[5];      // Operator
    char res[20];      // Result variable
    int quad_idx;      // Index of the quad
} ExpressionRecord;

// Maximum number of expressions to track
#define MAX_EXPRESSIONS 100

ExpressionRecord expr_table[MAX_EXPRESSIONS];
int expr_count = 0;

// Structure to track variable assignments (for value tracking)
typedef struct {
    char var[20];      // Variable name
    char value[20];    // Current value (could be constant or another variable)
    int quad_idx;      // Index of the quad where assignment occurred
} VariableAssignment;

// Maximum number of variables to track
#define MAX_VARIABLES 100

VariableAssignment var_table[MAX_VARIABLES];
int var_count = 0;


// Check if a variable has been modified between two quad indices
int var_modified_between(char* var, int start_idx, int end_idx) {
    int i;
    for (i = start_idx + 1; i < end_idx; i++) {
        if (strcmp(quad[i].res, var) == 0) {
            return 1;
        }
    }
    return 0;
}

// Find a matching expression in our table
int find_expression(char* op1, char* op2, char* oper) {
    int i;
    for (i = 0; i < expr_count; i++) {
        if (strcmp(expr_table[i].op1, op1) == 0 && 
            strcmp(expr_table[i].op2, op2) == 0 && 
            strcmp(expr_table[i].oper, oper) == 0) {
            return i;
        }
    }
    return -1;
}

// Add expression to our table
void add_expression(char* op1, char* op2, char* oper, char* res, int quad_idx) {
    if (expr_count < MAX_EXPRESSIONS) {
        strcpy(expr_table[expr_count].op1, op1);
        strcpy(expr_table[expr_count].op2, op2);
        strcpy(expr_table[expr_count].oper, oper);
        strcpy(expr_table[expr_count].res, res);
        expr_table[expr_count].quad_idx = quad_idx;
        expr_count++;
    }
}

// Record variable assignment
void record_variable_assignment(char* var, char* value, int quad_idx) {
    int i;
    // Check if variable already exists, update if it does
    for (i = 0; i < var_count; i++) {
        if (strcmp(var_table[i].var, var) == 0) {
            strcpy(var_table[i].value, value);
            var_table[i].quad_idx = quad_idx;
            return;
        }
    }
    
    // Add new variable if not found
    if (var_count < MAX_VARIABLES) {
        strcpy(var_table[var_count].var, var);
        strcpy(var_table[var_count].value, value);
        var_table[var_count].quad_idx = quad_idx;
        var_count++;
    }
}

// Find the current value of a variable
char* get_variable_value(char* var) {
    int i;
    for (i = 0; i < var_count; i++) {
        if (strcmp(var_table[i].var, var) == 0) {
            return var_table[i].value;
        }
    }
    return NULL;
}

// Find a variable that contains a specific value at a specific quad index
char* find_variable_with_value(char* value, int quad_idx) {
    int i;
    for (i = 0; i < var_count; i++) {
        if (strcmp(var_table[i].value, value) == 0 && 
            var_table[i].quad_idx < quad_idx &&
            !var_modified_between(var_table[i].var, var_table[i].quad_idx, quad_idx)) {
            return var_table[i].var;
        }
    }
    return NULL;
}

// Eliminate common subexpressions
void elim_expr_com() {
    int i, modified = 0;
    
    printf("\n--- Eliminating Common Subexpressions ---\n");
    
    // Reset tables
    expr_count = 0;
    var_count = 0;
    
    // First pass: identify and replace common expressions
    for (i = 0; i < qc; i++) {
        // Skip empty quads
        if (strcmp(quad[i].oper, "NOP") == 0) {
            continue;
        }
        
        // Handle arithmetic operations
        if (strcmp(quad[i].oper, "+") == 0 || 
            strcmp(quad[i].oper, "-") == 0 || 
            strcmp(quad[i].oper, "*") == 0 || 
            strcmp(quad[i].oper, "/") == 0) {
            
            // Check if this expression was computed before
            int expr_idx = find_expression(quad[i].op1, quad[i].op2, quad[i].oper);
            
            if (expr_idx != -1) {
                // We found a previous computation of this expression
                
                // Check if variables in the expression have been modified
                int var_modified = 0;
                
                // Only check for modifications if operands are variables (not constants)
                if (!is_numeric(quad[i].op1)) {
                    var_modified |= var_modified_between(quad[i].op1, 
                                        expr_table[expr_idx].quad_idx, i);
                }
                
                if (!is_numeric(quad[i].op2)) {
                    var_modified |= var_modified_between(quad[i].op2, 
                                        expr_table[expr_idx].quad_idx, i);
                }
                
                if (!var_modified) {
                    // Find the variable that holds this expression's result
                    char* var_to_use = expr_table[expr_idx].res;
                    
                    // Replace the duplicate computation with an assignment
                    // printf("Quad %d: Replacing %s = %s %s %s with %s = %s\n",
                        //    i, quad[i].res, quad[i].op1, quad[i].oper, quad[i].op2,
                        //    quad[i].res, var_to_use);
                    
                    strcpy(quad[i].oper, ":=");
                    strcpy(quad[i].op1, var_to_use);
                    strcpy(quad[i].op2, "vide");
                    modified++;
                    
                    // Record this assignment
                    record_variable_assignment(quad[i].res, var_to_use, i);
                }
            } else {
                // This is a new expression, add it to our table
                add_expression(quad[i].op1, quad[i].op2, quad[i].oper, quad[i].res, i);
                
                // If the result is a constant, record that information
                // This is just for logging purposes
                if (is_numeric(quad[i].res)) {
                    // printf("Warning: Quad %d produces a numeric result: %s = %s %s %s\n",
                        //    i, quad[i].res, quad[i].op1, quad[i].oper, quad[i].op2);
                }
            }
        }
        
        // Handle assignments
        if (strcmp(quad[i].oper, ":=") == 0) {
            // Record variable assignment
            record_variable_assignment(quad[i].res, quad[i].op1, i);
            
            // Check if we're assigning a constant but we could use a variable instead
            if (is_numeric(quad[i].op1)) {
                char* var_with_value = find_variable_with_value(quad[i].op1, i);
                if (var_with_value != NULL) {
                    // printf("Quad %d: Replacing %s = %s with %s = %s (using variable instead of constant)\n",
                        //    i, quad[i].res, quad[i].op1, quad[i].res, var_with_value);
                    strcpy(quad[i].op1, var_with_value);
                    modified++;
                }
            }

            // Special case handling for invalid assignments
            if (is_numeric(quad[i].res)) {
                // printf("Quad %d: Invalid assignment detected: %s = %s (constant assigned from variable)\n",
                    //    i, quad[i].res, quad[i].op1);
                
                strcpy(quad[i].oper, "NOP");
                modified++;
            }
        }
    }
    
    // Remove NOPs if any
    if (modified > 0) {
        int write_idx = 0;
        for (i = 0; i < qc; i++) {
            if (strcmp(quad[i].oper, "NOP") != 0) {
                if (i != write_idx) {
                    strcpy(quad[write_idx].oper, quad[i].oper);
                    strcpy(quad[write_idx].op1, quad[i].op1);
                    strcpy(quad[write_idx].op2, quad[i].op2);
                    strcpy(quad[write_idx].res, quad[i].res);
                }
                write_idx++;
            }
        }
        qc = write_idx;
    }
    
    printf("Common subexpression elimination complete: %d modifications made\n", modified);
}


//* 4. SIMPLIFICATION ALGEBRIQUE */
void algebraic_simplification() {
    int modified = 0;
    printf("\n--- Performing Algebraic Simplification ---\n");
    int i, j, k;
    
    // PART 0: Optimize operations with identity elements (0 and 1)
    // printf("Looking for operations with identity elements (0 and 1)...\n");
    int identity_modified = 0;
    
    for (i = 0; i < qc; i++) {
        // Skip if not an operation we want to optimize
        if (strcmp(quad[i].oper, "+") != 0 && 
            strcmp(quad[i].oper, "-") != 0 && 
            strcmp(quad[i].oper, "*") != 0 && 
            strcmp(quad[i].oper, "/") != 0) {
            continue;
        }
        
        // Flag to track if we need to delete this quad
        int delete_quad = 0;
        char target_var[200] = "";  // Variable to correctly handle assignments after deletion
        char source_var[200] = "";  // Value to assign to target after deletion
        
        // Check for addition/subtraction with 0
        if (strcmp(quad[i].oper, "+") == 0 || strcmp(quad[i].oper, "-") == 0) {
            // Check if op1 is 0
            if (strcmp(quad[i].op1, "0") == 0) {
                if (strcmp(quad[i].oper, "+") == 0) {
                    // 0 + x = x
                    // printf("Found 0 + %s = %s at quad %d, will be removed\n", 
                        //    quad[i].op2, quad[i].res, i);
                    delete_quad = 1;
                    strcpy(target_var, quad[i].res);
                    strcpy(source_var, quad[i].op2);
                    identity_modified++;
                }
            }
            // Check if op2 is 0
            else if (strcmp(quad[i].op2, "0") == 0) {
                // x + 0 = x or x - 0 = x
                // printf("Found %s %s 0 = %s at quad %d, will be removed\n", 
                    //    quad[i].op1, quad[i].oper, quad[i].res, i);
                delete_quad = 1;
                strcpy(target_var, quad[i].res);
                strcpy(source_var, quad[i].op1);
                identity_modified++;
            }
        }
        
        // Check for multiplication/division with 1
        else if (strcmp(quad[i].oper, "*") == 0 || strcmp(quad[i].oper, "/") == 0) {
            // Check if op1 is 1
            if (strcmp(quad[i].op1, "1") == 0) {
                if (strcmp(quad[i].oper, "*") == 0) {
                    // 1 * x = x
                    // printf("Found 1 * %s = %s at quad %d, will be removed\n", 
                        //    quad[i].op2, quad[i].res, i);
                    delete_quad = 1;
                    strcpy(target_var, quad[i].res);
                    strcpy(source_var, quad[i].op2);
                    identity_modified++;
                }
                // 1 / x is not handled (no simplification)
            }
            // Check if op2 is 1
            else if (strcmp(quad[i].op2, "1") == 0) {
                // x * 1 = x or x / 1 = x
                // printf("Found %s %s 1 = %s at quad %d, will be removed\n", 
                    //    quad[i].op1, quad[i].oper, quad[i].res, i);
                delete_quad = 1;
                strcpy(target_var, quad[i].res);
                strcpy(source_var, quad[i].op1);
                identity_modified++;
            }
        }
        
        // If we're deleting this quad, we need to process the deletion
        if (delete_quad) {
            // Look through all subsequent quads to replace uses of the result variable
            for (j = i + 1; j < qc; j++) {
                // Replace any usage of target_var with source_var
                if (strcmp(quad[j].op1, target_var) == 0) {
                    strcpy(quad[j].op1, source_var);
                }
                if (strcmp(quad[j].op2, target_var) == 0) {
                    strcpy(quad[j].op2, source_var);
                }
            }
            
            // Now shift all quads up by one position to remove the current quad
            for (j = i; j < qc - 1; j++) {
                strcpy(quad[j].oper, quad[j+1].oper);
                strcpy(quad[j].op1, quad[j+1].op1);
                strcpy(quad[j].op2, quad[j+1].op2);
                strcpy(quad[j].res, quad[j+1].res);
            }
            
            qc--; // Reduce the quad count
            i--;  // Adjust i to check the new quad at this position on the next iteration
        }
    }
    
    // printf("Identity element optimization complete: %d modifications made\n", identity_modified);
    modified += identity_modified;
    
    // PART 1: Optimize multiplications by constants
    // printf("\nLooking for multiplications by constants...\n");
    
    for (i = 0; i < qc; i++) {
        // Check if this is a multiplication operation
        if (strcmp(quad[i].oper, "*") == 0) {
            // Check for multiplication by constant value
            int constant_val = 0;
            char variable[200] = "";
            
            // Check if op1 is a constant and op2 is a variable
            if ((isdigit(quad[i].op1[0]) || (quad[i].op1[0] == '-' && isdigit(quad[i].op1[1]))) && 
                !(isdigit(quad[i].op2[0]) || (quad[i].op2[0] == '-' && isdigit(quad[i].op2[1]))))  {
                constant_val = atoi(quad[i].op1);
                strcpy(variable, quad[i].op2);
            } 
            // Check if op2 is a constant and op1 is a variable
            else if ((isdigit(quad[i].op2[0]) || (quad[i].op2[0] == '-' && isdigit(quad[i].op2[1]))) && 
                    !(isdigit(quad[i].op1[0]) || (quad[i].op1[0] == '-' && isdigit(quad[i].op1[1])))) {
                constant_val = atoi(quad[i].op2);
                strcpy(variable, quad[i].op1);
            }
            
            // Skip if both operands are constants or if constant_val <= 1
            if (constant_val <= 1 || strlen(variable) == 0) {
                continue;
            }
            
            // At this point, we have a multiplication of a variable by a constant > 1
            char result[200];
            strcpy(result, quad[i].res);
            
            // printf("Found multiplication by constant: %d * %s = %s at quad %d\n", 
                //    constant_val, variable, result, i);
            
            // Store the original result variable
            char original_result[200];
            strcpy(original_result, quad[i].res);
            
            // For constant == 2, simple replacement
            if (constant_val == 2) {
                // printf("  Optimizing: Replacing %s * 2 with %s + %s\n", 
                    //    variable, variable, variable);
                
                strcpy(quad[i].oper, "+");
                strcpy(quad[i].op1, variable);
                strcpy(quad[i].op2, variable);
                
                modified++;
            } 
            // For constants > 2, decompose into a series of additions
            else {
                char* temp = generer_temp();
                // printf("  Optimizing: Replacing %s * %d with a series of additions\n", 
                    //    variable, constant_val);
                // printf("  Creating temporary: %s = %s + %s\n", temp, variable, variable);
                
                // First step: temp = variable + variable
                strcpy(quad[i].oper, "+");
                strcpy(quad[i].op1, variable);
                strcpy(quad[i].op2, variable);
                strcpy(quad[i].res, temp);
                
                // Insert additional quads for the remaining additions
                int remaining = constant_val - 2;
                int current_quad = i;
                
                while (remaining > 0) {
                    // Shift quads down to make room for a new quad
                    for (j = qc; j > current_quad + 1; j--) {
                        strcpy(quad[j].oper, quad[j-1].oper);
                        strcpy(quad[j].op1, quad[j-1].op1);
                        strcpy(quad[j].op2, quad[j-1].op2);
                        strcpy(quad[j].res, quad[j-1].res);
                    }
                    
                    current_quad++;
                    
                    // Add a new quad: temp = temp + variable
                    strcpy(quad[current_quad].oper, "+");
                    strcpy(quad[current_quad].op1, temp);
                    strcpy(quad[current_quad].op2, variable);
                    strcpy(quad[current_quad].res, temp);
                    // printf("  Adding: %s = %s + %s\n", temp, temp, variable);
                    
                    qc++; // Increment quad counter
                    remaining--;
                    modified++;
                }
                
                // Final step: Add a quad to assign temp to original result
                for (j = qc; j > current_quad + 1; j--) {
                    strcpy(quad[j].oper, quad[j-1].oper);
                    strcpy(quad[j].op1, quad[j-1].op1);
                    strcpy(quad[j].op2, quad[j-1].op2);
                    strcpy(quad[j].res, quad[j-1].res);
                }
                
                current_quad++;
                strcpy(quad[current_quad].oper, ":=");
                strcpy(quad[current_quad].op1, temp);
                strcpy(quad[current_quad].op2, "vide");
                strcpy(quad[current_quad].res, original_result);
                qc++;
                
                i = current_quad; // Skip the quads we've just created
            }
        }
    }
    
    // printf("Multiplication optimization complete: %d modifications made\n", modified);
    
    // PART 2: Enhanced constant tracking and cancellation detection
    int const_cancel_modified = 0;
    // printf("\nLooking for expressions with canceling constants...\n");

    // Define a structure to track constant operations in a sequence
    typedef struct {
        char var[200];      // The base variable
        int net_const;      // Net constant after additions/subtractions
        int start_quad;     // Starting quadruple index
        int end_quad;       // Ending quadruple index
        char result[200];   // Final result variable
    } ConstTracker;

    // Loop through all quads to find sequences of operations
    for (i = 0; i < qc; i++) {
        // Skip if not an operation that could start a sequence
        if (strcmp(quad[i].oper, "+") != 0 && strcmp(quad[i].oper, "-") != 0 && 
            strcmp(quad[i].oper, ":=") != 0) {
            continue;
        }

        // Initialize a tracker for this potential sequence
        ConstTracker tracker;
        strcpy(tracker.var, "");
        tracker.net_const = 0;
        tracker.start_quad = i;
        
        // For assignment operations, set the base variable
        if (strcmp(quad[i].oper, ":=") == 0) {
            // Check if we're assigning a variable (not a constant)
            if (!(isdigit(quad[i].op1[0]) || (quad[i].op1[0] == '-' && isdigit(quad[i].op1[1])))) {
                strcpy(tracker.var, quad[i].op1);
                strcpy(tracker.result, quad[i].res);
            }
        }
        // For addition/subtraction with a variable and constant
        else {
            char var_name[200];
            int const_val = 0;
            int has_var_const = 0;
            
            // Check if op1 is a variable and op2 is a constant
            if (!(isdigit(quad[i].op1[0]) || (quad[i].op1[0] == '-' && isdigit(quad[i].op1[1]))) && 
                (isdigit(quad[i].op2[0]) || (quad[i].op2[0] == '-' && isdigit(quad[i].op2[1])))) {
                strcpy(var_name, quad[i].op1);
                const_val = atoi(quad[i].op2);
                has_var_const = 1;
            }
            // Check if op2 is a variable and op1 is a constant
            else if ((isdigit(quad[i].op1[0]) || (quad[i].op1[0] == '-' && isdigit(quad[i].op1[1]))) && 
                    !(isdigit(quad[i].op2[0]) || (quad[i].op2[0] == '-' && isdigit(quad[i].op2[1])))) {
                strcpy(var_name, quad[i].op2);
                const_val = atoi(quad[i].op1);
                has_var_const = 1;
            }
            
            if (has_var_const) {
                strcpy(tracker.var, var_name);
                strcpy(tracker.result, quad[i].res);
                
                // Add or subtract the constant based on operation
                if (strcmp(quad[i].oper, "+") == 0) {
                    tracker.net_const += const_val;
                } else { // "-" operation
                    // For subtraction, need to determine which operand is the constant
                    if ((isdigit(quad[i].op2[0]) || (quad[i].op2[0] == '-' && isdigit(quad[i].op2[1])))) {
                        tracker.net_const -= const_val;
                    } else {
                        // If variable is being subtracted from constant, logic is different
                        strcpy(tracker.var, var_name);
                        tracker.net_const = const_val;
                    }
                }
            }
        }
        
        // If we identified a starting point, look for a sequence
        if (strlen(tracker.var) > 0) {
            // Loop through subsequent quads to track constant operations
            int current = i + 1;
            int last_result_quad = i;
            char current_var[200];
            strcpy(current_var, tracker.result);
            
            while (current < qc) {
                // Track operations that use our current result
                if ((strcmp(quad[current].op1, current_var) == 0 || 
                     strcmp(quad[current].op2, current_var) == 0) && 
                    (strcmp(quad[current].oper, "+") == 0 || strcmp(quad[current].oper, "-") == 0)) {
                    
                    int const_val = 0;
                    int valid_operation = 0;
                    
                    // Check if we're adding/subtracting a constant
                    if (strcmp(quad[current].op1, current_var) == 0 && 
                        (isdigit(quad[current].op2[0]) || (quad[current].op2[0] == '-' && isdigit(quad[current].op2[1])))) {
                        const_val = atoi(quad[current].op2);
                        valid_operation = 1;
                    } 
                    else if (strcmp(quad[current].op2, current_var) == 0 && 
                             (isdigit(quad[current].op1[0]) || (quad[current].op1[0] == '-' && isdigit(quad[current].op1[1])))) {
                        const_val = atoi(quad[current].op1);
                        valid_operation = 1;
                    }
                    
                    if (valid_operation) {
                        // Update the tracker based on the operation
                        if (strcmp(quad[current].oper, "+") == 0) {
                            tracker.net_const += const_val;
                        } else { // "-" operation
                            // If our variable is op1, we subtract the constant
                            if (strcmp(quad[current].op1, current_var) == 0) {
                                tracker.net_const -= const_val;
                            } else {
                                // If our variable is op2, the constant is subtracted from our variable
                                // This is a special case that might need complex handling
                                // For now, we'll break the sequence
                                break;
                            }
                        }
                        
                        // Update tracking information
                        last_result_quad = current;
                        strcpy(current_var, quad[current].res);
                        current++;
                    } else {
                        // If operation involves another variable, break the sequence
                        break;
                    }
                }
                // Handle the final assignment case
                else if (strcmp(quad[current].oper, ":=") == 0 && 
                         strcmp(quad[current].op1, current_var) == 0) {
                    strcpy(tracker.result, quad[current].res);
                    last_result_quad = current;
                    current++;
                    break; // End of sequence
                }
                else {
                    // Operation doesn't involve our tracked variable, break sequence
                    break;
                }
            }
            
            // Update end_quad if we found a sequence
            tracker.end_quad = last_result_quad;
            
            // If the net constant is zero and we have a sequence (more than one quad)
            if (tracker.net_const == 0 && tracker.end_quad > tracker.start_quad) {
                // printf("Found expression with canceling constants at quads %d-%d\n", 
                    //    tracker.start_quad, tracker.end_quad);
                
                // Determine the correct result variable
                char result_var[200];
                strcpy(result_var, quad[tracker.end_quad].res);
                
                // printf("  Simplified: Net constant effect is 0, optimizing to direct assignment\n");
                // printf("  %s = %s (eliminating all constant operations)\n", result_var, tracker.var);
                
                // Replace with direct assignment
                strcpy(quad[tracker.start_quad].oper, ":=");
                strcpy(quad[tracker.start_quad].op1, tracker.var);
                strcpy(quad[tracker.start_quad].op2, "vide");
                strcpy(quad[tracker.start_quad].res, result_var);
                
                // Remove the unnecessary quads
                int quads_to_remove = tracker.end_quad - tracker.start_quad;
                for (j = tracker.start_quad + 1; j < qc - quads_to_remove; j++) {
                    strcpy(quad[j].oper, quad[j + quads_to_remove].oper);
                    strcpy(quad[j].op1, quad[j + quads_to_remove].op1);
                    strcpy(quad[j].op2, quad[j + quads_to_remove].op2);
                    strcpy(quad[j].res, quad[j + quads_to_remove].res);
                }
                
                qc -= quads_to_remove;
                const_cancel_modified += quads_to_remove;
                i = tracker.start_quad; // Reset i to check from this position again
            }
        }
    }
    
    // printf("Constant cancellation optimization complete: %d modifications made\n", const_cancel_modified);
    
    // PART 3: Special case optimization - handling multiplication by 0
    int mul_zero_modified = 0;
    // printf("\nLooking for multiplication by zero...\n");
    
    for (i = 0; i < qc; i++) {
        // Check for multiplication by 0
        if (strcmp(quad[i].oper, "*") == 0) {
            // Check if either operand is 0
            if (strcmp(quad[i].op1, "0") == 0 || strcmp(quad[i].op2, "0") == 0) {
                // printf("Found multiplication by zero at quad %d: %s = %s * %s\n", 
                    //    i, quad[i].res, quad[i].op1, quad[i].op2);
                
                // Replace with direct assignment of 0
                strcpy(quad[i].oper, ":=");
                strcpy(quad[i].op1, "0");
                strcpy(quad[i].op2, "vide");
                mul_zero_modified++;
            }
        }
    }
    
    // printf("Multiplication by zero optimization complete: %d modifications made\n", mul_zero_modified);
    printf("Total algebraic simplification modifications: %d\n", 
           modified + const_cancel_modified + mul_zero_modified);
}

// Helper function to check if a string is numeric
int is_numeric(char *str) {
    // Empty string or "vide" is not numeric
    if (str == NULL || *str == '\0' || strcmp(str, "vide") == 0) 
        return 0;
    
    // Check first character for minus sign
    int start = 0;
    if (str[0] == '-') start = 1;
    
    // Check remaining characters
    int i;
    for (i = start; str[i] != '\0'; i++) {
        if (!isdigit(str[i])) {
            return 0;
        }
    }
    
    return 1;
}


//* 4. ELIMINATION CODE INUTILE */
void elim_code_inutile() {
    int i;
    bool used[1000] = {false};  // Track which variables are used
    char var_name[2000];
    bool modified = false;
    
    // First pass: Identify all variable uses
    for (i = 0; i < qc; i++) {
        // Variables used as operands are considered used
        if (strcmp(quad[i].op1, "vide") != 0 && strcmp(quad[i].op1, "") != 0) {
            // Handle array access
            char base_var[2000];
            strcpy(base_var, quad[i].op1);
            char* bracket_pos = strchr(base_var, '[');
            if (bracket_pos != NULL) {
                *bracket_pos = '\0';  // Truncate at the bracket
            }
            // Mark as used
            int j;
            for (j = 0; j < qc; j++) {
                if (strcmp(quad[j].res, base_var) == 0) {
                    used[j] = true;
                }
            }
        }
        
        if (strcmp(quad[i].op2, "vide") != 0 && strcmp(quad[i].op2, "") != 0) {
            // Handle array access for op2
            char base_var[2000];
            strcpy(base_var, quad[i].op2);
            char* bracket_pos = strchr(base_var, '[');
            if (bracket_pos != NULL) {
                *bracket_pos = '\0';  // Truncate at the bracket
            }
            // Mark as used
            int j;
            for (j = 0; j < qc; j++) {
                if (strcmp(quad[j].res, base_var) == 0) {
                    used[j] = true;
                }
            }
        }
    }
    
    // Second pass: Mark all computations that influence control flow or output
    for (i = 0; i < qc; i++) {
        // Control flow instructions are always necessary
        if (strcmp(quad[i].oper, "BR") == 0 || 
            strcmp(quad[i].oper, "BZ") == 0 ||
            strcmp(quad[i].oper, "BNZ") == 0 ||
            strcmp(quad[i].oper, "BE") == 0 ||
            strcmp(quad[i].oper, "BNE") == 0 ||
            strcmp(quad[i].oper, "BG") == 0 ||
            strcmp(quad[i].oper, "BGE") == 0 ||
            strcmp(quad[i].oper, "BL") == 0 ||
            strcmp(quad[i].oper, "BLE") == 0) {
            used[i] = true;
        }
        
        // Array declarations are necessary
        if (strcmp(quad[i].oper, "ADEC") == 0) {
            used[i] = true;
        }
    }
    
    // Third pass: Mark computations that contribute to used variables
    bool change;
    do {
        change = false;
        for (i = qc - 1; i >= 0; i--) {
            if (!used[i]) {
                // Check if result of this computation is used anywhere
                bool is_used = false;
                int j;
                for (j = i + 1; j < qc && !is_used; j++) {
                    if (used[j]) {
                        if ((strcmp(quad[j].op1, quad[i].res) == 0) ||
                            (strcmp(quad[j].op2, quad[i].res) == 0)) {
                            is_used = true;
                        }
                    }
                }
                
                if (is_used) {
                    used[i] = true;
                    change = true;
                }
            }
        }
    } while (change);
    
    // Final pass: Remove unused quadruplets
    int new_qc = 0;
    for (i = 0; i < qc; i++) {
        if (used[i]) {
            if (i != new_qc) {
                // Copy this quad to its new position
                strcpy(quad[new_qc].oper, quad[i].oper);
                strcpy(quad[new_qc].op1, quad[i].op1);
                strcpy(quad[new_qc].op2, quad[i].op2);
                strcpy(quad[new_qc].res, quad[i].res);
                modified = true;
            }
            new_qc++;
        } else {
            modified = true;
            printf("Removed dead code: %s %s, %s, %s\n", 
                quad[i].oper, quad[i].op1, quad[i].op2, quad[i].res);
        }
    }
    
    if (modified) {
        printf("\nOptimized: Removed %d unused quadruplets\n", qc - new_qc);
        qc = new_qc;  // Update the quadruple count
    } else {
        printf("\nNo optimization needed - all code is used\n");
    }
}

// Main optimization function
void optimize_quads() {
    printf("\n**OPTIMISATION DU CODE**\n");
    
    // Apply optimizations iteratively until no changes
    int old_qc;
    do {
        old_qc = qc;
        propagation_copie();
        propagation_expr();
        elim_expr_com();
        algebraic_simplification();
        elim_code_inutile();
    } while (qc < old_qc); // Repeat if quadruplets were removed
    
    printf("**OPTIMISATION TERMINE**\n");
}
