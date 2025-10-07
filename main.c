#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

#define MAX_NAME 100
#define MAX_STATE 50
#define LINE_BUF 512
#define MAX_CARDS 200

typedef struct {
    char name[MAX_NAME];
    char state[MAX_STATE];
    int code;
    unsigned long population;
    double area;
    double pib;
    int touristic_points;

    double density;
    double pib_per_capita;
    float super_power; // mantém pra compatibilidade
} Card;

/* ---------- utilitários ---------- */

static void trim(char *s) {
    char *p = s;
    while (isspace((unsigned char)*p)) p++;
    if (p != s) memmove(s, p, strlen(p)+1);
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n-1])) s[--n] = '\0';
}

void clear_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

/* compacta número grande em string com 1 decimal e sufixo (K/M/B/T).
   - buf deve ter pelo menos 32 bytes
   - para valores pequenos retorna inteiro sem sufixo (população) ou uma casa decimal para float */
void format_compact_ulong(char *buf, size_t bufsize, unsigned long v) {
    double dv = (double)v;
    const char *suf = "";
    double out = dv;
    if (v >= 1000000000000UL) { out = dv / 1e12; suf = "T"; }
    else if (v >= 1000000000UL)    { out = dv / 1e9;  suf = "B"; }
    else if (v >= 1000000UL)       { out = dv / 1e6;  suf = "M"; }
    else if (v >= 1000UL)          { out = dv / 1e3;  suf = "K"; }
    else {
        // números pequenos: sem decimal para inteiros
        snprintf(buf, bufsize, "%lu", v);
        return;
    }
    snprintf(buf, bufsize, "%.1f%s", out, suf);
}

void format_compact_double(char *buf, size_t bufsize, double v) {
    // usado para PIB, area, pib per capita, density, super power
    double av = fabs(v);
    const char *suf = "";
    double out = v;
    if (av >= 1e12) { out = v / 1e12; suf = "T"; }
    else if (av >= 1e9)  { out = v / 1e9;  suf = "B"; }
    else if (av >= 1e6)  { out = v / 1e6;  suf = "M"; }
    else if (av >= 1e3)  { out = v / 1e3;  suf = "K"; }
    if (suf[0] == '\0') {
        // valores pequenos: imprimir com 1 decimal
        snprintf(buf, bufsize, "%.1f", v);
    } else {
        snprintf(buf, bufsize, "%.1f%s", out, suf);
    }
}

/* ---------- leitura do deck (CSV) com cálculo dos campos derivados ---------- */

int read_deck(const char *filename, Card deck[], int max_cards) {
    FILE *f = fopen(filename, "r");
    if (!f) return -1;

    char line[LINE_BUF];
    int count = 0;
    while (fgets(line, sizeof(line), f) && count < max_cards) {
        trim(line);
        if (line[0] == '\0' || line[0] == '#') continue;

        char *tokens[7];
        char *p = line;
        int i;
        for (i = 0; i < 7; ++i) {
            if (p == NULL) { tokens[i] = NULL; continue; }
            char *comma = strchr(p, ',');
            if (comma) {
                *comma = '\0';
                tokens[i] = p;
                p = comma + 1;
            } else {
                tokens[i] = p;
                p = NULL;
            }
            trim(tokens[i]);
        }
        int missing = 0;
        for (i = 0; i < 7; ++i) if (!tokens[i]) { missing = 1; break; }
        if (missing) continue;

        Card c;
        strncpy(c.name, tokens[0], MAX_NAME-1); c.name[MAX_NAME-1] = '\0';
        strncpy(c.state, tokens[1], MAX_STATE-1); c.state[MAX_STATE-1] = '\0';
        c.code = atoi(tokens[2]);
        c.population = (unsigned long)strtoul(tokens[3], NULL, 10);
        c.area = strtod(tokens[4], NULL);
        c.pib = strtod(tokens[5], NULL);
        c.touristic_points = atoi(tokens[6]);

        /* calcular valores derivados */
        if (c.area <= 0.0) c.density = INFINITY;
        else c.density = (double)c.population / c.area;

        if (c.population == 0UL) c.pib_per_capita = 0.0;
        else c.pib_per_capita = c.pib / (double)c.population;

        double inv_density = 0.0;
        if (isfinite(c.density) && c.density != 0.0) inv_density = 1.0 / c.density;

        double sum = 0.0;
        sum += (double)c.population;
        sum += c.area;
        sum += c.pib;
        sum += (double)c.touristic_points;
        sum += c.pib_per_capita;
        sum += inv_density;
        c.super_power = (float)sum;

        deck[count++] = c;
    }
    fclose(f);
    return count;
}

