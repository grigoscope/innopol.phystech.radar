#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//PowerShell - Measure-Command { ./_Finally/src/main.exe }

//Управление принтом
#define M_DEBUG false
#define M_PRINT_RES false

//Настройки для алгоритма 
#define NUM_ITERATION       20
#define EVAPORATION_RATE    0.2
#define ALPHA               1
#define BETA                2

//Переменные графа и весов
int Verticles;
int **Matrix;
float *Weight;

//Буферы для чтения
char Buf_line[1000];
char Buf_line_copy[1000];

//Функция чтения файла
int read_file(char* path)
{
    FILE *file;
    // Открытие файла
    file = fopen(path, "r");
    if (file == NULL) {
        printf("Error then opening file %s.\n", path);
        return 1;
    }

    //Чтение первой строки
    fgets(Buf_line, sizeof(Buf_line), file);

    //strtok уничтожает строку, делаем копию
    memcpy((void*)Buf_line_copy, (void*)Buf_line, sizeof(Buf_line)/sizeof(char));

    //Получение набора элементов строки
    char *token = strtok(Buf_line, ",");

    int count = 0;
    while (token != NULL) {
        count++;
        token = strtok(NULL,",");
    }

    //Запоминаем кол-во вершин
    Verticles = count;

    //Выделение памяти на матрицу
    Matrix = (int**) malloc(Verticles * sizeof(int*));
    for (int i = 0; i < Verticles; i++) 
        Matrix[i] = (int*) malloc(Verticles * sizeof(int));

    //Выделение памяти на веса
    Weight = (float*)malloc(Verticles * sizeof(float));

    //Заполнение матрицы
    for(int i=0;i<Verticles;i++ )
    {
        //Начинаем обработку строки
        token = strtok(Buf_line_copy,",");    

        //Забираем все элементы
        for(int j=0;j<Verticles;j++)
        {
            Matrix[i][j] = atoi(token);
            token = strtok(NULL, ","); 
        }
        fgets(Buf_line_copy, sizeof(Buf_line_copy), file);
    } 
    
    token = strtok(Buf_line_copy,","); 

    //Забираем все элементы весов
    for(int j=0;j<Verticles;j++)
    {
        Weight[j] = atof(token);
        token = strtok(NULL, ","); 
    }

    //Закрытие файла
    fclose(file);

    return 0;
}

//Расчет веса независимого множества
float calculate_weight(int* independent_set)
{
    float score = 0;
    for(int l=0; l<Verticles; l++)
    {
        if(independent_set[l] <0)
            continue;
        score += Weight[independent_set[l]];
    }
    return score; 
}


//Проверка на независимость множества
bool is_independent_set(int* vertices)
{
    for (int v1_=0; v1_<Verticles; v1_++)
        for (int v2_=0; v2_<Verticles; v2_++)
        {
            int v1 = vertices[v1_];
            int v2 = vertices[v2_];
            if(v1<0 || v2<0)
                continue;
            if (v1 != v2 && Matrix[v1][v2] == 1)
                return false;
        }
    return true;
}

//Проверка наличия значения в массиве
bool contains(int arr[], int size, int value) {
    for (int i = 0; i < size; i++) {
        if (arr[i] == value) {
            return true;
        }
    }
    return false;
}

