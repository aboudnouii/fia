#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SIZE 4
#define MAX_RULES 50

// ===== Environment =====
typedef struct {
    int x, y;
} Position;

typedef struct {
    Position pos;
    int has_trash;
    int is_obstacle;
    int is_cleaned;
} Cell;

typedef struct {
    Cell grid[SIZE][SIZE];
    Position robot_pos;
    int trash_count;
    int moves;
} Environment;

// ===== Rule Struct =====
typedef struct {
    char condition[256];
    char action[256];
} Rule;

Rule rules[MAX_RULES];
int rule_count = 0;

// ===== Predicate Functions =====
int is_obstacle(Environment *env, int x, int y) {
    return env->grid[x][y].is_obstacle;
}

int has_trash(Environment *env, int x, int y) {
    return env->grid[x][y].has_trash && !env->grid[x][y].is_cleaned;
}

int is_cleaned(Environment *env, int x, int y) {
    return env->grid[x][y].is_cleaned;
}

int robot_at(Environment *env, int x, int y) {
    return (env->robot_pos.x == x && env->robot_pos.y == y);
}

int can_move_to(Environment *env, int x, int y) {
    return (x >= 0 && x < SIZE && y >= 0 && y < SIZE && !is_obstacle(env, x, y));
}

int all_trash_cleaned(Environment *env) {
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            if (has_trash(env, i, j))
                return 0;
    return 1;
}

// ===== Load Rules =====
void load_rules(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) { 
        printf("Warning: Cannot open rules file. Creating default rules.\n"); 
        
        // إنشاء قواعد افتراضية إذا لم يوجد ملف
        strcpy(rules[0].condition, "robot_at trash not cleaned");
        strcpy(rules[0].action, "clean");
        rule_count = 1;
        
        strcpy(rules[1].condition, "trash nearby");
        strcpy(rules[1].action, "move toward trash");
        rule_count = 2;
        return;
    }

    char line[512];
    while (fgets(line, sizeof(line), fp) && rule_count < MAX_RULES) {
        if (line[0]=='#' || strlen(line)<=1) continue;
        char *arrow = strstr(line, ":-");
        if(arrow){
            *arrow='\0';
            arrow+=2;
            line[strcspn(line,"\n")]=0;
            arrow[strcspn(arrow,"\n")]=0;
            strcpy(rules[rule_count].condition,line);
            strcpy(rules[rule_count].action,arrow);
            rule_count++;
        }
    }
    fclose(fp);
}

// ===== Check Condition =====
int check_condition(Rule *rule, Environment *env) {
    int x = env->robot_pos.x;
    int y = env->robot_pos.y;
    
    if(strstr(rule->condition,"robot_at") && strstr(rule->condition,"trash") && strstr(rule->condition,"not cleaned")){
        if(robot_at(env,x,y) && has_trash(env,x,y) && !is_cleaned(env,x,y))
            return 1;
    }
    
    // قاعدة جديدة: إذا كان هناك قمامة في الخلية الحالية
    if(strstr(rule->condition,"trash at current position")){
        if(has_trash(env,x,y))
            return 1;
    }
    
    // قاعدة جديدة: البحث عن أقرب قمامة
    if(strstr(rule->condition,"trash nearby")){
        for(int i = 0; i < SIZE; i++) {
            for(int j = 0; j < SIZE; j++) {
                if(has_trash(env, i, j)) {
                    return 1;
                }
            }
        }
    }
    
    return 0;
}

// ===== Execute Action =====
void execute_action(Rule *rule, Environment *env) {
    int x = env->robot_pos.x;
    int y = env->robot_pos.y;
    
    if(strstr(rule->action,"clean")){
        if(has_trash(env,x,y) && !is_cleaned(env,x,y)){
            env->grid[x][y].is_cleaned = 1;
            env->trash_count--;
            printf("Robot cleaned trash at (%d,%d)\n",x,y);
            env->moves++;
        }
    }
    else if(strstr(rule->action,"move toward trash")){
        // البحث عن أقرب قمامة والتحرك نحوها
        int closest_x = -1, closest_y = -1;
        int min_distance = SIZE * 2;
        
        for(int i = 0; i < SIZE; i++) {
            for(int j = 0; j < SIZE; j++) {
                if(has_trash(env, i, j)) {
                    int distance = abs(x - i) + abs(y - j);
                    if(distance < min_distance) {
                        min_distance = distance;
                        closest_x = i;
                        closest_y = j;
                    }
                }
            }
        }
        
        if(closest_x != -1 && closest_y != -1) {
            // محاولة التحرك نحو القمامة
            int dx = (closest_x > x) ? 1 : (closest_x < x) ? -1 : 0;
            int dy = (closest_y > y) ? 1 : (closest_y < y) ? -1 : 0;
            
            // محاولة الاتجاه الأفقي أولاً
            if(dx != 0 && can_move_to(env, x + dx, y)) {
                env->robot_pos.x += dx;
                env->moves++;
                printf("Robot moved toward trash to (%d,%d)\n", env->robot_pos.x, env->robot_pos.y);
                return;
            }
            // إذا لم يستطع أفقياً، جرب عمودياً
            else if(dy != 0 && can_move_to(env, x, y + dy)) {
                env->robot_pos.y += dy;
                env->moves++;
                printf("Robot moved toward trash to (%d,%d)\n", env->robot_pos.x, env->robot_pos.y);
                return;
            }
        }
        
        // إذا لم يتمكن من التحرك نحو القمامة، تحرك عشوائياً
        int directions[4][2]={{0,1},{1,0},{0,-1},{-1,0}};
        for(int i=0;i<10;i++){
            int dir = rand() % 4;
            int nx = x + directions[dir][0];
            int ny = y + directions[dir][1];
            if(can_move_to(env,nx,ny)){
                env->robot_pos.x = nx;
                env->robot_pos.y = ny;
                env->moves++;
                printf("Robot moved randomly to (%d,%d)\n",nx,ny);
                return;
            }
        }
    }
}

