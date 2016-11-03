#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

clock_t start_t;
FILE *outputFile;

typedef struct Processes{
  int PID;
  int arrival_time;
  int remaining_CPU_time;
  int IO_Frequency;
  int IO_Duration;
  char current_state[];
} Process;

typedef struct Node {
   Process *process;
   struct Node *next;
}node;

typedef struct Queue{
  node *head;
  node *current;
}Queue;

Process *convertLineToProcess(char line[]);
void readyState();
void runningState(Process *p);
void IOSim(Process *p);
void printList(Queue *p);
Queue ready, processes;
bool programRunning = false;
int currentTime=0;
int timeElapsed =0;
bool isEmpty(){
bool answer;
answer = (ready.head==NULL);

  return answer;
}

//display the list
void printList(Queue *q) {
   node *ptr = q->head;
   printf("[ ");

   //start from the beginning
   while(ptr != NULL) {
      printf("(%d) ",ptr->process->PID);
      ptr = ptr->next;
   }

   printf(" ]\n");
}

void outputToFile(Process *p, char new[]){
  outputFile = fopen("processLog.txt", "a+");
  fprintf(outputFile," %-15d | %-15d | %-15s | %-15s \n", timeElapsed, p->PID, p->current_state, new);
  fputs("------------------------------------------------------------------------\n",outputFile);
  fclose(outputFile);
  strcpy(p->current_state,new);
}

void addToReadyQueue(Process *p){
  node *link = (node*) malloc(sizeof(node));
  link->process = p;

  if(ready.head==NULL){
    ready.head=link;
  }else{
    node *temp = ready.head;
    link->next = temp;
    ready.head=link;
  }
}

//insert link at the first location
void enqueue(Queue *q, Process *p) {
   //create a link
  node *link = (node*) malloc(sizeof(node));

  link->process = p;
//   //point it to old first node
  link->next = NULL;

  if(q->head==NULL){
      q->head = link;
  } else{
      node *temp = q->head;
      //point first to new first node
      while(temp->next!=NULL){
         temp=temp->next;
      }
      temp->next = link;
      temp=link;
  }
}

//delete first item
Process * dequeue(Queue *q) {
   //save reference to first link
   node *tempLink = q->head;
   //mark next to first link as first
   q->head = q->head->next;
   //return the deleted link
   return tempLink->process;
}

void readFromFile(char fileName[]){
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;

  fp = fopen(fileName, "r");

  while ((read = getline(&line, &len, fp)) != -1) {
    Process *p = convertLineToProcess(line);
    enqueue(&processes,p);

  }
};

Process * convertLineToProcess(char* line){
    Process *p = malloc(sizeof(Process));
    char * pData;
    char *token;

    pData = strdup(line);
    int i=0;

    while((token = strsep(&pData, "|"))!=NULL){
      if(i==0){
        p->PID = atoi(token);
      }else if(i==1){
        p->arrival_time=atoi(token);
      }else if(i==2){
        p->remaining_CPU_time=atoi(token);
      }else if(i==3){
        p->IO_Frequency=atoi(token);
      }else{
        p->IO_Duration=atoi(token);
      };
      i++;
    }

    strcpy(p->current_state,"New");
    return p;
}

void startSim(){
  bool doneProcesses=false;
  int time=0, i;
  int prevTime=0;
  int processRunning=false;
  Process *p = NULL;
  Process *running;
  int process_sent_to_running_at=0;
  int process_sent_to_waiting_at = 0;
  while(1){
    clock_t current_t = clock();
    timeElapsed =(current_t - start_t)/CLOCKS_PER_SEC;
    if(!doneProcesses && processes.head->process->arrival_time == timeElapsed){
      Process *k= dequeue(&processes);
      outputToFile(k,"Ready");
      printf("Time: %d \tProcess %d: Start to Ready\nReay Queue: ", timeElapsed,k->PID);
      enqueue(&ready, k);
      printList(&ready);
    }
      if(!processRunning){
        if(ready.head!=NULL){
          p = dequeue(&ready);
          outputToFile(p,"Running");
          printf("Time: %d \tProcess %d: Ready to Running\nReady Queue After Removal", timeElapsed,p->PID);
          processRunning=true;
          process_sent_to_running_at = timeElapsed;
          printList(&ready);
        }
      }else{
        if(p->IO_Frequency==0){
          if(timeElapsed==process_sent_to_running_at+p->remaining_CPU_time){
            printf("Time: %d \tDone serving process %d\n",timeElapsed,p->PID);
            processRunning=false;
          }
        }else{
          if(p->remaining_CPU_time>0){
            //The process has an IO to handle
            if(timeElapsed==process_sent_to_running_at+p->IO_Frequency){
              //send it to the waiting state
              process_sent_to_waiting_at = timeElapsed;
              p->remaining_CPU_time = p->remaining_CPU_time-(process_sent_to_waiting_at-process_sent_to_running_at);
              outputToFile(p,"Waiting");
              printf("Time: %d \tProcess %d: Running to Waiting", timeElapsed,p->PID);
              processRunning=false; // free the cpu
            }

            //This is the waiting state code
            if(timeElapsed==process_sent_to_waiting_at+p->IO_Duration){
              //send the processs back to ready queue
              enqueue(&ready, p);
              printf("Time: %d \tProcess %d: Waiting to Running", timeElapsed,p->PID);
            }
          }
          else{
            outputToFile(p,"Terminate");
            printf("Time: %d \tDone serving process %d\n",timeElapsed,p->PID);
          }
        }
      }
    if(processes.head==NULL){ //check if no more processes are left.
      //printList(sharedMemoryAddress);
      doneProcesses=true;
    }
  }
}

void runningState(Process *p){
  clock_t CPU_t = clock();
  programRunning = true;
  if(p->IO_Frequency==0){
  //   while((clock()-CPU_t)/CLOCKS_PER_SEC != p->remaining_CPU_time){
  // };
  int timeTaken = ((clock()-timeElapsed)/CLOCKS_PER_SEC);
  if(timeTaken==ready.head->process->remaining_CPU_time){
    printf("Time: %d \tDone serving process %d\n",timeTaken,ready.head->process->PID);
  }
    //dequeue(&ready);
  }
      // }else{
  //   while((clock()-CPU_t)/CLOCKS_PER_SEC != p->IO_Frequency){
  //   }; // wait
  //   p->remaining_CPU_time = p->remaining_CPU_time - p->IO_Frequency;
  //   printf("Processes %d added to io.\n", p->PID);
  // }

  //IOSim(p);
}

void IOSim(Process *p){
  clock_t IO_t = clock();
  programRunning = false;
  while((clock()-IO_t)/CLOCKS_PER_SEC != p->IO_Duration){};
  if(p->remaining_CPU_time!=0){
      enqueue(&ready, p);
  }

  printf("Processes %d added to ready queue.\n", p->PID);
}

int main(void){
  outputFile = fopen("processLog.txt", "a+");
  fputs("------------------------------------------------------------------------\n",outputFile);
  fprintf(outputFile," %-15s | %-15s | %-15s | %-15s \n", "Transition Time", "PID", "Old State", "New State");
  fputs("------------------------------------------------------------------------\n",outputFile);
  fclose(outputFile);

  readFromFile("part1c.txt");

  //start_t = clock();
  startSim();
  // int simProcessID = fork();
  //   if(simProcessID==0){
  //       startSim();
  //   }else{
  //       readyState();
  //   }



  return 0;
}
