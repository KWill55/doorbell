#include <dirent.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "lib/buttons.h"
#include "lib/camera.h"
#include "lib/client.h"
#include "lib/colors.h"
#include "lib/device.h"
#include "lib/display.h"
#include "lib/fonts/fonts.h"
#include "lib/image.h"
#include "lib/log.h"

volatile bool thread_status = false;

#define VIEWER_FOLDER "viewer/"
#define MAX_ENTRIES 8
#define MAX_TEXT_SIZE 400
#define MAX_FILE_NAME 100
#define SIZEOFYOURBUFFER (DISPLAY_WIDTH * DISPLAY_HEIGHT * 3) // times bytes per pixel?

// Colors — Feel free to change these to fit your preference
#define BACKGROUND_COLOR WHITE
#define FONT_COLOR BLACK
#define SELECTED_BG_COLOR BYU_BLUE
#define SELECTED_FONT_COLOR BYU_LIGHT_SAND

// Makes sure to deinitialize everything before program close
void intHandler(int dummy) {
    log_info("Exiting...");
    display_exit();
    exit(0);
}

/*
 * Takes in a folder, reads the contents of the folder, filtering out any files that do not end with
 * .log or .bmp. This function should check to make sure the folder exists. It fills in the entries
 * array with all of the entries in the folder, up to 8 (MAX_ENTRIES). The function returns the
 * number of entries it put into the entries array.
 */
// int get_entries(char *folder, char entries[][MAX_FILE_NAME]) { what they had
int get_entries(char *folder, char entries[MAX_ENTRIES][MAX_FILE_NAME]) {
    DIR *dp = opendir(folder); // directory pointer dp
    struct dirent *entry;
    int counter = 0;

    // Does the folder exist?
    if (dp == NULL) {
        printf("Folder doesn't work\n");
        return -1;
    }

    // For file in Folder
    while ((entry = readdir(dp)) != NULL) {
        size_t str_len = strlen(entry->d_name);
        const char *suffix = entry->d_name + str_len - 4;

        // does it end with .bmp or .log?
        if (str_len > 3 && (strncmp(suffix, ".bmp", 4) == 0 || strncmp(suffix, ".log", 4) == 0)) {
            strcpy(entries[counter], entry->d_name);
            counter++;
        }
    }
    closedir(dp);
    return counter;
}

/*
 * Draws the menu of the screen. It uses the entries array to create the menu, with the num_entries
 * specifying how many entries are in the entries array. The selected parameter is the item in the
 * menu that is selected and should be highlighted. Use BACKGROUND_COLOR, FONT_COLOR,
 * SELECTED_BG_COLOR, and SELECTED_FONT_COLOR to help specify the colors of the background, font,
 * select bar color, and selected text color.
 */
// void draw_menu(char entries[][MAX_FILE_NAME], int num_entries, int selected) {
void draw_secret_menu(char entries[][MAX_FILE_NAME], int num_entries, int selected) {
    // text initialization
    uint16_t x_start = 10;
    float y_start = -1 * (0.5 * ROW_HEIGHT) - 3;
    const char *str = "";
    sFONT *font;
    font = &Font8;
    uint16_t background_color = BACKGROUND_COLOR;
    uint16_t foreground_color = FONT_COLOR;

    // selection bar initialization
    uint16_t x_end = 128;
    uint16_t color = SELECTED_BG_COLOR;
    bool filled = 1;
    uint8_t line_weight = 1;

    for (int i = 0; i < num_entries; i++) {
        str = entries[i];
        y_start += ROW_HEIGHT;

        // draw selected portion
        if (i == selected) {
            // display_draw_rectangle(x_start, y_start, x_end, y_end, color, filled, line_weight);
            display_draw_rectangle(0, y_start - 3, x_end, 3 + y_start + (0.5) * ROW_HEIGHT, color,
                                   filled, line_weight);

            background_color = SELECTED_BG_COLOR;
            foreground_color = SELECTED_FONT_COLOR;
            display_draw_string(x_start, y_start, str, font, background_color, foreground_color);
            continue;
        }

        background_color = BACKGROUND_COLOR;
        foreground_color = FONT_COLOR;
        // draw everything else
        display_draw_string(x_start, y_start, str, font, background_color, foreground_color);
    }
}

