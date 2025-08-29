/**
 * @file oled_display.h
 * @brief OLED Display Interface for EsoCore Edge Device
 *
 * This file implements the OLED display interface for the EsoCore Edge device,
 * providing a user interface for status monitoring, configuration, and diagnostics.
 * Supports 0.96" 128×64 monochrome OLED displays with I²C interface.
 *
 * Features:
 * - 128x64 monochrome OLED display support
 * - I²C interface communication
 * - Text rendering with multiple fonts
 * - Graphics primitives (lines, rectangles, circles)
 * - Menu system with navigation
 * - Status indicators and icons
 * - Screen saver and power management
 * - Custom bitmap display
 *
 * @author EsoCore Development Team
 * @copyright Copyright © 2025 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_OLED_DISPLAY_H
#define ESOCORE_OLED_DISPLAY_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * OLED Display Configuration
 * ============================================================================ */

#define OLED_WIDTH              128     /* Display width in pixels */
#define OLED_HEIGHT             64      /* Display height in pixels */
#define OLED_PAGE_HEIGHT        8       /* Page height in pixels */
#define OLED_NUM_PAGES          8       /* Number of pages */
#define OLED_BUFFER_SIZE        (OLED_WIDTH * OLED_HEIGHT / 8) /* Frame buffer size */

#define OLED_I2C_ADDRESS        0x3C    /* Default I²C address */
#define OLED_DEFAULT_CONTRAST   0x7F    /* Default contrast level */

typedef enum {
    OLED_FONT_5x7     = 0,    /* 5x7 pixel font */
    OLED_FONT_8x16    = 1,    /* 8x16 pixel font */
    OLED_FONT_6x8     = 2,    /* 6x8 pixel font */
    OLED_FONT_12x16   = 3,    /* 12x16 pixel font */
} oled_font_t;

typedef enum {
    OLED_COLOR_BLACK  = 0,    /* Pixel off */
    OLED_COLOR_WHITE  = 1,    /* Pixel on */
    OLED_COLOR_INVERT = 2,    /* Invert pixel */
} oled_color_t;

typedef enum {
    OLED_ROTATION_0   = 0,    /* No rotation */
    OLED_ROTATION_90  = 1,    /* 90 degrees clockwise */
    OLED_ROTATION_180 = 2,    /* 180 degrees */
    OLED_ROTATION_270 = 3,    /* 270 degrees clockwise */
} oled_rotation_t;

/* OLED Configuration */
typedef struct {
    uint8_t i2c_address;              /* I²C address (usually 0x3C or 0x3D) */
    uint8_t contrast;                 /* Contrast level (0-255) */
    oled_rotation_t rotation;         /* Display rotation */
    bool enable_charge_pump;          /* Enable internal charge pump */
    bool invert_display;              /* Invert display colors */
    bool enable_scroll;               /* Enable scrolling */
    uint32_t update_rate_hz;          /* Display update rate */
    uint32_t screen_saver_timeout_sec; /* Screen saver timeout */
} oled_config_t;

/* ============================================================================
 * OLED Display Status
 * ============================================================================ */

typedef enum {
    OLED_STATUS_DISCONNECTED = 0,    /* Display not connected */
    OLED_STATUS_INITIALIZING = 1,    /* Display initializing */
    OLED_STATUS_READY        = 2,    /* Display ready for use */
    OLED_STATUS_ERROR        = 3,    /* Display error */
    OLED_STATUS_SLEEP        = 4,    /* Display in sleep mode */
} oled_status_t;

typedef struct {
    oled_status_t status;             /* Current display status */
    uint32_t display_time_sec;        /* Total display active time */
    uint32_t updates_count;           /* Total display updates */
    uint32_t errors_count;            /* Total errors */
    uint8_t contrast_level;           /* Current contrast level */
    bool screen_saver_active;         /* Screen saver active */
    uint32_t last_update_time;        /* Last update timestamp */
} oled_display_status_t;

/* ============================================================================
 * Graphics and Drawing Functions
 * ============================================================================ */

/**
 * @brief Initialize OLED display
 *
 * @param config Pointer to OLED configuration
 * @return true if initialization successful, false otherwise
 */
bool oled_display_init(const oled_config_t *config);

/**
 * @brief Deinitialize OLED display
 *
 * @return true if deinitialization successful, false otherwise
 */
bool oled_display_deinit(void);

/**
 * @brief Get OLED display status
 *
 * @param status Pointer to status structure to fill
 * @return true if status retrieved successfully, false otherwise
 */
