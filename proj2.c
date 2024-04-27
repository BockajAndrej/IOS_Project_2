#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <limits.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <sys/shm.h>
#include <time.h>  // For time(NULL)

#define OUTPUT_FILE_NAME "project2.out"

enum error_list
{
    FORK,
    FILEF,
    ARGC,
    ARGF,
    SDATA,
    MAL,
    CASE,
    SEM
};
enum bus_state_list
{
    START,
    DRIVE,
    ARIVE,
    LEAVE,
    FDRIVE,
    FARIVE,
    FLEAVE,
    FINISH
};

typedef struct Arguments
{
    int L;  // pocet lyziarov L < 20000
    int Z;  // pocet nastupnich zastaviek  0<Z<=10
    int K;  // kapacita skubusu 10<=K<=100
    int TL; // Maximalni cas v mikrosekundach ktory lyziar caka nez pride na zastavku 0<=TL<=10000
    int TB; // Maximalna doba jazdy autobusom medzi dvoma zastavkami 0<=TB<=1000
} Arguments;

typedef struct Shared_data
{
    FILE *file;

    int bstop[10]; // pole zastaviek, obsahuje pocet lyziarov na zastavke
    int cnt;    // pocitadlo vypisov

    struct Sbus
    {
        enum bus_state_list cur_state;
        enum bus_state_list next_state;
        int idZ; // aktualna zastavka
        int cur_number_of_skiers;
        int total_number_of_skiers;
    } bus;

    struct Skier
    {
        int bstop; // zastavka
    } skier;
} Shared_data;

struct Semaphore
{
    sem_t *s1;
    sem_t *s2;
    sem_t *s3;
    sem_t *s4;
    sem_t *s5;
    sem_t *s6;
    sem_t *bstop[10];
} sem;

sem_t *semaphore_init(sem_t *mutex, int init_val)
{
    mutex = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if (mutex == MAP_FAILED)
    {
        fprintf(stderr, "Error: Creating of semaphore mutex failed.\n");
        EXIT_FAILURE;
    }
    else if (sem_init(mutex, 1, init_val) == -1) // 3param = inicialna hodnota semaphoru
    {
        fprintf(stderr, "Error: Initialization of semaphore mutex failed.\n");
        EXIT_FAILURE;
    }
    return mutex;
}

void cleanup(Shared_data *sdata)
{
    sem_destroy(sem.s1);
    sem_destroy(sem.s2);
    sem_destroy(sem.s3);
    sem_destroy(sem.s4);
    sem_destroy(sem.s5);
    sem_destroy(sem.s6);

    for (int i = 0; i < 10; i++)
    {
        sem_destroy(sem.bstop[i]);
    }
    

    munmap(sem.s1, sizeof(sem_t));
    munmap(sem.s2, sizeof(sem_t));
    munmap(sem.s3, sizeof(sem_t));
    munmap(sem.s4, sizeof(sem_t));
    munmap(sem.s5, sizeof(sem_t));
    munmap(sem.s6, sizeof(sem_t));

    for (int i = 0; i < 10; i++)
    {
        munmap(sem.bstop[i], sizeof(sem_t));
    }

    fclose(sdata->file);

    munmap(sdata, sizeof(Shared_data));
}

void error_output(enum error_list err, Shared_data *sdata)
{
    switch (err)
    {
    case FORK:
        fprintf(stderr, "ERROR: Fork\n");
        cleanup(sdata);
        break;
    case FILEF:
        munmap(sdata, sizeof(Shared_data));
        fprintf(stderr, "ERROR: Open file\n");
        break;
    case ARGC:
        munmap(sdata, sizeof(Shared_data));
        fprintf(stderr, "ERROR: Number of parameters\n");
        break;
    case ARGF:
        munmap(sdata, sizeof(Shared_data));
        fprintf(stderr, "ERROR: Limit of some parameter\n");
        break;
    case SDATA:
        munmap(sdata, sizeof(Shared_data));
        fprintf(stderr, "ERROR: Initialization of shared data\n");
        break;
    case MAL:
        fclose(sdata->file);
        munmap(sdata, sizeof(Shared_data));
        fprintf(stderr, "ERROR: Malloc\n");
        break;
    case CASE:
        cleanup(sdata);
        fprintf(stderr, "ERROR: Undefined value in switch\n");
        break;
    case SEM:
        fclose(sdata->file);
        munmap(sdata, sizeof(Shared_data));
        fprintf(stderr, "ERROR: Initialize mutex\n");
        break;
    default:
        break;
    }
    exit(1);
}

