#pragma once
#include <unordered_map>
#include <vector>
#include <string>

struct SpriteAnimationComponent {
    struct Animation {
        int frames;
        float frameTime;
        bool loop;
    };

    std::unordered_map<std::string, Animation> animations;
    std::string currentAnim;
    int currentFrame = 0;
    float timer = 0.0f;
    bool playing = true;
    int sheetColumns = 1;
    int sheetRows = 1;
};
