#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h> 

#define MAX_THREADS 6

typedef struct {
    double x, y, z;
} Point;

typedef struct {
    Point *points;
    int num_points;
    int thread_id;
    int total_threads;
    double max_area;
    Point p1, p2, p3;
} ThreadData;


double triangle_area(Point a, Point b, Point c) {
    double ab = sqrt(pow(b.x - a.x, 2) + pow(b.y - a.y, 2) + pow(b.z - a.z, 2));
    double ac = sqrt(pow(c.x - a.x, 2) + pow(c.y - a.y, 2) + pow(c.z - a.z, 2));
    double bc = sqrt(pow(c.x - b.x, 2) + pow(c.y - b.y, 2) + pow(c.z - b.z, 2));
    double s = (ab + ac + bc) / 2;
    return sqrt(s * (s - ab) * (s - ac) * (s - bc));
}

void *find_max_area(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    double max_area = 0;
    Point p1, p2, p3;

    for (int i = data->thread_id; i < data->num_points - 2; i += data->total_threads) {
        for (int j = i + 1; j < data->num_points - 1; j++) {
            for (int k = j + 1; k < data->num_points; k++) {
                double area = triangle_area(data->points[i], data->points[j], data->points[k]);
                if (area > max_area) {
                    max_area = area;
                    p1 = data->points[i];
                    p2 = data->points[j];
                    p3 = data->points[k];
                }
            }
        }
    }

    data->max_area = max_area;
    data->p1 = p1;
    data->p2 = p2;
    data->p3 = p3;

    return NULL;
}

int main() {
    int num_points;

    printf("Введите количество точек: ");
    if (scanf("%d", &num_points) != 1 || num_points < 3) {
        fprintf(stderr, "Ошибка: необходимо ввести целое число >= 3.\n");
        return 1;
    }

    Point* points = (Point*)malloc(num_points * sizeof(Point));
    if (points == NULL) {
        fprintf(stderr, "Ошибка выделения памяти\n");
        return 1;
    }

    printf("Введите координаты точек (x, y, z):\n");
    for (int i = 0; i < num_points; ++i) {
        printf("Точка %d: ", i + 1);
        if (scanf("%lf %lf %lf", &points[i].x, &points[i].y, &points[i].z) != 3) {
            fprintf(stderr, "Ошибка ввода координат для точки %d\n", i + 1);
            free(points);
            return 1;
        }
    }

    int max_threads = (num_points < MAX_THREADS) ? num_points : MAX_THREADS; 

    pthread_t threads[MAX_THREADS];
    ThreadData thread_data[MAX_THREADS];

    clock_t start = clock();

    for (int i = 0; i < max_threads; i++) {
        thread_data[i].points = points;
        thread_data[i].num_points = num_points;
        thread_data[i].thread_id = i;
        thread_data[i].total_threads = max_threads;
        thread_data[i].max_area = 0;
        pthread_create(&threads[i], NULL, find_max_area, &thread_data[i]);
    }

    double overall_max_area = 0;
    Point final_p1, final_p2, final_p3;

    for (int i = 0; i < max_threads; i++) {
        pthread_join(threads[i], NULL);
        if (thread_data[i].max_area > overall_max_area) {
            overall_max_area = thread_data[i].max_area;
            final_p1 = thread_data[i].p1;
            final_p2 = thread_data[i].p2;
            final_p3 = thread_data[i].p3;
        }
    }

    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Максимальная площадь треугольника: %.6lf\n", overall_max_area);
    printf("Точки треугольника:\n");
    printf("1. (%.2lf, %.2lf, %.2lf)\n", final_p1.x, final_p1.y, final_p1.z);
    printf("2. (%.2lf, %.2lf, %.2lf)\n", final_p2.x, final_p2.y, final_p2.z);
    printf("3. (%.2lf, %.2lf, %.2lf)\n", final_p3.x, final_p3.y, final_p3.z);
    printf("Время выполнения: %.6f секунд\n", time_spent);

    free(points); 
    return 0;
}
