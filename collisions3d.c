#include <math.h>
#include <string.h>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

// Define a estrutura para representar uma bola
typedef struct {
    Vector3 pos;
    float vx, vy, vz;

    float mass;
    float radius;
    Color color;
} Ball ;


/* Calcula a distância entre dois vetores */
float magnitude(Vector3 v1, Vector3 v2){
    return sqrtf((v1.x-v2.x)*(v1.x-v2.x) + (v1.y-v2.y)*(v1.y-v2.y) + (v1.z-v2.z)*(v1.z-v2.z));
}

/* Calcula a velocidade do centro de massa entre duas bolas */
Vector3 center_of_mass_velocity(Ball *a, Ball *b) {
    float total_mass = a->mass + b->mass;
    Vector3 vcm;
    
    // Calcula a velocidade do centro de massa (Vcm)
    vcm.x = (a->mass * a->vx + b->mass * b->vx) / total_mass;
    vcm.y = (a->mass * a->vy + b->mass * b->vy) / total_mass;
    vcm.z = (a->mass * a->vz + b->mass * b->vz) / total_mass;
    
    return vcm;
}

void resolve_collision(Ball *a, Ball *b, float Cr) {
    Vector3 Vcm = center_of_mass_velocity(a, b);

    // Cálculo da velocidade relativa entre as bolas (antes da colisão)
    float rel_vx = b->vx - a->vx;
    float rel_vy = b->vy - a->vy;
    float rel_vz = b->vz - a->vz;
    float rel_velocity = rel_vx * (b->pos.x - a->pos.x) + rel_vy * (b->pos.y - a->pos.y) + rel_vz * (b->pos.z - a->pos.z);
    
    // Se as bolas estão se afastando (rel_velocity > 0), não há colisão
    if (rel_velocity > 0) return;

    // Calculando as novas velocidades das bolas após a colisão
    Vector3 V1f, V2f;
    
    // Bola A
    V1f.x = (1 + Cr) * Vcm.x - Cr * a->vx;
    V1f.y = (1 + Cr) * Vcm.y - Cr * a->vy;
    V1f.z = (1 + Cr) * Vcm.z - Cr * a->vz;

    // Bola B
    V2f.x = (1 + Cr) * Vcm.x - Cr * b->vx;
    V2f.y = (1 + Cr) * Vcm.y - Cr * b->vy;
    V2f.z = (1 + Cr) * Vcm.z - Cr * b->vz;

    // Atualizando as velocidades das bolas
    a->vx = V1f.x;
    a->vy = V1f.y;
    a->vz = V1f.z;

    b->vx = V2f.x;
    b->vy = V2f.y;
    b->vz = V2f.z;
}

