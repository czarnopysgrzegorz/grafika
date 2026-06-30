#include <SDL.h>
#include <stdio.h>
#include <math.h>
//compiler gcc main.c $(sdl2-config --cflags --libs) -o test
#define MAX_POINTS 10
#define POINT_RADIUS 6

typedef struct {
    float x, y;
} Point;

Point points[MAX_POINTS];
int numPoints = 0;
int draggingPoint = -1;

// --- Obliczanie punktu Béziera de Casteljau ---
void bezier_point(float t, Point *pts, int n, float *x, float *y) {
    float temp_x[MAX_POINTS];
    float temp_y[MAX_POINTS];
    for (int i = 0; i <= n; i++) {
        temp_x[i] = pts[i].x;
        temp_y[i] = pts[i].y;
    }
    for (int k = 1; k <= n; k++) {
        for (int i = 0; i <= n - k; i++) {
            temp_x[i] = (1 - t) * temp_x[i] + t * temp_x[i + 1];
            temp_y[i] = (1 - t) * temp_y[i] + t * temp_y[i + 1];
        }
    }
    *x = temp_x[0];
    *y = temp_y[0];
}

// --- Szukanie punktu do przeciągania ---
int find_point(float x, float y) {
    for (int i = 0; i < numPoints; i++) {
        float dx = x - points[i].x;
        float dy = y - points[i].y;
        if (sqrt(dx*dx + dy*dy) < 10) return i;
    }
    return -1;
}

// --- Rysowanie okręgu (punkt kontrolny) ---
void draw_point(SDL_Renderer *ren, float x, float y) {
    SDL_Rect r = { (int)(x - POINT_RADIUS), (int)(y - POINT_RADIUS), POINT_RADIUS*2, POINT_RADIUS*2 };
    SDL_RenderFillRect(ren, &r);
}

// --- Rysowanie poligonu kontrolnego ---
void draw_control_polygon(SDL_Renderer *ren) {
    SDL_SetRenderDrawColor(ren, 180, 180, 180, 255); // szare linie
    for (int i = 0; i < numPoints - 1; i++) {
        SDL_RenderDrawLine(ren, (int)points[i].x, (int)points[i].y,
                                    (int)points[i+1].x, (int)points[i+1].y);
    }
}

// --- Rysowanie krzywej Béziera ---
void draw_bezier(SDL_Renderer *ren) {
    if (numPoints < 2) return;

    SDL_SetRenderDrawColor(ren, 0, 120, 255, 255); // niebieska krzywa

    int steps = 1000;
    float prev_x, prev_y, x, y;
    bezier_point(0, points, numPoints - 1, &prev_x, &prev_y);
    for (int i = 1; i <= steps; i++) {
        float t = (float)i / steps;
        bezier_point(t, points, numPoints - 1, &x, &y);
        SDL_RenderDrawLine(ren, (int)prev_x, (int)prev_y, (int)x, (int)y);
        prev_x = x;
        prev_y = y;
    }
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow("Krzywa Béziera SDL2",
                                        SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED,
                                        800, 600, 0);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    int running = 1;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT:
                    running = 0;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (e.button.button == SDL_BUTTON_LEFT) {
                        int idx = find_point(e.button.x, e.button.y);
                        if (idx != -1) {
                            draggingPoint = idx;
                        } else if (numPoints < MAX_POINTS) {
                            points[numPoints].x = e.button.x;
                            points[numPoints].y = e.button.y;
                            numPoints++;
                        }
                    }
                    break;
                case SDL_MOUSEBUTTONUP:
                    draggingPoint = -1;
                    break;
                case SDL_MOUSEMOTION:
                    if (draggingPoint != -1) {
                        points[draggingPoint].x = e.motion.x;
                        points[draggingPoint].y = e.motion.y;
                    }
                    break;
            }
        }

        SDL_SetRenderDrawColor(ren, 255, 255, 255, 255); // tło białe
        SDL_RenderClear(ren);

        draw_control_polygon(ren);

        SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
        for (int i = 0; i < numPoints; i++) draw_point(ren, points[i].x, points[i].y);

        draw_bezier(ren);

        SDL_RenderPresent(ren);
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