void Setup_sdata(Shared_data *sdata)
{
    sdata->file = fopen(OUTPUT_FILE_NAME, "w+");
    if (sdata->file == NULL)
        error_output(FILEF, sdata);

    setbuf(sdata->file, NULL); // nastavenie bofrovania suboru na NULL

    for (int i = 0; i < 10; i++)
        sdata->bstop[i] = 0;

    sdata->cnt = 0;

    sdata->bus.cur_state = START;
    sdata->bus.next_state = DRIVE;
    sdata->bus.idZ = 0;
    sdata->bus.cur_number_of_skiers = 0;
    sdata->bus.total_number_of_skiers = 0;

    sem.s1 = semaphore_init(sem.s1, 1);
    sem.s2 = semaphore_init(sem.s2, 0);
    sem.s3 = semaphore_init(sem.s3, 0); 
    sem.s4 = semaphore_init(sem.s4, 0);
    sem.s5 = semaphore_init(sem.s5, 0);
    sem.s6 = semaphore_init(sem.s6, 1); 

    for (int i = 0; i < 10; i++)
    {
        sem.bstop[i] = semaphore_init(sem.bstop[i], 0);
    }
    
}

void Setup_args(int argc, char **argv, Arguments *args, Shared_data *sdata)
{
    char *checkptr = NULL;
    if (argc != 6)
        error_output(ARGC, sdata);

    if ((args->L = strtol(argv[1], &checkptr, 10)) < 0 || *checkptr != 0 || args->L >= 20000) // L<20000
        error_output(ARGF, sdata);
    if ((args->Z = strtol(argv[2], &checkptr, 10)) < 1 || *checkptr != 0 || args->Z > 10) // 0<Z<=10
        error_output(ARGF, sdata);
    if ((args->K = strtol(argv[3], &checkptr, 10)) < 10 || *checkptr != 0 || args->K > 100) // 10<=K<=100
        error_output(ARGF, sdata);
    if ((args->TL = strtol(argv[4], &checkptr, 10)) < 0 || *checkptr != 0 || args->TL > 10000) // 0<=TL<=10000
        error_output(ARGF, sdata);
    if ((args->TB = strtol(argv[5], &checkptr, 10)) < 0 || *checkptr != 0 || args->TB > 1000) // 0<=TB<=1000
        error_output(ARGF, sdata);
}

int iRandom(int min, int max)
{
    srand((time(NULL) * getpid()));
    return rand() % (max - min + 1) + min;
}

