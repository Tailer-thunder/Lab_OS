#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <yaml-cpp/yaml.h>
#include <set>
#include <algorithm>
#include <iomanip>
#include <sstream>

// ANSI-коды для цветов 
const std::string RESET = "\033[0m";
const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string CYAN = "\033[36m";

// Функция для логирования сообщений с цветами
void log_message(const std::string& level, const std::string& message) {
    std::string color;
    if (level == "LOAD" || level == "INFO" || level == "DEBUG" || level == "READY") {
        color = CYAN;
    } else if (level == "START" || level == "COMPLETE") {
        color = GREEN;
    } else if (level == "ERROR" || level == "FAIL" || level == "SCHEDULER" || level == "FINAL" || level == "SKIP") {
        color = RED;
    } else {
        color = RESET;
    }

    std::cout << color << "[" << level << "] " << message << RESET << std::endl;
}

// Структура для описания джобы
struct Job {
    int id;
    std::vector<int> dependencies;
};

// Статусы джоб
enum class JobStatus {
    PENDING,
    RUNNING,
    COMPLETED,
    FAILED
};

// Глобальные переменные для синхронизации и управления
std::mutex mtx;
std::condition_variable cv;
int active_jobs = 0;
const int MAX_PARALLEL_JOBS = 1;
bool dag_failed = false;

// Граф зависимостей (job_id -> список зависимых job_id)
std::map<int, std::vector<int>> graph;

// мапа indegree (job_id -> количество незавершённых зависимостей)
std::map<int, int> indegree_map;

// Очередь готовых к выполнению джоб (job_ids)
std::queue<int> ready_jobs;

// мапа джоб для быстрого доступа (job_id -> Job)
std::map<int, Job> job_map;

// мапа статусов джоб (job_id -> JobStatus)
std::map<int, JobStatus> job_status_map;

// Функция для выполнения джобы
void execute_job(int job_id) {
    {
        std::unique_lock<std::mutex> lock(mtx);
        if (dag_failed) {
            log_message("SKIP", "DAG has failed. Skipping job: " + std::to_string(job_id));
            active_jobs--;
            cv.notify_all();
            return;
        }
        // Устанавливаем статус RUNNING здесь, чтобы избежать дублирования
        job_status_map[job_id] = JobStatus::RUNNING;
    }

    log_message("START", "Executing job: " + std::to_string(job_id));
    std::this_thread::sleep_for(std::chrono::seconds(1)); // Симуляция работы

    // Симуляция ошибки для джобы с id == 5
    bool job_failed = false;
    //if (job_id == 5) {
      //  log_message("ERROR", "Job " + std::to_string(job_id) + " failed!");
        //job_failed = true;
    //}

    // Обновление состояния после выполнения
    {
        std::unique_lock<std::mutex> lock(mtx);
        if (job_failed) {
            dag_failed = true;
            job_status_map[job_id] = JobStatus::FAILED;
            log_message("FAIL", "Job " + std::to_string(job_id) + " has failed. Setting dag_failed=true.");
        } else {
            job_status_map[job_id] = JobStatus::COMPLETED;
            log_message("COMPLETE", "Completed job: " + std::to_string(job_id));
        }

        active_jobs--;

        if (job_failed) {
            cv.notify_all(); // Уведомляем все потоки о провале
            return;
        }

        // Обновляем зависимости для зависимых джоб
        for (int dependent_job_id : graph[job_id]) {
            indegree_map[dependent_job_id]--;
            log_message("UPDATE", "Decremented indegree of job " + std::to_string(dependent_job_id) + " to " + std::to_string(indegree_map[dependent_job_id]));

            // Проверка на отрицательное значение indegree
            if (indegree_map[dependent_job_id] < 0) {
                log_message("ERROR", "Indegree of job " + std::to_string(dependent_job_id) + " became negative!");
                dag_failed = true;
                cv.notify_all();
                return;
            }

            if (indegree_map[dependent_job_id] == 0 && job_status_map[dependent_job_id] == JobStatus::PENDING) {
                log_message("READY", "Job " + std::to_string(dependent_job_id) + " is now ready to execute.");
                ready_jobs.push(dependent_job_id);
            }
        }

        // Уведомляем главный поток о новых готовых джобах
        cv.notify_all();
    }
}