// ===== Make Move =====
void make_move(Environment *env) {
    int x = env->robot_pos.x;
    int y = env->robot_pos.y;
    
    // الأولوية: تنظيف القمامة إذا كان فوقها
    if(has_trash(env, x, y) && !is_cleaned(env, x, y)) {
        env->grid[x][y].is_cleaned = 1;
        env->trash_count--;
        env->moves++;
        printf("Robot cleaned trash at (%d,%d)\n", x, y);
        return;
    }
    
    // ثانياً: تطبيق القواعد
    for(int i=0;i<rule_count;i++){
        if(check_condition(&rules[i],env)){
            execute_action(&rules[i],env);
            return;
        }
    }

    // أخيراً: حركة عشوائية
    int directions[4][2]={{0,1},{1,0},{0,-1},{-1,0}};
    int moved=0;
    int attempts = 0;
    
    while (!moved && attempts < 10) {
        int dir = rand() % 4;
        int nx = x + directions[dir][0];
        int ny = y + directions[dir][1];
        if(can_move_to(env,nx,ny)){
            env->robot_pos.x = nx;
            env->robot_pos.y = ny;
            env->moves++;
            printf("Robot moved randomly to (%d,%d)\n",nx,ny);
            moved=1;
        }
        attempts++;
    }
    
    if (!moved) {
        printf("Robot cannot move! Stuck at (%d,%d)\n", x, y);
        env->moves++;
    }
}

// ===== Print Environment =====
void print_environment(Environment *env) {
    printf("\nEnvironment:\n");
    for(int i=0;i<SIZE;i++){
        for(int j=0;j<SIZE;j++){
            if(is_obstacle(env,i,j)) printf(" X ");
            else if(robot_at(env,i,j)) printf(" R ");
            else if(has_trash(env,i,j)) printf(" T ");
            else if(is_cleaned(env,i,j)) printf(" C ");
            else printf(" . ");
        }
        printf("\n");
    }
    printf("Trash remaining: %d | Moves: %d\n",env->trash_count,env->moves);
}

// ===== Main =====
int main() {
    Environment env;
    srand(time(NULL));

    // تهيئة البيئة - لا توجد عوائق
    for(int i=0;i<SIZE;i++)
        for(int j=0;j<SIZE;j++){
            env.grid[i][j].pos.x=i;
            env.grid[i][j].pos.y=j;
            env.grid[i][j].has_trash=0;
            env.grid[i][j].is_cleaned=0;
            env.grid[i][j].is_obstacle=0;
        }
    env.moves=0;

    printf("=== Robot Cleaning Simulation ===\n");
    printf("Grid size: %dx%d\n", SIZE, SIZE);
    printf("All cells are available - no obstacles!\n\n");

    // إدخال الروبوت
    int rx, ry;
    printf("Enter robot position (x y, from 0 to %d): ", SIZE-1);
    scanf("%d %d",&rx,&ry);
    while(rx<0 || rx>=SIZE || ry<0 || ry>=SIZE){
        printf("Invalid! Coordinates must be from 0 to %d.\n", SIZE-1);
        printf("Enter robot position: ");
        scanf("%d %d",&rx,&ry);
    }
    env.robot_pos.x=rx;
    env.robot_pos.y=ry;

    // إدخال القمامة
    int trash_num;
    int max_trash = SIZE * SIZE - 1;
    printf("Enter number of trash pieces (0-%d): ", max_trash);
    scanf("%d",&trash_num);
    while(trash_num < 0 || trash_num > max_trash) {
        printf("Invalid number! Enter again (0-%d): ", max_trash);
        scanf("%d",&trash_num);
    }
    env.trash_count=trash_num;

    for(int k=0;k<trash_num;k++){
        int tx, ty;
        printf("Enter trash %d position (x y, from 0 to %d): ", k+1, SIZE-1);
        scanf("%d %d",&tx,&ty);
        while(tx<0 || tx>=SIZE || ty<0 || ty>=SIZE || 
              (tx==rx && ty==ry) || env.grid[tx][ty].has_trash){
            printf("Invalid! Position (%d,%d) is not available.\n", tx, ty);
            printf("Reasons: robot position or already has trash\n");
            printf("Enter new position: ");
            scanf("%d %d",&tx,&ty);
        }
        env.grid[tx][ty].has_trash=1;
    }

    // تحميل القواعد
    load_rules("rules.txt");

    // عرض البيئة الأولية
    printf("\n=== Initial State ===\n");
    print_environment(&env);

    // التشغيل
    int max_iter=100;
    int iter=0;
    while(!all_trash_cleaned(&env) && iter<max_iter){
        make_move(&env);
        print_environment(&env);
        iter++;
        if (iter % 10 == 0) {
            printf("... Continuing (iteration %d)\n", iter);
        }
    }

    if(all_trash_cleaned(&env)) 
        printf("SUCCESS! All trash cleaned in %d moves!\n", env.moves);
    else 
        printf("FAILED! Could not clean all trash within %d moves.\n", max_iter);

    return 0;
}