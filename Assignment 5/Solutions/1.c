#include <stdio.h>
#define INFINITY 10000

void dvr(int n, int table[n][n], int next_hop[n][n])
{
    int count = n;
    while (--count)
    {
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                if (table[i][j] < INFINITY)
                {
                    for (int k = 0; k < n; k++)
                    {
                        if (table[i][j] + table[j][k] < table[i][k])
                        {
                            table[i][k] = table[i][j] + table[j][k];
                            next_hop[i][k] = j;
                        }
                    }
                }
            }
        }
    }
}

int main()
{
    int n, m;
    scanf("%d %d", &n, &m);
    int table[n][n], next_hop[n][n], source, dest, cost;
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            if (i == j)
            {
                next_hop[i][j] = j;
                table[i][j] = 0;
            }
            else
            {
                next_hop[i][j] = -1;
                table[i][j] = INFINITY;
            }
        }
    }
    for (int i = 0; i < m; i++)
    {
        scanf("%d %d %d", &source, &dest, &cost);
        table[source - 1][dest - 1] = cost;
        table[dest - 1][source - 1] = cost;
        next_hop[source - 1][dest - 1] = dest - 1;
        next_hop[dest - 1][source - 1] = source - 1;
    }
    dvr(n, table, next_hop);
    printf("\nFor DVR,\n");
    for (int i = 0; i < n; i++)
    {
        printf("\nRouting table at node %d\n", i + 1);
        for (int j = 0; j < n; j++)
            printf("%d\t%d\t%d\n", j + 1, next_hop[i][j] + 1, table[i][j]);
    }
    return 0;
}