#include "l8_common.h"

#define SPELL_TYPES 3
const char* spell_names[SPELL_TYPES] = {"Divination", "Summon Elemental", "Fireball"};
#define BOARD_SIZE 8
#define BACKLOG 16

#define MAX_QUEUE 10
#define THREAD_COUNT 3
#define FAMILIAR_DELAY 100

#define MAX_CLIENTS 2
#define MAX_NAME_LENGTH 14

typedef struct __attribute__((__packed__)) packed
{
    char c1;
    int i1;
    char c2;
    int i2;
};
typedef struct not_packed
{
    char c1;
    int i1;
    char c2;
    int i2;
};

void usage(char* name)
{
    printf("%s <in_port>\n", name);
    printf("  in_port - port that accepts messages\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv)
{
    printf("sizeof(struct packed) == %d\n", sizeof(struct packed));
    printf("sizeof(struct not_packed) == %d\n", sizeof(struct not_packed));
}
