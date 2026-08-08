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

#define orange (struct color){255,111,0,255}
struct color { char r, g, b, a; };
struct frame { int width; int height; unsigned char* pixels; } frame;
int quit, frames;
typedef struct v2 { float x, y; };
struct grid { int x, y; char size; };
struct item { char grid, cell; float x, y; };
struct button { float x, y; };
struct player { float x, y; int acc; };
struct grid a, b;
char grid, cell, sel, is_drag, drag, button, click;
struct item stuff[64];
struct button card[2];
struct player player;
float vx, vy;
int goal = 789;

float lerp(float a, float b, float f) {
    float r = a * (1.0 - f) + (b * f);
    return r;
}
float len(struct v2 v) { return sqrt(v.x * v.x + v.y * v.y); };
float dist(struct v2 a, struct v2 b) { return len((struct v2) { a.x - b.x, a.y - b.y }); };
int v2_box(int x, int y, int cx, int cy, int w, int h) {
    if ((x > cx && x < cx + w) && (y > cy && y < cy + h)) return 1;
    else   return 0;
}

void init() {
    printf("Hi!\n");
    a.size = 8;
    b.size = 4;
    for (int i = 0; i < 64; i++) {
        if ((rand()%23)==1) stuff[i].grid = 1;
        stuff[i].cell = rand() % 64;
        stuff[i].x = rand() % 678;
        stuff[i].y = rand() % 123;
    }
    sel = -1;
    button = -1;
    card[0].x = 111;
    card[0].y = 275;
    card[1].x = 345;
    card[1].y = 234;
};

void text(char* t, float x, float y, float h, float w) {
    SelectObject(frame_device_context, font);
    SetTextColor(frame_device_context, RGB(0,  255, 0));
    SetBkMode(frame_device_context, 2);
    SetBkColor(frame_device_context, RGB(0, 11, 11));
    RECT r = { x, frame.height-y-h, x+w, frame.height - y};
    DrawTextA(frame_device_context, t, -1, &r, 0);
}
void point(struct frame f, float x, float y, struct color c) {
    if (x >= 0 && x < f.width && y >= 0 && y < f.height) {
        f.pixels[0 + (int)(x + y * f.width) * 4] = c.b;
        f.pixels[1 + (int)(x + y * f.width) * 4] = c.g;
        f.pixels[2 + (int)(x + y * f.width) * 4] += c.r;
    }
}
void line(struct frame f, struct v2 a, struct v2 b) {
    float d = dist(a, b);
    int steps = (int)d;  // Convert to int properly
    if (steps < 1) steps = 1;  // Ensure at least 1 step
    for (int i = 0; i <= steps; i++) {
        point(f,
            lerp(a.x, b.x, i/ (float)steps),
            lerp(a.y, b.y, i/ (float)steps), orange);
    }
}
void box(struct frame f, struct v2 v) {
    line(frame, (struct v2){v.x,v.y}, (struct v2){v.x,v.y+16} );
    line(frame, (struct v2){v.x+16,v.y}, (struct v2){v.x+16,v.y+16} );
    line(frame, (struct v2){v.x,v.y}, (struct v2){v.x+16,v.y} );
    line(frame, (struct v2){v.x,v.y+16}, (struct v2){v.x+16,v.y+16} );
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

    int x, y;
    for (int i = 0; i < 64; i++) {
        char str[64]; sprintf(str, "%i", i);
        if (stuff[i].grid == 0) {
            text(str, stuff[i].x, stuff[i].y, 16, 16);
        }
        if (stuff[i].grid == 1) {
            x = stuff[i].cell % a.size * 16;
            y = stuff[i].cell / a.size * 16;
            text(str, a.x + x + 1, a.y + y, 16, 16);
        }
        if (stuff[i].grid == 2) {
            x = stuff[i].cell % b.size * 16;
            y = stuff[i].cell / b.size * 16;
            text(str, b.x + x + 1, b.y + y, 16, 16);
        }
    }
    text("~P", player.x, player.y, 16, 16);

    for (int i = 0; i < a.size+1; i++) {
    line(
        frame,
        (struct v2){a.x+i*16,a.y+0.},
        (struct v2){a.x+i*16,a.y+(a.size*16)}
    );
    line(
        frame,
        (struct v2){a.x+0,a.y+i*16},
        (struct v2){a.x+(a.size*16),a.y+i*16}
    );
    }


    for (int i = 0; i < b.size+1; i++) {
    line(
        frame,
        (struct v2){b.x+i*16,b.y+0.},
        (struct v2){b.x+i*16,b.y+(b.size*16)}
    );
    line(
        frame,
        (struct v2){b.x+0,b.y+i*16},
        (struct v2){b.x+(b.size*16),b.y+i*16}
    );
    }

    for (int i = 0; i < 2; i++) {
        box(frame, (struct v2) { card[i].x, card[i].y });
    }
    char str[129]; sprintf(str,
        "Hi! \n f:%ik g:%i c:%i \n select: %i \n button: %i \n acc: %i \n goal: %i", 
        frames / 1000, grid, cell, sel, button, player.acc, goal);
    text(str, 0, frame.height-111, 111, 111);
    text("w a s d lm mou", 0, frame.height - 200, 111, 111);

    if (player.acc > goal) {
        text("Well played!", 0, frame.height - 222, 111, 111);

    }
}
int get_item(char cell) {
        for (int j = 0; j < 64; j++) {
            if (stuff[j].grid == 1 && stuff[j].cell == cell) return j;
    }
        return -1;
};
int get_cell() {
    for (int i = 0; i < 64; i++) {
        if (get_item(i) == -1) return i;
    }
}

