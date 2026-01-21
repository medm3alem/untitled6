// test_audio.cpp
#include "raylib.h"
#include <iostream>

int main() {
    InitWindow(400, 200, "Audio Test");
    InitAudioDevice();
    
    if (!IsAudioDeviceReady()) {
        std::cout << "ERROR: Audio device NOT ready!" << std::endl;
        return 1;
    }
    std::cout << "Audio device ready!" << std::endl;
    
    Music music = LoadMusicStream("sounds/cover.wav");  // ou .mp3
    PlayMusicStream(music);
    SetMusicVolume(music, 0.5f);
    
    std::cout << "Playing music... Close window to stop." << std::endl;
    
    while (!WindowShouldClose()) {
        UpdateMusicStream(music);
        
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Audio Test - Music should be playing", 10, 80, 20, BLACK);
        EndDrawing();
    }
    
    UnloadMusicStream(music);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}