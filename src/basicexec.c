/* basicexec.c — interpreteur BASIC minimal, entiers uniquement.
 *
 * IMPORTANT : ce n'est PAS QBasic. Pas d'IDE, pas de flottants, pas
 * de tableaux, pas de sous-programmes. C'est un "tiny BASIC" dans
 * l'esprit du langage : numeros de ligne, PRINT, LET, IF/THEN, GOTO,
 * FOR/NEXT, INPUT — assez pour ecrire de petits programmes reels,
 * dans un espace de code minuscule (pas de flottant = pas besoin de FPU).
 */

#include "vga.h"
#include "strutil.h"

#define MAX_LINES 40
#define LINE_TEXT_LEN 96
#define FOR_STACK_DEPTH 8

typedef struct {
    int num;
    char text[LINE_TEXT_LEN];
    bool_t used;
} BasicLine;

static BasicLine program[MAX_LINES];
static int line_count = 0;
static int vars[26]; /* variables A..Z, entiers uniquement */

typedef struct {
    char var;
    int limit;
    int step;
    int body_index; /* index dans program[] juste apres le FOR */
} ForFrame;

static ForFrame for_stack[FOR_STACK_DEPTH];
static int for_sp = 0;

/* ---------- gestion du programme (stocke, trie, liste) ---------- */

static void basic_new(void) {
    line_count = 0;
    for (int i = 0; i < 26; i++) vars[i] = 0;
    for_sp = 0;
}

static void basic_store_line(int num, const char* text) {
    for (int i = 0; i < line_count; i++) {
        if (program[i].num == num) {
            if (str_len(text) == 0) {
                /* ligne vide = suppression : on tasse le tableau */
                for (int j = i; j < line_count - 1; j++) program[j] = program[j + 1];
                line_count--;
            } else {
                str_copy(program[i].text, text, LINE_TEXT_LEN);
            }
            return;
        }
    }
    if (str_len(text) == 0 || line_count >= MAX_LINES) return;

    int pos = line_count;
    for (int i = 0; i < line_count; i++) {
        if (program[i].num > num) { pos = i; break; }
    }
    for (int j = line_count; j > pos; j--) program[j] = program[j - 1];
    program[pos].num = num;
    str_copy(program[pos].text, text, LINE_TEXT_LEN);
    program[pos].used = TRUE;
    line_count++;
}

static void basic_list(void) {
    for (int i = 0; i < line_count; i++) {
        vga_print_int(program[i].num);
        vga_putchar(' ');
        vga_print(program[i].text);
        vga_putchar('\n');
    }
}

/* ---------- petites aides de parsing ---------- */

static void skip_spaces(char** p) {
    while (**p == ' ') (*p)++;
}

static bool_t is_digit(char c) { return c >= '0' && c <= '9'; }
static bool_t is_alpha(char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }

/* expr := term (('+'|'-') term)*  |  term := factor (('*'|'/') factor)* */

static int parse_expr(char** p);

static int parse_factor(char** p) {
    skip_spaces(p);
    if (**p == '-') { (*p)++; return -parse_factor(p); }
    if (**p == '(') {
        (*p)++;
        int v = parse_expr(p);
        skip_spaces(p);
        if (**p == ')') (*p)++;
        return v;
    }
    if (is_digit(**p)) {
        int v = 0;
        while (is_digit(**p)) { v = v * 10 + (**p - '0'); (*p)++; }
        return v;
    }
    if (is_alpha(**p)) {
        char name = **p;
        (*p)++;
        if (name >= 'a' && name <= 'z') name = name - 'a' + 'A';
        return vars[name - 'A'];
    }
    return 0;
}

static int parse_term(char** p) {
    int v = parse_factor(p);
    while (1) {
        skip_spaces(p);
        if (**p == '*') { (*p)++; v *= parse_factor(p); }
        else if (**p == '/') { (*p)++; int d = parse_factor(p); v = (d != 0) ? v / d : 0; }
        else break;
    }
    return v;
}

