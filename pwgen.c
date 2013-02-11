/*
 * pwgen.c -- Generates passwords using Apple's private API
 * Written by Anders Bergh <anders1@gmail.com> in 2012
 *
 * Public Domain
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <assert.h>

#include <CoreFoundation/CoreFoundation.h>

#include "SecurityFoundation/SFPasswordAssistant.h"

#define MIN_LENGTH 8
#define MAX_LENGTH 31

static void usage(const char *argv0) {
    printf("usage: %s [options]\n\n", argv0);
    printf("Option:            Meaning:\n");
    printf("  -c, --count        The number of passwords to generate.\n");
    printf("  -a, --algorithm    One of {memorable, random, letters, alphanumeric, numbers}.\n");
    printf("                     The default algorithm is `memorable'.\n");
    printf("  -l, --length       The desired length of the generated passwords.\n");
    printf("  -h, --help         Prints this message.\n");
    exit(1);
}

int main (int argc, char *argv[]) {
    // Default options
    int count = 1;
    int length = 12;
    SFPWAAlgorithm algorithm = kSFPWAAlgorithmMemorable;

    const struct option longopts[] = {
        { "algorithm", optional_argument, NULL, 'a' },
        { "count",     required_argument, NULL, 'c' },
        { "length",    required_argument, NULL, 'l' },
        { "help",      no_argument,       NULL, 'h' },
        { NULL,        0,                 NULL, 0   }
    };

    char ch;
    while ((ch = getopt_long(argc, argv, "c:a:l:h", longopts, NULL)) != -1) {
        switch (ch) {
            case 'a':
                if (strcmp(optarg, "memorable") == 0)
                    algorithm = kSFPWAAlgorithmMemorable;

                else if (strcmp(optarg, "random") == 0)
                    algorithm = kSFPWAAlgorithmRandom;

                else if (strcmp(optarg, "letters") == 0)
                    algorithm = kSFPWAAlgorithmLetters;

                else if (strcmp(optarg, "alphanumeric") == 0)
                    algorithm = kSFPWAAlgorithmAlphanumeric;

                else if (strcmp(optarg, "numbers") == 0)
                    algorithm = kSFPWAAlgorithmNumbers;

                else {
                    fprintf(stderr, "%s: error: unknown algorithm.\n", argv[0]);
                    usage(argv[0]);
                    return 1;
                }
                break;

            case 'c':
                count = atoi(optarg);
                break;

            case 'l':
                length = atoi(optarg);
                break;

            default:
                usage(argv[0]);
                return 1;
        }
    }

    if (count < 1)
        count = 1;
    
    if (length < MIN_LENGTH)
        length = MIN_LENGTH;

    else if (length > MAX_LENGTH)
        length = MAX_LENGTH;

    SFPWAContextRef ctx = SFPWAContextCreateWithDefaults();
    assert(ctx != NULL);
    
    CFDictionaryRef policy = SFPWAPolicyCopyDefault();
    assert(policy != NULL);
    
    CFMutableArrayRef suggestions = SFPWAPasswordSuggest(ctx, policy, length, 0, count, algorithm);
    assert(suggestions != NULL);
    
    for (int i = 0; i < CFArrayGetCount(suggestions); i++) {
        CFStringRef s = CFArrayGetValueAtIndex(suggestions, i);
        assert(s != NULL);
        
        int n = CFStringGetMaximumSizeForEncoding(CFStringGetLength(s), kCFStringEncodingUTF8) + 1;
        char *buf = malloc(n);
        assert(buf != NULL);
        
        if (CFStringGetCString(s, buf, n, kCFStringEncodingUTF8))
            printf("%s\n", buf);

        free(buf);
    }

    SFPWAContextRelease(ctx);
    CFRelease(policy);

    return 0;
}
