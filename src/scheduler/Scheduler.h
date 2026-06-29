#pragma once

#include <functional>
#include <vector>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/**
 * @brief Task scheduler using FreeRTOS for battery-efficient periodic tasks.
 *
 * Supports scheduling tasks with different intervals:
 * - milliseconds for fast updates (clock)
 * - seconds for regular operations
 * - minutes/hours for infrequent operations (weather, nameday)
 */
class Scheduler
{
public:
    using TaskCallback = std::function<void()>;

    Scheduler() = default;
    ~Scheduler();

    /**
     * @brief Schedule a task to run every N milliseconds.
     *
     * @param intervalMs Interval in milliseconds
     * @param callback Function to call periodically
     */
    void every(uint32_t intervalMs, TaskCallback callback);

    /**
     * @brief Schedule a task to run every N seconds.
     *
     * @param intervalSec Interval in seconds
     * @param callback Function to call periodically
     */
    void every(uint16_t intervalSec, TaskCallback callback, bool isSeconds);

    /**
     * @brief Schedule a task to run every N minutes.
     *
     * @param intervalMin Interval in minutes
     * @param callback Function to call periodically
     */
    void everyMinutes(uint16_t intervalMin, TaskCallback callback);

    /**
     * @brief Schedule a task to run every N hours.
     *
     * @param intervalHours Interval in hours
     * @param callback Function to call periodically
     */
    void everyHours(uint8_t intervalHours, TaskCallback callback);

    /**
     * @brief Start the scheduler (creates FreeRTOS tasks).
     */
    void start();

    /**
     * @brief Stop all scheduled tasks.
     */
    void stop();

    size_t taskCount() const noexcept;
    bool isRunning() const noexcept;

private:
    struct ScheduledTask
    {
        TaskCallback callback;
        uint32_t intervalMs;
        TaskHandle_t handle{nullptr};
        volatile unsigned long lastExecutionMs{0};
    };

    std::vector<ScheduledTask> tasks_;
    volatile bool isRunning_{false};

    static void taskWrapper(void* pvParameters);
};
