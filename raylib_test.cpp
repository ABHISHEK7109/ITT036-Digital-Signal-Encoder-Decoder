
#include "raylib.h"

int main() {
    const int screenWidth = 900;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "ITT036 - raylib GUI Test");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(Color{15, 23, 42, 255});

        DrawText(
            "ITT036 DIGITAL SIGNAL ENCODER",
            40, 35, 28, SKYBLUE
        );

        DrawText(
            "raylib 6.2 - GUI setup successful",
            40, 85, 20, RAYWHITE
        );

        DrawRectangle(40, 145, 820, 360, Color{30, 41, 59, 255});
        DrawText("WAVEFORM PREVIEW", 60, 165, 18, LIGHTGRAY);

        const int points[] = {
            60, 260, 160, 260,
            160, 410, 260, 410,
            260, 260, 360, 260,
            360, 410, 460, 410,
            460, 260, 560, 260,
            560, 410, 660, 410,
            660, 260, 820, 260
        };

        for (int i = 0; i + 3 < 28; i += 4) {
            DrawLineEx(
                Vector2{static_cast<float>(points[i]),
                        static_cast<float>(points[i + 1])},
                Vector2{static_cast<float>(points[i + 2]),
                        static_cast<float>(points[i + 3])},
                3.0f,
                SKYBLUE
            );
        }

        DrawText(
            "Test window - close it to exit",
            40, 545, 18, GRAY
        );

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
