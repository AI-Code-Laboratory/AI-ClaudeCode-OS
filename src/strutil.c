/* strutil.c — le noyau n'a pas de libc (mode "freestanding"), donc on
 * réimplémente le strict minimum de <string.h> dont on a besoin. */

#include "strutil.h"

int str_len(const char* s) {
    int n = 0;
    while (s[n] != '\0') n++;
    return n;
}

bool_t str_eq(const char* a, const char* b) {
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0') {
        if (a[i] != b[i]) return FALSE;
        i++;
    }
    return a[i] == b[i];
}

bool_t str_starts_with(const char* s, const char* prefix) {
    int i = 0;
    while (prefix[i] != '\0') {
        if (s[i] != prefix[i]) return FALSE;
        i++;
    }
    return TRUE;
}

void str_copy(char* dest, const char* src, int max_len) {
    int i = 0;
    while (src[i] != '\0' && i < max_len - 1) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

/* Découpe 'line' en mots séparés par des espaces, remplit argv[],
 * renvoie le nombre de mots trouvés (max max_args). */
int str_tokenize(char* line, char* argv[], int max_args) {
    int argc = 0;
    int i = 0;

    while (line[i] != '\0' && argc < max_args) {
        while (line[i] == ' ') i++;
        if (line[i] == '\0') break;

        argv[argc++] = &line[i];
        while (line[i] != '\0' && line[i] != ' ') i++;

        if (line[i] == ' ') {
            line[i] = '\0';
            i++;
        }
    }
    return argc;
}

/* Convertit un entier (signé, base 10) en chaîne. Renvoie TRUE si ok. */
bool_t str_to_int(const char* s, int* out) {
    int i = 0;
    bool_t negative = FALSE;
    if (s[0] == '-') { negative = TRUE; i = 1; }
    if (s[i] == '\0') return FALSE;

    int value = 0;
    for (; s[i] != '\0'; i++) {
        if (s[i] < '0' || s[i] > '9') return FALSE;
        value = value * 10 + (s[i] - '0');
    }
    *out = negative ? -value : value;
    return TRUE;
}
