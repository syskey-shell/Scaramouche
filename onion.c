#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include "onion.h"
#include "tor.h"

// helper to get http code from socket
static int get_code(int sock, char *host, int wait)
{
    char buf[4096];
    char ask[512];
    int got = 0;
    time_t start;
    int n;

    // make the http head request
    snprintf(ask, sizeof(ask),
             "HEAD / HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Connection: close\r\n"
             "\r\n",
             host);

    if (send(sock, ask, strlen(ask), 0) <= 0)
    {
        return 0;
    }

    start = time(NULL);
    got = 0;

    // read response but don't wait forever
    while (got < sizeof(buf) - 1)
    {
        if (time(NULL) - start > wait)
        {
            break; // took too long
        }

        fd_set readset;
        FD_ZERO(&readset);
        FD_SET(sock, &readset);

        struct timeval tv = {1, 0};
        if (select(sock + 1, &readset, NULL, NULL, &tv) <= 0)
        {
            continue;
        }

        n = recv(sock, buf + got, sizeof(buf) - got - 1, 0);
        if (n <= 0)
        {
            break;
        }

        got += n;
        buf[got] = 0;

        // got all headers?
        if (strstr(buf, "\r\n\r\n"))
        {
            break;
        }
    }

    // tries to parse http code
    if (got >= 12)
    { // Need at least 12 bytes for "HTTP/1.x CODE"
        if (strncmp(buf, "HTTP/", 5) == 0)
        {
            char num[4] = {0};
            int i;

            // Code is at position 9
            for (i = 0; i < 3 && (9 + i) < got; i++)
            {
                if (buf[9 + i] >= '0' && buf[9 + i] <= '9')
                {
                    num[i] = buf[9 + i];
                }
                else
                {
                    break;
                }
            }

            if (num[0] != 0)
            {
                return atoi(num);
            }
        }
    }
    return 0;
}

// this is to check the onion
onion_info check_onion(char *onion, int wait, int port)
{
    onion_info r = {0};
    int sock;
    struct timespec before, after;

    // copy the name
    strncpy(r.name, onion, sizeof(r.name) - 1);
    r.name[sizeof(r.name) - 1] = 0;

    if (!is_onion(onion))
    {
        return r;
    }

    // connect
    sock = tor_connect();
    if (sock < 0)
    {
        return r;
    }

    // socks5 handshake
    if (tor_handshake(sock) < 0)
    {
        close(sock);
        return r;
    }

    // time the actual connection
    clock_gettime(CLOCK_MONOTONIC, &before);

    if (tor_request(sock, onion, port) == 0)
    {
        clock_gettime(CLOCK_MONOTONIC, &after);

        // calc milliseconds
        r.took = (after.tv_sec - before.tv_sec) * 1000;
        r.took += (after.tv_nsec - before.tv_nsec) / 1000000;

        // https just needs connection
        if (port == 443)
        {
            r.code = 200;
            r.alive = 1;
        }
        else
        {
            // http needs actual response
            r.code = get_code(sock, onion, wait);
            r.alive = (r.code > 0);
        }
    }

    close(sock);
    return r;
}

// check if it looks like onion address
int is_onion(char *addr)
{
    if (!addr)
        return 0;

    // just see if .onion is in there somewhere
    if (strstr(addr, ".onion") == NULL)
    {
        return 0;
    }

    return 1;
}

void check_batch(FILE *in, int wait, int port, char *outfile, int quiet)
{
    char line[256]; // for checking the file
    int total = 0;
    int up = 0;
    FILE *out = NULL;

    if (outfile)
    {
        out = fopen(outfile, "w");
        if (out)
        {
            fprintf(out, "address,status,code,time_ms\n");
        }
    }

    if (!quiet)
    {
        printf("Checking onions (wait: %ds, port: %d)\n\n", wait, port);
    }

    while (fgets(line, sizeof(line), in))
    {

        line[strcspn(line, "\r\n")] = 0;

        // skip empty or comments
        if (line[0] == 0 || line[0] == '#')
        {
            continue;
        }

        total++;

        if (!quiet)
        {
            printf("[%d] %s... ", total, line);
            fflush(stdout);
        }

        onion_info r = check_onion(line, wait, port);

        if (!quiet)
        {
            if (r.alive)
            {
                printf("\033[32mUP\033[0m");
                if (port != 443)
                {
                    printf(" (%d)", r.code);
                }
                printf(" %ldms\n", r.took);
            }
            else
            {
                printf("\033[31mDOWN\033[0m\n");
            }
        }

        if (r.alive)
        {
            up++;
        }

        if (out)
        {
            fprintf(out, "%s,%d,%d,%ld\n",
                    r.name, r.alive, r.code, r.took);
        }
    }

    // shows stat if checked everything
    if (!quiet && total > 0)
    {
        printf("\n");
        printf("╔══════════════════════════════════════╗\n");
        printf("║             Results                  ║\n");
        printf("╠══════════════════════════════════════╣\n");
        printf("║ Checked: %-28d ║\n", total);
        printf("║ Up:      \033[32m%-28d\033[0m ║\n", up);
        printf("║ Down:    \033[31m%-28d\033[0m ║\n", total - up);

        float percent = 0;
        if (total > 0)
        {
            percent = (float)up / total * 100;
        }
        printf("║ Success: %-28.1f%% ║\n", percent);
        printf("╚══════════════════════════════════════╝\n");
    }

    if (out)
    {
        fclose(out);
    }
}

// show single result
void print_result(onion_info *r, int quiet)
{
    if (quiet)
        return;

    printf("%-60s ", r->name);

    if (r->alive)
    {
        printf("\033[32;1mUP\033[0m");
        if (r->code != 200)
        {
            printf(" [%d]", r->code);
        }
        printf(" %ldms", r->took);
    }
    else
    {
        printf("\033[31;1mDOWN\033[0m");
    }

    printf("\n");
}

// save to csv file
void save_to_csv(onion_info *r, char *filename)
{
    FILE *f = fopen(filename, "a");
    if (!f)
        return;

    // write header if new file
    fseek(f, 0, SEEK_END);
    if (ftell(f) == 0)
    {
        fprintf(f, "address,status,code,time_ms\n");
    }

    fprintf(f, "%s,%d,%d,%ld\n",
            r->name, r->alive, r->code, r->took);

    fclose(f);
}