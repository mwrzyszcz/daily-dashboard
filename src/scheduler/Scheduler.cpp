#include "Scheduler.h"
#include <Arduino.h>
#include "../Logger.h"

Scheduler::~Scheduler()
{
    stop();
}

void Scheduler::every(uint32_t intervalMs, TaskCallback callback)
{
    if (!callback) {
        Logger::error("Scheduler", "Cannot schedule null callback");
        return;
    }

    ScheduledTask task;
    task.callback = callback;
    task.intervalMs = intervalMs;
    tasks_.push_back(task);

    Logger::info("Scheduler", "Task scheduled");
}

void Scheduler::every(uint16_t intervalSec, TaskCallback callback, bool isSeconds)
{
    if (isSeconds) {
        every(static_cast<uint32_t>(intervalSec) * 1000UL, callback);
    } else {
        every(static_cast<uint32_t>(intervalSec), callback);
    }
}

void Scheduler::everyMinutes(uint16_t intervalMin, TaskCallback callback)
{
    every(static_cast<uint32_t>(intervalMin) * 60000UL, callback);
}

void Scheduler::everyHours(uint8_t intervalHours, TaskCallback callback)
{
    every(static_cast<uint32_t>(intervalHours) * 3600000UL, callback);
}

void Scheduler::start()
{
    if (isRunning_) {
        Logger::warn("Scheduler", "Already running");
        return;
    }

    isRunning_ = true;
    Logger::info("Scheduler", "Starting scheduler");

    // Create a single FreeRTOS task that manages all scheduled tasks
    xTaskCreatePinnedToCore(
        &Scheduler::taskWrapper,
        "SchedulerTask",
        4096,                          // Stack size
        this,                           // Parameter (this pointer)
        1,                              // Priority (low, so it doesn't block other tasks)
        nullptr,                        // Task handle
        1                               // Core (run on core 1, leaving core 0 free)
    );
}

void Scheduler::stop()
{
    isRunning_ = false;
    Logger::info("Scheduler", "Stopping");

    // In a real scenario, you'd store task handles and delete them
    // For now, we rely on the wrapper task checking isRunning_ flag
}

size_t Scheduler::taskCount() const noexcept
{
    return tasks_.size();
}

bool Scheduler::isRunning() const noexcept
{
    return isRunning_;
}

void Scheduler::taskWrapper(void* pvParameters)
{
    Scheduler* pThis = static_cast<Scheduler*>(pvParameters);

    while (pThis->isRunning_) {
        unsigned long currentTimeMs = millis();

        // Check each scheduled task
        for (auto& task : pThis->tasks_) {
            if (currentTimeMs - task.lastExecutionMs >= task.intervalMs) {
                task.lastExecutionMs = currentTimeMs;

                // Execute the callback
                try {
                    task.callback();
                } catch (const std::exception& e) {
                    Logger::error("Scheduler", "Task execution failed");
                }
            }
        }

        // Yield to other tasks (sleep for a short time)
        vTaskDelay(pdMS_TO_TICKS(100));  // Check every 100ms
    }

    // Task cleanup
    vTaskDelete(nullptr);
}
