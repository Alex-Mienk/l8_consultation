// Input from the user, client sends the packet



char line[256];

fgets(line, sizeof(line), stdin);

char *cmd = strtok(line, " \n");

if (cmd == NULL) {
    return;
}

if (strcmp(cmd, "cast") == 0) {
    char *spell_str = strtok(NULL, " \n");
    char *x_str = strtok(NULL, " \n");
    char *y_str = strtok(NULL, " \n");

    if (spell_str == NULL || x_str == NULL || y_str == NULL) {
        printf("Invalid cast command\n");
        return;
    }

    int spell = atoi(spell_str);
    int x = atoi(x_str);
    int y = atoi(y_str);

    char packet[8];

    packet[0] = 'c';
    packet[1] = 0;

    uint16_t spell_net = htons((uint16_t)spell);
    uint16_t x_net = htons((uint16_t)x);
    uint16_t y_net = htons((uint16_t)y);

    memcpy(packet + 2, &spell_net, sizeof(uint16_t));
    memcpy(packet + 4, &x_net, sizeof(uint16_t));
    memcpy(packet + 6, &y_net, sizeof(uint16_t));

    sendto(sock, packet, sizeof(packet), 0,
           (struct sockaddr *)&server_addr,
           sizeof(server_addr));
}





// Recieve from the client and print the packet

char buf[8];

ssize_t len = recvfrom(sock, buf, sizeof(buf), 0,
                       (struct sockaddr *)&client_addr,
                       &client_len);

if (len == 8 && buf[0] == 'c') {
    uint16_t spell_net;
    uint16_t x_net;
    uint16_t y_net;

    memcpy(&spell_net, buf + 2, sizeof(uint16_t));
    memcpy(&x_net,     buf + 4, sizeof(uint16_t));
    memcpy(&y_net,     buf + 6, sizeof(uint16_t));

    uint16_t spell = ntohs(spell_net);
    uint16_t x = ntohs(x_net);
    uint16_t y = ntohs(y_net);

    printf("Received cast: spell=%u x=%u y=%u\n", spell, x, y);
}