void bus_process(Arguments args, Shared_data *sdata)
{
    // START state
    fprintf(sdata->file, "%i: BUS: started\n", ++sdata->cnt);
    while (sdata->bus.next_state != FINISH)
    {
        sdata->bus.cur_state = sdata->bus.next_state;
        switch (sdata->bus.cur_state)
        {
        case DRIVE:
            sdata->bus.idZ++;
            usleep(iRandom(0, args.TB)); 
            sdata->bus.next_state = ARIVE;
            break;
        case ARIVE:
            fprintf(sdata->file, "%i: BUS: arrived to %i\n", ++sdata->cnt, sdata->bus.idZ);
            // povolenie nastup
            while (sdata->bstop[sdata->bus.idZ-1] != 0 && args.K != sdata->bus.cur_number_of_skiers) 
            {
                fprintf(sdata->file, "Start while sem = %i\n", sdata->bus.idZ-1);
                sem_post(sem.bstop[sdata->bus.idZ - 1]);
                sdata->bstop[sdata->bus.idZ - 1]--;
                sdata->bus.cur_number_of_skiers++;
                sdata->bus.total_number_of_skiers++;
                sem_post(sem.s1);
                fprintf(sdata->file, "waiting with bstop=%i, cur=%i, total=%i\n", sdata->bstop[sdata->bus.idZ - 1], sdata->bus.cur_number_of_skiers, sdata->bus.total_number_of_skiers);
                sem_wait(sem.s3);
            }
            sdata->bus.next_state = LEAVE;
            break;
        case LEAVE:
            fprintf(sdata->file, "%i: BUS: leaving %i\n", ++sdata->cnt, sdata->bus.idZ);
            if (sdata->bus.idZ < args.Z)
                sdata->bus.next_state = DRIVE;
            else
                sdata->bus.next_state = FDRIVE;
            break;
        case FDRIVE:
            usleep(iRandom(0, args.TB)); 
            sdata->bus.next_state = FARIVE;
            break;
        case FARIVE:
            fprintf(sdata->file, "%i: BUS: arived to final\n", ++sdata->cnt);
            // povolenie vystup
            while (sdata->bus.cur_number_of_skiers > 0)
            {
                sem_post(sem.s4);
                sdata->bus.cur_number_of_skiers--;
                sem_wait(sem.s5);
            }
            sdata->bus.next_state = FLEAVE;
            break;
        case FLEAVE:
            fprintf(sdata->file, "%i: BUS: leaving final\n", ++sdata->cnt);
            if (sdata->bus.total_number_of_skiers < args.L)
            {
                sdata->bus.idZ = 0;
                sdata->bus.next_state = DRIVE;
            }
            else
                sdata->bus.next_state = FINISH;
            break;
        default:
            error_output(CASE, sdata);
            break;
        }
    }
    // FINISH state
    fprintf(sdata->file, "%i: BUS: finish\n", ++sdata->cnt);
}

void skier_process(Arguments args, Shared_data *sdata, int id)
{
    // START
    fprintf(sdata->file, "%i: L %i: started\n", ++sdata->cnt, id);
    // BREAKFAST
    usleep(iRandom(0, args.TL));
    // BSTOP
    // sem_wait(sem.s1);
    fprintf(sdata->file, "%i: L %i: arrived to %i\n", ++sdata->cnt, id, sdata->skier.bstop);
    sdata->bstop[sdata->skier.bstop - 1]++;
    // sem_post(sem.s1);
    // BOARDING
    sem_wait(sem.bstop[sdata->skier.bstop-1]);
    fprintf(sdata->file, "%i: L %i: boarding\n", ++sdata->cnt, id);
    sem_wait(sem.s1);
    sem_post(sem.s3);
    // SKI
    sem_wait(sem.s4);
    fprintf(sdata->file, "%i: L %i: going to ski\n", ++sdata->cnt, id);
    sem_post(sem.s5);
} 

int main(int argc, char **argv)
{
    Arguments args;
    Shared_data *sdata = (Shared_data *)mmap(NULL, sizeof(Shared_data), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (sdata == MAP_FAILED)
        error_output(SDATA, NULL);

    Setup_args(argc, argv, &args, sdata);

    Setup_sdata(sdata);

    srand(time(NULL));

    pid_t pid = fork(); // Create a new process

    if (pid < 0)
        error_output(FORK, sdata);
    else if (pid == 0)
    {
        bus_process(args, sdata);
        exit(0);
    }
    else
    {
        for (int i = 1; i <= args.L; i++)
        {
            pid_t pid = fork(); // Create a new process

            if (pid < 0)
            {
                error_output(FORK, sdata);
            }
            else if (pid == 0)
            {
                sdata->skier.bstop = iRandom(1, args.Z);
                skier_process(args, sdata, i);
                exit(0);
            }
        }
    }

    while (wait(NULL) > 0)
        ; // pri fork dolezite neaktivne cakanie (uspi proces) kazdy na konci main, cakanie na child

    cleanup(sdata);

    return 0;
    exit(0);
}