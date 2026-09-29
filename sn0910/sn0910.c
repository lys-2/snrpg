#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#define count 1234
#define route 12
#define turns 127

struct set { int i[count]; int len; };
char input[16], name[16], pw[16], msg[128], path[128]; struct set set, set_def;
int i;
enum class2 { rogue, mage, fighter };
enum type { node, user, character, box, wall, tree, location, container, item, talk };
struct node {
	char name[256]; char pw[16]; enum type t; enum class class;
	char is_spawned, is_owned, is_root, depth, progress, turn;
	int at, owner, link;
};
struct state {
	struct node scene[count];
	int cur, session, controller, selection;
	char is_logged, is_joined, is_selected, bin[12345];
};
struct state s, def;

int spawn(struct node n) {
	n.is_spawned = 1;
	s.scene[s.cur] = n;
	s.cur++;
	return s.cur - 1;
};
int get_parent(int id, int d) {
	if (!d) return s.scene[id].at;
	else return get_parent(s.scene[id].at, d - 1);
	//   printf("d%i", d);
}
int get_by_name(char name[64]) {
	for (int i = 0; i < count; i++) {
		if (!strcmp(s.scene[i].name, name)) { return i; }
	}
	return -1;
}
char* get_path(int id) {
	path[0] = 0;
	strcat(path, s.scene[s.scene[id].at].name);
	strcat(path, "/");
	strcat(path, s.scene[id].name);

	return path;
}
struct set get_reach(int id) {
	set = set_def;
	set.i[set.len] = s.scene[id].at;
	set.len++;
	return set;
}
char is_reach(int id) {
	if (s.scene[id].is_root && strcmp(s.scene[id].name, "map")) return 0;
	if (
		s.scene[id].at == s.scene[s.controller].at ||
		get_parent(s.scene[s.controller].at, 0) == id ||
		s.scene[s.controller].at == id
		)
	{
		return 1;
	}
	return 0;
}

void on_reach(int id) { printf("R \n"); }

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

int create(char name[16], enum class2 c) {
	if (!s.is_logged) return;
	struct node n;
	n.t = character;
	n.is_owned = 1;
	n.owner = s.session;
	n.class = c;
	strcpy(n.name, name);
	int r = spawn(n);
	return r;
}
void join(int id) {
	if (s.scene[id].is_spawned && s.scene[id].owner == s.session
		&& s.scene[id].t == character
		) {
		s.controller = id;
		s.is_joined = 1;
		s.scene[id].at = s.scene[get_by_name("*start")].at;
	}
}
void say(char msg[128]) {
	if (!s.is_joined) return;
	printf("\n%s\n", msg);
}
void select(int id) {
	if (!s.is_joined) return;
	s.is_selected = 1;
	s.selection = id;
}
void move(int id) {
	if (!s.is_joined) return;
	if (s.scene[id].t != location) return;
	if (!is_reach(id)) return;
	s.scene[s.controller].at = id;
	for (int i = 0; i < count; i++) {
		if (is_reach(i)) on_reach(id);
	}
}
void scene() {
	FILE* fptr = fopen("scene.txt", "rb");
	if (fptr) {
		fread(&s.bin, sizeof(s.bin), 1, fptr);
		fclose(fptr);
	}
	printf("%s\n", s.bin);
	char w[256], d = 0, cur = 0, is_word = 0, depth = 0, ld = 0;
	int last = 0;
	strcpy(w, "node");
	struct node n = { 0 };
	struct node def = { 0 };
	for (int i = 0; i < 12345; i++) {
		n = def;
		if (!s.bin[i]) break;
		if (s.bin[i] == '\r') continue;
		if (s.bin[i] == '\n' && cur == 0) continue;
		if (s.bin[i] == ' ' && !is_word) { depth++, cur--; };
		if (s.bin[i] != ' ') { is_word = 1; };
		if (is_word) w[cur] = s.bin[i];
		cur++;
		if (s.bin[i] == '\n' && cur != 0) {
			w[cur - 1] = 0;
			n.is_root = depth == 0;
			n.depth = depth;
			if (depth > ld) n.at = last;
			if (depth <= ld) n.at = get_parent(last, ld - depth);
			strcpy(n.name, w);
			n.t = location;
			if (n.name[0] == '^') n.t = character;
			if (n.name[0] == '_') n.t = container;
			if (n.name[0] == '*') n.t = item;
			if (n.name[0] == '~') n.t = talk;
			last = spawn(n);
			ld = depth;
			cur = 0;
			depth = 0;
			is_word = 0;
		};
	};
}

int main() {

	scene();

	reg("USER", "asd");
	login("USER", "asd");
	int n = create("Player", rogue);
	join(n);

	while (1) {
		printf("Hi! actions: (r)egister (l)ogin (c)reate (j)oin sa(y) selec(t) (m)ove \n");
		if (s.is_logged) printf("~~ logged as %s[%i]! \n", s.scene[s.session].name, s.session);
		if (s.is_joined) printf("~~ playing as %s[%i]! progress:%i/%i turns:%i/%i\n",
			s.scene[s.controller].name, s.controller,
			s.scene[s.controller].progress, route,
			s.scene[s.controller].turn, turns
		);
		if (s.is_joined) printf("Location: %s[%i]\n",
			get_path(s.scene[s.controller].at), s.scene[s.controller].at);
		if (s.is_joined) {
			struct set set = get_reach(s.controller);
			printf("Reach: ");
			for (int i = 0; i < count; i++) {
				if (is_reach(i))	printf(" %s[%i] ", s.scene[i].name, i);
			}
			printf("\n");
		}
		if (s.is_selected) printf("Selection: %s[%i]\n", s.scene[s.selection].name, s.selection);
		if (!s.is_joined) {
			for (int i = 0; i < count; i++) {
				if (!s.scene[i].is_spawned) continue;
				if (s.scene[i].t == user)
					printf("%s %s USER\n", s.scene[i].name, s.scene[i].pw);
				if (s.scene[i].t == character)
					printf("%s[%i] CHAR\n", s.scene[i].name, i);
			}
		}
		for (int i = 0; i < 8; i++) {
			printf("LOG %i\n", 8-i-1);
		}
		printf("::");
		scanf("%s", &input);
		// printf("~%s!\n", input);

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
			if (r == 1) create(name, i);
		}

		if (!strcmp(input, "j")) {
			printf("Join \n");
			printf(" character id: ");
			int r = scanf("%i", &i);
			if (r == 1) join(i);
		}
		if (!strcmp(input, "y")) {
			printf("Say: \n");
			int r = scanf("%s", &msg);
			say(msg);
		}
		if (!strcmp(input, "t")) {
			printf("Select: \n");
			int r = scanf("%i", &i);
			if (r == 1) select(i);
		}
		if (!strcmp(input, "m")) {
			printf("Move: \n");
			int r = scanf("%i", &i);
			if (r == 1) move(i);
		}
		input[0] = 0;
		name[0] = 0;
		pw[0] = 0;
		msg[0] = 0;
		i = 0;
		s.scene[s.controller].turn++;
		printf("~~~~~~~~~~~~\n\n");

	};
};