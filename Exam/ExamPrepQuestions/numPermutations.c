#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHILDREN 100

typedef struct Name {
    char *name;
    int count;
} Name;

int factorial(int num){
    if (num == 0){
        return 1;
    }
    return (num * factorial(num - 1));
}

int permutations(Name **names, int numNames){
    if (numNames == 0) {
        return 0;
    }

    int numerator = factorial(numNames);
    int denominator = 1;
    for (int i = 0; i < numNames; i++){
        denominator *= factorial(names[i]->count);
    }
    return numerator / denominator;
}


int main() {
    Name *names[MAX_CHILDREN];
    int numNames = 0;

    char name[128];

    while (numNames < MAX_CHILDREN) {
        printf("Enter names or 'done' if there aren't anymore names: ");
        scanf("%s", name);

        if (strcmp(name, "done") == 0) {
            break;
        }

        int found = 0;
        for (int i = 0; i < numNames; i++) {
            if (strcmp(name, names[i]->name) == 0) {
                names[i]->count++;
                found = 1;
                break;
            }
        }
        if (!found) {
            names[numNames] = malloc(sizeof(Name));
            names[numNames]->name = strdup(name);
            names[numNames]->count = 1;
            numNames++;
        }
    }

    // Calculate and print the number of permutations
    printf("Number of permutations: %d\n", permutations(&names[0], numNames));


    // Output the names and their counts
    for (int i = 0; i < numNames; i++) {
        printf("%s: %d\n", names[i]->name, names[i]->count);
        free(names[i]->name);
        free(names[i]);
    }

    
    return 0;
}