static int parse_expr(char** p) {
    int v = parse_term(p);
    while (1) {
        skip_spaces(p);
        if (**p == '+') { (*p)++; v += parse_term(p); }
        else if (**p == '-') { (*p)++; v -= parse_term(p); }
        else break;
    }
    return v;
}

/* condition := expr (=|<|>|<=|>=|<>) expr */
static bool_t parse_condition(char** p) {
    int left = parse_expr(p);
    skip_spaces(p);

    char op1 = **p; (*p)++;
    char op2 = **p;
    bool_t two_char = (op2 == '=' && (op1 == '<' || op1 == '>'));
    if (two_char) (*p)++;

    int right = parse_expr(p);

    if (op1 == '=') return left == right;
    if (op1 == '<' && two_char) return left <= right;
    if (op1 == '>' && two_char) return left >= right;
    if (op1 == '<') return left < right;
    if (op1 == '>') return left > right;
    return FALSE;
}

/* ---------- execution d'une instruction ---------- */

typedef struct {
    bool_t jump;
    int target_line;
} ExecResult;

static void exec_print(char* p) {
    skip_spaces(&p);
    while (*p != '\0') {
        skip_spaces(&p);
        if (*p == '"') {
            p++;
            while (*p != '"' && *p != '\0') { vga_putchar(*p); p++; }
            if (*p == '"') p++;
        } else if (*p != '\0' && *p != ';' && *p != ',') {
            int v = parse_expr(&p);
            vga_print_int(v);
        }
        skip_spaces(&p);
        if (*p == ';' || *p == ',') { p++; continue; }
        break;
    }
    vga_putchar('\n');
}

static void exec_let(char* p) {
    skip_spaces(&p);
    char name = *p;
    if (name >= 'a' && name <= 'z') name = name - 'a' + 'A';
    p++;
    skip_spaces(&p);
    if (*p == '=') p++;
    vars[name - 'A'] = parse_expr(&p);
}

extern void keyboard_read_line(char*, int);

static void exec_input(char* p) {
    skip_spaces(&p);
    char name = *p;
    if (name >= 'a' && name <= 'z') name = name - 'a' + 'A';

    vga_print("? ");
    char buf[32];
    keyboard_read_line(buf, sizeof(buf));
    int v = 0;
    str_to_int(buf, &v);
    vars[name - 'A'] = v;
}

/* Execute une instruction. Renvoie un saut eventuel (GOTO/IF/NEXT). */
static ExecResult exec_statement(char* stmt, int current_index) {
    ExecResult r = { FALSE, 0 };
    skip_spaces(&stmt);

    if (str_starts_with(stmt, "REM")) {
        return r;
    }
    if (str_starts_with(stmt, "PRINT")) {
        exec_print(stmt + 5);
        return r;
    }
    if (str_starts_with(stmt, "LET")) {
        exec_let(stmt + 3);
        return r;
    }
    if (str_starts_with(stmt, "INPUT")) {
        exec_input(stmt + 5);
        return r;
    }
    if (str_starts_with(stmt, "GOTO")) {
        char* p = stmt + 4;
        skip_spaces(&p);
        r.jump = TRUE;
        r.target_line = parse_expr(&p);
        return r;
    }
    if (str_starts_with(stmt, "IF")) {
        char* p = stmt + 2;
        bool_t cond = parse_condition(&p);
        skip_spaces(&p);
        if (str_starts_with(p, "THEN")) p += 4;
        skip_spaces(&p);
        if (cond) {
            r.jump = TRUE;
            r.target_line = parse_expr(&p);
        }
        return r;
    }
    if (str_starts_with(stmt, "FOR")) {
        char* p = stmt + 3;
        skip_spaces(&p);
        char name = *p;
        if (name >= 'a' && name <= 'z') name = name - 'a' + 'A';
        p++;
        skip_spaces(&p);
        if (*p == '=') p++;
        int start = parse_expr(&p);
        skip_spaces(&p);
        if (str_starts_with(p, "TO")) p += 2;
        int limit = parse_expr(&p);

        vars[name - 'A'] = start;
        if (for_sp < FOR_STACK_DEPTH) {
            for_stack[for_sp].var = name;
            for_stack[for_sp].limit = limit;
            for_stack[for_sp].step = 1;
            for_stack[for_sp].body_index = current_index + 1;
            for_sp++;
        }
        return r;
    }
    if (str_starts_with(stmt, "NEXT")) {
        if (for_sp > 0) {
            ForFrame* f = &for_stack[for_sp - 1];
            vars[f->var - 'A'] += f->step;
            if (vars[f->var - 'A'] <= f->limit) {
                r.jump = TRUE;
                r.target_line = program[f->body_index].num;
                return r;
            } else {
                for_sp--;
            }
        }
        return r;
    }
    if (str_starts_with(stmt, "END")) {
        r.jump = TRUE;
        r.target_line = -1; /* convention : fin de programme */
        return r;
    }

    vga_print("Erreur BASIC : instruction inconnue -> ");
    vga_print(stmt);
    vga_putchar('\n');
    return r;
}

