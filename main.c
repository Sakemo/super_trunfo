#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Sara Ketlen Moreira Chaves
// Programado no VS Code
// Nomeei as variaveis em inglês por costume, com exceção de PIB por razões obvias.
// Dividi as cosias em funções por boa prática e melhor estrutura do código.

#define MAX_NAME 100
#define MAX_STATE 50

typedef struct
{
    char name[MAX_NAME];
    char state[MAX_STATE];
    int code;
    unsigned long population;
    double area;
    double pib;
    int touristic_points;

    double density;
    double pib_per_capita;
    float super_power;
} Card;

void clear_buffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
    {
    }
}

void read_card(Card *c, int idx)
{
    const char *inv = "Entrada inválida. "; // CORREÇÃO: string constante

    printf("\n--- Insira os dados da carta %d ---\n", idx);

    printf("Nome: ");
    if (fgets(c->name, MAX_NAME, stdin) == NULL)
    {
        c->name[0] = '\0';
    }
    else
    {
        size_t ln = strlen(c->name);
        if (ln > 0 && c->name[ln - 1] == '\n')
            c->name[ln - 1] = '\0';
    }

    printf("Estado: ");
    if (fgets(c->state, MAX_STATE, stdin) == NULL)
    {
        c->state[0] = '\0';
    }
    else
    {
        size_t ln = strlen(c->state);
        if (ln > 0 && c->state[ln - 1] == '\n')
            c->state[ln - 1] = '\0';
    }

    printf("Código (inteiro): ");
    while (scanf("%d", &c->code) != 1)
    {
        printf("%sDigite um inteiro para código: ", inv);
        clear_buffer();
    }
    clear_buffer();

    printf("Populacao: ");
    while (scanf("%lu", &c->population) != 1)
    {
        printf("%sDigite um número inteiro não-negativo para população: ", inv);
        clear_buffer();
    }
    clear_buffer();

    printf("Area (km^2): ");
    while (scanf("%lf", &c->area) != 1)
    {
        printf("%sDigite um número válido para área: ", inv);
        clear_buffer();
    }
    clear_buffer();

    printf("PIB: ");
    while (scanf("%lf", &c->pib) != 1)
    {
        printf("%sDigite um número válido para PIB: ", inv);
        clear_buffer();
    }
    clear_buffer();

    printf("Pontos Turisticos: ");
    while (scanf("%d", &c->touristic_points) != 1)
    {
        printf("%sDigite um número inteiro para pontos turisticos: ", inv);
        clear_buffer();
    }
    clear_buffer();
}

void calculate_values(Card *c)
{
    if (c->area <= 0.0)
    {
        c->density = INFINITY;
    }
    else
    {
        c->density = (double)c->population / c->area;
    }

    if (c->population == 0UL)
    {
        c->pib_per_capita = 0.0;
    }
    else
    {
        c->pib_per_capita = c->pib / (double)c->population;
    }

    double inv_density = 0.0;
    if (isfinite(c->density) && c->density != 0.0)
        inv_density = 1.0 / c->density;

    double sum = 0.0;
    sum += (double)c->population;
    sum += c->area;
    sum += c->pib;
    sum += (double)c->touristic_points;
    sum += c->pib_per_capita;
    sum += inv_density;

    c->super_power = (float)sum;
}

void show_card(const Card *c, int idx)
{
    printf("\n--- Carta %d: %s ---\n", idx, c->name);
    printf("Estado: %s\n", c->state);
    printf("Codigo: %d\n", c->code);
    printf("Populacao: %lu\n", c->population);
    printf("Area: %.2f km^2\n", c->area);
    printf("PIB: %.2f\n", c->pib);
    printf("Pontos Turisticos: %d\n", c->touristic_points);
    printf("Densidade Populacional: %.4f hab/km^2\n", c->density);
    printf("PIB per Capita: %.6f\n", c->pib_per_capita);
    printf("Super Poder: %.6f\n", c->super_power);
}

int main(void)
{
    printf("***************************************\n");
    printf("*     SUPER TRUNFO - BATALHA DE CARTAS    *\n");
    printf("***************************************\n");

    Card c1, c2;

    read_card(&c1, 1);
    read_card(&c2, 2);

    calculate_values(&c1);
    calculate_values(&c2);

    show_card(&c1, 1);
    show_card(&c2, 2);

    int pop_wins = (c1.population > c2.population) ? 1 : 0;
    int area_wins = (c1.area > c2.area) ? 1 : 0;
    int pib_wins = (c1.pib > c2.pib) ? 1 : 0;
    int points_wins = (c1.touristic_points > c2.touristic_points) ? 1 : 0;
    int density_wins = (c1.density < c2.density) ? 1 : 0;
    int pibpc_wins = (c1.pib_per_capita > c2.pib_per_capita) ? 1 : 0;
    int super_wins = (c1.super_power > c2.super_power) ? 1 : 0;

    printf("\nComparacao de Cartas:\n\n");
    printf("Populacao: Carta %s venceu (%d)\n", pop_wins ? "1" : "2", pop_wins);
    printf("Area: Carta %s venceu (%d)\n", area_wins ? "1" : "2", area_wins);
    printf("PIB: Carta %s venceu (%d)\n", pib_wins ? "1" : "2", pib_wins);
    printf("Pontos Turisticos: Carta %s venceu (%d)\n", points_wins ? "1" : "2", points_wins);
    printf("Densidade Populacional: Carta %s venceu (%d)\n", density_wins ? "1" : "2", density_wins);
    printf("PIB per Capita: Carta %s venceu (%d)\n", pibpc_wins ? "1" : "2", pibpc_wins);
    printf("Super Poder: Carta %s venceu (%d)\n", super_wins ? "1" : "2", super_wins);

    printf("\nFim da batalha!\n");
    if (super_wins)
        printf("Carta 1 venceu!\n");
    else
        printf("Carta 2 venceu!\n");

    return 0;
}
