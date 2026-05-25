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

typedef struct
{
    cast_message_t buff[MAX_QUEUE];
    pthread_mutex_t mutex;
    sem_t sem;
    int head;
    int tail;
    int count;

} queue_t;

typedef struct
{
    struct sockaddr_in addr;
    char name[MAX_NAME_LENGTH + 1];
    int pebbles;
    int logged;
} beloved_t;

typedef struct
{
    beloved_t Misha;
    beloved_t Zosia;
    int started;
    queue_t queue;
    pthread_mutex_t mutex;
} gamestate_t;

void queue_init(queue_t* queue)
{
    pthread_mutex_init(&queue->mutex, NULL);
    sem_init(&queue->sem, 0, 0);
    queue->head = queue->tail = queue->count = 0;
}
void queue_destroy(queue_t* queue)
{
    pthread_mutex_destroy(&queue->mutex);
    sem_destroy(&queue->sem);
}
void queue_push(queue_t* queue, cast_message_t* message)
{
    pthread_mutex_lock(&queue->mutex);

    if (queue->count < MAX_QUEUE)
    {
        memcpy(&queue->buff[queue->tail], message, sizeof(cast_message_t));
        queue->tail = (queue->tail + 1) % MAX_QUEUE;
        queue->count++;
        sem_post(&queue->sem);
    }
    pthread_mutex_unlock(&queue->mutex);
}

cast_message_t queue_pop(queue_t* queue)
{
    sem_wait(&queue->sem);

    pthread_mutex_lock(&queue->mutex);
    cast_message_t message = queue->buff[queue->head];
    queue->head = (queue->head + 1) % MAX_QUEUE;
    queue->count--;
    pthread_mutex_unlock(&queue->mutex);

    return message;
}

void gamestate_init(gamestate_t* game)
{
    game->Misha.name[0] = 0;
    game->Zosia.name[0] = 0;
    game->Misha.pebbles = 10;
    game->Zosia.pebbles = 10;
    game->Zosia.logged = 0;
    game->Misha.logged = 0;
    game->started = 0;
    queue_init(&game->queue);
}

void add_beloved(beloved_t beloved, gamestate_t* game)
{
    if ((game->Misha.logged &&
         !memcmp(&game->Misha.addr, (struct sockaddr_in*)&beloved.addr, sizeof(struct sockaddr_in))) ||
        (game->Zosia.logged &&
         !memcmp(&game->Zosia.addr, (struct sockaddr_in*)&beloved.addr, sizeof(struct sockaddr_in))))
    {
        printf("Rejecting the beloved because of self-love\n");
        return;
    }

    if (!game->Misha.logged)
    {
        game->Misha = beloved;
        printf("Misha joined\n");
    }

    else if (!game->Zosia.logged)
    {
        game->Zosia = beloved;
        game->started = 1;
        printf("Zosia joined\n");
    }

    else
    {
        printf("We like monogamy\n");
    }
}

int is_logged_in(gamestate_t* game, struct sockaddr_in addr)
{
    if (game->Misha.logged && !memcmp(&game->Misha.addr, &addr, sizeof(addr)))
    {
        return 1;
    }
    else if (game->Zosia.logged && !memcmp(&game->Zosia.addr, &addr, sizeof(addr)))
    {
        return 1;
    }
    return 0;
}

int get_beloved_index(gamestate_t* game, struct sockaddr_in addr)
{
    if (game->Misha.logged && !memcmp(&game->Misha.addr, &addr, sizeof(addr)))
    {
        return 0;
    }
    else if (game->Zosia.logged && !memcmp(&game->Zosia.addr, &addr, sizeof(addr)))
    {
        return 1;
    }
    return -1;
}

void usage(char* name)
{
    printf("%s <in_port>\n", name);
    printf("  in_port - port that accepts messages\n");
    exit(EXIT_FAILURE);
}

void* worker(void* args)
{
    gamestate_t* game = args;

    while (1)
    {
        cast_message_t message = queue_pop(&game->queue);

        pthread_mutex_lock(&game->mutex);
        char* name;
        if (message.padding == 0)
        {
            name = game->Misha.name;
        }
        else
        {
            name = game->Zosia.name;
        }
        printf("[Cast] <%s> casts <%s> onto <%hu>,<%hu>\n", name, spell_names[message.spell], message.X, message.Y);
        pthread_mutex_unlock(&game->mutex);

        // for that player implement the logic, some if statements
    }
}

void doServer(int fd)
{
    pthread_t threads[THREAD_COUNT];

    gamestate_t game;
    gamestate_init(&game);

    for (int i = 0; i < THREAD_COUNT; i++)
    {
        if (pthread_create(&threads[i], NULL, worker, &game) < 0)
            ERR("pthread_create");
    }

    struct sockaddr_in addr;
    socklen_t addr_len;
    char buff[MAX_BUFF_LEN + 1];
    memset(buff, 0, 17);
    int count_message = 0;
    while (count_message <= 4)
    {
        int message_len;
        if ((message_len = recvfrom(fd, buff, MAX_BUFF_LEN, 0, (struct sockaddr*)&addr, &addr_len)) < 0)
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
            login_message_t* message;
            message = (login_message_t*)buff;
            printf("[Login] Welcome, <%s>\n", message->name);
            count_message++;

            beloved_t beloved;
            beloved.addr = addr;
            strcpy(beloved.name, message->name);
            beloved.logged = 1;
            beloved.pebbles = 10;
            pthread_mutex_lock(&game.mutex);
            add_beloved(beloved, &game);
            pthread_mutex_unlock(&game.mutex);
        }
        else
        {
            pthread_mutex_lock(&game.mutex);
            if (!is_logged_in(&game, addr))
            {
                printf("Third wheel\n");
                pthread_mutex_unlock(&game.mutex);
                continue;
            }
            pthread_mutex_unlock(&game.mutex);
            if (buff[0] == 'c')
            {
                cast_message_t* message;
                message = (cast_message_t*)buff;

                message->spell = ntohs(message->spell);
                message->X = ntohs(message->X);
                message->Y = ntohs(message->Y);
                pthread_mutex_lock(&game.mutex);
                message->padding = get_beloved_index(&game, addr);
                pthread_mutex_unlock(&game.mutex);

                if (message->spell >= SPELL_TYPES || message->X >= BOARD_SIZE || message->Y >= BOARD_SIZE)
                {
                    printf("Error in the arguments, value out of range\n");
                    continue;
                }

                count_message++;
                queue_push(&game.queue, message);
            }

            else if (buff[0] == 'q')
            {
                break;
                count_message++;
            }

            else
            {
                printf("Incorrect message type\n");
            }
        }
    }
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        pthread_join(threads[i], NULL);
    }
}

int main(int argc, char** argv)
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
