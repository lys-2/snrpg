#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>
#include <math.h>
static HDC device_context, frame_device_context;
HFONT font;
static BITMAPINFO frame_bitmap_info;
static HBITMAP frame_bitmap;
static PAINTSTRUCT ps;
static WNDCLASS window_class;
static HWND window_handle;
MSG msg;
HANDLE hConsole;
HWND consoleWindow;

#define count 12345
#define orange (struct color){255,111,0,255}
#define cyan (struct color){0,255,255,255}
#define green (struct color){0,255,0,0}
int quit;
struct color { char r, g, b, a; };
struct frame { int width; int height; unsigned char* pixels; } frame;
struct v2 { float x, y; };
enum type { node, grid, character, spawner, bullet, tbox, tring };
struct node { char name[8], type, is_spawned, is_attached, is_controlled; 
float x, y, wx, wy, ms, r; 
int at, select, hit, exp; struct color c; struct v2 v; 
};

struct state {
    struct node scene[count];
    int nodes, spawned;
    int frame;
    float vx, vy;
};
struct state s, def;

float lerp(float a, float b, float f) {
    float r = a * (1.0 - f) + (b * f);
    return r;
}
float len(struct v2 v) { return sqrt(v.x * v.x + v.y * v.y); };
float dist(struct v2 a, struct v2 b) { return len((struct v2) { a.x - b.x, a.y - b.y }); };
int v2_box(int x, int y, int cx, int cy, int w, int h) {
    if ((x > cx && x < cx + w) && (y > cy && y < cy + h)) return 1;
    else { return 0; }
}
int v2_circle(struct v2 v, struct v2 c, float r) {
    if (dist(v, c) < r) return 1;
    return 0;
}

void load() {
    FILE* fptr = fopen("save_nr", "rb");
    fread(&s, sizeof(s), 1, fptr);
    fclose(fptr);
}
void save() {
    FILE* fptr = fopen("save_nr", "wb");
    if (fptr) fwrite(&s, sizeof(s), 1, fptr);
    fclose(fptr);
}

void init() {
    printf("Hi!\n");
    int p = spawn((struct node) {
        .type = character, "p", .x = 33, .y = 33, .c = orange, .is_controlled=1 });
    spawn((struct node) {
.type = node, "a", .x = 8, .y = 2, .c = orange, .is_attached = 1, .at = p
    });
    spawn((struct node) {
        .type = node, "b", .x = -8, .y = 3, .c = orange, .is_attached = 1, .at = p
    });

    for (int i = 0; i < 24; i++) {
        spawn((struct node) {
            .type = tbox, "b", .x = rand()%789, .y = 111+rand() % 123, .c = orange
        });
    }
    for (int i = 0; i < 24; i++) {
        spawn((struct node) {
            .type = tring, "b", .x = rand() % 789, .y = 111+rand() % 123, .c = cyan
        });
    }
    for (int i = 0; i < 1123; i++) {
        spawn((struct node) {
            .type = bullet, "b", .x = 123+rand() % 567, .y = 33 + rand() % 56, .c = cyan,
                .v.y = .01+(rand()%32)/123., .v.x = -.1 + (rand() % 32) / 123.,
                .exp =1234-rand()%123
        });
    }
};
void reset() { s = def; init(); }
void delete(int id) {
    s.scene[id].is_spawned = 0; s.spawned--;
    // printf("- %i\n", id);
}
int slot() {
    if (!s.scene[s.nodes % count].is_spawned) return s.nodes % count;
    for (int i = 0; i < count; i++) {
        if (!s.scene[i].is_spawned) return i;
    }
    return -1;
}
int spawn(struct node n) {
    int sl = slot();
    if (sl == -1) return sl;
    n.is_spawned = 1;
    n.ms = .1;
    if (n.exp==0) n.exp = -1;
    s.scene[slot()] = n;
    s.spawned++;
    s.nodes++;
    //  printf("++ %i\n", sl);
    return sl;
}

