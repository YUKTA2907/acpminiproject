#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define WIDTH 60
#define HEIGHT 20
#define MAX_OBJECTS 20
#define NUM_SLOTS 6 // Expanded to 6 distinct drawing boxes!

typedef enum { CIRCLE, RECTANGLE, LINE, TRIANGLE } ShapeType;

// Structure to hold shape information including color properties
typedef struct {
    int id;
    ShapeType type;
    int x1, y1; 
    int x2, y2; 
    int x3, y3; 
    char symbol; 
    int color_code; // 1: Red, 2: Green, 3: Yellow, 4: Blue, 5: Magenta, 6: Cyan
    int is_filled;  // 1 for solid filled shape, 0 for hollow outline (mainly for Rectangle)
    int active;  
} GraphicObject;

typedef struct {
    char canvas_sym[HEIGHT][WIDTH];
    int canvas_color[HEIGHT][WIDTH]; // Stores color maps for each pixel
    GraphicObject registry[MAX_OBJECTS];
    int object_counter;
    int bg_color_code; // Workspace background color
} Workspace;

Workspace slots[NUM_SLOTS];
int current_slot = 0; 

// --- COLOR ENGINE UTILITIES ---

void apply_fg_color(int code) {
    switch(code) {
        case 1: printf("\033[1;31m"); break; // Red
        case 2: printf("\033[1;32m"); break; // Green
        case 3: printf("\033[1;33m"); break; // Yellow
        case 4: printf("\033[1;34m"); break; // Blue
        case 5: printf("\033[1;35m"); break; // Magenta
        case 6: printf("\033[1;36m"); break; // Cyan
        default: printf("\033[0m");    break; // Reset
    }
}

void apply_bg_color(int code) {
    switch(code) {
        case 1: printf("\033[41m"); break; // Red BG
        case 2: printf("\033[42m"); break; // Green BG
        case 3: printf("\033[43m"); break; // Yellow BG
        case 4: printf("\033[44m"); break; // Blue BG
        case 5: printf("\033[45m"); break; // Magenta BG
        case 6: printf("\033[46m"); break; // Cyan BG
        default: printf("\033[0m");  break; // Reset (Black/Default Terminal)
    }
}

void reset_color() {
    printf("\033[0m");
}

// --- CANVAS MANAGEMENT ---

void clear_single_canvas(int slot) {
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            slots[slot].canvas_sym[i][j] = ' ';
            slots[slot].canvas_color[i][j] = 0; // 0 means use BG color
        }
    }
}

void display_current_canvas() {
    printf("\n=================== VIEWING BOX SLOT: [%d] ===================\n", current_slot + 1);
    
    // Top Frame Border
    for (int j = 0; j < WIDTH + 2; j++) printf("-");
    printf("\n");

    for (int i = 0; i < HEIGHT; i++) {
        printf("|"); // Side border
        for (int j = 0; j < WIDTH; j++) {
            int pixel_color = slots[current_slot].canvas_color[i][j];
            
            if (pixel_color != 0) {
                // Shape pixel: apply its specific color foreground over background
                apply_bg_color(slots[current_slot].bg_color_code);
                apply_fg_color(pixel_color);
                printf("%c", slots[current_slot].canvas_sym[i][j]);
                reset_color();
            } else {
                // Background pixel
                if (slots[current_slot].bg_color_code != 0) {
                    apply_bg_color(slots[current_slot].bg_color_code);
                    printf(" ");
                    reset_color();
                } else {
                    printf(" ");
                }
            }
        }
        printf("|\n"); 
    }

    // Bottom Frame Border
    for (int j = 0; j < WIDTH + 2; j++) printf("-");
    printf("\n");
}

void plot(int x, int y, char sym, int color) {
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        slots[current_slot].canvas_sym[y][x] = sym;
        slots[current_slot].canvas_color[y][x] = color;
    }
}

// --- RENDER ENGINE WITH FILL SUPPORT ---