// Функция для проверки корректности DAG
bool validate_dag(const std::vector<Job>& jobs) {
    if (jobs.empty()) {
        log_message("VALIDATION ERROR", "DAG is empty.");
        return false;
    }

    // Временные карты для валидации
    std::map<int, int> temp_indegree; // job_id -> indegree
    std::map<int, std::vector<int>> temp_graph; // job_id -> список зависимых job_ids
    std::map<int, std::vector<int>> undirected_graph; // job_id -> список связанных job_ids (неориентированный граф)

    // Инициализация графов
    for (const auto& job : jobs) {
        temp_indegree[job.id] = 0;
        temp_graph[job.id] = {};
        undirected_graph[job.id] = {};
    }

    // Построение графов зависимостей и неориентированного графа для проверки связности
    for (const auto& job : jobs) {
        for (int dep : job.dependencies) {
            if (temp_graph.find(dep) == temp_graph.end()) {
                log_message("VALIDATION ERROR", "Dependency error: Job " + std::to_string(dep) + " not found.");
                return false;
            }
            temp_graph[dep].push_back(job.id);
            temp_indegree[job.id]++;
            log_message("DEBUG", "Processing dependency: Job " + std::to_string(job.id) + " depends on Job " + std::to_string(dep) + ". New indegree: " + std::to_string(temp_indegree[job.id]));

            // Для неориентированного графа
            undirected_graph[job.id].push_back(dep);
            undirected_graph[dep].push_back(job.id);
        }
    }

    // Создаём копию indegree_map для валидации
    std::map<int, int> validation_indegree = temp_indegree;

    // Проверка на наличие циклов с помощью топологической сортировки
    std::queue<int> q;
    for (const auto& [id, degree] : validation_indegree) {
        if (degree == 0) {
            q.push(id);
        }
    }

    int processed = 0;
    while (!q.empty()) {
        int current = q.front();
        q.pop();
        processed++;

        for (int neighbor : temp_graph[current]) {
            validation_indegree[neighbor]--;
            if (validation_indegree[neighbor] == 0) {
                q.push(neighbor);
            }
        }
    }

    if (processed != jobs.size()) {
        log_message("VALIDATION ERROR", "Invalid DAG configuration: contains cycles.");
        return false;
    }

    // Проверка связности графа с помощью BFS на неориентированном графе
    std::queue<int> bfs_q;
    std::set<int> bfs_visited;

    int start_node = jobs[0].id;
    bfs_q.push(start_node);
    bfs_visited.insert(start_node);

    while (!bfs_q.empty()) {
        int current = bfs_q.front();
        bfs_q.pop();

        for (int neighbor : undirected_graph[current]) {
            if (bfs_visited.find(neighbor) == bfs_visited.end()) {
                bfs_visited.insert(neighbor);
                bfs_q.push(neighbor);
            }
        }
    }

    if (bfs_visited.size() != jobs.size()) {
        log_message("VALIDATION ERROR", "Invalid DAG configuration: contains disconnected components.");
        return false;
    }

    // Проверка наличия хотя бы одной стартовой и одной завершающей джобы
    int start_jobs = 0;
    int end_jobs = 0;

    for (const auto& job : jobs) {
        if (job.dependencies.empty()) {
            start_jobs++;
        }

        bool is_end = true;
        for (const auto& other_job : jobs) {
            if (std::find(other_job.dependencies.begin(), other_job.dependencies.end(), job.id) != other_job.dependencies.end()) {
                is_end = false;
                break;
            }
        }
        if (is_end) {
            end_jobs++;
        }
    }

    if (start_jobs == 0) {
        log_message("VALIDATION ERROR", "Invalid DAG configuration: no start jobs (jobs without dependencies).");
        return false;
    }

    if (end_jobs == 0) {
        log_message("VALIDATION ERROR", "Invalid DAG configuration: no end jobs (jobs without dependents).");
        return false;
    }

    // Дополнительный отладочный вывод исходного indegree_map
    std::string indegree_output = "indegree_map after validation:\n";
    for (const auto& [id, degree] : temp_indegree) {
        indegree_output += "  Job ID: " + std::to_string(id) + ", indegree: " + std::to_string(degree) + "\n";
    }
    log_message("DEBUG", indegree_output);

    // Если все проверки пройдены, инициализируем глобальные графы и indegree_map
    {
        std::unique_lock<std::mutex> lock(mtx);
        graph = temp_graph;
        indegree_map = temp_indegree; // Используем неизменённые значения
    }

    log_message("VALIDATION", "DAG validation successful.");
    return true;
}