void text(char* t, float x, float y, float h, float w) {
    SelectObject(frame_device_context, font);
    SetTextColor(frame_device_context, RGB(0, 255, 0));
    SetBkMode(frame_device_context, 2);
    SetBkColor(frame_device_context, RGB(0, 11, 11));
    RECT r = { x, frame.height - y - h, x + w, frame.height - y };
    DrawTextA(frame_device_context, t, -1, &r, 0);
}
void point(struct frame f, float x, float y, struct color c) {
    x = floor(x);
    y = floor(y);
    if (x >= 0 && x < f.width && y >= 0 && y < f.height) {
        f.pixels[0 + (int)(x + y * f.width) * 4] = c.b;
        f.pixels[1 + (int)(x + y * f.width) * 4] = c.g;
        f.pixels[2 + (int)(x + y * f.width) * 4] += c.r;
    }
}
void line(struct frame f, struct v2 a, struct v2 b) {
    float d = dist(a, b);
    for (int i = 0; i <= (int)d; i++) {
        point(f,
            lerp(a.x, b.x, i / d),
            lerp(a.y, b.y, i / d), orange);
    }
}
void box(struct frame f, struct v2 v) {
    line(frame, (struct v2) { v.x, v.y }, (struct v2) { v.x, v.y + 16 });
    line(frame, (struct v2) { v.x + 16, v.y }, (struct v2) { v.x + 16, v.y + 16 });
    line(frame, (struct v2) { v.x, v.y }, (struct v2) { v.x + 16, v.y });
    line(frame, (struct v2) { v.x, v.y + 16 }, (struct v2) { v.x + 16, v.y + 16 });
}
void ring(struct frame f, struct v2 o, struct color c, float r) {
    for (int i = 0; i < (int)r * 16; i++) {
        int x = o.x + (int)(r * cos(i));
        int y = o.y + (int)(r * sin(i));
        point(f, x, y, c);
    }
}
void clear(struct frame f) {
    for (int i = 0; i < f.width * f.height; i++) {
        f.pixels[0 + i * 4] = 0;
        f.pixels[1 + i * 4] = 0;
        f.pixels[2 + i * 4] = 0;
        f.pixels[3 + i * 4] = 0;
    }
}
void paint(struct frame f) {

    for (int i = 0; i < f.width * f.height; i++) {
        /*        int y = i / f.width;
                int x = i % f.width;
                f.pixels[1 + i * 4] = x % 33;
                f.pixels[i * 4] = (y / 5) % 77;*/
    };

    for (int i = 0; i < count; i++) {
        if (!s.scene[i].is_spawned) continue;
        if (s.scene[i].type == tbox) {
            box(frame, (struct v2) { s.scene[i].wx-8, s.scene[i].wy-8 });
        };
        if (s.scene[i].type == tring) {
            ring(frame, (struct v2) { s.scene[i].wx, s.scene[i].wy }, orange, 16);
        };
        point(f, s.scene[i].wx, s.scene[i].wy, s.scene[i].c);
        if (s.scene[i].type != bullet) ring(frame, (struct v2) { s.scene[i].wx, s.scene[i].wy }, orange, 4);
        if (s.scene[i].is_controlled) text("~P", s.scene[i].wx, s.scene[i].wy, 16, 16);
        ;

    };
    int p = get_controlled();
    if (s.scene[p].select !=-1)
    ring(frame, 
        (struct v2) { s.scene[s.scene[p].select].wx, s.scene[s.scene[p].select].wy },
        cyan, 5);

    char str[64]; sprintf(str, "~~~ sel:%i \n f:%ik", s.scene[p].select, s.frame / 1000);
    text(str, 0, frame.height - 32, 32, 161);
}
int get_controlled() {
    for (int i = 0; i < count; i++) {
        if (!s.scene[i].is_spawned) continue;
        if (s.scene[i].is_controlled) return i;
    }
    return -1;
};

int query_v2(struct v2 v) {

    for (int i = 0; i < count; i++) {
        if (!s.scene[i].is_spawned) continue;
        switch (s.scene[i].type) {
        case tbox:
            if (v2_box(v.x, v.y, s.scene[i].wx, s.scene[i].wy, 8, 8))
            {
                printf("box  "); return i;
            }

        case tring:
            if (v2_circle(v, (struct v2) { s.scene[i].wx, s.scene[i].wy }, 16))
            {
                printf("ring  "); return i;
            }

        }

    }
    return -1;
}

void process(float dt) {
    int p = get_controlled();
    s.scene[p].x += s.vx * s.scene[p].ms;
    s.scene[p].y += s.vy * s.scene[p].ms;

    for (int i = 0; i < count; i++) {
        if (!s.scene[i].is_spawned) continue;
        if (s.scene[i].exp!=-1 && s.scene[i].exp < s.frame) delete(i);
        s.scene[i].wx = 0;
        s.scene[i].wy = 0;
        s.scene[i].wx += s.scene[i].x;
        s.scene[i].wy += s.scene[i].y;
        if (s.scene[i].is_attached) {
            s.scene[i].wx += s.scene[s.scene[i].at].x;
            s.scene[i].wy += s.scene[s.scene[i].at].y;
        }
    }
    for (int i = 0; i < count; i++) {
        if (!s.scene[i].is_spawned) continue;
        s.scene[i].x += s.scene[i].v.x;
        s.scene[i].y += s.scene[i].v.y;
        if (s.scene[i].type == bullet) {
            if (query_v2((struct v2) { s.scene[i].wx, s.scene[i].wy })>-1) 
            { printf("hit! "); delete(i); }
        };

    }
    s.frame++;
}

