

#include "raylib.h"
#include "Slugcat.hpp"
#include <vector>


int main(void)
{
    PhysicalSolver solver;
    Camera2D cam;
    Slugcat survivor(solver, cam);
    SetTargetFPS(60);
    float dt = 1/60.f;
    int heldIndex = -1;

    Vector2 globalMousePos = GetMousePosition() + GetWindowPosition();
    while (!WindowShouldClose())
    {
        if(length(GetMouseDelta()) > 0.1)
            globalMousePos = GetMousePosition() + GetWindowPosition();
        if(heldIndex != -1)
        {
            solver.points[heldIndex].pos = globalMousePos;
            solver.points[heldIndex].oldpos = globalMousePos;
        }
        solver.update(dt);
        survivor.update(dt);
        BeginDrawing();
            BeginMode2D(cam);
                if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                {
                    heldIndex = solver.getPointAtPos(globalMousePos);
                }
                else if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
                {
                    heldIndex = -1;
                }
                ClearBackground(BLANK);
                //solver.debugDraw();
                survivor.draw();
            EndMode2D();
        EndDrawing();
        dt = 1.0 / (float)GetFPS();
    }
    CloseWindow();
    return 0;
}
