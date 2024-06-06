#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define MAX 9

int preferences[MAX][MAX];
bool locked[MAX][MAX];

typedef struct
{
    int winner;
    int loser;
} pair;

char candidates[MAX][100]; // Assuming maximum length of candidate name is 100
pair pairs[MAX * (MAX - 1) / 2];

int pair_count;
int candidate_count;

bool vote(int rank, char* name, int ranks[]);
void record_preferences(int ranks[]);
void add_pairs(void);
void sort_pairs(void);
void lock_pairs(void);
void print_winner(FILE *confidential_file, char password[], int confidential);
bool check_cycle(int win, int lose);

int main(void)
{
    int confidential;
    FILE *confidential_file;
    char password[100];

    printf("Set a password: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = 0; // Remove newline character

    printf("Do you want to make this voting result confidential? If yes, enter 1 else 0: ");
    while (true)
    {
        scanf("%d", &confidential);
        if (confidential == 0 || confidential == 1)
        {
            break;
        }
        else
        {
            printf("Invalid input. Please enter either 0 or 1.\n");
        }
    }

    if (confidential == 1)
    {
        confidential_file = fopen("confidential.txt", "w");

        if (confidential_file == NULL)
        {
            printf("Error opening the file.\n\n");
            return 1;
        }
    }

    printf("Enter the number of candidates: ");
    scanf("%d", &candidate_count);
    if (candidate_count < 2 || candidate_count > MAX)
    {
        printf("Invalid number of candidates. Please enter between 2 and %i.\n", MAX);
        return 1;
    }

    // Prompt the user to enter the names of candidates
    for (int i = 0; i < candidate_count; i++)
    {
        printf("Enter the name of candidate %d: ", i + 1);
        scanf("%s", candidates[i]);
    }

    // Clear graph of locked in pairs
    for (int i = 0; i < candidate_count; i++)
    {
        for (int j = 0; j < candidate_count; j++)
        {
            locked[i][j] = false;
        }
    }

    pair_count = 0;

    // Prompt the user to enter the number of voters
    int voter_count;
    printf("Enter the number of voters: ");
    scanf("%d", &voter_count);

    // Query for votes
    for (int i = 0; i < voter_count; i++)
    {
        // ranks[i] is voter's ith preference
        int ranks[candidate_count];

        // Query for each rank
        printf("Voter %d preferences:\n", i + 1);
        for (int j = 0; j < candidate_count; j++)
        {
            char name[100]; // candidate name
            printf("Rank %d: ", j + 1);
            scanf("%s", name);

            if (!vote(j, name, ranks))
            {
                printf("Invalid vote.\n");
                return 2;
            }
        }

        record_preferences(ranks);
        printf("\n");
    }

    add_pairs();
    sort_pairs();
    lock_pairs();
    print_winner(confidential_file, password, confidential);

    if (confidential == 1)
    {
        fclose(confidential_file);
    }

    return 0;
}

// Update ranks given a new vote
bool vote(int rank, char name[], int ranks[])
{
    for (int i = 0; i < candidate_count; i++)
    {
        if (strcasecmp(candidates[i], name) == 0)
        {
            ranks[rank] = i;
            return true;
        }
    }
    return false;
}

void record_preferences(int ranks[])
{
    for (int i = 0; i < candidate_count; i++)
    {
        for (int j = i + 1; j < candidate_count; j++)
        {
            preferences[ranks[i]][ranks[j]]++;
        }
    }
}

void add_pairs(void)
{
    pair_count = 0;
    for (int i = 0; i < candidate_count; i++)
    {
        for (int j = 0; j < candidate_count; j++)
        {
            if (preferences[i][j] > preferences[j][i])
            {
                pairs[pair_count].winner = i;
                pairs[pair_count].loser = j;
                pair_count++;
            }
        }
    }
}

// Sort pairs in decreasing order by strength of victory
void sort_pairs(void)
{
    for (int i = 0; i < pair_count - 1; i++)
    {
        for (int j = 0; j < pair_count - 1 - i; j++)
        {
            int strength_a = preferences[pairs[j].winner][pairs[j].loser] - preferences[pairs[j].loser][pairs[j].winner];
            int strength_b = preferences[pairs[j + 1].winner][pairs[j + 1].loser] - preferences[pairs[j + 1].loser][pairs[j + 1].winner];
            if (strength_a < strength_b)
            {
                pair temp = pairs[j];
                pairs[j] = pairs[j + 1];
                pairs[j + 1] = temp;
            }
        }
    }
}

void lock_pairs(void)
{
    for (int i = 0; i < pair_count; i++)
    {
        if (!check_cycle(pairs[i].loser, pairs[i].winner))
        {
            locked[pairs[i].winner][pairs[i].loser] = true;
        }
    }
}

bool check_cycle(int lose, int win)
{
    if (win == lose)
    {
        return true;
    }
    for (int i = 0; i < candidate_count; i++)
    {
        if (locked[lose][i])
        {
            if (check_cycle(i, win))
            {
                return true;
            }
        }
    }
    return false;
}

void print_winner(FILE *confidential_file, char password[], int confidential)
{
    for (int i = 0; i < candidate_count; i++)
    {
        bool is_source = true;
        for (int j = 0; j < candidate_count; j++)
        {
            if (locked[j][i])
            {
                is_source = false;
                break;
            }
        }
        if (is_source)
        {
            char pass_check[100];
            printf("What is the password? ");
            scanf("%s", pass_check);

            if (strcmp(password, pass_check) == 0)
            {
                if (confidential == 1)
                {
                    fprintf(confidential_file, "Winner: %s\n", candidates[i]);
                    printf("Open the file to see the winner.\n");
                }
                else
                {
                    printf("The winner is: %s\n", candidates[i]);
                }
            }
            else
            {
                if (confidential == 1)
                {
                    remove("confidential.txt");
                    printf("You typed the wrong password. Check the file.\n");
                }
                else
                {
                    printf("The password is not correct.\n");
                }
            }
            return;
        }
    }
}
