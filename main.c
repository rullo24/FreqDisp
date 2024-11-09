// including std C libs
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// including raylib libs
#include <raylib.h>
#define RAYGUI_IMPLEMENTATION // required for RAYGUI lib to work
#include <raygui.h>

// macro defs
#define WORK_ON_WSL_USING_X11_SERVER false
#define MAX_MONITOR_REFRESH_RATE_HZ 999

// struct defs
typedef struct {
    bool main_window_init;
} CLEAN_VARS;

// init of all functions that are called after main
int conv_freq_to_ms(int freq_hz);
void cleanup(CLEAN_VARS *clean_vars);

int main(int argc, char *argv[]) {
    // init hardware vars
    int def_width = GetMonitorWidth(0); // default monitor width (if avail)
    int def_height = GetMonitorHeight(0); // default monitor height (if avail)
    int def_refresh_rate = GetMonitorRefreshRate(0); // default monitor max refresh rate (if avail)
    CLEAN_VARS clean_vars = { .main_window_init = false }; // for cleanup function
    double last_refresh_time_ms = GetTime() * 1000.0; // init the timer

    // assessing whether to throw an error if can't find the default monitor
    if (WORK_ON_WSL_USING_X11_SERVER) { // create the application based on constants
        def_width = 600;
        def_height = 600;
        def_refresh_rate = 60;
    } else { // creating the application based on the monitor dimensions
        // checking if monitor was successfully retrieved
        if (def_width <= 0 || def_height <= 0 || def_refresh_rate <= 0) {
            fprintf(stderr, "ERROR: %s\n", "failed to get the default monitor height or width\n");
            cleanup(&clean_vars);
            return -1;
        }

        // ensuring that buffer overflow doesn't occur as a result of some crazy refresh rate
        if (def_refresh_rate > MAX_MONITOR_REFRESH_RATE_HZ) {
            fprintf(stderr, "ERROR: %s\n", "refresh rate > 999Hz\n");
            cleanup(&clean_vars);
            return -1;
        }
    }

    // creating buffer to store user entry (frequency)
    int max_freq_buf_size = 4; // defined by the max refresh rate possible (also accounts for \0)
    char curr_freq_str[max_freq_buf_size]; // init all memory to zeros
    memset(curr_freq_str, '\0', max_freq_buf_size); // ensuring that all memory is zeroed (conforming to C99 std)
    strncpy(curr_freq_str, "0", max_freq_buf_size-1); // init memory to "0"

    // creating buffer to store user entry (duty cycle)
    int max_duty_cycle_buf_size = 4; // defined by the max refresh rate possible (also accounts for \0)
    char curr_duty_cycle_str[max_duty_cycle_buf_size]; // init all memory to zeros
    memset(curr_duty_cycle_str, '\0', max_duty_cycle_buf_size); // ensuring that all memory is zeroed (conforming to C99 std)
    strncpy(curr_duty_cycle_str, "0", max_duty_cycle_buf_size-1); // init memory to "0"

    // init raylib vars + creating a Raylib window from the found resolution
    InitWindow(def_width, def_height, "FreqDisp");
    SetTargetFPS(def_refresh_rate); // limt FPS to make timing consistent
    Color background_colour = DARKGRAY; // starting colour
    clean_vars.main_window_init = true;

    int button_size_x = def_width/5; // based on the default monitor res
    int text_box_size_x = def_width/4; // based on the default monitor res
    int widget_size_y = def_height/20; // based on the default monitor res

    bool refresh_freq_box_active = false;
    long int refresh_freq_hz_int = 0;
    long int refresh_time_ms_int = 0;

    bool duty_cycle_box_active = false;
    long int duty_cycle_percent_int = 0;

    long int on_time_ms = 0;
    long int off_time_ms = 0;
    bool is_on_flag = false;

    // game loop --> runs until window close true
    while (!WindowShouldClose()) {
        // create background + init
        BeginDrawing();
        ClearBackground(background_colour);
        double current_time_ms = GetTime() * 1000.0; // constantly updating timer
        Vector2 curr_mouse_pos = GetMousePosition(); // current position of the user's mouse

        // handle frequency user input through text box --> top widget
        Rectangle refresh_freq_text_box_dims = { ((def_width-text_box_size_x) / 2), ((def_height-widget_size_y) / 2) - widget_size_y, text_box_size_x, widget_size_y}; // x, y, w, h
        if (CheckCollisionPointRec(GetMousePosition(), refresh_freq_text_box_dims) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) { // checking if this widget is currently clicked (drawing focus)
            refresh_freq_box_active = true;
            duty_cycle_box_active = false; // Deactivate the other box
        }
        refresh_freq_box_active = GuiTextBox(refresh_freq_text_box_dims, curr_freq_str, sizeof(curr_freq_str), refresh_freq_box_active);

        // handle duty cycle user input through text box --> bottom widget
        Rectangle duty_cycle_percent_text_box_dims = { ((def_width-text_box_size_x) / 2), ((def_height-widget_size_y) / 2) + widget_size_y, text_box_size_x, widget_size_y}; // x, y, w, h
        if (CheckCollisionPointRec(GetMousePosition(), duty_cycle_percent_text_box_dims) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) { // checking if this widget is currently clicked (drawing focus)
            refresh_freq_box_active = false; // Deactivate the other box
            duty_cycle_box_active = true;
        }
        duty_cycle_box_active = GuiTextBox(duty_cycle_percent_text_box_dims, curr_duty_cycle_str, sizeof(curr_duty_cycle_str), duty_cycle_box_active);

        // handle button click --> middle widget
        Rectangle refresh_freq_go_button_dims = { ((def_width-button_size_x) / 2), ((def_height-widget_size_y) / 2), button_size_x, widget_size_y}; // x, y, w, h
        if (GuiButton(refresh_freq_go_button_dims, "GO")) {
            // attempt to conv user input (freq) to an int
            char *refresh_freq_end_ptr;
            refresh_freq_hz_int = strtol(curr_freq_str, &refresh_freq_end_ptr, 10); // conv str --> long int (grabbing user input)

            // checking if the conv was successful (user input correctly validated)
            if (*refresh_freq_end_ptr != '\0' || refresh_freq_hz_int <= 0 || refresh_freq_hz_int > 999) { // failed str --> int conversion
                refresh_freq_hz_int = 0; // resolving failed conversion
                refresh_time_ms_int = 0;
            } else {
                refresh_time_ms_int = (long int)(1.0/(double)refresh_freq_hz_int * 1000.0); // conv user freq to a time
            }

            // attempting to conv user input (duty cycle) to an int
            char *duty_cycle_end_ptr;
            duty_cycle_percent_int = strtol(curr_duty_cycle_str, &duty_cycle_end_ptr, 10); // conv str --> long int (grabbing user input)

            // checking if the conv was successful (user input correctly validated)
            if (*duty_cycle_end_ptr != '\0' || duty_cycle_percent_int <= 0 || duty_cycle_percent_int > 100) {
                duty_cycle_percent_int = 0;
            }
        }

        // handle user clicking outside of all GUI elements (removing focus from all widgets)
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
            CheckCollisionPointRec(curr_mouse_pos, refresh_freq_text_box_dims) && 
            CheckCollisionPointRec(curr_mouse_pos, refresh_freq_go_button_dims) &&
            CheckCollisionPointRec(curr_mouse_pos, duty_cycle_percent_text_box_dims)) {
            refresh_freq_box_active = false;
            duty_cycle_box_active = false;
        }

        // calculating the duty cycle parameters for flashing
        on_time_ms = refresh_time_ms_int * (long int)((double)duty_cycle_percent_int / 100.0);
        off_time_ms = refresh_time_ms_int - on_time_ms;

        // checking if colour change is required (duty cycle dependent)
        if (is_on_flag && current_time_ms - last_refresh_time_ms >= (double)on_time_ms) { // time since last whole cycle
            is_on_flag = false;
            last_refresh_time_ms = current_time_ms;
            background_colour = DARKGRAY; // change to the "off" state colour

        } else if (!is_on_flag && current_time_ms - last_refresh_time_ms >= (double)off_time_ms) {
            is_on_flag = true;           
            last_refresh_time_ms = current_time_ms;
            background_colour = DARKBLUE; // change to the "on" state colour
        }

        EndDrawing();
    }

    cleanup(&clean_vars);
    return 0;
}

int conv_freq_to_ms(int freq_hz) {
    int output_ms = (long int) (1.0 / ((double)freq_hz));
    return output_ms;
}

// called on program leave --> deallocating memory
void cleanup(CLEAN_VARS *clean_vars) {
    if (clean_vars->main_window_init) { // if raylib window has been initialised
        CloseWindow();
    }
}