bool oled_display_get_status(oled_display_status_t *status);

/**
 * @brief Clear the display
 *
 * @return true if cleared successfully, false otherwise
 */
bool oled_display_clear(void);

/**
 * @brief Update the display with current buffer contents
 *
 * @return true if updated successfully, false otherwise
 */
bool oled_display_update(void);

/**
 * @brief Set display contrast
 *
 * @param contrast Contrast level (0-255)
 * @return true if contrast set successfully, false otherwise
 */
bool oled_display_set_contrast(uint8_t contrast);

/**
 * @brief Set display rotation
 *
 * @param rotation Display rotation
 * @return true if rotation set successfully, false otherwise
 */
bool oled_display_set_rotation(oled_rotation_t rotation);

/**
 * @brief Invert display colors
 *
 * @param invert true to invert, false for normal
 * @return true if inversion set successfully, false otherwise
 */
bool oled_display_invert(bool invert);

/**
 * @brief Put display to sleep
 *
 * @return true if sleep mode entered successfully, false otherwise
 */
bool oled_display_sleep(void);

/**
 * @brief Wake display from sleep
 *
 * @return true if wake successful, false otherwise
 */
bool oled_display_wake(void);

/**
 * @brief Draw a pixel at coordinates
 *
 * @param x X coordinate (0-127)
 * @param y Y coordinate (0-63)
 * @param color Pixel color
 * @return true if pixel drawn successfully, false otherwise
 */
bool oled_draw_pixel(uint8_t x, uint8_t y, oled_color_t color);

/**
 * @brief Draw a line between two points
 *
 * @param x0 Starting X coordinate
 * @param y0 Starting Y coordinate
 * @param x1 Ending X coordinate
 * @param y1 Ending Y coordinate
 * @param color Line color
 * @return true if line drawn successfully, false otherwise
 */
bool oled_draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, oled_color_t color);

/**
 * @brief Draw a rectangle
 *
 * @param x X coordinate of top-left corner
 * @param y Y coordinate of top-left corner
 * @param width Rectangle width
 * @param height Rectangle height
 * @param color Rectangle color
 * @param filled true for filled rectangle, false for outline
 * @return true if rectangle drawn successfully, false otherwise
 */
bool oled_draw_rectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height,
                        oled_color_t color, bool filled);

/**
 * @brief Draw a circle
 *
 * @param x X coordinate of center
 * @param y Y coordinate of center
 * @param radius Circle radius
 * @param color Circle color
 * @param filled true for filled circle, false for outline
 * @return true if circle drawn successfully, false otherwise
 */
bool oled_draw_circle(uint8_t x, uint8_t y, uint8_t radius, oled_color_t color, bool filled);

/**
 * @brief Draw a bitmap
 *
 * @param x X coordinate of top-left corner
 * @param y Y coordinate of top-left corner
 * @param width Bitmap width in pixels
 * @param height Bitmap height in pixels
 * @param bitmap Pointer to bitmap data (1 bit per pixel, MSB first)
 * @param color Bitmap color
 * @return true if bitmap drawn successfully, false otherwise
 */
bool oled_draw_bitmap(uint8_t x, uint8_t y, uint8_t width, uint8_t height,
                     const uint8_t *bitmap, oled_color_t color);

/* ============================================================================
 * Text and Font Functions
 * ============================================================================ */

/**
 * @brief Set current font
 *
 * @param font Font to use
 * @return true if font set successfully, false otherwise
 */
bool oled_set_font(oled_font_t font);

/**
 * @brief Get current font
 *
 * @return Current font
 */
oled_font_t oled_get_font(void);

/**
 * @brief Draw a single character
 *
 * @param x X coordinate
 * @param y Y coordinate
 * @param c Character to draw
 * @param color Character color
 * @return Width of character drawn
 */
uint8_t oled_draw_char(uint8_t x, uint8_t y, char c, oled_color_t color);

/**
 * @brief Draw a string of text
 *
 * @param x X coordinate
 * @param y Y coordinate
 * @param str String to draw
 * @param color Text color
 * @return Width of string drawn
 */
uint8_t oled_draw_string(uint8_t x, uint8_t y, const char *str, oled_color_t color);

/**
 * @brief Draw formatted text
 *
 * @param x X coordinate
 * @param y Y coordinate
 * @param color Text color
 * @param format Format string (printf style)
 * @param ... Format arguments
 * @return Width of text drawn
 */
uint8_t oled_draw_printf(uint8_t x, uint8_t y, oled_color_t color, const char *format, ...);

