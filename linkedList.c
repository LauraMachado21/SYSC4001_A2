#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "process.h"


//is list empty
bool isEmpty() {
   return head == NULL;
}

int length() {
   int length = 0;
   struct node *current;

   for(current = head; current != NULL; current = current->next) {
      length++;
   }

   return length;
}
