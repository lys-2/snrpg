#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#define count 123

char input[16], name[16], pw[16], msg[64];
int i;
enum class { rogue, mage, fighter };
enum type {node, user, character, box, wall, tree};
struct node { 
	char name[16]; char pw[16]; enum type t; enum class class;
	char is_spawned, is_owned; int at;
};
struct state { struct node scene[count]; int cur, session; char is_logged; };
struct state s, def;

int spawn(struct node n) { 
	n.is_spawned = 1;
	n.pw[0] = 0;
	s.scene[s.cur] = n;
	s.cur++; };

void reg(char name[16], char pw[16]) { 
	for (int i = 0; i < count; i++) {
		if (s.scene[i].t != user) continue;
		if (!strcmp(s.scene[i].name, name)) return;
	}
	struct node n;
	n.t = user;
	strcpy(n.name, name);
	strcpy(n.pw, pw);
	spawn(n);
};
void login(char name[16], char pw[16]) {
	for (int i = 0; i < count; i++) {
		if (s.scene[i].t != user) continue;
		if (strcmp(s.scene[i].name, name)) continue;
		if (strcmp(s.scene[i].pw, pw)) continue;
		s.session = i;
		s.is_logged = 1;
	}
};

void create(char name[16], enum class c) {
	struct node n;
	n.t = character;
	strcpy(n.name, name);
	
	spawn(n);
}

int main() {
	while (1) {
		printf("Hi! try (r)egister (l)ogin (c)reate (j)oin sa(y) selec(t) \n");
if (s.is_logged) printf("~~ logged as %s[%i]! \n", s.scene[s.session].name, s.session);
		for (int i = 0; i < count; i++) {
			if (!s.scene[i].is_spawned) continue;
			printf("%s %s %i\n", s.scene[i].name, s.scene[i].pw, s.scene[i].t);
		}
		printf("::");
		scanf("%s", &input);
		printf("~%s!\n", input);

		if (!strcmp(input, "r")) {
			printf("register new user \n");
			printf("Name: ");
			scanf("%s", &name);
			printf("Password: ");
			scanf("%s", &pw);
			reg(name, pw);
		}
		if (!strcmp(input, "l")) {
			printf("Logging in \n");
			printf("Name: ");
			scanf("%s", &name);
			printf("Password: ");
			scanf("%s", &pw);
			login(name, pw);
		}
		if (!strcmp(input, "c")) {
			printf("Create character \n");
			printf("Name: ");
			scanf("%s", &name);
			printf("Class 0:rogue 1:mage 2:fighter ");
			int r = scanf("%i", &i);
			if (r==1)
			create(name, i);
		}
		input[0] = 0;
		name[0] = 0;
		pw[0] = 0;
		i = 0;
	};
};