LRESULT CALLBACK wpm(HWND window_handle,
    UINT message, WPARAM wParam, LPARAM lParam) {

    int x = LOWORD(lParam);
    int y = frame.height - HIWORD(lParam);

    if (message == WM_KEYDOWN && wParam == 'R') reset();
    if (message == WM_KEYDOWN && wParam == 'W') s.vy = 1;
    if (message == WM_KEYUP && wParam == 'W') s.vy = 0;
    if (message == WM_KEYDOWN && wParam == 'D') s.vx = 1;
    if (message == WM_KEYUP && wParam == 'D') s.vx = 0;
    if (message == WM_KEYDOWN && wParam == 'A') s.vx = -1;
    if (message == WM_KEYUP && wParam == 'A') s.vx = 0;
    if (message == WM_KEYDOWN && wParam == 'S') s.vy = -1;
    if (message == WM_KEYUP && wParam == 'S') s.vy = 0;

    if (message == WM_KEYDOWN && wParam == VK_ESCAPE) { quit = 1; }
    if (message == WM_LBUTTONDOWN) {
    }
    if (message == WM_LBUTTONUP) {
    }
    switch (message) {
    case WM_QUIT: {} break;
    case WM_DESTROY: {
        quit = 1;
    } break;
    case WM_MOUSEMOVE: {
        s.scene[get_controlled()].select = query_v2((struct v2) { x, y });
    } break;

    case WM_PAINT: {

        device_context = BeginPaint(window_handle, &ps);

        clear(frame);
        paint(frame);

        BitBlt(device_context,
            ps.rcPaint.left,
            ps.rcPaint.top,
            ps.rcPaint.right - ps.rcPaint.left,
            ps.rcPaint.bottom - ps.rcPaint.top,
            frame_device_context,
            ps.rcPaint.left, ps.rcPaint.top,
            SRCCOPY);

        EndPaint(window_handle, &ps);

    } break;

    case WM_SIZE: {

        frame_bitmap_info.bmiHeader.biWidth = LOWORD(lParam);
        frame_bitmap_info.bmiHeader.biHeight = HIWORD(lParam);

        if (frame_bitmap) DeleteObject(frame_bitmap);
        frame_bitmap = CreateDIBSection(NULL, &frame_bitmap_info,
            DIB_RGB_COLORS, (void**)&frame.pixels, 0, 0);
        SelectObject(frame_device_context, frame_bitmap);

        frame.width = LOWORD(lParam);
        frame.height = HIWORD(lParam);

    } break;

    default: {
        return DefWindowProc(window_handle, message, wParam, lParam);
    }
    }
    return 0;
}

void console() {
    FILE* conin = stdin;
    FILE* conout = stdout;
    FILE* conerr = stderr;
    AllocConsole();
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    freopen_s(&conin, "CONIN$", "r", stdin);
    freopen_s(&conout, "CONOUT$", "w", stdout);
    freopen_s(&conerr, "CONOUT$", "w", stderr);
    //SetConsoleTitleA("console ");
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PSTR pCmdLine, int nCmdShow) {

    window_class.lpfnWndProc = wpm;
    window_class.hInstance = hInstance;
    window_class.lpszClassName = "My Window Class";
    window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&window_class);

    frame_bitmap_info.bmiHeader.biSize = sizeof(frame_bitmap_info.bmiHeader);
    frame_bitmap_info.bmiHeader.biPlanes = 1;
    frame_bitmap_info.bmiHeader.biBitCount = 32;
    frame_bitmap_info.bmiHeader.biCompression = BI_RGB;
    frame_device_context = CreateCompatibleDC(0);
    window_handle = CreateWindow("My Window Class", L"snry template", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        24, 24, 789, 345, NULL, NULL, hInstance, NULL);
    if (window_handle == NULL) { return -1; }

    font = CreateFont(16, 0, 0, 0, FW_NORMAL, 0, 0, 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH, L"Comic Sans MS");

    init();

    while (!quit) {
        process(.01);
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            DispatchMessage(&msg);
        }
        if (s.frame == 2) {
            console();
            consoleWindow = GetConsoleWindow();
            SetWindowPos(consoleWindow, 0, 33, 432, 512, 256, 0);
            SetForegroundWindow(window_handle);
        }

        InvalidateRect(window_handle, NULL, FALSE);
        UpdateWindow(window_handle);
    }
    return 0;
}