int main(int argc, char* argv[]) {
    // Имя YAML файла, можно передавать через аргументы командной строки
    std::string filename = "dag_config.yaml";
    if (argc > 1) {
        filename = argv[1];
    }

    // Чтение и загрузка YAML файла
    YAML::Node config;
    try {
        config = YAML::LoadFile(filename);
        log_message("LOAD", "Successfully loaded " + filename);
    } catch (const YAML::BadFile& e) {
        log_message("ERROR", "Unable to open file '" + filename + "'. Check if the file exists and is accessible.");
        return 1;
    } catch (const YAML::ParserException& e) {
        log_message("ERROR", "Error parsing YAML file: " + std::string(e.what()));
        return 1;
    }

    // Парсинг джобов из YAML файла
    std::vector<Job> jobs;
    try {
        for (const auto& node : config["jobs"]) {
            Job job;
            job.id = node["job_id"].as<int>();
            if (node["dependencies"]) {
                for (const auto& dep : node["dependencies"]) {
                    job.dependencies.push_back(dep.as<int>());
                }
            }
            jobs.push_back(job);
        }

        // Вывод загруженных джоб для отладки
        std::string info_output = "Jobs loaded:\n";
        for (const auto& job : jobs) {
            info_output += "  Job ID: " + std::to_string(job.id) + ", Dependencies: ";
            if (job.dependencies.empty()) {
                info_output += "None";
            } else {
                for (size_t i = 0; i < job.dependencies.size(); ++i) {
                    info_output += std::to_string(job.dependencies[i]);
                    if (i != job.dependencies.size() - 1) {
                        info_output += ", ";
                    }
                }
            }
            info_output += "\n";
        }
        log_message("INFO", info_output);
    } catch (const YAML::Exception& e) {
        log_message("ERROR", "Error parsing YAML: " + std::string(e.what()));
        return 1;
    }

    // Проверка корректности DAG
    if (!validate_dag(jobs)) {
        return 1;
    }

    // Создание карты джоб для быстрого доступа и инициализация статусов
    {
        std::unique_lock<std::mutex> lock(mtx);
        for (const auto& job : jobs) {
            job_map[job.id] = job;
            job_status_map[job.id] = JobStatus::PENDING;
        }
    }

    // Инициализация очереди готовых джоб (indegree_map == 0)
    {
        std::unique_lock<std::mutex> lock(mtx);
        std::string ready_jobs_output = "";
        while (!ready_jobs.empty()) ready_jobs.pop(); // Очистка очереди на всякий случай
        for (const auto& [id, degree] : indegree_map) {
            if (degree == 0) {
                ready_jobs_output += std::to_string(id) + ", ";
                ready_jobs.push(id);
            }
        }
        if (!ready_jobs_output.empty()) {
            ready_jobs_output.pop_back(); // Удаление последней запятой
            ready_jobs_output.pop_back();
            log_message("READY", "Job " + ready_jobs_output + " are ready to execute.");
        }
    }

    // Вектор для хранения всех потоков
    std::vector<std::thread> threads;

    // Основной цикл управления выполнением джоб
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&] {
            return (!ready_jobs.empty() && active_jobs < MAX_PARALLEL_JOBS) ||
                   dag_failed ||
                   (active_jobs == 0 && ready_jobs.empty());
        });

        if (dag_failed) {
            log_message("SCHEDULER", "DAG execution failed. Stopping scheduler.");
            break;
        }

        // Запуск готовых джоб
        while (!ready_jobs.empty() && active_jobs < MAX_PARALLEL_JOBS) {
            int current_job_id = ready_jobs.front();
            ready_jobs.pop();

            // Увеличиваем счетчик активных джоб
            active_jobs++;

            // Запускаем джобу в отдельном потоке с обработкой исключений
            threads.emplace_back([current_job_id]() {
                try {
                    execute_job(current_job_id);
                } catch (const std::exception& e) {
                    log_message("ERROR", "Exception in job " + std::to_string(current_job_id) + ": " + e.what());
                }
            });
        }

        // Если все джобы выполнены и нет активных джоб, завершить цикл
        if (active_jobs == 0 && ready_jobs.empty()) {
            log_message("COMPLETE", "All jobs have been executed.");
            break;
        }
    }

    // Дожидание завершения всех потоков
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    if (dag_failed) {
        log_message("FINAL", "DAG execution was aborted due to job failure.");
        return 1;
    }

    log_message("FINAL", "DAG execution completed successfully!");
    return 0;
}