int main(int argc, char *argv[]) {
    srand(time(NULL));

    int no_sound = 0;
    int width, height, depth;
    
    // Verifica argumentos de linha de comando
    for (int i = 0; i < argc; i++)
    {   
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Uso: %s [opções]\n", argv[0]);
            printf("Opções:\n");
            printf("  --help, -h       Exibe esta mensagem de ajuda\n");
            printf("  --no-sound       Desativa o som de colisão\n");
            printf("  --resolution WxH Define a resolução da janela (ex: --resolution 800x600)\n");
            return 0;
        }
        if (strcmp(argv[i], "--no-sound") == 0) {
            no_sound = 1; // Ativar modo sem som
            printf("Som desativado.\n");
        }
        if (strcmp(argv[i], "--resolution") == 0 && i + 1 < argc) {
            // Espera-se que a resolução seja fornecida no formato WxH
            sscanf(argv[++i], "%dx%d", &width, &height);
            printf("Resolução definida para %dx%d\n", width, height);
        }
    }
    
    if (!width || !height) {
        printf("Tamanho da janela (ex: 800x600): ");
        scanf("%dx%d", &width, &height);
    }
    
    const int screenWidth = width;
    const int screenHeight = height;
    depth = width;

    // Número de bolas
    int n_balls;
    printf("Quantidade de bolas: ");
    scanf("%d", &n_balls);

    // Coeficiente de restituição
    float restitution;
    printf("Coeficiente de restituição (0.0 a 1.0): ");
    scanf("%f", &restitution);

    if (restitution < 0.4)
    {
        no_sound = 1; // Desativar som se o coeficiente de restituição for muito baixo
        printf("Coeficiente de restituição muito baixo, som de colisão desativado.\n");
    }
    

    int info_enabled = 1;

    // Inicializa a janela
    InitWindow(screenWidth, screenHeight, "Collisions - Caio Furlan Traebert");

    // Cria bolas
    Ball balls[n_balls];
    for (int i=0; i<n_balls; i++) {
        Ball new_ball;

        new_ball.radius = 50;

        // Gera uma posição aleatória para a bola dentro dos limites da janela
        new_ball.pos = (Vector3){
            (float) (rand() % (int)(width-(new_ball.radius)*2))+new_ball.radius,
            (float) (rand() % (int)(height-(new_ball.radius)*2))+new_ball.radius,
            (float) (rand() % (int)(depth-(new_ball.radius)*2))+new_ball.radius
        };

        // Gera uma velocidade aleatória para a bola entre -500 e 500
        new_ball.vx = (float) (rand() % 1000 - 500);
        new_ball.vy = (float) (rand() % 1000 - 500);
        new_ball.vz = (float) (rand() % 1000 - 500);

        new_ball.mass = (rand() % 100 + 10)/10.0; // Massa entre 1.0 e 10.0

        new_ball.color = ColorFromHSV(rand()%361, 1, 1); // Cor aleatória em HSV

        
        balls[i] = new_ball;
    }

    SetTargetFPS(60); // Define a taxa de quadros alvo


    // Configura camera
    Camera camera = {0};
    camera.position = (Vector3){ width/2.0f, height/2.0f, -1000.0 };
    camera.target = (Vector3){ width/2.0f, height/2.0f, depth/2.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;


    // Som de impacto
    InitAudioDevice();
    Sound collisionSound = LoadSound("/home/user/projects/collision/resources/bonk.wav");
    if (collisionSound.stream.buffer == NULL)
        printf("Erro ao carregar o som de colisão.\n");

    SetSoundVolume(collisionSound, 1.0f); // Ajustar volume do som de colisão

    while (!WindowShouldClose()) {
        const float delta_time = GetFrameTime();

        // Atualiza as posições das bolas
        for (int i=0; i<n_balls; i++) {
            balls[i].pos.x += balls[i].vx * delta_time;
            balls[i].pos.y += balls[i].vy * delta_time;
            balls[i].pos.z += balls[i].vz * delta_time;

            // Verifica colisão com as paredes e inverte a velocidade se necessário
            if (balls[i].pos.x - balls[i].radius < 0 || balls[i].pos.x + balls[i].radius > screenWidth) {
                balls[i].vx *= -1;
            }
            if (balls[i].pos.y - balls[i].radius < 0 || balls[i].pos.y + balls[i].radius > screenHeight) {
                balls[i].vy *= -1;
            }
            if (balls[i].pos.z - balls[i].radius < 0) {
                balls[i].pos.z = balls[i].radius;
                balls[i].vz *= -1;
            }
            if (balls[i].pos.z + balls[i].radius > depth) {
                balls[i].pos.z = depth - balls[i].radius;
                balls[i].vz *= -1;
            }
        }

        // Checar collisões
        for (int i=0; i<n_balls; i++) {
            for (int j=i+1; j<n_balls; j++) {
                float rsum = balls[i].radius + balls[j].radius;
                // Se a distância entre as bolas for menor que a soma dos raios, há colisão
                if (magnitude(balls[i].pos, balls[j].pos) < rsum) {
                    resolve_collision(&balls[i], &balls[j], restitution);
                    printf("Colisão entre bola %d e bola %d\n", i+1, j+1);

                    if (no_sound) continue; // Se no_sound estiver ativado, não tocar som
                    SetSoundPitch(collisionSound, 0.8f + ((rand() % 41) / 100.0f)); // Ajustar pitch do som de colisão
                    PlaySound(collisionSound); // Tocar som de colisão
                }
            }
        }

        

        BeginDrawing();
        
        ClearBackground(BLACK);

        DrawText(TextFormat("FPS: %d", GetFPS()), 10, 10, 20, WHITE); // Contador de FPS

        rlSetClipPlanes(0.1, 100000.0); // Distância de clipping

        BeginMode3D(camera);

        // Linhas de referencia
        DrawCubeWires((Vector3){width/2.0f, height/2.0f, depth/2.0f}, width, height, depth, RED);
        for (int i=1; i<10; i++) {
            DrawLine3D((Vector3){width, 0.0f, depth-(depth/10.0)*i}, (Vector3){0.0f, 0.0f, depth-(depth/10.0)*i}, WHITE);
        }
        for (int i=1; i<10; i++) {
            DrawLine3D((Vector3){width-(width/10.0)*i, 0.0, 0.0}, (Vector3){width-(width/10.0)*i, 0.0, depth}, WHITE);
        }
        DrawPlane((Vector3){width/2.0, -1.5, depth/2.0}, (Vector2){width, depth}, (Color){255,255,255,20});


        Vector3 lightPos = { width/2.0f, height, -depth };
        // Desenhar a bola
        for (int i=0; i<n_balls; i++) {
            // Calcular a intensidade da luz porcamente
            Vector3 toLight = Vector3Normalize(Vector3Subtract(lightPos, balls[i].pos));
            Vector3 normal = {0, 1, 0};
            float dot = Vector3DotProduct(toLight, normal);

            float intensity = 0.5f + 0.5f * fmaxf(dot, 0.0f);
            Color shaded = {
                (unsigned char)(balls[i].color.r * intensity),
                (unsigned char)(balls[i].color.g * intensity),
                (unsigned char)(balls[i].color.b * intensity),
                255
            };


            DrawSphere(balls[i].pos, balls[i].radius, shaded);

            // Efeito de highlight
            Vector3 toHighlight = balls[i].pos;
            toHighlight.x += 2;
            toHighlight.y += 2;
            toHighlight.z -= 10;
            DrawSphere(toHighlight, balls[i].radius*0.9, (Color){255,255,255,50});

        }   

        EndMode3D();

        // Desenhar informações das bolas
        if (info_enabled)
        {
            for (int i = 0; i < n_balls; i++) {
                float ball_speed = sqrtf(balls[i].vx * balls[i].vx + balls[i].vy * balls[i].vy + balls[i].vz * balls[i].vz);
                DrawText(TextFormat("Bola %d", i+1, balls[i].mass, ball_speed), 10, 60 + i * 45, 20, balls[i].color);
                DrawText(TextFormat("Massa: %.1f, Velocidade: %.2f", i+1, balls[i].mass, ball_speed), 10, 80 + i * 45, 20, balls[i].color);
            }
        }
        if (IsKeyPressed(KEY_F1)) {
            info_enabled = !info_enabled;
        }

        if (IsKeyPressed(KEY_ESCAPE)) {
            break;
        }

        // Desenhar informações gerais
        DrawText(TextFormat("Bolas: %d   Coeficiente de restituição: %.2f", n_balls, restitution), 10, 30, 20, WHITE);
        if (no_sound) {
            DrawText("Som de colisão desativado", 10, screenHeight - 60, 20, RED);
        }

        DrawText("ESC: Sair   F1: Informações", 10, screenHeight - 30, 20, WHITE);
        EndDrawing();
        
    }
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
