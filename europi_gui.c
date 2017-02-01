// Copyright 2016 Richard R. Goodwin / Audio Morphology
//
// Author: Richard R. Goodwin (richard.goodwin@morphology.co.uk)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.

#include <stdio.h>
 
#include "europi.h"
#include "../raylib/release/rpi/raylib.h"

extern int clock_freq;
extern int prog_running;
extern Vector2 touchPosition;
extern int currentGesture1;
extern int lastGesture;
extern menu Menu[]; 
extern char input_txt[]; 
extern Texture2D KeyboardTexture;
extern Texture2D DialogTexture;
extern Texture2D TextInputTexture;
extern Texture2D MainScreenTexture;
extern Texture2D TopBarTexture;
extern Texture2D ButtonBarTexture; 
extern char *kbd_chars[4][11];
extern int kbd_char_selected;
extern enum encoder_focus_t encoder_focus;
extern enum btnA_func_t btnA_func;
extern enum btnB_func_t btnB_func;
extern enum btnC_func_t btnC_func;
extern enum btnD_func_t btnD_func;
extern int btnA_state;
extern int btnB_state;
extern int btnC_state;
extern int btnD_state;
extern struct screen_overlays ScreenOverlays;
extern enum display_page_t DisplayPage;
extern struct europi Europi;
extern char **files;
extern size_t file_count;                      
extern int file_selected;
extern int first_file;

/*
 * GUI_8x8 Attempts to display more detail in a subset of tracks and steps
 * 8 Tracks x 8 Steps
 */
void gui_8x8(void){
    Rectangle stepRectangle = {0,0,0,0};
    int track, column;
    int step, offset, txt_len;
    char txt[20]; 

    BeginDrawing();
    //ClearBackground(RAYWHITE);
    DrawTexture(MainScreenTexture,0,0,WHITE);
    for(track = 0; track < 8; track++){
        // for each track, we need to know where the 
        // current step in the sequence is for that
        // particular track, and display the appropriate
        // 8 steps that contain the current step
        offset = Europi.tracks[track].current_step / 8;
        // Display the step offset at the start of each track
        sprintf(txt,"%d",offset);
        txt_len = MeasureText(txt,20);
        DrawText(txt,20-txt_len,10+(track * 25),20,DARKGRAY);
        // Check for doube-tap on the Track Number
        stepRectangle.x = 20-txt_len;
        stepRectangle.y = 10+(track * 25);
        stepRectangle.width = txt_len; 
        stepRectangle.height = 20;
        if (CheckCollisionPointRec(touchPosition, stepRectangle) && (currentGesture1 == GESTURE_TAP)){
            
            //if (currentGesture != lastGesture){
                // Open this Track in isolation
                //currentGesture = GESTURE_NONE;
                ClearScreenOverlays();
                DisplayPage = SingleChannel;
                select_track(track);
            //}
        }
        for(column=0;column<8;column++){
            stepRectangle.x = 30 + (column * 25);
            stepRectangle.y = 10 + (track * 25);
            stepRectangle.width = 22;
            stepRectangle.height = 22;
            // Check gesture collision
            if (CheckCollisionPointRec(touchPosition, stepRectangle) && (currentGesture1 != GESTURE_NONE)){
                // Paint this step Blue
                DrawRectangleRec(stepRectangle, BLUE); 
            }
            else {
                if((offset*8)+column == Europi.tracks[track].last_step){
                    // Paint last step
                    DrawRectangleRec(stepRectangle, BLACK); 
                }
                else if((offset*8)+column == Europi.tracks[track].current_step){
                    // Paint current step
                    DrawRectangleRec(stepRectangle, LIME);   
                }
                else {
                    // paint blank step
                    DrawRectangleRec(stepRectangle, MAROON); 
                }
            } 
        }        
    }
    // Handle any screen overlays - these need to 
    // be added within the Drawing loop
    ShowScreenOverlays();
    EndDrawing();
}

