#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define WIDTH 60
#define HEIGHT 20
#define MAX_OBJECTS 15
#define NUM_SLOTS 3 // 3 separate drawing boxes

typedef enum { CIRCLE, RECTANGLE, LINE, TRIANGLE } ShapeType;

typedef struct {
    int id;
    ShapeType type;
    int x1, y1; 
    int x2, y2; 
    int x3, y3; 
    char symbol; // Holds the distinct custom symbol for this specific shape
    int active;  
} GraphicObject;

// Workspace structure for separate boxes
typedef struct {
    char canvas[HEIGHT][WIDTH];
    GraphicObject registry[MAX_OBJECTS];
    int object_counter;
} Workspace;

Workspace slots[NUM_SLOTS];
int current_slot = 0; 

// --- CANVAS MANAGEMENT ---

void clear_single_canvas(int slot) {
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            slots[slot].canvas[i][j] = ' ';
        }
    }
}

void display_current_canvas() {
    printf("\n=================== VIEWING BOX SLOT: [%d] ===================\n", current_slot + 1);
    printf(".");
    for (int j = 0; j < WIDTH; j++) printf("-");
    printf(".\n");

    for (int i = 0; i < HEIGHT; i++) {
        printf("|"); 
        for (int j = 0; j < WIDTH; j++) {
            printf("%c", slots[current_slot].canvas[i][j]);
        }
        printf("|\n"); 
    }

    printf(".");
    for (int j = 0; j < WIDTH; j++) printf("-");
    printf(".\n");
}

void plot(int x, int y, char sym) {
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        slots[current_slot].canvas[y][x] = sym;
    }
}

// --- RENDERING ALGORITHMS ---

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

void render_current_workspace() {
    clear_single_canvas(current_slot);
    int limit = slots[current_slot].object_counter;
    
    for (int i = 0; i < limit; i++) {
        GraphicObject obj = slots[current_slot].registry[i];
        if (!obj.active) continue;

        switch (obj.type) {
            case CIRCLE:
                draw_circle(obj.x1, obj.y1, obj.x2, obj.symbol);
                break;
            case RECTANGLE:
                draw_rectangle(obj.x1, obj.y1, obj.x2, obj.y2, obj.symbol);
                break;
            case LINE:
                draw_line(obj.x1, obj.y1, obj.x2, obj.y2, obj.symbol);
                break;
            case TRIANGLE:
                draw_triangle(obj.x1, obj.y1, obj.x2, obj.y2, obj.x3, obj.y3, obj.symbol);
                break;
        }
    }
}

void list_objects() {
    printf("\n--- Active Objects in Box Slot [%d] ---\n", current_slot + 1);
    int count = 0;
    int limit = slots[current_slot].object_counter;
    
    for (int i = 0; i < limit; i++) {
        GraphicObject obj = slots[current_slot].registry[i];
        if (obj.active) {
            printf("ID: %d | Symbol: '%c' | Type: ", obj.id, obj.symbol);
            if (obj.type == CIRCLE) printf("Circle (Center: %d,%d, Rad: %d)", obj.x1, obj.y1, obj.x2);
            if (obj.type == RECTANGLE) printf("Rectangle (Pos: %d,%d, W: %d, H: %d)", obj.x1, obj.y1, obj.x2, obj.y2);
            if (obj.type == LINE) printf("Line (Start: %d,%d, End: %d,%d)", obj.x1, obj.y1, obj.x2, obj.y2);
            if (obj.type == TRIANGLE) printf("Triangle (%d,%d / %d,%d / %d,%d)", obj.x1, obj.y1, obj.x2, obj.y2, obj.x3, obj.y3);
            printf("\n");
            count++;
        }
    }
    if (count == 0) printf("No objects found in this box slot.\n");
}

// --- MAIN CONTROL INTERFACE ---

int main() {
    int choice;
    
    for(int s = 0; s < NUM_SLOTS; s++) {
        clear_single_canvas(s);
        slots[s].object_counter = 0;
    }

    while (1) {
        render_current_workspace();
        display_current_canvas();

        printf("\n=== Multi-Box Custom Symbol Studio ===\n");
        printf("1. Add Shape to CURRENT Box\n");
        printf("2. Delete Shape from CURRENT Box\n");
        printf("3. Modify Shape (Move or Change Symbol Style)\n");
        printf("4. List Registry for CURRENT Box\n");
        printf("5. SWITCH to a Different Drawing Box (1-%d)\n", NUM_SLOTS);
        printf("6. Exit\n");
        printf("Selection: ");
        scanf("%d", &choice);

        if (choice == 6) break;

        if (choice == 1) {
            int current_count = slots[current_slot].object_counter;
            if (current_count >= MAX_OBJECTS) {
                printf("Error: Current Box slot limit reached!\n");
                continue;
            }
            int type;
            char sym;
            printf("Select Shape Type (0:Circle, 1:Rectangle, 2:Line, 3:Triangle): ");
            scanf("%d", &type);
            
            // Explicit prompt for unique rendering symbol
            printf("Enter unique single character symbol for this shape (e.g. *, #, @, x): ");
            scanf(" %c", &sym);

            GraphicObject obj;
            obj.id = current_count + 1;
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

            slots[current_slot].registry[slots[current_slot].object_counter++] = obj;

        } else if (choice == 2) {
            list_objects();
            int del_id;
            printf("Enter Object ID to delete: ");
            scanf("%d", &del_id);
            int limit = slots[current_slot].object_counter;
            for (int i = 0; i < limit; i++) {
                if (slots[current_slot].registry[i].id == del_id) {
                    slots[current_slot].registry[i].active = 0;
                    printf("Object %d removed.\n", del_id);
                    break;
                }
            }
        } else if (choice == 3) {
            list_objects();
            int mod_id;
            printf("Enter Object ID to modify: ");
            scanf("%d", &mod_id);
            int limit = slots[current_slot].object_counter;
            for (int i = 0; i < limit; i++) {
                if (slots[current_slot].registry[i].id == mod_id && slots[current_slot].registry[i].active) {
                    int sub_choice;
                    printf("Modification Options:\n1. Move Position\n2. Change Symbol Skin\nChoice: ");
                    scanf("%d", &sub_choice);
                    
                    if (sub_choice == 1) {
                        int dx, dy;
                        printf("Enter Shift X (dx) and Shift Y (dy): ");
                        scanf("%d %d", &dx, &dy);
                        slots[current_slot].registry[i].x1 += dx; slots[current_slot].registry[i].y1 += dy;
                        slots[current_slot].registry[i].x2 += dx; slots[current_slot].registry[i].y2 += dy;
                        if (slots[current_slot].registry[i].type == TRIANGLE) {
                            slots[current_slot].registry[i].x3 += dx; slots[current_slot].registry[i].y3 += dy;
                        }
                    } else if (sub_choice == 2) {
                        char new_sym;
                        printf("Enter new single-character symbol: ");
                        scanf(" %c", &new_sym);
                        slots[current_slot].registry[i].symbol = new_sym;
                        printf("Symbol changed successfully.\n");
                    }
                    break;
                }
            }
        } else if (choice == 4) {
            list_objects();
        } else if (choice == 5) {
            int target_slot;
            printf("Enter Box Slot to switch to (1-%d): ", NUM_SLOTS);
            scanf("%d", &target_slot);
            if (target_slot >= 1 && target_slot <= NUM_SLOTS) {
                current_slot = target_slot - 1; 
                printf("Switched to Box Slot %d.\n", target_slot);
            } else {
                printf("Invalid slot number!\n");
            }
        }
    }
    return 0;
}