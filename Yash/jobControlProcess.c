
typedef enum {false, true} bool;


typedef struct jobs{
    jobs* next;
    int cpid;
    bool status;
    char* command; 
} jobs;