/* ---------- exibir cartas ---------- */

void show_card_brief(const Card *c, int idx) {
    char bufpop[32], bufarea[32], bufpib[32];
    format_compact_ulong(bufpop, sizeof(bufpop), c->population);
    format_compact_double(bufarea, sizeof(bufarea), c->area);
    format_compact_double(bufpib, sizeof(bufpib), c->pib);
    printf("[%2d] %-25s (%-10s)  Pop: %-6s  Area: %-6s km^2  PIB: %s\n",
           idx, c->name, c->state, bufpop, bufarea, bufpib);
}

void show_card_full(const Card *c) {
    char bufpop[32], bufarea[32], bufpib[32], bufdensity[32], bufpibpc[32], bufsuper[32];
    format_compact_ulong(bufpop, sizeof(bufpop), c->population);
    format_compact_double(bufarea, sizeof(bufarea), c->area);
    format_compact_double(bufpib, sizeof(bufpib), c->pib);
    format_compact_double(bufdensity, sizeof(bufdensity), c->density);
    format_compact_double(bufpibpc, sizeof(bufpibpc), c->pib_per_capita);
    format_compact_double(bufsuper, sizeof(bufsuper), c->super_power);

    printf("\n--- %s (%s) ---\n", c->name, c->state);
    printf("Codigo: %d\n", c->code);
    printf("Populacao: %s\n", bufpop);
    printf("Area: %s km^2\n", bufarea);
    printf("PIB: %s\n", bufpib);
    printf("Pontos Turisticos: %d\n", c->touristic_points);
    printf("Densidade: %s hab/km^2\n", bufdensity);
    printf("PIB per capita: %s\n", bufpibpc);
    printf("Super Poder: %s\n", bufsuper);
    printf("--------------------------\n");
}

/* ---------- escolha de índices ---------- */

int choose_index(const char *prompt, int max_index) {
    int idx;
    while (1) {
        printf("%s (0 - %d): ", prompt, max_index-1);
        if (scanf("%d", &idx) != 1) {
            printf("Entrada invalida. Tente novamente.\n");
            clear_buffer();
            continue;
        }
        clear_buffer();
        if (idx < 0 || idx >= max_index) {
            printf("Indice fora do intervalo. Tente novamente.\n");
            continue;
        }
        return idx;
    }
}

/* ---------- atributos (enum) ---------- */

enum Attr { A_POP=1, A_AREA, A_PIB, A_POINTS, A_DENSITY, A_PIBPC, A_SUPER, A_SHOW_BOTH, A_EXIT };

/* função auxiliar que imprime o menu de atributos, omitindo o atributo "exclude" (-1 para nenhum) */
void print_attribute_menu_excluding(int exclude) {
    printf("\nEscolha o atributo para comparar:\n");
    if (exclude != A_POP) printf("1 - Populacao\n");
    if (exclude != A_AREA) printf("2 - Area\n");
    if (exclude != A_PIB) printf("3 - PIB\n");
    if (exclude != A_POINTS) printf("4 - Numero de Pontos Turisticos\n");
    if (exclude != A_DENSITY) printf("5 - Densidade Demografica (menor vence)\n");
    if (exclude != A_PIBPC) printf("6 - PIB per Capita\n");
    if (exclude != A_SUPER) printf("7 - Super Poder\n");
    printf("8 - Mostrar dados completos das duas cartas\n");
    printf("9 - Voltar ao menu principal / Cancelar\n");
    printf("Selecao: ");
}

/* retorna o valor "para comparar" (respeitando densidade regra) e o valor "para somar" (densidade: inverso) */
/* value_for_display is original numeric value (for printing) */
double get_attr_value_for_compare(const Card *c, int attr) {
    switch (attr) {
        case A_POP: return (double)c->population;
        case A_AREA: return c->area;
        case A_PIB: return c->pib;
        case A_POINTS: return (double)c->touristic_points;
        case A_DENSITY: return c->density; // smaller is better
        case A_PIBPC: return c->pib_per_capita;
        case A_SUPER: return (double)c->super_power;
        default: return 0.0;
    }
}

/* For summing: if density -> use inverse density (higher is better). For other attrs use raw numeric value scaled to double */
double get_attr_value_for_sum(const Card *c, int attr) {
    switch (attr) {
        case A_POP: return (double)c->population;
        case A_AREA: return c->area;
        case A_PIB: return c->pib;
        case A_POINTS: return (double)c->touristic_points;
        case A_DENSITY: {
            if (isfinite(c->density) && c->density != 0.0) return 1.0 / c->density;
            else return 0.0;
        }
        case A_PIBPC: return c->pib_per_capita;
        case A_SUPER: return (double)c->super_power;
        default: return 0.0;
    }
}

