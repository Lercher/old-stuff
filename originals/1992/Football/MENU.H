#define NAMELEN	20
#define SCHEMES 10

struct data {
	int active;
	struct position pos;
	enum type typ;
	enum symbol symbol;
	int sleep;
	int fnr;
};

struct scheme {
	int active;
	char name[NAMELEN];
	struct data data[PLAYERS];
};

void menu(void);
struct scheme *getscheme(void);
void loadschemes(void);