/*
 * GUI_GRID Displays a grid of 32 steps by 24 channels - a bit
 * too dense to be useable on a TFT display, but an interesting
 * experiment in displaying a lot of data on a small screen
 */
void gui_grid(void){
    ClearBackground(RAYWHITE);
    int track;
    int step;
    char track_no[20];
    int txt_len;
    for(track=0;track<24;track++){
        // Track Number
        sprintf(track_no,"%d",track+1);
        txt_len = MeasureText(track_no,10);
        DrawText(track_no,12-txt_len,track * 10,10,DARKGRAY);
        for(step=0;step<32;step++){
            if(step == Europi.tracks[track].last_step){
                // Paint last step
                DrawRectangle(15 + (step * 9), track * 10, 8, 9, BLACK); 
            }
            else if(step == Europi.tracks[track].current_step){
                // Paint current step
                DrawRectangle(15 + (step * 9), track * 10, 8, 9, LIME); 
                // Gate state for current step
                if (Europi.tracks[track].channels[GATE_OUT].steps[Europi.tracks[track].current_step].gate_value == 1){
                    if (Europi.tracks[track].channels[GATE_OUT].steps[Europi.tracks[track].current_step].retrigger > 0) {
                        DrawRectangle(15 + (32 * 9), track * 10, 8, 9, BLACK);	
                    }
                    else {
                        DrawRectangle(15 + (32 * 9), track * 10, 8, 9, VIOLET);	
                    }	
                }
                else {
                    DrawRectangle(15 + (32 * 9), track * 10, 8, 9, WHITE);	
                }
            }
            else {
                // paint blank step
                DrawRectangle(15 + (step * 9), track * 10, 8, 9, MAROON); 
            }
        }
    }
}
/*
 * gui_SingleChannel displays just the currently
 * selected channel on a page of its own
 */
void gui_SingleChannel(void){
    int track;
    int step;
    int val;
    char track_no[20];
    int txt_len;
    Rectangle stepRectangle = {0,0,0,0};
    BeginDrawing();
    ClearBackground(RAYWHITE);
    for (track = 0; track < MAX_TRACKS; track++){
        if (Europi.tracks[track].selected == TRUE){
            sprintf(track_no,"%d",track+1);
            txt_len = MeasureText(track_no,10);
            DrawText(track_no,12-txt_len,220,10,DARKGRAY);
            for (step = 0; step < MAX_STEPS; step++){
                if(step < Europi.tracks[track].last_step){
                    val = (int)(((float)Europi.tracks[track].channels[CV_OUT].steps[step].scaled_value / (float)60000) * 220);
                    if(step == Europi.tracks[track].current_step){
                        DrawRectangle(15 + (step*9),220-val,8,val,MAROON);
                    }
                    else{
                        DrawRectangle(15 + (step*9),220-val,8,val,LIME);
                    }
                    // Gate State
                    if (Europi.tracks[track].channels[GATE_OUT].steps[step].gate_value == 1){
                        sprintf(track_no,"%d",Europi.tracks[track].channels[GATE_OUT].steps[step].retrigger);
                        DrawText(track_no,15 + (step*9),220,10,DARKGRAY);
                    }
                    // Slew
                    if (Europi.tracks[track].channels[CV_OUT].steps[step].slew_type != Off){
                        switch (Europi.tracks[track].channels[CV_OUT].steps[step].slew_shape){
                            case Both:
                                DrawText("V",15 + (step*9),230,10,DARKGRAY);
                            break;
                            case Rising:
                                DrawText("/",15 + (step*9),230,10,DARKGRAY);
                            break;
                            case Falling:
                                DrawText("\\",15 + (step*9),230,10,DARKGRAY);
                            break;
                        }
                    }
                }
            }
        }
    }
    // Handle any screen overlays - these need to 
    // be added within the Drawing loop
    ShowScreenOverlays();
    EndDrawing();
}

