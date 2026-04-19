#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>

#define count 12345
#define orange (struct color){255,111,0,255}
#define cyan (struct color){0,255,255,255}
#define yellow (struct color){255,255,0,255}

struct v2 { float x, y; };
struct color { char r, g, b, a; };
struct data { int start, len; };
enum type { node, leaf, tile };
struct frame { int width; int height; unsigned char* pixels; } frame;
struct node {
    char name[32], type, is_spawned, is_attached; int x, y, at, q;
    struct color c; struct data d;
};
struct state {
    struct node scene[count];
    int nodes, frames, spawned;
    char quit, log;
    char bin[1234];
};

struct state s, def;

float lerp(float a, float b, float f) {
    float r = a * (1.0 - f) + (b * f);
    return r;
}
float len(struct v2 v) { return sqrt(v.x * v.x + v.y * v.y); };
float dist(struct v2 a, struct v2 b) { return len((struct v2) { a.x - b.x, a.y - b.y }); };

void delete(int id) {
    s.scene[id].is_spawned = 0; s.spawned--;
    printf("- %i\n", id);
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
    if (sl == -1) return -1;
    n.is_spawned = 1;
    n.q = 1;
    s.scene[slot()] = n;
    s.spawned++;
    s.nodes++;
    printf("++%s %i\n", n.name, sl);
    return sl;
}

void point(struct frame f, float x, float y, struct color c) {
    if (x >= 0 && x < f.width && y >= 0 && y < f.height) {
        f.pixels[0 + (int)(x + y * f.width) * 4] = c.b;
        f.pixels[1 + (int)(x + y * f.width) * 4] = c.g;
        f.pixels[2 + (int)(x + y * f.width) * 4] += c.r;
    }
}

void load() {
    FILE* fptr = fopen("save_nr", "rb");
    fread(&s, sizeof(s), 1, fptr);
    fclose(fptr);
}

int get_parent(int id, int d) {
       if (!d) return s.scene[id].at;
       else return get_parent(s.scene[id].at, d - 1);
    //   printf("d%i", d);
}

void init() {
    printf("HELLO!||\n\n\n");
    FILE* fptr = fopen("scene.txt", "rb");
    fread(&s.bin, sizeof(s.bin), 1, fptr);
    fclose(fptr);
    char w[32], d = 0, cur = 0, is_word = 0, depth = 0, ld = 0;
    int last = 0;
    strcpy(w, "node");
    struct node n = {0};
    struct node def = {0};
    for (int i = 0; i < 1234; i++) {
        n = def;
        if (!s.bin[i]) break;
        if (s.bin[i] == '\r') continue;
        if (s.bin[i] == '\n' && cur==0) continue;
        if (s.bin[i] == ' ' && !is_word) { depth++, cur--; };
        if (s.bin[i] != ' ') { is_word = 1; };
        if (is_word) w[cur] = s.bin[i];
        cur++;
        if (s.bin[i] == '\n' && cur!=0) {
            w[cur-1] = 0;
            n.type = node;
            n.is_attached = depth>0;
            printf("\nd%i ld%i\n", depth, ld);
            if (depth>ld) n.at = last;
            if (depth==ld) n.at = get_parent(last, 0);
            if (depth < ld) n.at = get_parent(last, ld-depth);
            strcpy(n.name, w);
            last = spawn(n);
            ld = depth;
            cur = 0;
            depth = 0;
            is_word = 0;
        };

    }
    for (int i = 0; i < count; i++) {
        if (!s.scene[i].is_spawned) continue;
        printf("%i:%s", i, s.scene[i].name);
        if (s.scene[i].is_attached)  printf("@%s\n", s.scene[s.scene[i].at].name); 
        else  printf("%s", " ROOT\n");
    }
};

void save() {
    FILE* fptr = fopen("save_nr", "wb");
    if (fptr) fwrite(&s, sizeof(s), 1, fptr);
    fclose(fptr);
}
void reset() { s = def; }

void process(float dt) {

    if (s.frames < count) {
        //   spawn((struct node) {.x = rand() % frame.width, .y = rand() % frame.height});
    }

    for (int i = 0; i < count; i++) {
        if (!s.scene[i].is_spawned) continue;
        if (s.scene[i].type == leaf && rand() % 12 == 8) {
            s.scene[i].y += 2 - rand() % 5;
            s.scene[i].x += 2 - rand() % 5;
        }
        if (s.scene[i].type == leaf && rand() % 115 == 0) delete(i);
    }
    s.frames++;
}

