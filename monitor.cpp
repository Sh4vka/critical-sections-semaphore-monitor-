#include <iostream>
#include <queue>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <vector>

struct Call {
    int passengerID;
    int floor;
    std::string direction;
};

// Очередь вызовов и механизм синхронизации
std::queue<Call> callQueue;
std::mutex queueMutex;
std::condition_variable callAvailable;

// Флаг завершения добавления вызовов
bool allCallsAdded = false;

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
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            callQueue.push(call);
            std::cout << "Passenger " << call.passengerID << " calls elevator "
                      << call.direction << " from floor " << call.floor << "\n";
        }
        callAvailable.notify_one(); // Уведомление лифта о новом вызове
    }
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        allCallsAdded = true;
    }
    callAvailable.notify_all(); // Уведомление лифта о завершении добавления вызовов
}

void elevator() {
    while (true) {
        std::unique_lock<std::mutex> lock(queueMutex);
        // Ожидание, пока не появится вызов или все вызовы не будут добавлены
        callAvailable.wait(lock, [] { return !callQueue.empty() || allCallsAdded; });

        if (callQueue.empty() && allCallsAdded) {
            break; 
        }

        // Обработка вызова
        Call call = callQueue.front();
        callQueue.pop();
        lock.unlock(); 

        std::cout << "Elevator responding to: Passenger " << call.passengerID
                  << " calls elevator " << call.direction
                  << " from floor " << call.floor << "\n";

        std::this_thread::sleep_for(std::chrono::seconds(2)); 
        std::cout << "Elevator reached floor " << call.floor
                  << " for Passenger " << call.passengerID << "\n";
    }
}

int main() {
    // Запуск потока для добавления вызовов
    std::thread addCallsThread(addCalls);

    // Запуск потока лифта
    std::thread elevatorThread(elevator);

    // Ожидание завершения потоков
    addCallsThread.join();
    elevatorThread.join();

    return 0;
}