/*
 * ShowScreenOverlays is called from within the 
 * drawing loop of each main type of display
 * page, and enables Menus, user input controls
 * etc to be overlayed on top of the currently
 * displayed page
 */
void ShowScreenOverlays(void){
    if(ScreenOverlays.MainMenu == 1){
        ClearScreenOverlays();
        ScreenOverlays.MainMenu = 1;
        gui_MainMenu();
    }
    if(ScreenOverlays.SetLoop == 1){
        int track = 0;
        char str[80];
        for(track = 0; track < MAX_TRACKS; track++) {
            if (Europi.tracks[track].selected == TRUE){
                if(encoder_focus == track_select){
                    sprintf(str,"Track: [%02d] Loop Point: %02d",track+1,Europi.tracks[track].last_step);
                }
                else if (encoder_focus == set_loop) {
                    sprintf(str,"Track: %02d Loop Point: [%02d]",track+1,Europi.tracks[track].last_step);
                }
                DrawRectangle(20, 30, 250, 20, BLACK);
                DrawText(str,25,35,10,WHITE);
            }
        }
    }
        if(ScreenOverlays.SetPitch == 1){ 
        int track = 0;
        char str[80];
        for(track = 0; track < MAX_TRACKS; track++) {
            if (Europi.tracks[track].selected == TRUE){
                if(encoder_focus == track_select){
                    sprintf(str,"Track: [%02d] Step: %02d Pitch: %05d",track+1,Europi.tracks[track].current_step+1,Europi.tracks[track].channels[CV_OUT].steps[Europi.tracks[track].current_step].raw_value);
                }
                else if (encoder_focus == step_select) {
                    sprintf(str,"Track: %02d Step: [%02d] Pitch: %05d",track+1,Europi.tracks[track].current_step+1,Europi.tracks[track].channels[CV_OUT].steps[Europi.tracks[track].current_step].raw_value);
                }
                else if (encoder_focus == set_pitch){
                    sprintf(str,"Track: %02d Step: %02d Pitch: [%05d]",track+1,Europi.tracks[track].current_step+1,Europi.tracks[track].channels[CV_OUT].steps[Europi.tracks[track].current_step].raw_value);
                }
                DrawRectangle(20, 30, 250, 20, BLACK);
                DrawText(str,25,35,10,WHITE);
            }
        }
    }
    if(ScreenOverlays.SetQuantise == 1){
        int track = 0;
        char str[80];
        for(track = 0; track < MAX_TRACKS; track++) {
            if (Europi.tracks[track].selected == TRUE){
                if(encoder_focus == track_select){
                    sprintf(str,"Track: [%02d] Quantisation: %s",track+1,scale_names[Europi.tracks[track].channels[CV_OUT].quantise]);
                }
                else if (encoder_focus == set_quantise) {
                    sprintf(str,"Track: %02d Quantisation: [%s]",track+1,scale_names[Europi.tracks[track].channels[CV_OUT].quantise]);
                }
                DrawRectangle(20, 30, 250, 20, BLACK);
                DrawText(str,25,35,10,WHITE);
            }
        }
    }
    
    
    if(ScreenOverlays.Keyboard == 1){
//        lastGesture = currentGesture;
        Rectangle btnHighlight = {0,0,0,0};
//        touchPosition = GetTouchPosition(0);
//        currentGesture = GetGestureDetected();
        int button;
        int row, col;
        DrawTexture(KeyboardTexture,KBD_GRID_TL_X,KBD_GRID_TL_Y,WHITE);
        for(button=0;button < (KBD_ROWS * KBD_COLS);button++){
            row = button / KBD_COLS;
            col = button % KBD_COLS;
            btnHighlight.x = (KBD_GRID_TL_X + KBD_BTN_TL_X) + (col * KBD_COL_WIDTH);
            btnHighlight.y = (KBD_GRID_TL_Y + KBD_BTN_TL_Y) + (row * KBD_ROW_HEIGHT);
            btnHighlight.width = KBD_BTN_WIDTH;
            btnHighlight.height = KBD_BTN_HEIGHT;
            if(button == kbd_char_selected){
                //Highlight this button
                DrawRectangleLines((KBD_GRID_TL_X + KBD_BTN_TL_X) + (col * KBD_COL_WIDTH),
                (KBD_GRID_TL_Y + KBD_BTN_TL_Y) + (row * KBD_ROW_HEIGHT),
                KBD_BTN_WIDTH,
                KBD_BTN_HEIGHT,WHITE);
            }
            // Check for touch input
            if (CheckCollisionPointRec(touchPosition, btnHighlight) && (currentGesture1 != GESTURE_NONE)){
                if(currentGesture1 != lastGesture){
                    kbd_char_selected = button;
                    row = button / KBD_COLS;
                    col = button % KBD_COLS;
                    //Add this to the input_txt buffer
                    sprintf(input_txt,"%s%s", input_txt,kbd_chars[row][col]);
                    /*                // Store gesture string
                switch (currentGesture1)
                {
                    case GESTURE_TAP: sprintf(input_txt,"%s%s", input_txt,"TAP"); break;
                    case GESTURE_DOUBLETAP: sprintf(input_txt,"%s%s", input_txt,"DOUBLETAP"); break;
                    case GESTURE_HOLD: sprintf(input_txt,"%s%s", input_txt,"HOLD"); break;
                    case GESTURE_DRAG: sprintf(input_txt,"%s%s", input_txt,"DRAG"); break;
                    case GESTURE_SWIPE_RIGHT: sprintf(input_txt,"%s%s", input_txt,"RIGHT"); break;
                    case GESTURE_SWIPE_LEFT: sprintf(input_txt,"%s%s", input_txt,"LEFT"); break;
                    case GESTURE_SWIPE_UP: sprintf(input_txt,"%s%s", input_txt,"UP"); break;
                    case GESTURE_SWIPE_DOWN: sprintf(input_txt,"%s%s", input_txt,"DOWN"); break;
                    default: break;
                }*/

                }
            }
            
        }
    }
    if(ScreenOverlays.FileOpen == 1){
        Rectangle fileHighlight = {0,0,0,0};
        //touchPosition = GetTouchPosition(0);
        //currentGesture = GetGestureDetected();
        DrawTexture(DialogTexture,0,0,WHITE);
        DrawText("File Open",10,5,20,DARKGRAY);
        // OK & Cancel buttons are handled by the 
        // soft buttons
        btnA_func = btnA_none;
        btnB_func = btnB_open;
        btnC_func = btnC_cancel;
        btnD_func = btnD_none;
        // List the files
        int i;
        int j=0;
        for(i=0;i<file_count;i++){
            if((i >= first_file) & (i < first_file + DLG_ROWS)){
                fileHighlight.x=5;
                fileHighlight.y=27+(j*20);
                fileHighlight.width=310;
                fileHighlight.height=20;
                // Check for touch input
                if (CheckCollisionPointRec(touchPosition, fileHighlight) && (currentGesture1 != GESTURE_NONE)){
                    file_selected = i;
                }

                if(i == file_selected) {
                    //highlight this file
                    DrawRectangleRec(fileHighlight,LIGHTGRAY);
                }
                DrawText(files[i],10,27+(j*20),20,DARKGRAY);
                j++;
            }
        }
        // Check for Open button
        if (btnB_state == 1){
            // Open Button Touched
            btnB_state = 0;
            char filename[100]; 
            snprintf(filename, sizeof(filename), "resources/sequences/%s", files[file_selected]);
            load_sequence(filename);
            ClearScreenOverlays();
            buttonsDefault();
        }
        if (btnC_state == 1){
            // Cancel Button Touched
            btnC_state = 0;
            ClearScreenOverlays();
            buttonsDefault();
        }
        
    }
    if(ScreenOverlays.TextInput == 1){
        DrawTexture(TopBarTexture,0,0,WHITE);
        DrawTexture(TextInputTexture,103,1,WHITE);
        DrawText("Save As:",10,MENU_TOP_MARGIN,20,DARKGRAY);
        DrawText(input_txt,110,4,20,DARKGRAY);
    }
    // The soft button function bar is always displayed 
    // at the bottom of the screen
    gui_ButtonBar();
}

