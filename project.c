#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define WIDTH 60
#define HEIGHT 20
#define MAX_OBJECTS 30

typedef enum { CIRCLE, RECTANGLE, LINE, TRIANGLE } ShapeType;

// Structure updated to hold unique symbols per object
typedef struct {
    int id;
    ShapeType type;
    int x1, y1; 
    int x2, y2; 
    int x3, y3; 
    char symbol; // Unique character symbol for this specific object
    int active;  
} GraphicObject;

char canvas[HEIGHT][WIDTH];
GraphicObject registry[MAX_OBJECTS];
int object_counter = 0;

// --- CANVAS MANAGEMENT ---

void clear_canvas() {
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            canvas[i][j] = ' '; // Default empty background
        }
    }
}

void display_canvas() {
    // Upper border border
    printf(".");
    for (int j = 0; j < WIDTH; j++) printf("-");
    printf(".\n");

    for (int i = 0; i < HEIGHT; i++) {
        printf("|"); // Left border
        for (int j = 0; j < WIDTH; j++) {
            printf("%c", canvas[i][j]);
        }
        printf("|\n"); // Right border
    }

    // Lower border
    printf(".");
    for (int j = 0; j < WIDTH; j++) printf("-");
    printf(".\n");
}

void plot(int x, int y, char sym) {
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        canvas[y][x] = sym; // Plots the custom symbol
    }
}

// --- RENDER ENGINE ALGORITHMS ---

void draw_line(int x0, int y0, int x1, int y1, char sym) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (1) {
        plot(x0, y0, sym);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void draw_rectangle(int x, int y, int w, int h, char sym) {
    draw_line(x, y, x + w - 1, y, sym);         
    draw_line(x, y + h - 1, x + w - 1, y + h - 1, sym); 
    draw_line(x, y, x, y + h - 1, sym);         
    draw_line(x + w - 1, y, x + w - 1, y + h - 1, sym); 
}

void draw_circle(int xc, int yc, int r, char sym) {
    int x = 0, y = r;
    int d = 3 - 2 * r;
    while (y >= x) {
        plot(xc + x, yc + y, sym); plot(xc - x, yc + y, sym);
        plot(xc + x, yc - y, sym); plot(xc - x, yc - y, sym);
        plot(xc + y, yc + x, sym); plot(xc - y, yc + x, sym);
        plot(xc + y, yc - x, sym); plot(xc - y, yc - x, sym);
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
    }
}

void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, char sym) {
    draw_line(x1, y1, x2, y2, sym);
    draw_line(x2, y2, x3, y3, sym);
    draw_line(x3, y3, x1, y1, sym);
}

// Redraws the scene by reading each object's distinct symbol
void render_all_objects() {
    clear_canvas();
    for (int i = 0; i < object_counter; i++) {
        if (!registry[i].active) continue;

        switch (registry[i].type) {
            case CIRCLE:
                draw_circle(registry[i].x1, registry[i].y1, registry[i].x2, registry[i].symbol);
                break;
            case RECTANGLE:
                draw_rectangle(registry[i].x1, registry[i].y1, registry[i].x2, registry[i].y2, registry[i].symbol);
                break;
            case LINE:
                draw_line(registry[i].x1, registry[i].y1, registry[i].x2, registry[i].y2, registry[i].symbol);
                break;
            case TRIANGLE:
                draw_triangle(registry[i].x1, registry[i].y1, registry[i].x2, registry[i].y2, registry[i].x3, registry[i].y3, registry[i].symbol);
                break;
        }
    }
}

void list_objects() {
    printf("\n--- Active Objects Registry ---\n");
    int count = 0;
    for (int i = 0; i < object_counter; i++) {
        if (registry[i].active) {
            printf("ID: %d | Symbol: '%c' | Type: ", registry[i].id, registry[i].symbol);
            if (registry[i].type == CIRCLE) printf("Circle (Center: %d,%d, Rad: %d)", registry[i].x1, registry[i].y1, registry[i].x2);
            if (registry[i].type == RECTANGLE) printf("Rectangle (Pos: %d,%d, W: %d, H: %d)", registry[i].x1, registry[i].y1, registry[i].x2, registry[i].y2);
            if (registry[i].type == LINE) printf("Line (Start: %d,%d, End: %d,%d)", registry[i].x1, registry[i].y1, registry[i].x2, registry[i].y2);
            if (registry[i].type == TRIANGLE) printf("Triangle (%d,%d / %d,%d / %d,%d)", registry[i].x1, registry[i].y1, registry[i].x2, registry[i].y2, registry[i].x3, registry[i].y3);
            printf("\n");
            count++;
        }
    }
    if (count == 0) printf("No objects found.\n");
}

