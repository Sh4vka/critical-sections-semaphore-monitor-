#include <iostream>
#include <queue>
#include <thread>
#include <semaphore.h>
#include <chrono>
#include <vector>
#include <atomic>

struct Call {
    int passengerID;
    int floor;
    std::string direction;
};

// Очередь вызовов
std::queue<Call> callQueue;

// Семафоры
sem_t queueAccess;   // Доступ к очереди
sem_t callAvailable; // Количество вызовов

// Флаг завершения добавления вызовов
std::atomic<bool> allCallsAdded(false);

// Массив вызовов для упрощения порядка создания
std::vector<Call> calls = {
    {1, 2, "UP"},
    {2, 5, "DOWN"},
    {3, 3, "UP"},
    {4, 6, "DOWN"}
};

// Функция для добавления вызовов
void addCalls() {
    for (const auto& call : calls) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        sem_wait(&queueAccess); // Захват доступа к очереди
        callQueue.push(call);
        std::cout << "Passenger " << call.passengerID << " calls elevator "
                  << call.direction << " from floor " << call.floor << "\n";
        sem_post(&queueAccess); // Освобождение доступа к очереди
        sem_post(&callAvailable); // Увеличение количества вызовов
    }
    // Установка флага завершения добавления вызовов
    allCallsAdded.store(true);
}

void elevator() {
    while (true) {
        // Проверяем, есть ли вызовы или завершено добавление вызовов
        if (allCallsAdded.load() && callQueue.empty()) {
            break;
        }

        sem_wait(&callAvailable); // Ожидание вызова
        sem_wait(&queueAccess);   // Захват доступа к очереди

        if (!callQueue.empty()) {
            Call call = callQueue.front();
            callQueue.pop();
            sem_post(&queueAccess); // Освобождение доступа к очереди

            std::cout << "Elevator responding to: Passenger " << call.passengerID
                      << " calls elevator " << call.direction
                      << " from floor " << call.floor << "\n";

            std::this_thread::sleep_for(std::chrono::seconds(2)); 
            std::cout << "Elevator reached floor " << call.floor
                      << " for Passenger " << call.passengerID << "\n";
        } else {
            sem_post(&queueAccess); // Освобождение доступа к очереди
        }
    }
}

int main() {
    // Инициализация семафоров
    sem_init(&queueAccess, 0, 1); // Один поток может захватывать очередь
    sem_init(&callAvailable, 0, 0); // Изначально вызовов нет

    // Запуск потока для добавления вызовов
    std::thread addCallsThread(addCalls);

    // Запуск потока лифта
    std::thread elevatorThread(elevator);

    // Ожидание завершения потоков
    addCallsThread.join();
    elevatorThread.join();

    sem_destroy(&queueAccess);
    sem_destroy(&callAvailable);

    return 0;
}