/**
 * @brief Get text width for a string
 *
 * @param str String to measure
 * @return Width of string in pixels
 */
uint8_t oled_get_string_width(const char *str);

/**
 * @brief Get font height
 *
 * @return Font height in pixels
 */
uint8_t oled_get_font_height(void);

/* ============================================================================
 * Menu System Functions
 * ============================================================================ */

typedef enum {
    OLED_MENU_ITEM_TEXT    = 0,    /* Text-only menu item */
    OLED_MENU_ITEM_VALUE   = 1,    /* Menu item with value */
    OLED_MENU_ITEM_CHECKBOX = 2,   /* Menu item with checkbox */
    OLED_MENU_ITEM_SUBMENU = 3,    /* Menu item with submenu */
} oled_menu_item_type_t;

typedef struct {
    const char *label;              /* Menu item label */
    oled_menu_item_type_t type;     /* Menu item type */
    void *value;                    /* Pointer to value (for VALUE and CHECKBOX types) */
    const char **options;           /* Options array (for VALUE type) */
    uint8_t num_options;            /* Number of options */
    void (*callback)(void);         /* Callback function when selected */
    bool enabled;                   /* Menu item enabled/disabled */
} oled_menu_item_t;

typedef struct {
    const char *title;              /* Menu title */
    oled_menu_item_t *items;        /* Array of menu items */
    uint8_t num_items;              /* Number of menu items */
    uint8_t selected_item;          /* Currently selected item */
    uint8_t first_visible_item;     /* First visible item (for scrolling) */
    uint8_t max_visible_items;      /* Maximum visible items */
} oled_menu_t;

/**
 * @brief Initialize menu system
 *
 * @return true if initialization successful, false otherwise
 */
bool oled_menu_init(void);

/**
 * @brief Create a new menu
 *
 * @param menu Pointer to menu structure to initialize
 * @param title Menu title
 * @param items Array of menu items
 * @param num_items Number of menu items
 * @return true if menu created successfully, false otherwise
 */
bool oled_menu_create(oled_menu_t *menu, const char *title,
                     oled_menu_item_t *items, uint8_t num_items);

/**
 * @brief Display a menu
 *
 * @param menu Pointer to menu to display
 * @return true if menu displayed successfully, false otherwise
 */
bool oled_menu_display(const oled_menu_t *menu);

/**
 * @brief Navigate menu up
 *
 * @param menu Pointer to menu
 * @return true if navigation successful, false otherwise
 */
bool oled_menu_navigate_up(oled_menu_t *menu);

/**
 * @brief Navigate menu down
 *
 * @param menu Pointer to menu
 * @return true if navigation successful, false otherwise
 */
bool oled_menu_navigate_down(oled_menu_t *menu);

/**
 * @brief Select current menu item
 *
 * @param menu Pointer to menu
 * @return true if selection successful, false otherwise
 */
bool oled_menu_select(oled_menu_t *menu);

/**
 * @brief Go back to parent menu
 *
 * @param menu Pointer to menu
 * @return true if back navigation successful, false otherwise
 */
bool oled_menu_back(oled_menu_t *menu);

/* ============================================================================
 * Status Display Functions
 * ============================================================================ */

/**
 * @brief Display system status screen
 *
 * @param system_name System name
 * @param status_text Status text
 * @param ip_address IP address string
 * @param uptime_seconds System uptime in seconds
 * @return true if status displayed successfully, false otherwise
 */
bool oled_display_status_screen(const char *system_name, const char *status_text,
                               const char *ip_address, uint32_t uptime_seconds);

/**
 * @brief Display sensor data screen
 *
 * @param sensor_name Sensor name
 * @param sensor_value Sensor value
 * @param sensor_unit Unit of measurement
 * @param sensor_status Sensor status string
 * @return true if sensor data displayed successfully, false otherwise
 */
bool oled_display_sensor_screen(const char *sensor_name, float sensor_value,
                               const char *sensor_unit, const char *sensor_status);

/**
 * @brief Display network information screen
 *
 * @param wifi_ssid WiFi SSID
 * @param wifi_signal WiFi signal strength (0-100)
 * @param ip_address IP address
 * @param connection_status Connection status string
 * @return true if network info displayed successfully, false otherwise
 */
bool oled_display_network_screen(const char *wifi_ssid, uint8_t wifi_signal,
                                const char *ip_address, const char *connection_status);

/**
 * @brief Display error message screen
 *
 * @param error_title Error title
 * @param error_message Error message
 * @param error_code Error code
 * @return true if error displayed successfully, false otherwise
 */
