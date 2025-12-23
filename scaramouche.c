#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "onion.h"


void show_banner() {
    puts("");
    puts("    ╔══════════════════════════════════════╗");
    puts("    ║             Scaramouche              ║");
    puts("    ║                                      ║");
    puts("    ╚══════════════════════════════════════╝");
    puts("");
}

//help
void usage(char *prog) {
    show_banner();
    printf("use: %s [stuff] onion_or_file\n\n", prog);
    printf("stuff can be:\n");
    printf("  -t sec    how long to wait (default 10)\n");
    printf("  -p port   port (default 80, 443 for https)\n");
    printf("  -o file   write results here\n");
    printf("  -q        be quiet about it\n");
    printf("  -h        this message\n\n");
    printf("like:\n");
    printf("  %s onion.onion\n", prog);
    printf("  %s -t 5 list.txt\n", prog);
    printf("  %s -o results.csv onions.txt\n", prog);
}

int main(int argc, char **argv) {
    int wait = 10;
    int port = 80;
    char *outfile = NULL;
    int quiet = 0;
    int c;
    FILE *fp;
    onion_info result;
    
    
    while ((c = getopt(argc, argv, "t:p:o:qh")) != -1) {
        switch (c) {
            case 't':
                wait = atoi(optarg);
                if (wait < 1) wait = 1;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'o':
                outfile = optarg;
                break;
            case 'q':
                quiet = 1;
                break;
            case 'h':
                usage(argv[0]);
                return 0;
            default:
                usage(argv[0]);
                return 1;
        }
    }
    
    
    if (optind >= argc) {
        if (!quiet) {   //needs one argument 
            fprintf(stderr, "need an onion address or file\n");
        }
        usage(argv[0]);
        return 1;
    }
    
    char *target = argv[optind];
    
    if (!quiet) {
        show_banner();
        printf("checking: %s\n", target);
        printf("timeout: %d seconds, port: %d\n", wait, port);
        if (outfile) {
            printf("output: %s\n", outfile);
        }
        printf("\n");
    }
    
    // try to open as file first
    fp = fopen(target, "r");
    if (fp) {
        check_batch(fp, wait, port, outfile, quiet);
        fclose(fp);
    } else {
        // just one onion
        result = check_onion(target, wait, port);
        print_result(&result, quiet);
        if (outfile) {
            save_to_csv(&result, outfile);
        }
    }
    
    return 0;
}