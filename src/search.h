typedef struct {
    int ply;
    int best_move;
    unsigned long long nodes;
} Search;

int search(int);
int negamax(int, int, int, Search*);