bool oled_display_error_screen(const char *error_title, const char *error_message,
                              uint32_t error_code);

/**
 * @brief Display progress screen
 *
 * @param title Progress title
 * @param progress Progress percentage (0-100)
 * @param status_text Status text
 * @return true if progress displayed successfully, false otherwise
 */
bool oled_display_progress_screen(const char *title, uint8_t progress, const char *status_text);

/**
 * @brief Display logo screen
 *
 * @param logo_data Pointer to logo bitmap data
 * @param logo_width Logo width
 * @param logo_height Logo height
 * @param company_name Company name
 * @param product_name Product name
 * @return true if logo displayed successfully, false otherwise
 */
bool oled_display_logo_screen(const uint8_t *logo_data, uint8_t logo_width, uint8_t logo_height,
                             const char *company_name, const char *product_name);

/* ============================================================================
 * Animation and Effects Functions
 * ============================================================================ */

/**
 * @brief Scroll text horizontally
 *
 * @param text Text to scroll
 * @param y Y coordinate for scrolling
 * @param speed Scroll speed (pixels per frame)
 * @param color Text color
 * @return true if scrolling successful, false otherwise
 */
bool oled_scroll_text(const char *text, uint8_t y, uint8_t speed, oled_color_t color);

/**
 * @brief Display loading animation
 *
 * @param x X coordinate of animation center
 * @param y Y coordinate of animation center
 * @param radius Animation radius
 * @param color Animation color
 * @return true if animation displayed successfully, false otherwise
 */
bool oled_display_loading_animation(uint8_t x, uint8_t y, uint8_t radius, oled_color_t color);

/**
 * @brief Fade display to black
 *
 * @param duration_ms Fade duration in milliseconds
 * @return true if fade successful, false otherwise
 */
bool oled_fade_out(uint16_t duration_ms);

/**
 * @brief Fade display from black
 *
 * @param duration_ms Fade duration in milliseconds
 * @return true if fade successful, false otherwise
 */
bool oled_fade_in(uint16_t duration_ms);

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

/**
 * @brief Get display width
 *
 * @return Display width in pixels
 */
uint8_t oled_get_width(void);

/**
 * @brief Get display height
 *
 * @return Display height in pixels
 */
uint8_t oled_get_height(void);

/**
 * @brief Check if point is within display bounds
 *
 * @param x X coordinate
 * @param y Y coordinate
 * @return true if point is within bounds, false otherwise
 */
bool oled_is_within_bounds(uint8_t x, uint8_t y);

/**
 * @brief Get display buffer pointer
 *
 * @return Pointer to display buffer
 */
uint8_t *oled_get_buffer(void);

/**
 * @brief Set display buffer (for custom rendering)
 *
 * @param buffer Pointer to new buffer
 * @param size Buffer size
 * @return true if buffer set successfully, false otherwise
 */
bool oled_set_buffer(uint8_t *buffer, uint16_t size);

/**
 * @brief Save current display buffer
 *
 * @param buffer Pointer to buffer for saving
 * @param size Buffer size
 * @return true if buffer saved successfully, false otherwise
 */
bool oled_save_buffer(uint8_t *buffer, uint16_t size);

/**
 * @brief Restore display buffer
 *
 * @param buffer Pointer to buffer for restoring
 * @param size Buffer size
 * @return true if buffer restored successfully, false otherwise
 */
bool oled_restore_buffer(const uint8_t *buffer, uint16_t size);

/**
 * @brief Get OLED display statistics
 *
 * @param updates Pointer to store number of updates
 * @param errors Pointer to store number of errors
 * @param uptime Pointer to store uptime in seconds
 * @return true if statistics retrieved successfully, false otherwise
 */
bool oled_get_statistics(uint32_t *updates, uint32_t *errors, uint32_t *uptime);

/**
 * @brief Clear OLED display statistics
 *
 * @return true if statistics cleared successfully, false otherwise
 */
bool oled_clear_statistics(void);

/**
 * @brief Enable/disable screen saver
 *
 * @param enable true to enable, false to disable
 * @param timeout_seconds Screen saver timeout
 * @return true if screen saver configured successfully, false otherwise
 */
bool oled_enable_screen_saver(bool enable, uint32_t timeout_seconds);

/**
 * @brief Reset screen saver timer
 *
 * @return true if timer reset successfully, false otherwise
 */
bool oled_reset_screen_saver_timer(void);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_OLED_DISPLAY_H */
