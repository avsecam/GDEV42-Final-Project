#include <raylib.h>
#include <raymath.h>

#include <fstream>
#include <iostream>
#include <vector>

#include "headers/uihandler.hpp"

const float WINDOW_WIDTH(1280);
const float WINDOW_HEIGHT(720);
const char* WINDOW_TITLE("HAKENSLASH THE PLATFORMER UI TEST");

const int TARGET_FPS(60);
const float TIMESTEP(1.0f / (float)TARGET_FPS);

int main() {
    UIState state;
    MenuHandler menuHandler;
    menuHandler.initialize(WINDOW_WIDTH, WINDOW_HEIGHT);

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(TARGET_FPS);

    Texture heartFull = LoadTexture("./assets/Heart_Full.png");
    Texture heartHalf = LoadTexture("./assets/Heart_Half.png");
    Texture heartEmpty = LoadTexture("./assets/Heart_Empty.png");

    menuHandler.inGameGUI.hpBar.InitBar(20);

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_W)) {
            menuHandler.inGameGUI.hpBar.UpdateHealth(1);
        }

        if (IsKeyPressed(KEY_S)) {
            menuHandler.inGameGUI.hpBar.UpdateHealth(-1);
        }

        std::cout << currentGameState << std::endl;
        menuHandler.inGameGUI.hpBar.heart_full = heartFull;
        menuHandler.inGameGUI.hpBar.heart_half = heartHalf;
        menuHandler.inGameGUI.hpBar.heart_empty = heartEmpty;

        state = menuHandler.getState();
        menuHandler.Update();
        
        BeginDrawing();
        ClearBackground(WHITE);
        menuHandler.Draw();
        EndDrawing();
    }

    UnloadTexture(heartFull);
    UnloadTexture(heartHalf);
    UnloadTexture(heartEmpty);

    CloseWindow();

    return 0;
}