/*
 * gui_ButtonBar
 * Draws the button bar at the bottom of the screen according 
 * to the current function of each of the 4 Soft Buttons. It
 * also scans for Touch Input for each button, and calls the
 * same function that the button would
 */
void gui_ButtonBar(void){
    Rectangle buttonRectangle = {0,0,0,0};
    //DrawTexture(ButtonBarTexture,0,213,WHITE);
    // Button A
    buttonRectangle.x = 0;
    buttonRectangle.y = 213;
    buttonRectangle.width = 80;
    buttonRectangle.height = 17;
    if (CheckCollisionPointRec(touchPosition, buttonRectangle) && (currentGesture1 != GESTURE_NONE)){
        btnA_state = 1;
    }
    switch(btnA_func){
        case btnA_quit:
            DrawText("Quit",17,217,20,DARKGRAY);
            if (btnA_state == 1){
                btnA_state = 0;
                prog_running = 0;
            }

        break;

        case btnA_none:
        default:
        break;
    }
    // Button B
    buttonRectangle.x = 80;
    buttonRectangle.y = 213;
    buttonRectangle.width = 80;
    buttonRectangle.height = 17;
    if (CheckCollisionPointRec(touchPosition, buttonRectangle) && (currentGesture1 != GESTURE_NONE)){
        btnB_state = 1;
    }
    switch(btnB_func){
        case btnB_menu:
            DrawText("Menu",95,217,20,DARKGRAY);
            if (btnB_state == 1){
                btnB_state = 0;
                ScreenOverlays.MainMenu ^= 1;
                if(ScreenOverlays.MainMenu == 1){
                    encoder_focus = menu_on;
                }
                else {
                    ClearMenus();
                    MenuSelectItem(0,0);    // Select just the first item of the first branch
                }
            }
        break;
        case btnB_open:
            DrawText("Open",95,217,20,DARKGRAY);
        break;
        case btnB_save:
            DrawText("Save",95,217,20,DARKGRAY);
        break;

        case btnB_none:
        default:
        break;
    }
    // Button C
    buttonRectangle.x = 160;
    buttonRectangle.y = 213;
    buttonRectangle.width = 80;
    buttonRectangle.height = 17;
    if (CheckCollisionPointRec(touchPosition, buttonRectangle) && (currentGesture1 != GESTURE_NONE)){
        btnC_state = 1;
    }
    switch(btnC_func){
        case btnC_bpm_dn:
            DrawText("BPM-",177,217,20,DARKGRAY);
            if (CheckCollisionPointRec(touchPosition, buttonRectangle) && (currentGesture1 != GESTURE_NONE)){
                clock_freq -= 10;
                if (clock_freq < 1) clock_freq = 1;
                gpioHardwarePWM(MASTER_CLK,clock_freq,500000);
            }
        break;
        case btnC_cancel:
            DrawText("Cancel",167,217,20,DARKGRAY);
        break;

        case btnC_none:
        default:
        break;
    }
    // Button D
    buttonRectangle.x = 240;
    buttonRectangle.y = 213;
    buttonRectangle.width = 80;
    buttonRectangle.height = 17;
    if (CheckCollisionPointRec(touchPosition, buttonRectangle) && (currentGesture1 != GESTURE_NONE)){
        btnD_state = 1;
    }
    switch(btnD_func){
        case btnD_bpm_up:
            DrawText("BPM+",257,217,20,DARKGRAY);
            if (CheckCollisionPointRec(touchPosition, buttonRectangle) && (currentGesture1 != GESTURE_NONE)){
                clock_freq += 10;
                gpioHardwarePWM(MASTER_CLK,clock_freq,500000);
            }
        break;

        case btnD_none:
        default:
        break;
    }
}

