#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

// Sara Ketlen Moreira Chaves
// Programado no VS Code
// Nivel Aventureiro
// Decidi criar um banco de cartas como um baralho, questão de organização e não precisar me preucupar com cartas e cadastros.

#define MAX_NAME 100
#define MAX_STATE 50
#define LINE_BUF 512
#define MAX_CARDS 200

typedef struct
{
    char name[MAX_NAME];
    char state[MAX_STATE]; // pais/região, achei que seria mais legal implementar um baralho de cidades pelo mundo sem compromenter a lógica pedida.
    int code;
    unsigned long population;
    double area;
    double pib;
    int touristic_points;

    double density;
    double pib_per_capita;
    float super_power;
} Card;

// Só uma função pra remover espaços, facilitando e abstraindo a leitura do arquivo do deck de cartas
static void trim(char *s)
{
    char *p = s;
    while (isspace((unsigned char)*p))
    {
        p++;
    };
    if (p != s)
    {
        memmove(s, p, strlen(p) + 1);
    };
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1]))
    {
        s[--n] = '\0';
    };
}

void clear_buffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
    {
    }
}

// read card substiuda nessa versão por:
int read_deck(const char *filename, Card deck[], int max_cards)
{
    FILE *f = fopen(filename, "r");
    if (!f)
        return -1; // verificando se o arquivo existe

    char line[LINE_BUF];
    int count = 0;
    while (fgets(line, sizeof(line), f) && count < max_cards)
    {
        trim(line);
        if (line[0] == '\0' || line[0] == '#')
            continue; // pulando linhas vazias ou com comentarios

        char *tokens[7];
        char *p = line;
        int i = 0;
        for (i = 0; i < 7; i++)
        {
            if (p == NULL)
            {
                tokens[i] = NULL;
                continue;
            }
            char *comma = strchr(p, ',');
            if (comma)
            {
                *comma = '\0';
                tokens[i] = p;
                p = comma + 1;
            }
            else
            {
                tokens[i] = p;
                p = NULL;
            }
            trim(tokens[i]);
        }
        int missing = 0;
        for (i = 0; i < 7; ++i)
        {
            if (!tokens[i])
            {
                missing = 1;
                break;
            }
        }
        if (missing)
            continue;

        Card c;
        strncpy(c.name, tokens[0], MAX_NAME - 1);
        c.name[MAX_NAME - 1] = '\0';

        strncpy(c.state, tokens[1], MAX_STATE - 1);
        c.state[MAX_STATE - 1] = '\0';

        c.code = atoi(tokens[2]);
        c.population = (unsigned long)strtoul(tokens[3], NULL, 10);
        c.area = strtod(tokens[4], NULL);
        c.pib = strtod(tokens[5], NULL);
        c.touristic_points = atoi(tokens[6]);

        if (c.area <= 0.0)
            c.density = INFINITY;
        else
            c.density = (double)c.population / c.area;

        if (c.population == 0UL)
            c.pib_per_capita = 0.0;
        else
            c.pib_per_capita = c.pib / (double)c.population;

        double inv_density = 0.0;
        if (isfinite(c.density) && c.density != 0.0)
            inv_density = 1.0 / c.density;

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

void show_card_brief(const Card *c, int idx)
{
    printf("[%2d] %-25s (%s)  Pop: %lu  Area: %.2f km^2  PIB: %.2f\n",
           idx, c->name, c->state, c->population, c->area, c->pib);
}

void show_card_full(const Card *c)
{
    printf("--- %s (%s) ---\n", c->name, c->state);
    printf("Codigo: %d\n", c->code);
    printf("Populacao: %lu\n", c->population);
    printf("Area: %.2f km^2\n", c->area);
    printf("PIB: %.2f\n", c->pib);
    printf("Pontos Turisticos: %d\n", c->touristic_points);
    printf("Densidade: %.4f hab/km^2\n", c->density);
    printf("PIB per capita: %.6f\n", c->pib_per_capita);
    printf("Super Poder: %.6f\n", c->super_power);
    printf("--------------------------\n");
}

int choose_index(const char *prompt, int max_index)
{
    int idx;
    while (1)
    {
        printf("%s (0 - %d): ", prompt, max_index - 1);
        if (scanf("%d", &idx) != 1)
        {
            printf("Entrada invalida. Tente novamente.\n");
            clear_buffer();
            continue;
        }
        clear_buffer();
        if (idx < 0 || idx >= max_index)
        {
            printf("Indice fora do intervalo. Tente novamente.\n");
            continue;
        }
        return idx;
    }
}

// switch

enum Attr
{
    A_POP = 1,
    A_AREA,
    A_PIB,
    A_POINTS,
    A_DENSITY,
    A_PIBPC,
    A_SUPER,
    A_SHOW_BOTH,
    A_EXIT
};

void print_attribute_menu(void)
{
    printf("\nEscolha o atributo para comparar:\n");
    printf("1 - Populacao\n");
    printf("2 - Area\n");
    printf("3 - PIB\n");
    printf("4 - Numero de Pontos Turisticos\n");
    printf("5 - Densidade Demografica (menor vence)\n");
    printf("6 - PIB per Capita\n");
    printf("7 - Super Poder\n");
    printf("8 - Mostrar dados completos das duas cartas\n");
    printf("9 - Voltar ao menu principal / Sair\n");
    printf("Selecao: ");
}

void compare_and_show(const Card *a, const Card *b, int attr)
{
    printf("\nComparacao: '%s'  VS  '%s'\n", a->name, b->name);

    switch (attr)
    {
    case A_POP:
    {
        unsigned long va = a->population, vb = b->population;
        printf("Atributo: Populacao\n");
        printf("%-20s : %lu\n", a->name, va);
        printf("%-20s : %lu\n", b->name, vb);
        if (va > vb)
            printf("Resultado: %s venceu!\n", a->name);
        else if (vb > va)
            printf("Resultado: %s venceu!\n", b->name);
        else
            printf("Empate!\n");
        break;
    }
    case A_AREA:
    {
        double va = a->area, vb = b->area;
        printf("Atributo: Area (km^2)\n");
        printf("%-20s : %.2f\n", a->name, va);
        printf("%-20s : %.2f\n", b->name, vb);
        if (va > vb)
            printf("Resultado: %s venceu!\n", a->name);
        else if (vb > va)
            printf("Resultado: %s venceu!\n", b->name);
        else
            printf("Empate!\n");
        break;
    }
    case A_PIB:
    {
        double va = a->pib, vb = b->pib;
        printf("Atributo: PIB\n");
        printf("%-20s : %.2f\n", a->name, va);
        printf("%-20s : %.2f\n", b->name, vb);
        if (va > vb)
            printf("Resultado: %s venceu!\n", a->name);
        else if (vb > va)
            printf("Resultado: %s venceu!\n", b->name);
        else
            printf("Empate!\n");
        break;
    }
    case A_POINTS:
    {
        int va = a->touristic_points, vb = b->touristic_points;
        printf("Atributo: Pontos Turisticos\n");
        printf("%-20s : %d\n", a->name, va);
        printf("%-20s : %d\n", b->name, vb);
        if (va > vb)
            printf("Resultado: %s venceu!\n", a->name);
        else if (vb > va)
            printf("Resultado: %s venceu!\n", b->name);
        else
            printf("Empate!\n");
        break;
    }
    case A_DENSITY:
    {
        double va = a->density, vb = b->density;
        printf("Atributo: Densidade Demografica (hab/km^2) - menor vence\n");
        printf("%-20s : %.4f\n", a->name, va);
        printf("%-20s : %.4f\n", b->name, vb);
        if (va < vb)
            printf("Resultado: %s venceu!\n", a->name);
        else if (vb < va)
            printf("Resultado: %s venceu!\n", b->name);
        else
            printf("Empate!\n");
        break;
    }
    case A_PIBPC:
    {
        double va = a->pib_per_capita, vb = b->pib_per_capita;
        printf("Atributo: PIB per Capita\n");
        printf("%-20s : %.6f\n", a->name, va);
        printf("%-20s : %.6f\n", b->name, vb);
        if (va > vb)
            printf("Resultado: %s venceu!\n", a->name);
        else if (vb > va)
            printf("Resultado: %s venceu!\n", b->name);
        else
            printf("Empate!\n");
        break;
    }
    case A_SUPER:
    {
        float va = a->super_power, vb = b->super_power;
        printf("Atributo: Super Poder\n");
        printf("%-20s : %.6f\n", a->name, va);
        printf("%-20s : %.6f\n", b->name, vb);
        if (va > vb)
            printf("Resultado: %s venceu!\n", a->name);
        else if (vb > va)
            printf("Resultado: %s venceu!\n", b->name);
        else
            printf("Empate!\n");
        break;
    }
    default:
        printf("Atributo desconhecido.\n");
    }
}

int main(void)
{
    srand((unsigned)time(NULL));
    Card deck[MAX_CARDS];
    int deck_size = 0;

    const char *filename = "deck.csv";
    int loaded = read_deck(filename, deck, MAX_CARDS);
    if (loaded < 0)
    {
        printf("Erro: nao foi possivel abrir '%s'.\n", filename);
        printf("Coloque um arquivo CSV com as cartas no mesmo diretorio do executavel.\n");
        printf("Formato: name,state,code,population,area,pib,touristic_points\n");
        return 1;
    }
    deck_size = loaded;
    printf("Deck carregado: %d cartas de '%s'.\n", deck_size, filename);

    int exit_program = 0;
    while (!exit_program)
    {
        printf("\n======= SUPER TRUNFO - MENU PRINCIPAL =======\n");
        printf("1 - Listar cartas (resumo)\n");
        printf("2 - Escolher duas cartas por indice\n");
        printf("3 - Escolher duas cartas aleatoriamente\n");
        printf("4 - Sair\n");
        printf("Escolha: ");

        int choice;
        if (scanf("%d", &choice) != 1)
        {
            printf("Entrada invalida.\n");
            clear_buffer();
            continue;
        }
        clear_buffer();

        if (choice == 1)
        {
            printf("\n--- Lista de cartas ---\n");
            for (int i = 0; i < deck_size; ++i)
                show_card_brief(&deck[i], i);
            continue;
        }
        else if (choice == 2 || choice == 3)
        {
            int i1, i2;
            if (choice == 2)
            {
                printf("\nEscolha a primeira carta:\n");
                for (int i = 0; i < deck_size; ++i)
                    show_card_brief(&deck[i], i);
                i1 = choose_index("Indice da primeira carta", deck_size);
                printf("\nEscolha a segunda carta:\n");
                i2 = choose_index("Indice da segunda carta", deck_size);
                if (i1 == i2)
                {
                    printf("Voce escolheu a mesma carta duas vezes. Deseja continuar com a mesma carta? (N para cancelar) ");
                    char resp = getchar();
                    clear_buffer();
                    if (resp == 'N' || resp == 'n')
                    {
                        printf("Operacao cancelada.\n");
                        continue;
                    }
                }
            }
            else
            { // aleatorio
                if (deck_size < 2)
                {
                    printf("Deck precisa ter ao menos 2 cartas.\n");
                    continue;
                }
                i1 = rand() % deck_size;
                do
                {
                    i2 = rand() % deck_size;
                } while (i2 == i1);
                printf("\nCartas aleatorias escolhidas: %d (%s) e %d (%s)\n", i1, deck[i1].name, i2, deck[i2].name);
            }

            // menu de atributos e comparar repetidamente até voltar
            int back_to_main = 0;
            while (!back_to_main)
            {
                print_attribute_menu();
                int attr_choice;
                if (scanf("%d", &attr_choice) != 1)
                {
                    printf("Entrada invalida.\n");
                    clear_buffer();
                    continue;
                }
                clear_buffer();
                switch (attr_choice)
                {
                case A_POP:
                case A_AREA:
                case A_PIB:
                case A_POINTS:
                case A_DENSITY:
                case A_PIBPC:
                case A_SUPER:
                    compare_and_show(&deck[i1], &deck[i2], attr_choice);
                    break;
                case A_SHOW_BOTH:
                    show_card_full(&deck[i1]);
                    show_card_full(&deck[i2]);
                    break;
                case A_EXIT:
                    back_to_main = 1;
                    break;
                default:
                    printf("Opcao invalida. Tente novamente.\n");
                }
            }
        }
        else if (choice == 4)
        {
            exit_program = 1;
        }
        else
        {
            printf("Opcao invalida. Tente novamente.\n");
        }
    }

    printf("Obrigado por jogar! Ate a proxima.\n");
    return 0;
}