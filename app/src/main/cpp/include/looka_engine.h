#ifndef LOOKA_ENGINE_H
#define LOOKA_ENGINE_H

#include <atomic>
#include <memory>

// تعريف كلاس المحرك الرئيسي
class LookaEngineFinal {
public:
    LookaEngineFinal();
    ~LookaEngineFinal();
    
    void run();
    void stop();
    int getCurrentFPS() const;
    
    // مكونات المحرك
    std::unique_ptr<class DynamicAimAssist> aim_assist;
    std::unique_ptr<class HardcoreAimbot> hardcore_aimbot;
    std::unique_ptr<class FlickDetector> flick_detector;
    
private:
    std::atomic<bool> running{false};
    int current_fps{0};
};

#endif // LOOKA_ENGINE_H