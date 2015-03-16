#include <pebble.h>
#include <stdbool.h>

static Window *window;
static TextLayer *text_layer;
static TextLayer *score_layer;
static AppTimer *resetGameTimer;
static AppTimer *timer;
static unsigned int score;
static char buffer[11];

static void resetGameCallback();

static void timer_callback() {
    text_layer_set_text(text_layer, "OUT OF TIME");
    
    // Wait 3 seconds and then restart game
    resetGameTimer = app_timer_register(3000, resetGameCallback, NULL);
}

static void continueGame() {
    if (timer) {
        app_timer_cancel(timer);
    }
    
    int randNum = rand() % 4 + 1;
    
    text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    
    switch (randNum) {
        case 1:
            text_layer_set_text(text_layer, "Select!");
            break;
        case 2:
            text_layer_set_text(text_layer, "Up!");
            break;
        case 3:
            text_layer_set_text(text_layer, "Down!");
            break;
        case 4:
            text_layer_set_text(text_layer, "Shake!");
            break;
        default:
            break;
    }
    
    timer = app_timer_register(2500, timer_callback, NULL);
}

static void updateScore() {
    ++score;
    snprintf(buffer, sizeof(buffer), "Score: %d", score);
    text_layer_set_text(score_layer, buffer);
}

static void resetGameCallback() {
    // Reset timer
    app_timer_cancel(resetGameTimer);
    score = 0;
    snprintf(buffer, sizeof(buffer), "Score: %d", score);
    text_layer_set_text(score_layer, buffer);
    text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
    text_layer_set_text(text_layer, "Press SELECT to start");
}

static void reset_game() {
    if (strcmp(text_layer_get_text(text_layer), "OUT OF TIME") != 0) {
        text_layer_set_text(text_layer, "WRONG");
    }
    app_timer_cancel(timer);
    // Wait 3 seconds and then restart game
    resetGameTimer = app_timer_register(3000, resetGameCallback, NULL);
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
    if (strcmp(text_layer_get_text(text_layer), "Press SELECT to start") != 0) {
        if (strcmp(text_layer_get_text(text_layer), "Shake!") != 0) {
            reset_game();
        } else {
            updateScore();
            continueGame();
        }
    }
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
    if (strcmp(text_layer_get_text(text_layer), "Press SELECT to start") == 0) {
        score = 0;
        continueGame();
    } else {
        if (strcmp(text_layer_get_text(text_layer), "Select!") != 0) {
            reset_game();
        } else {
            updateScore();
            continueGame();
        }
    }
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
    if (strcmp(text_layer_get_text(text_layer), "Press SELECT to start") != 0) {
        if (strcmp(text_layer_get_text(text_layer), "Up!") != 0) {
            reset_game();
        } else {
            updateScore();
            continueGame();
        }
    }
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
    if (strcmp(text_layer_get_text(text_layer), "Press SELECT to start") != 0) {
        if (strcmp(text_layer_get_text(text_layer), "Down!") != 0) {
            reset_game();
        } else {
            updateScore();
            continueGame();
        }
    }
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    text_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, 40 } });
    text_layer_set_text(text_layer, "Press SELECT to start");
    text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(text_layer));
    
    score_layer = text_layer_create(GRect(0,30,144,20));
    text_layer_set_font(score_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    snprintf(buffer, sizeof(buffer), "Score: %d", score);
    text_layer_set_text(score_layer, buffer);
    text_layer_set_text_alignment(score_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(score_layer));
    
    // Seed random number generator
    srand(time(NULL));
}

static void window_unload(Window *window) {
    text_layer_destroy(text_layer);
    text_layer_destroy(score_layer);
}

static void init(void) {
    window = window_create();
    window_set_click_config_provider(window, click_config_provider);
    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
    const bool animated = true;
    window_stack_push(window, animated);
    
    // Subscribe to tap service
    accel_tap_service_subscribe(tap_handler);
}

static void deinit(void) {
    window_destroy(window);
    
    // Unsubscribe from tap service
    accel_tap_service_unsubscribe();
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