/*
 * Displays an image or a log file. This function detects the type of file that should be draw. If
 * it is a bmp file, then it calls display_draw_image. If it is a log file, it opens the file, reads
 * 100 characters (MAX_TEXT_SIZE), and displays the text using display_draw_string. Combine folder
 * and file_name to get the complete file path.
 */
void draw_file(char *folder, char *file_name) {
    // Copy your code from the previous lab
}

void *send_image(void *config) {
    Config *config_ptr = (Config *)config;

    // allocate space for image data
    uint8_t *my_new_buf = malloc(sizeof(uint8_t) * IMG_SIZE);
    if (my_new_buf == NULL) {
        printf("Memory allocation failed\n");
        return NULL;
    }

    size_t bufsize = IMG_SIZE;

    char new_photo_name[100];
    strcpy(new_photo_name, "viewer/");
    strcat(new_photo_name, "doorbell.bmp");

    camera_capture_data(my_new_buf, bufsize);
    camera_save_to_file(my_new_buf, bufsize, "viewer/doorbell.bmp"); // new_photo_name);

    Bitmap bmp;
    create_bmp(&bmp, my_new_buf);
    // display_draw_image_data(bmp.pxl_data, bmp.img_width, bmp.img_height);
    // delay_ms(2000);

    Config config_local = {
        .port = "2240",
        .host = "ecen224.byu.edu",
        .payload = NULL,
        .hw_id = "C3CC0A24D",
        .payload_size = strlen("C3CC0A24D") + IMG_SIZE,
    };

    /* combines config.hw_id and new_buf into combined_data
    and stores it as config.payload*/
    uint8_t *combined_data = malloc(config_local.payload_size);
    if (combined_data == NULL) {
        printf("Memory alloaction failed\n:");
        free(my_new_buf);
        return NULL;
    }

    memcpy(combined_data, config_local.hw_id, strlen(config_local.hw_id));
    memcpy(combined_data + strlen(config_local.hw_id), my_new_buf, IMG_SIZE);

    config_local.payload = combined_data;

    int socketfd = client_connect(&config_local);
    client_send_image(socketfd, &config_local);
    client_receive_response(socketfd);
    client_close(socketfd);

    // free stuff
    free(config_local.payload);
    free(my_new_buf);
    destroy_bmp(&bmp);
    // free(config_ptr->payload); // Free the payload memory allocated in main thread
    return NULL;
}

void draw_greeting() {
    display_clear(WHITE);
    const char *str = "Welcome!";
    uint16_t x_start = DISPLAY_WIDTH / 2 - strlen(str) * 5 * .5; // char = 5 pixels
    uint16_t y_start = DISPLAY_HEIGHT / 2;
    sFONT *font;
    font = &Font8;
    uint16_t background_color = WHITE;
    uint16_t foreground_color = BLUE;
    display_draw_string(x_start, y_start, str, font, background_color, foreground_color);
    delay_ms(2000);
}