void draw_line(int x0, int y0, int x1, int y1, char sym, int color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (1) {
        plot(x0, y0, sym, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void draw_rectangle(int x, int y, int w, int h, char sym, int color, int is_filled) {
    if (is_filled) {
        // Nested loops fill the inside space completely
        for (int row = y; row < y + h; row++) {
            for (int col = x; col < x + w; col++) {
                plot(col, row, sym, color);
            }
        }
    } else {
        // Draw standard wireframe outline
        draw_line(x, y, x + w - 1, y, sym, color);         
        draw_line(x, y + h - 1, x + w - 1, y + h - 1, sym, color); 
        draw_line(x, y, x, y + h - 1, sym, color);         
        draw_line(x + w - 1, y, x + w - 1, y + h - 1, sym, color); 
    }
}

void draw_circle(int xc, int yc, int r, char sym, int color) {
    int x = 0, y = r;
    int d = 3 - 2 * r;
    while (y >= x) {
        plot(xc + x, yc + y, sym, color); plot(xc - x, yc + y, sym, color);
        plot(xc + x, yc - y, sym, color); plot(xc - x, yc - y, sym, color);
        plot(xc + y, yc + x, sym, color); plot(xc - y, yc + x, sym, color);
        plot(xc + y, yc - x, sym, color); plot(xc - y, yc - x, sym, color);
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
    }
}

void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, char sym, int color) {
    draw_line(x1, y1, x2, y2, sym, color);
    draw_line(x2, y2, x3, y3, sym, color);
    draw_line(x3, y3, x1, y1, sym, color);
}

void render_current_workspace() {
    clear_single_canvas(current_slot);
    int limit = slots[current_slot].object_counter;
    
    for (int i = 0; i < limit; i++) {
        GraphicObject obj = slots[current_slot].registry[i];
        if (!obj.active) continue;

        switch (obj.type) {
            case CIRCLE:
                draw_circle(obj.x1, obj.y1, obj.x2, obj.symbol, obj.color_code);
                break;
            case RECTANGLE:
                draw_rectangle(obj.x1, obj.y1, obj.x2, obj.y2, obj.symbol, obj.color_code, obj.is_filled);
                break;
            case LINE:
                draw_line(obj.x1, obj.y1, obj.x2, obj.y2, obj.symbol, obj.color_code);
                break;
            case TRIANGLE:
                draw_triangle(obj.x1, obj.y1, obj.x2, obj.y2, obj.x3, obj.y3, obj.symbol, obj.color_code);
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
            printf("ID: %d | Symbol: '%c' | Color Code: %d | Filled: %s | Type: ", 
                   obj.id, obj.symbol, obj.color_code, obj.is_filled ? "Yes" : "No");
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

// --- MAIN LOOP ---

int main() {
    int choice;
    
    for(int s = 0; s < NUM_SLOTS; s++) {
        clear_single_canvas(s);
        slots[s].object_counter = 0;
        slots[s].bg_color_code = 0; // Default transparent/black background
    }

    while (1) {
        render_current_workspace();
        display_current_canvas();

        printf("\n=== Chromatic Multi-Box Studio (Slots 1-%d) ===\n", NUM_SLOTS);
        printf("1. Add Shape to CURRENT Box\n");
        printf("2. Delete Shape from CURRENT Box\n");
        printf("3. Modify Shape Options\n");
        printf("4. Set CURRENT Box Background Color\n");
        printf("5. SWITCH to a Different Drawing Box (1-%d)\n", NUM_SLOTS);
        printf("6. Exit\n");
        printf("Selection: ");
        scanf("%d", &choice);

        if (choice == 6) break;

        if (choice == 1) {
            int current_count = slots[current_slot].object_counter;
            if (current_count >= MAX_OBJECTS) {
                printf("Error: Object registry capacity reached!\n");
                continue;
            }
            int type, clr, fill = 0;
            char sym;
            
            printf("Select Shape Type (0:Circle, 1:Rectangle, 2:Line, 3:Triangle): ");
            scanf("%d", &type);
            printf("Enter character symbol: ");
            scanf(" %c", &sym);
            printf("Select Color (1:Red, 2:Green, 3:Yellow, 4:Blue, 5:Magenta, 6:Cyan): ");
            scanf("%d", &clr);
            
            if (type == 1) { // Rectangle fill configuration choice
                printf("Fill shape solid? (1 for Yes, 0 for Hollow Outline): ");
                scanf("%d", &fill);
            }

            GraphicObject obj;
            obj.id = current_count + 1;
            obj.type = (ShapeType)type;
            obj.symbol = sym;
            obj.color_code = clr;
            obj.is_filled = fill;
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
                    printf("Options:\n1. Move Position\n2. Change Symbol Skin\n3. Toggle Fill Option\n4. Change Color\nChoice: ");
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
                    } else if (sub_choice == 3) {
                        slots[current_slot].registry[i].is_filled = !slots[current_slot].registry[i].is_filled;
                        printf("Fill toggle switched.\n");
                    } else if (sub_choice == 4) {
                        int new_clr;
                        printf("Select New Color (1-6): ");
                        scanf("%d", &new_clr);
                        slots[current_slot].registry[i].color_code = new_clr;
                    }
                    break;
                }
            }
        } else if (choice == 4) {
            int bg_clr;
            printf("Select Canvas Background Color (0:None/Black, 1:Red, 2:Green, 3:Yellow, 4:Blue, 5:Magenta, 6:Cyan): ");
            scanf("%d", &bg_clr);
            slots[current_slot].bg_color_code = bg_clr;
            printf("Background updated successfully.\n");
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