/* dosexec.c — interpréteur inspiré des commandes MS-DOS.
 *
 * IMPORTANT : ce n'est PAS un émulateur MS-DOS. Il ne fait tourner
 * aucun vrai binaire .EXE/.COM x86 réel-mode — ça demanderait un
 * émulateur complet façon DOSBox (des dizaines de milliers de lignes).
 * C'est un ré-implémentation "dans l'esprit de" DOS : mêmes commandes,
 * même vocabulaire, sur un mini système de fichiers en mémoire vive.
 */

#include "vga.h"
#include "strutil.h"

#define MAX_FILES 8
#define MAX_FILE_CONTENT 512

typedef struct {
    char name[13];      /* format 8.3 façon DOS */
    char content[MAX_FILE_CONTENT];
    bool_t used;
} DosFile;

static DosFile files[MAX_FILES];
static int file_count = 0;

static void dos_seed_filesystem(void) {
    if (file_count > 0) return; /* déjà initialisé */

    str_copy(files[0].name, "README.TXT", 13);
    str_copy(files[0].content,
        "windOS 0.01 - mini interpreteur facon DOS.\n"
        "Tape HELP pour la liste des commandes.\n", MAX_FILE_CONTENT);
    files[0].used = TRUE;

    str_copy(files[1].name, "AUTOEXEC.BAT", 13);
    str_copy(files[1].content, "ECHO windOS demarre.\n", MAX_FILE_CONTENT);
    files[1].used = TRUE;

    str_copy(files[2].name, "CONFIG.SYS", 13);
    str_copy(files[2].content, "FILES=8\nBUFFERS=4\n", MAX_FILE_CONTENT);
    files[2].used = TRUE;

    file_count = 3;
}

static DosFile* dos_find_file(const char* name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used && str_eq(files[i].name, name)) return &files[i];
    }
    return 0;
}

static void dos_cmd_dir(void) {
    vga_print(" Repertoire de C:\\\n\n");
    int total = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (!files[i].used) continue;
        vga_print(" ");
        vga_print(files[i].name);
        vga_print("\t");
        vga_print_int(str_len(files[i].content));
        vga_print(" octets\n");
        total++;
    }
    vga_print("\n ");
    vga_print_int(total);
    vga_print(" fichier(s)\n");
}

static void dos_cmd_type(const char* filename) {
    if (filename == 0) {
        vga_print("Syntaxe : TYPE <fichier>\n");
        return;
    }
    DosFile* f = dos_find_file(filename);
    if (f == 0) {
        vga_print("Fichier introuvable - ");
        vga_print(filename);
        vga_putchar('\n');
        return;
    }
    vga_print(f->content);
}

static void dos_cmd_del(const char* filename) {
    DosFile* f = dos_find_file(filename);
    if (f == 0) {
        vga_print("Fichier introuvable.\n");
        return;
    }
    f->used = FALSE;
    vga_print("Fichier supprime.\n");
}

static void dos_print_help(void) {
    vga_print(
        "Commandes disponibles :\n"
        "  DIR            liste les fichiers\n"
        "  TYPE <fichier> affiche le contenu d'un fichier\n"
        "  DEL <fichier>  supprime un fichier\n"
        "  ECHO <texte>   affiche du texte\n"
        "  CLS            efface l'ecran\n"
        "  VER            affiche la version\n"
        "  EXIT           retourne au shell windOS\n"
    );
}

void dos_run(void) {
    dos_seed_filesystem();

    vga_print("\nwindOS - interpreteur facon DOS (sous-ensemble)\n");
    vga_print("Tape HELP pour la liste des commandes, EXIT pour revenir.\n\n");

    char line[128];
    char* argv[8];

    while (1) {
        vga_print("C:\\>");

        extern void keyboard_read_line(char*, int);
        keyboard_read_line(line, sizeof(line));

        int argc = str_tokenize(line, argv, 8);
        if (argc == 0) continue;

        if (str_eq(argv[0], "EXIT") || str_eq(argv[0], "exit")) {
            vga_print("Retour a windOS.\n");
            return;
        } else if (str_eq(argv[0], "DIR") || str_eq(argv[0], "dir")) {
            dos_cmd_dir();
        } else if (str_eq(argv[0], "TYPE") || str_eq(argv[0], "type")) {
            dos_cmd_type(argc > 1 ? argv[1] : 0);
        } else if (str_eq(argv[0], "DEL") || str_eq(argv[0], "del")) {
            dos_cmd_del(argc > 1 ? argv[1] : 0);
        } else if (str_eq(argv[0], "ECHO") || str_eq(argv[0], "echo")) {
            for (int i = 1; i < argc; i++) {
                vga_print(argv[i]);
                if (i < argc - 1) vga_putchar(' ');
            }
            vga_putchar('\n');
        } else if (str_eq(argv[0], "CLS") || str_eq(argv[0], "cls")) {
            vga_clear();
        } else if (str_eq(argv[0], "VER") || str_eq(argv[0], "ver")) {
            vga_print("windOS [Interpreteur facon DOS 0.01]\n");
        } else if (str_eq(argv[0], "HELP") || str_eq(argv[0], "help")) {
            dos_print_help();
        } else {
            vga_print("Commande inconnue : ");
            vga_print(argv[0]);
            vga_print("\nTape HELP pour la liste des commandes.\n");
        }
    }
}
