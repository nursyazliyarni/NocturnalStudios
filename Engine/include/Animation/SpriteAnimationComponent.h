#pragma once
#include <unordered_map>
#include <vector>
#include <string>

struct SpriteAnimationComponent {
    struct Animation {
        std::vector<int> frames;
        float frameTime = 0.4f;
        bool loop = true;
    };

    std::unordered_map<std::string, Animation> animations;
    std::string currentAnim;
    int currentFrame = 0;
    float timer = 0.0f;
    bool playing = false;
    int sheetColumns = 1;
    int sheetRows = 1;
};