/* helper to print an attribute's label */
const char* attr_label(int attr) {
    switch (attr) {
        case A_POP: return "Populacao";
        case A_AREA: return "Area (km^2)";
        case A_PIB: return "PIB";
        case A_POINTS: return "Pontos Turisticos";
        case A_DENSITY: return "Densidade (hab/km^2) [menor vence]";
        case A_PIBPC: return "PIB per Capita";
        case A_SUPER: return "Super Poder";
        default: return "Desconhecido";
    }
}

/* print numeric value nicely depending on attr */
void print_attr_value_nice(const Card *c, int attr, char *buf, size_t bufsz) {
    switch (attr) {
        case A_POP: format_compact_ulong(buf, bufsz, c->population); break;
        case A_AREA: format_compact_double(buf, bufsz, c->area); break;
        case A_PIB: format_compact_double(buf, bufsz, c->pib); break;
        case A_POINTS: snprintf(buf, bufsz, "%d", c->touristic_points); break;
        case A_DENSITY: format_compact_double(buf, bufsz, c->density); break;
        case A_PIBPC: format_compact_double(buf, bufsz, c->pib_per_capita); break;
        case A_SUPER: format_compact_double(buf, bufsz, c->super_power); break;
        default: snprintf(buf, bufsz, "N/A"); break;
    }
}

/* ---------- comparação de dois atributos ---------- */

void compare_two_attributes(const Card *a, const Card *b, int attr1, int attr2) {
    char buf_a1[32], buf_b1[32], buf_a2[32], buf_b2[32];
    print_attr_value_nice(a, attr1, buf_a1, sizeof(buf_a1));
    print_attr_value_nice(b, attr1, buf_b1, sizeof(buf_b1));
    print_attr_value_nice(a, attr2, buf_a2, sizeof(buf_a2));
    print_attr_value_nice(b, attr2, buf_b2, sizeof(buf_b2));

    printf("\nComparacao entre '%s' e '%s'\n", a->name, b->name);
    printf("Atributo 1: %s\n", attr_label(attr1));
    printf("  % -20s : %s\n", a->name, buf_a1);
    printf("  % -20s : %s\n", b->name, buf_b1);

    /* Resultado do atributo 1 (lembrar que densidade: menor vence) */
    double va1_cmp = get_attr_value_for_compare(a, attr1);
    double vb1_cmp = get_attr_value_for_compare(b, attr1);
    if (attr1 == A_DENSITY) {
        if (va1_cmp < vb1_cmp) printf("Resultado (Atributo 1): %s venceu!\n", a->name);
        else if (vb1_cmp < va1_cmp) printf("Resultado (Atributo 1): %s venceu!\n", b->name);
        else printf("Resultado (Atributo 1): Empate!\n");
    } else {
        if (va1_cmp > vb1_cmp) printf("Resultado (Atributo 1): %s venceu!\n", a->name);
        else if (vb1_cmp > va1_cmp) printf("Resultado (Atributo 1): %s venceu!\n", b->name);
        else printf("Resultado (Atributo 1): Empate!\n");
    }

    printf("\nAtributo 2: %s\n", attr_label(attr2));
    printf("  % -20s : %s\n", a->name, buf_a2);
    printf("  % -20s : %s\n", b->name, buf_b2);

    double va2_cmp = get_attr_value_for_compare(a, attr2);
    double vb2_cmp = get_attr_value_for_compare(b, attr2);
    if (attr2 == A_DENSITY) {
        if (va2_cmp < vb2_cmp) printf("Resultado (Atributo 2): %s venceu!\n", a->name);
        else if (vb2_cmp < va2_cmp) printf("Resultado (Atributo 2): %s venceu!\n", b->name);
        else printf("Resultado (Atributo 2): Empate!\n");
    } else {
        if (va2_cmp > vb2_cmp) printf("Resultado (Atributo 2): %s venceu!\n", a->name);
        else if (vb2_cmp > va2_cmp) printf("Resultado (Atributo 2): %s venceu!\n", b->name);
        else printf("Resultado (Atributo 2): Empate!\n");
    }

    /* Soma dos atributos usando valores apropriados (densidade -> inverso) */
    double suma = get_attr_value_for_sum(a, attr1) + get_attr_value_for_sum(a, attr2);
    double sumb = get_attr_value_for_sum(b, attr1) + get_attr_value_for_sum(b, attr2);

    char buf_suma[32], buf_sumb[32];
    format_compact_double(buf_suma, sizeof(buf_suma), suma);
    format_compact_double(buf_sumb, sizeof(buf_sumb), sumb);

    printf("\nSoma dos atributos: \n");
    printf("  % -20s : %s\n", a->name, buf_suma);
    printf("  % -20s : %s\n", b->name, buf_sumb);

    if (fabs(suma - sumb) < 1e-9) {
        printf("\nResultado final: Empate!\n");
    } else if (suma > sumb) {
        printf("\nResultado final: %s venceu!\n", a->name);
    } else {
        printf("\nResultado final: %s venceu!\n", b->name);
    }
}