static void basic_run(void) {
    if (line_count == 0) {
        vga_print("Aucun programme. Tape des lignes numerotees, puis RUN.\n");
        return;
    }
    for (int i = 0; i < 26; i++) vars[i] = 0;
    for_sp = 0;

    int index = 0;
    int guard = 0; /* garde-fou anti-boucle infinie en cas de bug utilisateur */

    while (index >= 0 && index < line_count) {
        if (++guard > 100000) {
            vga_print("\n(arret : trop d'iterations, boucle infinie probable)\n");
            return;
        }

        ExecResult r = exec_statement(program[index].text, index);
        if (r.jump) {
            if (r.target_line == -1) return; /* END */
            int found = -1;
            for (int i = 0; i < line_count; i++) {
                if (program[i].num == r.target_line) { found = i; break; }
            }
            if (found == -1) {
                vga_print("Erreur : ligne cible introuvable -> ");
                vga_print_int(r.target_line);
                vga_putchar('\n');
                return;
            }
            index = found;
        } else {
            index++;
        }
    }
}

static void basic_print_help(void) {
    vga_print(
        "Mini-BASIC (entiers uniquement) :\n"
        "  10 PRINT \"texte\"      LIST     affiche le programme\n"
        "  20 LET X = 1 + 2       RUN      execute le programme\n"
        "  30 IF X < 10 THEN 10   NEW      efface le programme\n"
        "  40 GOTO 10             EXIT     retourne au shell windOS\n"
        "  FOR I = 1 TO 5 / NEXT I    INPUT X    END\n"
        "Tape une ligne sans numero pour l'executer immediatement.\n"
    );
}

void basic_run_shell(void) {
    vga_print("\nwindOS - mini interpreteur BASIC\n");
    vga_print("Tape HELP pour les instructions, EXIT pour revenir.\n\n");

    char line[LINE_TEXT_LEN];

    while (1) {
        vga_print("] ");
        keyboard_read_line(line, sizeof(line));

        char* p = line;
        skip_spaces(&p);
        if (*p == '\0') continue;

        if (str_eq(p, "EXIT") || str_eq(p, "exit")) {
            vga_print("Retour a windOS.\n");
            return;
        }
        if (str_eq(p, "LIST")) { basic_list(); continue; }
        if (str_eq(p, "RUN"))  { basic_run(); continue; }
        if (str_eq(p, "NEW"))  { basic_new(); vga_print("Programme efface.\n"); continue; }
        if (str_eq(p, "HELP")) { basic_print_help(); continue; }

        if (is_digit(*p)) {
            int num = 0;
            while (is_digit(*p)) { num = num * 10 + (*p - '0'); p++; }
            skip_spaces(&p);
            basic_store_line(num, p);
        } else {
            exec_statement(p, -1);
        }
    }
}