void gui_MainMenu(void){
    int x = 5;
    int txt_len;
    int PanelHeight = MENU_FONT_SIZE + MENU_TB_MARGIN;
    Color menu_colour;
    Rectangle menuRectangle = {0,0,0,0}; 
    //lastGesture = currentGesture;
    //currentGesture = GetGestureDetected();
    //touchPosition = GetTouchPosition(0);
    // Top Bar
    DrawTexture(TopBarTexture,0,0,WHITE);
    int i = 0;
    while(Menu[i].name != NULL){
        txt_len = MeasureText(Menu[i].name,MENU_FONT_SIZE);
        //Draw a box a bit bigger than this
        if (Menu[i].highlight == 1) menu_colour = CLR_LIGHTBLUE; else menu_colour = CLR_DARKBLUE;
        menuRectangle.x = x;
        menuRectangle.y = 2;
        menuRectangle.width = txt_len+(MENU_LR_MARGIN * 2);
        menuRectangle.height = PanelHeight;
        if (CheckCollisionPointRec(touchPosition, menuRectangle) && (currentGesture1 == GESTURE_HOLD)){
            // Toggle the expansion of this menu item
            if (Menu[i].highlight == 0) {
                // this Menu item not currently highlighted, so
                // de-hilight the existing one, and highlight this
                int m = 0;
                while(Menu[m].name != NULL){
                    Menu[m].highlight = 0;
                    Menu[m].expanded = 0;
                    m++;
                }
                Menu[i].highlight = 1;
                Menu[m].expanded = 1;
            }
            toggle_menu();
        }
        DrawRectangleRec(menuRectangle,menu_colour);
        DrawText(Menu[i].name,x+MENU_LR_MARGIN,MENU_TOP_MARGIN,MENU_FONT_SIZE,DARKGRAY);
        if(Menu[i].expanded == 1){
            // Draw sub-menus
            int j = 0;
            int y = PanelHeight;
            int sub_len = 0;
            int tmp_len;
            // Measure the length of the longest sub menu
            while(Menu[i].child[j]->name != NULL){
                tmp_len = MeasureText(Menu[i].child[j]->name,MENU_FONT_SIZE);
                if(tmp_len > sub_len) sub_len = tmp_len;
                j++;
            }
            j = 0;
            while(Menu[i].child[j]->name != NULL){
                if (Menu[i].child[j]->highlight == 1) menu_colour = CLR_LIGHTBLUE; else menu_colour = CLR_DARKBLUE;
                // Deal with the direction this leaf opens
                switch (Menu[i].child[j]->direction){
                    case dir_right:
                        menuRectangle.x = x;
                    break;
                    case dir_left:
                        menuRectangle.x = x + txt_len - sub_len;
                    break;
                    default:
                        menuRectangle.x = x;
                    break;
                }
                menuRectangle.y = y;
                menuRectangle.width = sub_len+(MENU_LR_MARGIN * 2);
                menuRectangle.height = PanelHeight;
                if (CheckCollisionPointRec(touchPosition, menuRectangle) && (currentGesture1 == GESTURE_HOLD)){
                    // Call function pointed to by this menu item
                    MenuSelectItem(i,j);
                    if (Menu[i].child[j]->funcPtr != NULL) Menu[i].child[j]->funcPtr();
                }
                DrawRectangleRec(menuRectangle,menu_colour);
                DrawText(Menu[i].child[j]->name,menuRectangle.x+MENU_LR_MARGIN,y+MENU_TB_MARGIN,MENU_FONT_SIZE,DARKGRAY);
                y+=PanelHeight;
                j++;
            }
        }
        x+=txt_len+(MENU_LR_MARGIN * 2)+MENU_HORIZ_SPACE;
        i++;
    }
}

