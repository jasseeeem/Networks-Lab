#include <stdio.h>
#define INFINITY 10000

void lsr(int n, int table[n][n], int source)
{

    int cost[n], prev[n], visited[n], count = n, min_cost, next_node;
    for (int i = 0; i < n; i++)
    {
        cost[i] = table[source][i];
        prev[i] = source;
        visited[i] = 0;
    }
    cost[source] = 0;
    visited[source] = 1;
    while (--count)
    {
        min_cost = INFINITY;
        for (int i = 0; i < n; i++)
        {
            if (cost[i] < min_cost && !visited[i])
            {
                min_cost = cost[i];
                next_node = i;
            }
        }
        visited[next_node] = 1;
        for (int i = 0; i < n; i++)
        {
            if (!visited[i] && min_cost + table[next_node][i] < cost[i])
            {
                cost[i] = min_cost + table[next_node][i];
                prev[i] = next_node;
            }
        }
    }
    for (int i = 0; i < n; i++)
    {
        int j = i, path[n];
        count = 0;
        do
        {
            j = prev[j];
            path[count++] = j;
        } while (j != source);
        printf("%d", source + 1);
        for (int k = count - 2; k >= 0; k--)
            printf("->%d", path[k] + 1);
        printf("->%d\t%d\n", i + 1, cost[i]);
    }
}

int main()
{
    int n, m, source, dest, cost;
    scanf("%d %d", &n, &m);
    int table[n][n];
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            if (i == j)
                table[i][j] = 0;

            else
                table[i][j] = INFINITY;
        }
    }
    for (int i = 0; i < m; i++)
    {
        scanf("%d %d %d", &source, &dest, &cost);
        table[source - 1][dest - 1] = cost;
        table[dest - 1][source - 1] = cost;
    }
    printf("\nFor LSR,\n");
    for (int i = 0; i < n; i++)
    {
        printf("\nAt node %d\n\n", i + 1);
        lsr(n, table, i);
    }
    return 0;
}
