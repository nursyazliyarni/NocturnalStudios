/*!------------------------------------------------------------------------------------------------
    File: Physics.cpp
    Project: NocturnalStudio

    This file contains all the behaviour of our sprite when the correct input key is detected

    Primary Author:
         Ang Yu Xuan (60%)
        - implement functions for movement, scaling and rotation for our sprite.

    Secondary Author:
        Rishika Bimal Attodi (33%)
        - upgrade the scaling and rotation function.

    Secondary Author:
        Chang Qi Xuan Celest (7%)
            - Implemented stop movement on collision in update_sprite_movement.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

// integration, gravity, movement
#include <algorithm> // std::fill
#include "input/Input.h"
#include "physics/Physics.h"
#include "physics/AABB.h"
#include "graphics/DebugDraw.h"

#include <unordered_map>
#include <vector>

static bool showCollision = false;

std::unordered_map<int, std::vector<float>> Physics::sceneScales;
bool Physics::globalResetRequested = false;


int Physics::take_control(Renderer& render_obj, Input& input_obj) {
    int sel = -1; // NONE by default
    auto& kb = input_obj.GET_keyboard();

    // whichever key currently has take_control "wins"
    for (size_t i = 0; i < kb.size(); ++i) {
        int key = std::get<0>(kb[i]);
        Input_action st = std::get<2>(kb[i]);
        if (st == Input_action::take_control) {
            if (key == GLFW_KEY_0) sel = -2;                              // ALL
            else if (key >= GLFW_KEY_1 && key <= GLFW_KEY_9) sel = (key - GLFW_KEY_1); // 1->0
            // (cleanup ensures only one active, so no need to break)
        }
    }

    // clamp to sprite count
    size_t n = render_obj.get_sprite_vector().size();
    if (sel >= 0 && (size_t)sel >= n) sel = (n ? (int)n - 1 : -1);
    return sel;
}


void Physics::update_sprite_movement(Renderer& render_obj, Input& input_obj){
    for (auto& [tuple_key, event_action, key_action] : input_obj.GET_keyboard()) {
        if (key_action == Input_action::toggle_collision) {
            static bool pressed = false;
            if (!pressed) {
                showCollision = !showCollision;  // flip flag
                pressed = true;
            }

            else {
                // reset when key is released
                pressed = false;
            }
        }
    }

	int index = take_control(render_obj, input_obj);
	auto& sprite_vector = render_obj.get_sprite_vector();

    static std::vector<bool> wasColliding;
    if (wasColliding.size() < sprite_vector.size())
        wasColliding.resize(sprite_vector.size(), false);


	if (index >= 0) {
		auto& sprite = sprite_vector[index]; //get sprite to move
		//save old position in case of collision
		float oldX = sprite.x;
		float oldY = sprite.y;
		auto& keyboard_action = input_obj.GET_keyboard();
		for (auto& [tuple_key, event_action, key_action] : keyboard_action) {
			if (key_action == Input_action::move_up) {
				sprite_vector[index].y -= 0.001f;
			}

			else if (key_action == Input_action::move_down) {
				sprite_vector[index].y += 0.001f;
			}

			else if (key_action == Input_action::move_left) {
				sprite_vector[index].x -= 0.001f;
			}

			else if (key_action == Input_action::move_right) {
				sprite_vector[index].x += 0.001f;
			}
		}
		//collision detection with other sprites through movement and AABB
		bool collided = false; //flag to indicate if a collision occurred
		for (size_t i = 0; i < sprite_vector.size(); i++) {
			if (i == index) continue; //skip the controlled sprite
			const auto& other = sprite_vector[i];

			Vector2D pos1(sprite.x, sprite.y);
			Vector2D size1(sprite.w, sprite.h);

			Vector2D pos2(other.x, other.y);
			Vector2D size2(other.w, other.h);

            bool hit = checkAABBCollision(pos1, size1, pos2, size2);
            if (hit && !wasColliding[i]) {
                std::cout << "Collision detected with sprite " << (i + 1) << std::endl;
            }
            wasColliding[i] = hit;
            if (hit) collided = true;


		}if (collided) {
			//revert to old position on collision
			sprite.x = oldX;
			sprite.y = oldY; 
		}
	}

    if (showCollision) {
        Debug::Drawer drawer;
        for (auto& sprite : sprite_vector) {
            drawer.DebugDrawRectangle(sprite.x, sprite.y, sprite.w, sprite.h, 0xFF00FF00);
        }
    }
}

static void reset_selection_transforms(
    Renderer& render_obj, int sel,
    std::vector<bool>& cw, std::vector<bool>& ccw,
    std::vector<float>& ox, std::vector<float>& oy,
    std::vector<float>& ow, std::vector<float>& oh,
    std::vector<float>& oang)
{
    auto& sprites = render_obj.get_sprite_vector();
    size_t n = sprites.size();
    if (n == 0) return;

    if (sel == -2) {
        for (size_t i = 0; i < n; ++i) {
            auto& s = sprites[i];
            s.x = ox[i]; s.y = oy[i];
            s.w = ow[i]; s.h = oh[i];
            s.angle = oang[i];
            cw[i] = false; ccw[i] = false;   // stop rotation
        }
    }
    else if (sel >= 0 && (size_t)sel < n) {
        auto& s = sprites[(size_t)sel];
        s.x = ox[(size_t)sel]; s.y = oy[(size_t)sel];
        s.w = ow[(size_t)sel]; s.h = oh[(size_t)sel];
        s.angle = oang[(size_t)sel];
        cw[(size_t)sel] = false; ccw[(size_t)sel] = false;
    }
}

void Physics::update_sprite_scaling(Renderer& render_obj, Input& input_obj)
{
    // -1 = none, -2 = ALL (key 0), 0..N-1 = specific sprite
    int index = take_control(render_obj, input_obj);
    if (index == -1) { // only bail for "none"
        return;
    }

    size_t sprites_size = render_obj.get_sprite_vector().size();
    if (sprites_size == 0) {
        return;
    }

    // caches (original sizes + per-sprite scale)
    static std::vector<float> origW;
    static std::vector<float> origH;
    static std::vector<float> scaleFactor;

    if (origW.size() != sprites_size || origH.size() != sprites_size || scaleFactor.size() != sprites_size) {
        origW.resize(sprites_size);
        origH.resize(sprites_size);
        scaleFactor.resize(sprites_size, 1.0f);
        size_t i = 0;
        while (i < sprites_size) {
            origW[i] = render_obj.get_sprite_vector()[i].w;
            origH[i] = render_obj.get_sprite_vector()[i].h;
            // keep existing scaleFactor[i] if it already had a value
            if (scaleFactor[i] <= 0.0f) scaleFactor[i] = 1.0f;
            i = i + 1;
        }
    }

    // input state
    std::vector<std::tuple<int, int, Input_action>>& keys = input_obj.GET_keyboard();
    bool magnifyNow = false;
    bool minifyNow = false;

    size_t k = 0;
    while (k < keys.size()) {
        Input_action a = std::get<2>(keys[k]);
        if (a == Input_action::magnify) magnifyNow = true;
        if (a == Input_action::minify)  minifyNow = true;
        k = k + 1;
    }

    const float minScale = 0.2f;
    const float maxScale = 3.0f;
    const float step = 0.1f;

    if (index == -2) {
        static bool wasMagnifyAll = false;
        static bool wasMinifyAll = false;

        bool didChangeAll = false;
        bool wentUpAll = false;
        bool wentDownAll = false;

        if (magnifyNow && !wasMagnifyAll) {
            size_t i = 0;
            while (i < sprites_size) {
                scaleFactor[i] += step;
                if (scaleFactor[i] > maxScale) scaleFactor[i] = maxScale;
                ++i;
            }
            didChangeAll = true; wentUpAll = true;
        }
        wasMagnifyAll = magnifyNow;

        if (minifyNow && !wasMinifyAll) {
            size_t i = 0;
            while (i < sprites_size) {
                scaleFactor[i] -= step;
                if (scaleFactor[i] < minScale) scaleFactor[i] = minScale;
                ++i;
            }
            didChangeAll = true; wentDownAll = true;
        }
        wasMinifyAll = minifyNow;

        if (!didChangeAll) return;

        // apply to all sprites, keep centers
        size_t i = 0;
        while (i < sprites_size) {
            float oldW = render_obj.get_sprite_vector()[i].w;
            float oldH = render_obj.get_sprite_vector()[i].h;
            float cx = render_obj.get_sprite_vector()[i].x + oldW * 0.5f;
            float cy = render_obj.get_sprite_vector()[i].y + oldH * 0.5f;

            float newW = origW[i] * scaleFactor[i];
            float newH = origH[i] * scaleFactor[i];

            render_obj.get_sprite_vector()[i].w = newW;
            render_obj.get_sprite_vector()[i].h = newH;
            render_obj.get_sprite_vector()[i].x = cx - newW * 0.5f;
            render_obj.get_sprite_vector()[i].y = cy - newH * 0.5f;

            ++i;
        }

        std::stringstream sAll;
        sAll.setf(std::ios::fixed);
        if (wentUpAll && !wentDownAll)      sAll << "All sprites scaled up to " << std::setprecision(1) << scaleFactor[(sprites_size > 1) ? 1 : 0];
        else if (wentDownAll && !wentUpAll) sAll << "All sprites scaled down to " << std::setprecision(1) << scaleFactor[(sprites_size > 1) ? 1 : 0];
        else                                sAll << "All sprites scaled to " << std::setprecision(1) << scaleFactor[(sprites_size > 1) ? 1 : 0];
        logger.log(sAll.str());
        return;
    }


    if (index < 0 || static_cast<size_t>(index) >= sprites_size) return;

    static bool wasMagnify = false;
    static bool wasMinify = false;

    bool didChange = false;
    bool wentUp = false;
    bool wentDown = false;

    if (magnifyNow && !wasMagnify) {
        scaleFactor[index] += step;
        if (scaleFactor[index] > maxScale) scaleFactor[index] = maxScale;
        didChange = true; wentUp = true;
    }
    wasMagnify = magnifyNow;

    if (minifyNow && !wasMinify) {
        scaleFactor[index] -= step;
        if (scaleFactor[index] < minScale) scaleFactor[index] = minScale;
        didChange = true; wentDown = true;
    }
    wasMinify = minifyNow;

    if (!didChange) return;

    float oldW = render_obj.get_sprite_vector()[index].w;
    float oldH = render_obj.get_sprite_vector()[index].h;
    float cx = render_obj.get_sprite_vector()[index].x + oldW * 0.5f;
    float cy = render_obj.get_sprite_vector()[index].y + oldH * 0.5f;

    float newW = origW[index] * scaleFactor[index];
    float newH = origH[index] * scaleFactor[index];

    render_obj.get_sprite_vector()[index].w = newW;
    render_obj.get_sprite_vector()[index].h = newH;
    render_obj.get_sprite_vector()[index].x = cx - newW * 0.5f;
    render_obj.get_sprite_vector()[index].y = cy - newH * 0.5f;

    std::stringstream s;
    s.setf(std::ios::fixed);
    s << "Sprite " << (index + 1) << " is scaled ";
    if (wentUp && !wentDown) s << "up to ";
    else if (wentDown && !wentUp) s << "down to ";
    else s << "to ";
    s << std::setprecision(1) << scaleFactor[index];
    logger.log(s.str());
}






void Physics::update_sprite_rotation(Renderer& render_obj, Input& input_obj, double dt, int spriteIndex)
{
    auto& sprites = render_obj.get_sprite_vector();
    if (spriteIndex < 0 || spriteIndex >= (int)sprites.size()) return;

    constexpr float ROT_DEG_PER_SEC = 90.0f;
    float step = ROT_DEG_PER_SEC * static_cast<float>(dt);

    // persistent spin toggles per sprite
    static std::vector<bool> spinCW;
    static std::vector<bool> spinCCW;
    static std::vector<bool> prevPressCW;
    static std::vector<bool> prevPressCCW;

    size_t n = sprites.size();
    if (spinCW.size() != n) {
        spinCW.assign(n, false);
        spinCCW.assign(n, false);
        prevPressCW.assign(n, false);
        prevPressCCW.assign(n, false);
    }

    // --- detect current key states ---
    bool pressCW = false, pressCCW = false;
    for (auto& [tuple_key, event_action, key_action] : input_obj.GET_keyboard()) {
        if (key_action == Input_action::rotate_clockwise)     pressCW = true;   // J
        if (key_action == Input_action::rotate_anticlockwise) pressCCW = true;  // K
    }

    // --- toggle only on *new* press (not held down) ---
    if (pressCW && !prevPressCW[spriteIndex]) {
        spinCW[spriteIndex] = !spinCW[spriteIndex];      // toggle spin
        if (spinCW[spriteIndex]) spinCCW[spriteIndex] = false;
    }
    if (pressCCW && !prevPressCCW[spriteIndex]) {
        spinCCW[spriteIndex] = !spinCCW[spriteIndex];    // toggle spin
        if (spinCCW[spriteIndex]) spinCW[spriteIndex] = false;
    }

    prevPressCW[spriteIndex] = pressCW;
    prevPressCCW[spriteIndex] = pressCCW;

    // --- apply spin if toggled ---
    if (spinCW[spriteIndex])  sprites[spriteIndex].angle += step;
    if (spinCCW[spriteIndex]) sprites[spriteIndex].angle -= step;

    // --- wrap to [0, 360) ---
    if (sprites[spriteIndex].angle >= 360.0f) sprites[spriteIndex].angle -= 360.0f;
    if (sprites[spriteIndex].angle < 0.0f)    sprites[spriteIndex].angle += 360.0f;
}


void Physics::update_sprite_scaling(Renderer& render_obj, Input& input_obj, double dt, int spriteIndex, int sceneId)
{
    auto& sprites = render_obj.get_sprite_vector();
    if (spriteIndex < 0 || spriteIndex >= (int)sprites.size()) return;

    constexpr float STEP = 0.1f;
    constexpr float MIN_SCALE = 0.3f;
    constexpr float MAX_SCALE = 1.5f;

    // --- detect input states ---
    bool scaleUp = false, scaleDown = false, resetAll = false;
    bool resetPressed = false;

    // edge-trigger detection
    static bool prevReset = false;

    for (auto& [tuple_key, event_action, key_action] : input_obj.GET_keyboard()) {
        if (key_action == Input_action::magnify)  scaleUp = true;   // U
        if (key_action == Input_action::minify)   scaleDown = true; // I
        if (key_action == Input_action::reset_transform) resetAll = true; // X
    }

    // trigger reset only when first pressed, not while held
    if (resetPressed && !prevReset)
        globalResetRequested = true;

    // --- global reset flag ---
    if (resetAll)
        globalResetRequested = true;

    // --- prepare per-scene scales ---
    auto& scales = sceneScales[sceneId];
    if (scales.size() != sprites.size())
        scales.assign(sprites.size(), 1.0f);

    // --- handle reset across all scenes ---
    if (globalResetRequested) {
        for (auto& [sid, vec] : sceneScales) {
            for (float& s : vec) s = 1.0f;
        }
        for (auto& spr : sprites) {
            float cx = spr.x + spr.w * 0.5f;
            float cy = spr.y + spr.h * 0.5f;
            spr.w = spr.w / spr.w * spr.w; // reset logic below will fix
        }
        logger.log("Global scale reset triggered (X)");
        globalResetRequested = false;
        return;
    }

    // --- single-step press detection ---
    static std::unordered_map<int, std::vector<bool>> prevUpMap;
    static std::unordered_map<int, std::vector<bool>> prevDownMap;

    auto& prevUp = prevUpMap[sceneId];
    auto& prevDown = prevDownMap[sceneId];
    if (prevUp.size() != sprites.size()) {
        prevUp.assign(sprites.size(), false);
        prevDown.assign(sprites.size(), false);
    }

    bool pressedUp = scaleUp && !prevUp[spriteIndex];
    bool pressedDown = scaleDown && !prevDown[spriteIndex];

    prevUp[spriteIndex] = scaleUp;
    prevDown[spriteIndex] = scaleDown;

    // --- apply scale ---
    float& s = scales[spriteIndex];
    float prev = s;

    if (pressedUp) s += STEP;
    if (pressedDown) s -= STEP;
    s = std::clamp(s, MIN_SCALE, MAX_SCALE);

    if (s != prev) {
        float cx = sprites[spriteIndex].x + sprites[spriteIndex].w * 0.5f;
        float cy = sprites[spriteIndex].y + sprites[spriteIndex].h * 0.5f;
        sprites[spriteIndex].w *= (s / prev);
        sprites[spriteIndex].h *= (s / prev);
        sprites[spriteIndex].x = cx - sprites[spriteIndex].w * 0.5f;
        sprites[spriteIndex].y = cy - sprites[spriteIndex].h * 0.5f;

        std::stringstream ss;
        ss << "Scene " << sceneId
            << " scaled sprite[" << spriteIndex << "] "
            << (pressedUp ? "up" : "down")
            << " to " << std::fixed << std::setprecision(1) << s;
        logger.log(ss.str());
    }
}
