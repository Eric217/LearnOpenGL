//
//  Config.hpp
//  08-framebuffer
//
//  Created by Eric on 2022/10/22.
//

#ifndef Config_hpp
#define Config_hpp

#include <glm/glm.hpp>

class Scene;
class Camera;

namespace config {

extern int initWindowPointSizeW;
extern int initWindowPointSizeH;

extern int screenPixelW;
extern int screenPixelH;

extern float cameraStep;
extern float cameraFarPlane;

Scene loadScene();

/// 调用者负责 delete
Camera* loadCamera();

glm::mat4 projectionMatrix(float fovDegree);
glm::mat4 perspective(float fovDegree, float whRatio, float near, float far);


}
 

#if NDEBUG
#else

#include <map>

class EfficiencyTool {
public:
    static std::map<int, size_t> TIMES;
    static std::map<int, size_t> TOTAL;
    static void clear() { TIMES.clear(); TOTAL.clear();}
    
    int program;
    int reportInterval;
    clock_t time;
    size_t &times;
    size_t &total;
    EfficiencyTool(int program, int interval): program(program) ,time(clock()),
    times(TIMES[program]), total(TOTAL[program]), reportInterval(interval) {
        if (reportInterval == 0) {
            reportInterval = 1;
        }
    }
    ~EfficiencyTool() {
        auto sec = clock() - time;
        times += 1;
        total += sec;
        if (times % reportInterval) return;
        auto avg = total * 1.0 / CLOCKS_PER_SEC / times;
        printf("fill %d avg: %.3f ms, %lu times, total %0.2f s\n", program, avg * 1000, times, total * 1.0 / CLOCKS_PER_SEC);
    }
};

#endif

#endif /* Config_hpp */