void process(float dt) {
    a.x = card[0].x;
    a.y = card[0].y - 4 - a.size * 16;
    b.x = card[1].x;
    b.y = card[1].y - 4 - b.size * 16;
    player.y += vy;
    player.x += vx;
    player.acc = 0;
    for (int i = 0; i < 64; i++) {
        if (stuff[i].grid == 2) { player.acc += i; }
        if (stuff[i].grid == 0 &&
            (dist((struct v2) { player.x, player.y }, (struct v2) { stuff[i].x, stuff[i].y }))<10
            && sel == -1
            )

        {
            stuff[i].grid = 1;
            stuff[i].cell = get_cell();
        }
    }

    frames++;
}

LRESULT CALLBACK wpm(HWND window_handle,
    UINT message, WPARAM wParam, LPARAM lParam) {

    int x = LOWORD(lParam);
    int y = frame.height - HIWORD(lParam);

    if (message == WM_KEYDOWN && wParam == 'R') printf("R");
    if (message == WM_KEYDOWN && wParam == 'W') vy = 1;
    if (message == WM_KEYUP && wParam == 'W') vy=0;
    if (message == WM_KEYDOWN && wParam == 'D') vx = 1;
    if (message == WM_KEYUP && wParam == 'D') vx = 0;
    if (message == WM_KEYDOWN && wParam == 'A') vx = -1;
    if (message == WM_KEYUP && wParam == 'A') vx = 0;
    if (message == WM_KEYDOWN && wParam == 'S') vy-=1;
    if (message == WM_KEYUP && wParam == 'S') vy = 0;

    if (message == WM_KEYDOWN && wParam == VK_ESCAPE) { quit = 1; }
    if (message == WM_LBUTTONDOWN) {
        printf("CLICK!  ");
        is_drag = 1;
        drag = sel;
        click = button;
    }
    if (message == WM_LBUTTONUP) {
        printf("CLICK! 2  " );
        if (grid && sel==-1 ) {
            is_drag = 0;
            stuff[drag].cell = cell;
            stuff[drag].grid = grid;
        }
        if (!grid) {
            is_drag = 0;
            stuff[drag].x = x;
            stuff[drag].y = y;
            stuff[drag].grid = 0;
        }
        if (click >= 0) {

            card[click].x = x;
            card[click].y = y;
        }
    }
    switch (message) {
    case WM_QUIT: {} break;
    case WM_DESTROY: {
        quit = 1;
    } break;
    case WM_MOUSEMOVE: {
        int dx;
        int dy;
     //   if (is_drag) break;
        for (int i = 0; i < 2; i++) {
            if (
                v2_box(x, y, card[i].x, card[i].y, 16, 16)
                ) {
                button = i; break;
            }
            else { button = -1; }
        }
        if (
        v2_box(x, y, a.x, a.y, a.size*16, a.size*16)
            ) {
            dx = (x - a.x) / 16;
            dy = (y - a.y) / 16;
            cell = (dy * a.size) + dx;
            grid = 1;
            printf("box 1 ");
            printf("cell %i  ", cell);
            for (int i = 0; i < 64; i++) {
                if (stuff[i].grid==grid && stuff[i].cell == cell)
                { sel=i; break; }
                else { sel = -1; };
            }
            break;
        }
        if (
        v2_box(x, y, b.x, b.y, b.size * 16, b.size * 16)
            ) {
            dx = (x - b.x) / 16;
            dy = (y - b.y) / 16;
            cell = (dy * b.size) + dx;
            grid = 2;
            printf("box 2  ");
            printf("cell %i  ", cell);
            for (int i = 0; i < 64; i++) {
                if (stuff[i].grid==grid && stuff[i].cell == cell)
                { sel=i; break; }
                else { sel = -1; };
            }
            break;
        }
        grid = 0;
        cell = 0;
        sel = -1;
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
        24, 24, 789, 345, NULL, NULL, hInstance, NULL);
    if (window_handle == NULL) { return -1; }

    font = CreateFont(16, 0, 0, 0, FW_NORMAL, 0, 0, 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH, L"Comic Sans MS");

    init();

    while (!quit) {

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            DispatchMessage(&msg);
        }

        if (frames == 2) {
            console();
            consoleWindow = GetConsoleWindow();
            SetWindowPos(consoleWindow, 0, 33, 432, 512, 256, 0);
            SetForegroundWindow(window_handle);
            init();
        }
        process(.01);

        InvalidateRect(window_handle, NULL, FALSE);
        UpdateWindow(window_handle);

    }
    return 0;

}