int* ant_colony_optimization(int* block)
{
    //Random seed
    srand(time(NULL));

    //Массивы решений и вес решения
    int* best_solution = (int*) malloc(Verticles * sizeof(int));
    int* current_solution = (int*) malloc(Verticles * sizeof(int));
    memset((void*)current_solution, -1, Verticles*sizeof(int));
    memset((void*)best_solution, -1, Verticles*sizeof(int));

    //Массив обработки решений
    float** next_try = (float**)malloc(Verticles * sizeof(float*));
    for (int i = 0; i < Verticles; i++) 
    {
        next_try[i] = (float*) malloc(2 * sizeof(float));
        next_try[i][0] = -1.0;
        next_try[i][1] = -1.0;
            
    }

    //Массив для проверки совместности
    int* next_ = (int*)malloc(Verticles * sizeof(int));
    memset((void*)next_, -1, Verticles*sizeof(int));

    //Выделение памяти на Феромоны и заполнение стартовыми значениями
    float** pheromone = (float**)malloc(Verticles * sizeof(float*));
    for (int i = 0; i < Verticles; i++) 
    {
        pheromone[i] = (float*) malloc(Verticles * sizeof(float));
        for (int j = 0; j < Verticles; j++) 
            pheromone[i][j] = 1.0;
    }

    float best_weight = (float)INT_MIN;
    int next_node = 0;

    for (int iteration=0; iteration<NUM_ITERATION; iteration++)
    {
        //Сбрасываем текущее решение
        memset((void*)current_solution, -1, Verticles*sizeof(int));

        //Индексы
        int cs_i = 0;     
        //Стартуем со случайной вершины
        int current_node = rand() % (Verticles);

        //но эта вершина не должна быть в блоке
        while(contains(block, 5, current_node))
            current_node = rand() % (Verticles);

        //Добавляем вершину в решение
        current_solution[cs_i++] = current_node;

        //
        while (cs_i < Verticles)
        //for(int jj=0; jj<100; jj++)
        {   
            //Индексы
            int nt_i = 0; 

            //Сбрсываем тестовую попытку
            for(int i=0;i<Verticles;i++)
            {
                next_try[i][0] = -1.0;
                next_try[i][1] = -1.0;
            }

            for (int i=0; i<Verticles; i++)
            {   
                //Если вершина заблокированна, ее нельзя использовать
                if(contains(block, 5, i))
                    continue;

                //Если вершина уже находится в решении, она игнорируется
                if(contains(current_solution, Verticles, i))
                    continue;

                //Попытка принять узел в решение
                memset((void*)next_, -1, Verticles*sizeof(int));
                memcpy((void*)next_, (void*)current_solution, Verticles*sizeof(int));
                next_[(cs_i+1)] = i;

                //Проверка на независимость узла
                if(is_independent_set(next_) == false)
                    continue; 

                //Добавление узла в решение
                next_try[nt_i][0] = i;
                float ver = pow(pheromone[current_node][i],ALPHA);
                ver *= pow(Weight[i], BETA);
                next_try[nt_i++][1] = ver;

            }

            //Проверка на наличие решений
            bool break_ = true;
            for (int i=0; i<Verticles; i++)
            {   
                if(next_try[i][0] != -1)
                    break_ = false;
            }
            //Если их нет - выходим
            if(break_)
                break;


            //Операция нормирования вероятностей
            float rnd_sum = 0;
            for(int t=0;t<Verticles;t++)
            {
                if(next_try[t][1] <0)
                    continue;
                rnd_sum+=next_try[t][1];
            }
            for(int t=0;t<Verticles;t++)
            {
                if(next_try[t][1] <0)
                    continue;
                next_try[t][1] /= rnd_sum;
            }                

            //Выбираем следующую вершину для муравья
            float rr = (float)rand() / RAND_MAX;
            rnd_sum = 0;
            for(int t=0;t<Verticles;t++)
            {
                if(next_try[t][1] <0)
                    continue;
                rnd_sum +=next_try[t][1];
                if(rnd_sum>rr)
                {
                    next_node = next_try[t][0];
                    break;
                }
            }

            current_solution[cs_i++] = next_node;
            current_node = next_node;
        }

        //Сравнение текущего результата с лучшим
        float current_weight = calculate_weight(current_solution);

        if (current_weight > best_weight)
        {
            memcpy((void*)best_solution, (void*)current_solution, Verticles*sizeof(int));
            best_weight = current_weight;
        }

        //Вывод промежуточного результа
        #if M_DEBUG
            printf("%d = %5.3f =", iteration, best_weight);
            for(int t=0;t<Verticles;t++)
                printf(" %d", best_solution[t]);
            printf("\n");
        #endif

        //Обновление феромонов
        for (int i = 0; i < Verticles; i++) 
        {
            for (int j = 0; j < Verticles; j++) 
                pheromone[i][j] *= (1.0 - EVAPORATION_RATE);
        }
        for (int i=0; i<Verticles-1; i++)
        {
            if(current_solution[i] <0 || current_solution[i+1] <0)
                continue;

            int indx_i = current_solution[i];
            int indx_j = current_solution[i+1];
            pheromone[indx_i][indx_j] += current_weight/50.0;
        } 
    }
    //Освобождаем память
    free(current_solution);
    free(next_);

    for (int i=0; i<Verticles; i++)
    {
        free(pheromone[i]);
        free(next_try[i]);
    }
    free(pheromone);
    free(next_try);

    return best_solution;
}


