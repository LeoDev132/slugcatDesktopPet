#pragma once
#include "raylib.h"
#include <atomic>
#include <cmath>
#include <vector>
#include <iostream>

inline Vector2 operator*(Vector2& v, float f)
{
    return  {v.x * f, v.y * f};
}

inline Vector2 operator-(Vector2 l, Vector2 r)
{
    return {l.x - r.x, l.y - r.y};
}

inline Vector2 operator+(Vector2 l, Vector2 r)
{
    return {l.x + r.x, l.y + r.y};
}

inline Vector2 operator/(Vector2 l, float f)
{
    return {l.x / f, l.y / f};
}

float length(Vector2 vec)
{
    return std::hypot(vec.x, vec.y);
}

struct PhysicalPoint
{
    PhysicalPoint(Vector2 pos, float radius)
    {
        this->pos = pos;
        this->oldpos = pos;
        this->acc = {0, 0};
        this->radius = radius;
    }
    bool isStatic = false;
    float friction = 200.f;
    Vector2 pos;
    Vector2 oldpos;
    Vector2 acc;
    float radius;
};

struct Constraint
{
    Constraint(int id1, int id2, float dist)
    :id1(id1), id2(id2), dist(dist)
    {
    }
    float dist;
    int id1;
    int id2;
};

class PhysicalSolver
{
    public:
        int addPoint(PhysicalPoint point)
        {
            points.push_back(point);
            return points.size()-1;
        }

        int addConstraint(int id1, int id2)
        {
            constraints.push_back(
                Constraint(
                    id1,
                    id2,
                    length(points[id1].pos - points[id2].pos)));
            return constraints.size()-1;
        }

        int getPointAtPos(Vector2 position)
        {
            int i = 0;
            for(auto& p : points)
            {
                if(length(p.pos - position) <= p.radius)
                {
                    return i;
                }
                i++;
            }
            return -1;
        }

        void debugDraw()
        {
            for(auto& p: points)
            {
                DrawCircle(p.pos.x, p.pos.y, p.radius, RAYWHITE);
            }
            for(auto& c: constraints)
            {
                PhysicalPoint& p1 = points[c.id1];
                PhysicalPoint& p2 = points[c.id2];
                Vector2 diff = p1.pos - p2.pos;
                float angle = std::atan2(diff.y, diff.x);
                Vector2 angleVec;
                angleVec.x = cos(angle + PI/2.f);
                angleVec.y = sin(angle + PI/2.f);
                DrawTriangle(p1.pos, p1.pos - (Vector2){10, 10}, p1.pos+(Vector2){10, 0}, YELLOW);
                Vector2 topLeft = p1.pos - angleVec * p1.radius;
                Vector2 topRight = p1.pos + angleVec * p1.radius;
                Vector2 bottomLeft =  p2.pos - angleVec * p2.radius;
                Vector2 bottomRight =  p2.pos + angleVec * p2.radius;
                DrawTriangle(topRight, topLeft, bottomLeft, WHITE);
                DrawTriangle(topRight, bottomLeft, bottomRight, WHITE);
                //DrawTriangle(p2.pos + angleVec * p2.radius, p2.pos - angleVec * p2.radius, p1.pos - angleVec * p1.radius, YELLOW);
            }
        }

        void update(float dt)
        {
            for(auto& c: constraints)
            {
                PhysicalPoint& p1 = points[c.id1];
                PhysicalPoint& p2 = points[c.id2];
                Vector2 diff = p1.pos - p2.pos;
                float dist = length(diff);
                float distDiff = c.dist - dist;
                if(dist != c.dist)
                {
                    Vector2 diffNormalized = diff / dist;
                    if(!p2.isStatic)
                        p2.pos = p2.pos + diffNormalized * (-distDiff * 0.5f);
                    if(!p1.isStatic)
                        p1.pos = p1.pos - diffNormalized * (-distDiff * 0.5f);
                }
            }

            for(auto& p: points)
            {
                if(!p.isStatic)
                {
                    Vector2 buffer = p.pos;
                    p.pos = p.pos * 2.f - p.oldpos + p.acc * (dt * dt);
                    p.acc = {0, 998};
                    //hard limits on sides of screen to avoid going out of bounds
                    int monitor = GetCurrentMonitor();
                    int height = GetMonitorHeight(monitor);
                    int width = GetMonitorWidth(monitor);
                    Vector2 vel = p.pos - p.oldpos;
                    if(p.pos.y + p.radius > height)
                    {
                        p.pos.y = height - p.radius;
                        p.acc.x += vel.x * -p.friction;
                    }
                    if(p.pos.y - p.radius < 0) p.pos.y = p.radius;
                    if(p.pos.x + p.radius > width) p.pos.x = width - p.radius;
                    if(p.pos.x - p.radius < 0) p.pos.x = p.radius;
                    p.oldpos = buffer;
                }
            }
        }
        float epsilon = 0.001f;
        std::vector<PhysicalPoint> points;
        std::vector<Constraint> constraints;
};