// --- MAIN LOOP ---

int main() {
    int choice;
    clear_canvas();

    while (1) {
        render_all_objects();
        display_canvas();

        printf("\n=== Multi-Symbol Dynamic Graphics Canvas ===\n");
        printf("1. Add Object (Choose custom symbol)\n");
        printf("2. Delete Object\n");
        printf("3. Modify Object (Move or Change Symbol)\n");
        printf("4. List Registry Details\n");
        printf("5. Exit\n");
        printf("Selection: ");
        scanf("%d", &choice);

        if (choice == 5) break;

        if (choice == 1) {
            if (object_counter >= MAX_OBJECTS) {
                printf("Error: Canvas layer limit reached!\n");
                continue;
            }
            int type;
            char sym;
            printf("Select Shape Type (0:Circle, 1:Rectangle, 2:Line, 3:Triangle): ");
            scanf("%d", &type);
            
            printf("Enter unique single character symbol for this shape (e.g. *, #, @, x, o): ");
            scanf(" %c", &sym); // Note the space before %c to clear trailing inputs

            GraphicObject obj;
            obj.id = object_counter + 1;
            obj.type = (ShapeType)type;
            obj.symbol = sym;
            obj.active = 1;

            if (type == 0) {
                printf("Enter Center X, Center Y, and Radius: ");
                scanf("%d %d %d", &obj.x1, &obj.y1, &obj.x2);
            } else if (type == 1) {
                printf("Enter Top-Left X, Top-Left Y, Width, Height: ");
                scanf("%d %d %d %d", &obj.x1, &obj.y1, &obj.x2, &obj.y2);
            } else if (type == 2) {
                printf("Enter Start X, Y and End X, Y: ");
                scanf("%d %d %d %d", &obj.x1, &obj.y1, &obj.x2, &obj.y2);
            } else if (type == 3) {
                printf("Enter X1 Y1, X2 Y2, X3 Y3: ");
                scanf("%d %d %d %d %d %d", &obj.x1, &obj.y1, &obj.x2, &obj.y2, &obj.x3, &obj.y3);
            }

            registry[object_counter++] = obj;

        } else if (choice == 2) {
            list_objects();
            int del_id;
            printf("Enter Object ID to delete: ");
            scanf("%d", &del_id);
            for (int i = 0; i < object_counter; i++) {
                if (registry[i].id == del_id) {
                    registry[i].active = 0;
                    printf("Object %d removed successfully.\n", del_id);
                    break;
                }
            }
        } else if (choice == 3) {
            list_objects();
            int mod_id;
            printf("Enter Object ID to modify: ");
            scanf("%d", &mod_id);
            for (int i = 0; i < object_counter; i++) {
                if (registry[i].id == mod_id && registry[i].active) {
                    int sub_choice;
                    printf("Modification Options:\n1. Translate/Shift Position\n2. Change Drawing Symbol\nChoice: ");
                    scanf("%d", &sub_choice);
                    
                    if (sub_choice == 1) {
                        int dx, dy;
                        printf("Enter Delta X shift and Delta Y shift: ");
                        scanf("%d %d", &dx, &dy);
                        registry[i].x1 += dx; registry[i].y1 += dy;
                        registry[i].x2 += dx; registry[i].y2 += dy;
                        if (registry[i].type == TRIANGLE) {
                            registry[i].x3 += dx; registry[i].y3 += dy;
                        }
                        printf("Object %d moved.\n", mod_id);
                    } else if (sub_choice == 2) {
                        char new_sym;
                        printf("Enter new single-character symbol: ");
                        scanf(" %c", &new_sym);
                        registry[i].symbol = new_sym;
                        printf("Object %d symbol updated to '%c'.\n", mod_id, new_sym);
                    }
                    break;
                }
            }
        } else if (choice == 4) {
            list_objects();
        }
    }
    return 0;
}