void clear(struct frame f) {
    for (int i = 0; i < f.width * f.height; i++) {
        f.pixels[0 + i * 4] = 0;
        f.pixels[1 + i * 4] = 0;
        f.pixels[2 + i * 4] = 0;
        f.pixels[3 + i * 4] = 0;
    }
}

static HDC device_context, frame_device_context;
HFONT font;
void text(char* t, float x, float y) {
    SelectObject(frame_device_context, font);
    SetTextColor(frame_device_context, RGB(255, (s.frames / 22) % 255, 255));
    SetBkMode(frame_device_context, 2);
    SetBkColor(frame_device_context, RGB(0, 111, 111));
    RECT r = { x, y, 234, 64 };
    DrawTextA(frame_device_context, t, -1, &r, 0);
}
void paint(struct frame f) {
    for (int i = 0; i < f.width * f.height; i++) {
        int y = i / f.width;
        int x = i % f.width;
        f.pixels[1 + i * 4] = x % 33;
        f.pixels[i * 4] = (y / 5) % 77;
    }
    for (int i = 0; i < count; i++) {
        if (s.scene[i].is_spawned) {
            point(f, s.scene[i].x, s.scene[i].y,
                s.scene[i].c
            );
        }
    }
    char str[64]; sprintf(str, "Hi! %i/:%i\n f:%ik", count, s.spawned, s.frames / 1000);
    text(str, 0, 0);
}

static BITMAPINFO frame_bitmap_info;
static HBITMAP frame_bitmap;
static PAINTSTRUCT ps;
static WNDCLASS window_class;
static HWND window_handle;
MSG msg;
LRESULT CALLBACK wpm(HWND window_handle,
    UINT message, WPARAM wParam, LPARAM lParam) {

    int x = LOWORD(lParam);
    int y = frame.height - HIWORD(lParam);

    if (message == WM_KEYDOWN && wParam == 'R') reset();
    if (message == WM_KEYDOWN && wParam == 'L') load();
    if (message == WM_KEYDOWN && wParam == 'J') save();
    if (message == WM_KEYDOWN && wParam == VK_ESCAPE) { s.quit = 1; }
    if (message == WM_RBUTTONDOWN) {
        for (int i = 0; i < count; i++) {
            if (!s.scene[i].is_spawned || s.scene[i].type != tile) continue;
            if (dist((struct v2) { x, y }, (struct v2) { s.scene[i].x, s.scene[i].y }) < 30.)
                delete(i);
        }
    }
    if (message == WM_LBUTTONDOWN) {
        printf("L click! %i %i\n", x, y);
        for (int i = 0; i < count; i++) {
            if (!s.scene[i].is_spawned || s.scene[i].type != tile) continue;
            if (dist((struct v2) { x, y }, (struct v2) { s.scene[i].x, s.scene[i].y }) < 30.)
                delete(i);
        }
        for (int i = 0; i < frame.width * frame.height; i++) {
            int my = i / frame.width;
            int mx = i % frame.width;
            if (dist((struct v2) { x, y }, (struct v2) { mx, my }) < 30.)
                spawn((struct node) { .type = tile, "asd", .x = mx, .y = my, .c = orange });
        }
    }

    switch (message) {
    case WM_QUIT: {} break;
    case WM_DESTROY: {
        s.quit = 1;
    } break;
    case WM_MOUSEMOVE: {
        spawn((struct node) { .type = leaf, .x = x, .y = y, .c = cyan });
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
        //  SetWindowTextA(window_handle, "snry rpg sn0833");

    } break;

    case WM_SIZE: {

        frame_bitmap_info.bmiHeader.biWidth = LOWORD(lParam);
        frame_bitmap_info.bmiHeader.biHeight = HIWORD(lParam);

        if (frame_bitmap) DeleteObject(frame_bitmap);
        frame_bitmap = CreateDIBSection(NULL, &frame_bitmap_info, DIB_RGB_COLORS, (void**)&frame.pixels, 0, 0);
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

HANDLE hConsole;
HWND consoleWindow;
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
        123, 123, 512, 256, NULL, NULL, hInstance, NULL);
    if (window_handle == NULL) { return -1; }

    font = CreateFont(24, 0, 0, 0, FW_NORMAL, 0, 0, 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH, L"Comic Sans MS");

    while (!s.quit) {
        process(.01);

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            DispatchMessage(&msg);
        }

        if (s.frames == 2) {
            console();
            consoleWindow = GetConsoleWindow();
            SetWindowPos(consoleWindow, 0, 33, 432, 512, 256, 0);
            SetForegroundWindow(window_handle);
            init();
        }

        InvalidateRect(window_handle, NULL, FALSE);
        UpdateWindow(window_handle);

    }
    return 0;

}