class Slugcat
{
    public:
        Slugcat(PhysicalSolver& solver, Camera2D& camera)
        {
            this->cam = &camera;
            this->solver = &solver;
            float halfScreen = screenHeight/2.f;
            points.push_back(solver.addPoint(PhysicalPoint({halfScreen, halfScreen}, bodyRadius)));
            points.push_back(solver.addPoint(PhysicalPoint({halfScreen+headToBodyDist, halfScreen}, bodyRadius)));
            points.push_back(solver.addPoint(PhysicalPoint({halfScreen + headToBodyDist + tailDist * 1.f, halfScreen}, tail1Radius)));
            points.push_back(solver.addPoint(PhysicalPoint({halfScreen + headToBodyDist + tailDist * 2.f, halfScreen}, tail2Radius)));
            constraints.push_back(solver.addConstraint(0, 1));
            constraints.push_back(solver.addConstraint(1, 2));
            constraints.push_back(solver.addConstraint(2, 3));
            SetConfigFlags(FLAG_WINDOW_TRANSPARENT | FLAG_WINDOW_UNDECORATED);
            InitWindow(screenWidth, screenHeight, "skug");
        }

        void changeColor(Color newColor)
        {
            this->color = newColor;
        }

        void update(float dt)
        {
            Vector2 winSize = {(float)screenWidth,(float)screenHeight};
            Vector2 body = solver->points[points[BODY]].pos;
            Vector2 winPos = body  - winSize/2.f;
            SetWindowPosition(winPos.x, winPos.y);
            cam->target = body;
            cam->zoom = 1.0f;
            cam->rotation = 0.f;
            cam->offset = (winSize/2.F);
        }

        //Takes a x vector and y vector and applies an offset locally accordingly
        Vector2 offsetFromVectors(Vector2 xVec, Vector2 yVec, Vector2 origin, Vector2 offset)
        {
            return origin + (xVec * offset.x) + (yVec * offset.y);
        }

        void draw()
        {
            for(int& i: points)
            {
                PhysicalPoint& p = solver->points[i];
                DrawCircle(p.pos.x, p.pos.y, p.radius, RAYWHITE);
            }
            for(auto& i: constraints)
            {
                Constraint& c = solver->constraints[i];
                PhysicalPoint& p1 = solver->points[c.id1];
                PhysicalPoint& p2 = solver->points[c.id2];
                Vector2 diff = p1.pos - p2.pos;
                float angle = std::atan2(diff.y, diff.x);
                Vector2 angleVec;
                angleVec.x = cos(angle + PI/2.f);
                angleVec.y = sin(angle + PI/2.f);
                Vector2 angleVecVertical;
                angleVecVertical.x = cos(angle);
                angleVecVertical.y = sin(angle);
                DrawTriangle(p1.pos, p1.pos - (Vector2){10, 10}, p1.pos+(Vector2){10, 0}, YELLOW);
                Vector2 topLeft = p1.pos - angleVec * p1.radius;
                Vector2 topRight = p1.pos + angleVec * p1.radius;
                Vector2 bottomLeft =  p2.pos - angleVec * p2.radius;
                Vector2 bottomRight =  p2.pos + angleVec * p2.radius;
                DrawTriangle(topRight, topLeft, bottomLeft, WHITE);
                DrawTriangle(topRight, bottomLeft, bottomRight, WHITE);
                if(i == HEAD_TO_BODY)
                {
                    float eyeSpacingHorizontal = 0.4;
                    float eyeSpacingVertical = 0.3;
                    Vector2 ear = offsetFromVectors(angleVec, angleVecVertical, p1.pos, {-p1.radius * 0.7f, p1.radius * 1.f});
                    DrawCircle(ear.x, ear.y, 0.25*bodyRadius, WHITE);
                    ear = offsetFromVectors(angleVec, angleVecVertical, p1.pos, {p1.radius * 0.7f, p1.radius * 1.f});
                    DrawCircle(ear.x, ear.y, 0.25*bodyRadius, WHITE);
                    Vector2 eye = offsetFromVectors(angleVec, angleVecVertical, p1.pos, {-p1.radius * eyeSpacingHorizontal, p1.radius * eyeSpacingVertical});
                    DrawCircle(eye.x, eye.y, 0.25*bodyRadius, BLACK);
                    eye = offsetFromVectors(angleVec, angleVecVertical, p1.pos, {p1.radius * eyeSpacingHorizontal, p1.radius * eyeSpacingVertical});
                    DrawCircle(eye.x, eye.y, 0.25*bodyRadius, BLACK);

                }
                //DrawTriangle(p2.pos + angleVec * p2.radius, p2.pos - angleVec * p2.radius, p1.pos - angleVec * p1.radius, YELLOW);
            }
        }

        enum PART
        {
            HEAD,
            BODY,
            TAIL_1,
            TAIL_2
        };

        enum CONSTRAINT_PART
        {
            HEAD_TO_BODY,
            BODY_TO_TAIL_1,
            TAIL_1_TO_TAIL_2
        };
    private:
        const float scale = 2.0;
        const int screenWidth = 200 * scale;
        const int screenHeight = 200 * scale;
        float bodyRadius = 8 * scale;
        float tail1Radius = 4.5 * scale;
        float tail2Radius = 2.5 * scale;
        PhysicalSolver* solver;
        Camera2D* cam;
        Color color;
        float headToBodyDist = 20 * scale;
        float tailDist = 14 * scale;
        std::vector<int> points;
        std::vector<int> constraints;
};