int compare(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}

int main(int argc, char *argv[])
{
    #if M_PRINT_RES
        //Засекаем время
        clock_t begin = clock();
    #endif

    //Проверка кол-ва аргументов
    if (argc != 3) {
            printf("Задайте 2 входных аргумента как пути к файлам! : путь_к_входному_файлу путь_к_выходному_файлу \nВведено - %s\n", argv[0]);
            return 1;
        }

    //Получение путей к файлам
    char *input_path = argv[1];
    char *output_path = argv[2];

    #if M_DEBUG
        printf("Путь к файлу 1: %s\n", input_path);
        printf("Путь к файлу 2: %s\n", output_path);
    #endif

    //Чтение входных данных
    // char input_path[] = "C:\\Users\\vnekt\\Desktop\\Roga\\RadarHACK\\26_04\\input.csv";
    // char output_path[] ="C:\\Users\\vnekt\\Desktop\\Roga\\RadarHACK\\26_04\\output.csv";

    if(read_file(input_path) != 0)
        return 1;

    //Преобразуем матрицу совместности траекторий в матрицу смежности
    for (int i = 0; i < Verticles; i++) 
    {
        for (int j = 0; j < Verticles; j++) 
        {
            if (Matrix[i][j] == 0 && i<j)
                Matrix[i][j] = 1;
            else 
                Matrix[i][j] = 0;
        }
    }

    //Вывод матрицы смежности и вектора весов
    #if M_DEBUG
        for (int i = 0; i < Verticles; i++) 
        {
            for (int j = 0; j < Verticles; j++) 
            {
                printf("%d ", Matrix[i][j]);
            }
            printf("\n");
        }
        printf("\n\n");
        for (int i = 0; i < Verticles; i++) 
        {
            printf("%4.3f ", Weight[i]);
        }
        printf("\n");
    #endif


    //Открытие файла для сохранения данных
    FILE *file = fopen(output_path, "w");
    if (file == NULL) {
        printf("Error opening file %s.\n", output_path);
        return 1;
    }

    // Заполнение выходного файла
    // Проверочные данные начинаются с 1, но нумерация в C с 0
    for (int i = 0; i < Verticles; i++) {
        fprintf(file, ",TH%d", i + 1);
    }
    fprintf(file, ",sum(w)");

    //Блокировщик наименее важных узлов
    int block[5] = {-1,-1,-1,-1,-1};
    int block_indx = 0;

    //Генерация 5 независимых множеств с самой большой суммой
    for(int try_=0; try_<5 ;try_++)
    {
        //Генерация решения
        int* best_sol = ant_colony_optimization(block);
        qsort(best_sol, Verticles, sizeof(int), compare);
        float weight = calculate_weight(best_sol);

        #if M_PRINT_RES
            for (int i = 0; i < Verticles; i++) 
            {
                printf("%d, ", best_sol[i]);
            }   
            printf(" --- w = %4.3f\n", weight);
        #endif

        //Преобразование данных
        int* out_data = (int*)malloc(Verticles*sizeof(int));
        memset((void*)out_data, 0, Verticles*sizeof(int));

        for(int i = 0; i < Verticles; i++) 
        {
            if(best_sol[i] < 0)
                continue;
            out_data[best_sol[i]] = 1;
        }

        //Сохранение результата
        fprintf(file, "\nGH%d", try_ + 1);
        for (int i = 0; i < Verticles; i++) {
            fprintf(file, ",%d", out_data[i]);
        }
        fprintf(file, ",%.4f", weight);
        free(out_data);
        
        //Поиск слабого звена
        int min_ = INT_MAX;
        int indx = -1;
        for (int i = 0; i < Verticles; i++) 
        {
            if(best_sol[i] <0)
                continue;
            if(min_ > Weight[best_sol[i]])
            {
                min_ = Weight[best_sol[i]];
                indx = best_sol[i];
            }
        }
        //Блокировка узла
        block[block_indx++] = indx;
        free(best_sol);
    }
   
    fclose(file);

    //Возврат памяти
    for (int i = 0; i < Verticles; i++) 
    {
        free(Matrix[i]);
    }
    free(Matrix);
    free(Weight);
    // free(best_sol);

    #if M_PRINT_RES
        clock_t end = clock();
        double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        printf("The elapsed time is %f seconds", time_spent);
    #endif

    return 0;
}