/* ---------- main ---------- */

int main(void) {
    srand((unsigned)time(NULL));

    Card deck[MAX_CARDS];
    const char *filename = "deck.csv";
    int deck_size = read_deck(filename, deck, MAX_CARDS);
    if (deck_size < 0) {
        printf("Erro: nao foi possivel abrir '%s'. Certifique-se de que o arquivo existe.\n", filename);
        return 1;
    }
    printf("Deck carregado: %d cartas de '%s'.\n", deck_size, filename);

    int exit_program = 0;
    while (!exit_program) {
        printf("\n======= SUPER TRUNFO - MENU PRINCIPAL =======\n");
        printf("1 - Listar cartas (resumo)\n");
        printf("2 - Escolher duas cartas por indice\n");
        printf("3 - Escolher duas cartas aleatoriamente\n");
        printf("4 - Sair\n");
        printf("Escolha: ");
        int choice;
        if (scanf("%d", &choice) != 1) { printf("Entrada invalida.\n"); clear_buffer(); continue; }
        clear_buffer();

        if (choice == 1) {
            printf("\n--- Lista de cartas ---\n");
            for (int i = 0; i < deck_size; ++i) show_card_brief(&deck[i], i);
            continue;
        } else if (choice == 2 || choice == 3) {
            int i1, i2;
            if (choice == 2) {
                printf("\nEscolha a primeira carta:\n");
                for (int i = 0; i < deck_size; ++i) show_card_brief(&deck[i], i);
                i1 = choose_index("Indice da primeira carta", deck_size);
                printf("\nEscolha a segunda carta:\n");
                i2 = choose_index("Indice da segunda carta", deck_size);
                if (i1 == i2) {
                    printf("Voce escolheu a mesma carta duas vezes. Deseja continuar com a mesma carta? (N para cancelar) ");
                    char resp = getchar(); clear_buffer();
                    if (resp == 'N' || resp == 'n') { printf("Operacao cancelada.\n"); continue; }
                }
            } else { // aleatorio
                if (deck_size < 2) { printf("Deck precisa ter ao menos 2 cartas.\n"); continue; }
                i1 = rand() % deck_size;
                do { i2 = rand() % deck_size; } while (i2 == i1);
                printf("\nCartas aleatorias escolhidas: %d (%s) e %d (%s)\n", i1, deck[i1].name, i2, deck[i2].name);
            }

            /* Menu dinâmico para escolher dois atributos distintos */
            int attr1 = -1, attr2 = -1;
            while (attr1 < 0) {
                print_attribute_menu_excluding(-1);
                int sel;
                if (scanf("%d", &sel) != 1) { printf("Entrada invalida.\n"); clear_buffer(); continue; }
                clear_buffer();
                if (sel >= A_POP && sel <= A_SUPER) { attr1 = sel; break; }
                else if (sel == A_SHOW_BOTH) { show_card_full(&deck[i1]); show_card_full(&deck[i2]); }
                else if (sel == A_EXIT) break;
                else printf("Opcao invalida. Tente novamente.\n");
            }
            if (attr1 < 0) continue;

            while (attr2 < 0) {
                print_attribute_menu_excluding(attr1);
                int sel;
                if (scanf("%d", &sel) != 1) { printf("Entrada invalida.\n"); clear_buffer(); continue; }
                clear_buffer();
                if (sel == A_SHOW_BOTH) { show_card_full(&deck[i1]); show_card_full(&deck[i2]); continue; }
                if (sel == A_EXIT) { break; }
                if (sel == attr1) { printf("Voce nao pode escolher o mesmo atributo duas vezes. Escolha outro.\n"); continue; }
                if (sel >= A_POP && sel <= A_SUPER) { attr2 = sel; break; }
                printf("Opcao invalida. Tente novamente.\n");
            }
            if (attr2 < 0) continue;

            /* Finalmente, comparar com os dois atributos escolhidos */
            compare_two_attributes(&deck[i1], &deck[i2], attr1, attr2);
        }
        else if (choice == 4) exit_program = 1;
        else { printf("Opcao invalida. Tente novamente.\n"); }
    }

    printf("Obrigado por jogar! Ate a proxima.\n");
    return 0;
}
