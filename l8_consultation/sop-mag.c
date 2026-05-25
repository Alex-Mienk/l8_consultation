#include "l8_common.h"

#define SPELL_TYPES 3
const char *spell_names[SPELL_TYPES] = {"Divination", "Summon Elemental", "Fireball"};
#define BOARD_SIZE 8
#define BACKLOG 16

#define MAX_QUEUE 10
#define THREAD_COUNT 3
#define FAMILIAR_DELAY 100

#define MAX_CLIENTS 2
#define MAX_NAME_LENGTH 14

#define MAX_BUFF_LEN 16

// typedef struct __attribute__((__packed__)) packed
// {
//     char c1;
//     int i1;
//     char c2;
//     int i2;
// };
// typedef struct not_packed
// {
//     char c1;
//     int i1;
//     char c2;
//     int i2;
// };

typedef struct message_t
{
    char buff[16];
} message_t;

typedef struct __attribute__((__packed__)) cast_message_t
{
    char type;
    char padding;
    uint16_t spell;
    uint16_t X;
    uint16_t Y;

} cast_message_t;

typedef struct __attribute__((__packed__)) quit_message_t
{
    char type;

} quit_message_t;

typedef struct __attribute__((__packed__)) login_message_t
{
    char type;
    char padding;
    char name[MAX_NAME_LENGTH + 1];

} login_message_t;

void usage(char *name)
{
    printf("%s <in_port>\n", name);
    printf("  in_port - port that accepts messages\n");
    exit(EXIT_FAILURE);
}

void doServer(int fd)
{
    struct sockaddr_in addr;
    socklen_t addr_len;
    char buff[MAX_BUFF_LEN + 1];
    memset(buff, 0, 17);
    int count_message = 0;
    while (count_message <= 4)
    {
        int message_len;
        if ((message_len = recvfrom(fd, buff, MAX_BUFF_LEN, 0, (struct sockaddr *)&addr, &addr_len)) < 0)
        {
            perror("recvfrom");
            continue;
        }

        if (message_len < 16)
        {
            printf("message of incorrect length\n");
            continue;
        }

        if (buff[0] == 'l')
        {
            login_message_t *message;
            message = (login_message_t *)buff;
            printf("[Login] Welcome, <%s>\n", message->name);
            count_message++;
        }

        else if (buff[0] == 'c')
        {
            cast_message_t *message;
            message = (cast_message_t *)buff;

            message->spell = ntohs(message->spell);
            message->X = ntohs(message->X);
            message->Y = ntohs(message->Y);

            if (message->spell >= SPELL_TYPES || message->X >= BOARD_SIZE || message->Y >= BOARD_SIZE)
            {
                printf("Error in the arguments, value out of range\n");
                continue;
            }
            printf("[Cast] Someone casts <%s> onto <%hu>,<%hu>\n", spell_names[message->spell], message->X, message->Y);
            count_message++;
        }

        else if (buff[0] == 'q')
        {
            count_message++;
        }

        else
        {
            printf("Incorrect message type\n");
        }
    }
}

int main(int argc, char **argv)
{
    // printf("sizeof(struct packed) == %d\n", sizeof(struct packed));
    // printf("sizeof(struct not_packed) == %d\n", sizeof(struct not_packed));

    if (argc != 2)
    {
        usage(argv[0]);
    }

    uint16_t port = atoi(argv[1]);

    int fd = bind_inet_socket(port, SOCK_DGRAM, 10);

    doServer(fd);

    if (close(fd) < 0)
        ERR("close");

    return 0;
}