int main(void) {
    sleep(15);
    signal(SIGINT, intHandler);
    log_info("Starting...");
    // TODO: Initialize the hardware
    display_init();
    buttons_init();

    // print greeting to screen
    draw_greeting();

    Config config = {
        .port = "2240",
        .host = "ecen224.byu.edu",
        .payload = NULL,
        .hw_id = "C3CC0A24D",
        .payload_size = strlen("C3CC0A24D") + IMG_SIZE,
    };
    int secret_counter = 0;
    while (true) {
        delay_ms(200);

        // TODO: Put button logic here
        if (button_center() == 0) {
            // Do something upon detecting button press
            display_clear(WHITE);

            // print doorbell rung to screen
            const char *str = "Doorbell rung!";
            display_draw_string(DISPLAY_WIDTH / 2 - strlen(str) * 5 * .5, DISPLAY_HEIGHT / 2, str,
                                &Font8, WHITE, BLUE);
            delay_ms(2000);
            display_clear(WHITE);

            // return to welcom screen
            draw_greeting();

            // start thread to capture and send image
            pthread_t my_thread;
            pthread_create(&my_thread, NULL, send_image, (void *)&config);
            log_info("after thread sends image");
        }
        // int secretCode[] = {button_up, button_up, button_down};

        if (button_up() == 0) {
            if (secret_counter == 1) {
                secret_counter = 2;
                log_info("Value of secret_counter: %d", secret_counter);
            } else {
                secret_counter = 1;
                log_info("Value of secret_counter: %d", secret_counter);
            }
        } else {
            if (secret_counter == 2) {
                if (button_down() == 0) {
                    log_info("Open up secret menu");
                    char entries[MAX_ENTRIES][MAX_FILE_NAME];
                    int num_entries =
                        get_entries(VIEWER_FOLDER, entries); //[MAX_ENTRIES][MAX_FILE_NAME]);
                    display_clear(WHITE);
                    int selected = 0;
                    draw_secret_menu(entries, num_entries, selected);
                    while (true) {
                        delay_ms(200);
                        if (button_up() == 0) {
                            display_clear(WHITE);
                            selected -= 1;
                            if (selected < 0) {
                                selected = num_entries - 1;
                            }
                            draw_secret_menu(entries, num_entries, selected);
                            while (button_up() == 0) {
                                // Do something while the button is pressed
                                delay_ms(1);
                            }
                        }
                        if (button_down() == 0) {
                            display_clear(WHITE);
                            selected += 1;
                            if (selected > num_entries - 1) {
                                selected = 0;
                            }
                            display_clear(WHITE);
                            draw_secret_menu(entries, num_entries, selected);

                            while (button_down() == 0) {
                                // Do something while the button is pressed
                                delay_ms(1);
                            }
                        }
                        if (button_right() == 0) {
                            // Do something upon detecting button press
                            display_clear(WHITE);
                            char *my_file = entries[selected];
                            size_t str_len = strlen(my_file);
                            const char *suffix = my_file + str_len - 4;

                            if (strncmp(suffix, ".bmp", 4) == 0) {
                                // display image for two seconds
                                char my_complete_file[100];
                                strcpy(my_complete_file, "viewer/");
                                strcat(my_complete_file, my_file);
                                // string my_complete_file = strcat("/viewer", my_file);
                                display_draw_image(my_complete_file);
                                delay_ms(2000);

                                display_clear(WHITE);
                                draw_secret_menu(entries, num_entries, selected);
                            }

                            if (strncmp(suffix, ".log", 4) == 0) {
                                // display contents for 2 seconds
                                char my_complete_file[100];
                                strcpy(my_complete_file, "viewer/");
                                strcat(my_complete_file, my_file);

                                FILE *fp = fopen(my_complete_file, "r");
                                if (fp != NULL) {
                                    char log_content[MAX_TEXT_SIZE];
                                    memset(log_content, 0, sizeof(log_content));
                                    fread(log_content, sizeof(char), MAX_TEXT_SIZE, fp);
                                    display_draw_string(5, 5, log_content, &Font8, BACKGROUND_COLOR,
                                                        FONT_COLOR);
                                    delay_ms(2000);
                                    fclose(fp);

                                    display_clear(WHITE);
                                    draw_secret_menu(entries, num_entries, selected);
                                }

                                display_clear(WHITE);
                                draw_secret_menu(entries, num_entries, selected);
                            }
                        }
                        if (button_left() == 0) {
                            draw_greeting();
                            break;
                        }
                    }
                }
            }

            if (button_up() == 0 || button_down() == 0 || button_left() == 0 ||
                button_right() == 0 || button_center() == 0) {
                secret_counter = 0;
            }
            // log_info("Value of secret_counter: %d", secret_counter);
        }
    }
    display_clear(WHITE);
    log_info("It shouldn't go here");
    return 0;
}

// problems to fix::::

// make the counter reset when you enter something wrong

// do thge service file stuff

// cd /etc/systemd/system right?