#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#include "rcamera.h"

#define CUBE_SIZE 1

typedef struct {
    Vector4 *items;
    size_t count, capacity;
} Points;

typedef struct {
    int a, b;
} Line;

typedef struct {
    Line *items;
    size_t count, capacity;
} Lines;

void cube(Points *out, float size) {
    for (int x = 0; x <= 1; x++) {
        for (int y = 0; y <= 1; y++) {
            for (int z = 0; z <= 1; z++) {
                for (int w = 0; w <= 1; w++) {
                    Vector4 point = {x, y, z, w};
                    point = Vector4Scale(point, size);
                    point = Vector4Subtract(point, Vector4Scale(Vector4One(), size / 2));
                    da_append(out, point);
                }
            }
        }
    }
}

void lines(Lines *out, Points points, float cube_size) {
    for (size_t i = 0; i < points.count; i++) {
        for (size_t j = 0; j < points.count; j++) {
            Vector4 a = points.items[i];
            Vector4 b = points.items[j];

            float dist_sq = Vector4DistanceSqr(a, b);
            if (dist_sq == cube_size*cube_size) {
                da_append(out, (CLITERAL(Line) {i, j}));
            }
        }
    }
}

void print_matrix_impl(Matrix m, const char *name) {
    float16 floats = MatrixToFloatV(m);
    printf("%s = [\n", name);
    for (int i = 0; i < 4; i++) {
        printf("    ");
        for (int j = 0; j < 4; j++) {
            printf("%f ", floats.v[j * 4 + i]);
        }
        printf("\n");
    }
    printf("]\n");
}
#define print_matrix(m) print_matrix_impl(m, #m)

Vector4 vec4_times_matrix(Vector4 v, Matrix mat) {
    Vector4 result = { 0 };

    float x = v.x;
    float y = v.y;
    float z = v.z;
    float w = v.w;

    result.x = mat.m0*x + mat.m4*y + mat.m8*z  + mat.m12*w;
    result.y = mat.m1*x + mat.m5*y + mat.m9*z  + mat.m13*w;
    result.z = mat.m2*x + mat.m6*y + mat.m10*z + mat.m14*w;
    result.w = mat.m3*x + mat.m7*y + mat.m11*z + mat.m15*w;

    return result;
}

Vector3 project_vec4(Vector4 point) {
    float w = 1 / (1 - point.w);
    Matrix projection = {
        w, 0, 0, 0,
        0, w, 0, 0,
        0, 0, w, 0,
        0, 0, 0, w,
    };

    Vector4 p4 = vec4_times_matrix(point, projection);
    Vector3 p3 = {p4.x, p4.y, p4.z};

    return p3;
}

Matrix hyperrot_matrix(float angle, int axis1, int axis2) {
    float16 floats = MatrixToFloatV(MatrixIdentity());

    // c 0 0 0
    // 0 1 0 0
    // 0 0 1 0
    // 0 0 0 1
    floats.v[axis1 * 4 + axis1] = cosf(angle);
    floats.v[axis1 * 4 + axis2] = -sinf(angle);
    floats.v[axis2 * 4 + axis1] = sinf(angle);
    floats.v[axis2 * 4 + axis2] = cosf(angle);

    return *(Matrix*)&floats;
}

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "4D Hypercube");

    Camera camera = {
        .position = { 0.0f, 0.0f, CUBE_SIZE + 4.0f },
        .target = Vector3Zero(),
        .up = { 0.0f, 1.0f, 0.0f },
        .fovy = 100.0f,
        .projection = CAMERA_PERSPECTIVE,
    };

    DisableCursor();

    Points cube_points = {0};
    cube(&cube_points, CUBE_SIZE);

    Lines cube_lines = {0};
    lines(&cube_lines, cube_points, CUBE_SIZE);

    float angle = M_PI;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        UpdateCamera(&camera, CAMERA_FIRST_PERSON);
        if (IsKeyDown(KEY_SPACE)) CameraMoveUp(&camera,  5.4f*dt);
        if (IsKeyDown(KEY_LEFT_SHIFT)) CameraMoveUp(&camera, -5.4f*dt);

        if (IsKeyPressed(KEY_F)) ToggleBorderlessWindowed();

        angle += 1*dt;

        BeginDrawing();
            ClearBackground(BLACK);
            BeginMode3D(camera);
            Matrix transform = MatrixIdentity();
            transform = MatrixMultiply(transform, hyperrot_matrix(angle, 1, 3));
            transform = MatrixMultiply(transform, hyperrot_matrix(angle, 2, 3));
            //transform = MatrixMultiply(transform, hyperrot_matrix(angle, 2, 1));
            for (size_t i = 0; i < cube_points.count; i++) {
                Vector4 point = cube_points.items[i];

                point = vec4_times_matrix(point, transform);

                DrawSphere(project_vec4(point), 0.1f, WHITE);
            }

            for (size_t i = 0; i < cube_lines.count; i++) {
                Line line = cube_lines.items[i];

                Vector4 point = cube_points.items[line.a];
                point = vec4_times_matrix(point, transform);
                Vector3 pa = project_vec4(point);

                point = cube_points.items[line.b];
                point = vec4_times_matrix(point, transform);
                Vector3 pb = project_vec4(point);
                DrawLine3D(pa, pb, WHITE);
            }
            EndMode3D();
        EndDrawing();

        //break;
    }

    CloseWindow();
    return 0;
}
