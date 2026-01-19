#include "raylib.h"
#include "objets.h"
#include "game.h"
#include <string>
#include "network.h"
#include <csignal>


double last_update = 0;
bool connected = false;

bool event(double time) {
    double current_time = GetTime();
    if (current_time - last_update > time) {
        last_update = current_time;
        return true;
    }
    return false;
}



int main() {
    signal(SIGPIPE, SIG_IGN); // pour ignorer quand on envoie sur une socket fermée



    Color darkblue = {44,44,127, 255};
    InitWindow(500, 620, "TETRIS");
    InitAudioDevice();
    //SetAudioStreamBufferSizeDefault(2048);
    SetTargetFPS(60);
    Game jeu = Game();
/*
jeu.mode : 0 solo
            1 en ligne
*/
    if (jeu.mode == 1){
        network_connect();
        network_start_listener();
    }



    SetMusicVolume(jeu.music, 0.3f);
    SetSoundVolume(jeu.destroy_sound, 1.0f);
    SetSoundVolume(jeu.rotate_sound, 0.5f);
    ClearBackground(darkblue);

    float volume = 0.5f;

    Rectangle sliderBar   = { 320, 580, 150, 8 };
    Rectangle sliderKnob  = { 320 + 150 * volume - 6, 574, 12, 16 };

    bool dragging = false;

    while (!WindowShouldClose()) {
        UpdateMusicStream(jeu.music);

        
        while (network_has_message() && jeu.mode == 1) {
            std::string msg = network_pop_message();
            std::cout << "Processing message: " << msg << std::endl;
            jeu.apply_network_message(msg);
        }

        if (jeu.start == 1) jeu.input();
        if (event(0.2/(jeu.get_niveau()+1)) && jeu.mode!=2) jeu.move_down();

        BeginDrawing();
        ClearBackground(darkblue);
        DrawText("Score", 345,15, 38, WHITE);
        DrawText("level", 365,125, 38, WHITE);
        DrawText("Next", 365,210, 38, WHITE);






        Rectangle btn_mode = { 320, 490, 120, 40 };
        Rectangle btn_start = { 500, 490, 120, 40 };

        Vector2 mouse = GetMousePosition();

        bool hover_mode = CheckCollisionPointRec(mouse, btn_mode);

        if (hover_mode && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            jeu.mode = (jeu.mode +1) % 2;
        }

        bool hover_start = CheckCollisionPointRec(mouse, btn_start);

        if (hover_start && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            jeu.start = (jeu.start +1) % 2;
        }

        
        // couleur selon l'état
// baddel bouton mode game w bouton start 

        Color col_mode;
        if (jeu.mode == 0 && connected) {
            col_mode = RED;
            disconnect();
            connected = false;
        }
        if (jeu.mode == 1 && !connected) {
            network_connect();
            network_start_listener();
            col_mode = GREEN;
            connected = true;
        }

        const char* text_mode = (jeu.mode == 0) ? "SOLO" : "ONLINE";

        DrawText(
            text_mode,
            btn_mode.x + 10,
            btn_mode.y + 10,
            20,
            WHITE
        );
    






        // volume slider
        DrawText("Volume", 320, 550, 20, WHITE);
        DrawRectangleRounded(sliderBar, 0.5f, 6, GRAY);
        DrawRectangleRounded(sliderKnob, 0.5f, 6, WHITE);

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
            CheckCollisionPointRec(mouse, sliderKnob)) {
            dragging = true;
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            dragging = false;
        }

        if (dragging) {
            sliderKnob.x = mouse.x - sliderKnob.width / 2;

            if (sliderKnob.x < sliderBar.x)
                sliderKnob.x = sliderBar.x;

            if (sliderKnob.x > sliderBar.x + sliderBar.width - sliderKnob.width)
                sliderKnob.x = sliderBar.x + sliderBar.width - sliderKnob.width;

            volume = (sliderKnob.x - sliderBar.x) /
                     (sliderBar.width - sliderKnob.width);

            SetMasterVolume(volume);
        }
        // fin volume slider


        DrawText(jeu.get_msg().c_str(), 320, 440, 27, WHITE);
        DrawRectangleRounded({320, 55, 170, 60}, 0.3, 6, {59, 85, 162, 255}); // cadre score
        DrawRectangleRounded({320, 260, 170, 140}, 0.3, 6, {59, 85, 162, 255});// cadre next
        DrawRectangleRounded({320, 160, 170, 40}, 0.3, 6, {59, 85, 162, 255});// cadre level

        jeu.dessiner();
        jeu.dessiner_next();
        jeu.destroy();

        // Envoi des lignes à l'adversaire
        if (jeu.linesToSend > 0 && jeu.mode == 1) {
            std::string msg = "LINES|" + std::to_string(jeu.linesToSend) + "\n";
            network_send(msg);
            std::cout << "Sending " << jeu.linesToSend << " lines to opponent" << std::endl;
            jeu.linesToSend = 0;
        }

        // Envoi du game over
        if (jeu.justLost && jeu.mode == 1) {
            network_send("GAMEOVER\n");
            std::cout << "Sending GAMEOVER" << std::endl;
            jeu.justLost = false;
        }




        char sc[10];
        sprintf(sc, "%d", jeu.get_score());
        DrawText(sc, 345,70, 27, BLACK);

        char lvl[10];
        sprintf(lvl, "%d", jeu.get_niveau());
        DrawText(lvl, 345,168, 27, BLACK);

        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}