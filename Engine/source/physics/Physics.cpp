// integration, gravity, movement
#include <algorithm> // std::fill
#include "input/Input.h"
#include "physics/Physics.h"
#include "physics/AABB.h"

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
	int index = take_control(render_obj, input_obj);
	auto& sprite_vector = render_obj.get_sprite_vector();

	if (index >= 0) {
		auto& sprite = sprite_vector[index]; //get sprite to move
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
		for (size_t i = 0; i < sprite_vector.size(); i++) { //ignore background at index 0
			if (i == index) continue; //skip the controlled sprite
			const auto& other = sprite_vector[i];

			Vector2D pos1(sprite.x, sprite.y);
			Vector2D size1(sprite.w, sprite.h);

			Vector2D pos2(other.x, other.y);
			Vector2D size2(other.w, other.h);

			if (checkAABBCollision(pos1, size1, pos2, size2)) {
				std::cout << "Collision detected with sprite " << i+1 << std::endl;

				// Optional: Handle the collision (undo movement, bounce back, etc.)
			}
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

    const float minScale = 1.0f;
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

void Physics::update_sprite_rotation(Renderer& render_obj, Input& input_obj)
{
    int sel = take_control(render_obj, input_obj); // -1 NONE, -2 ALL, 0..N-1
    auto& sprites = render_obj.get_sprite_vector();
    size_t n = sprites.size();
    if (n == 0) return;

    // persistent per-sprite rotation toggles
    static std::vector<bool> cw, ccw;
    if (cw.size() != n || ccw.size() != n) {
        cw.assign(n, false);
        ccw.assign(n, false);
    }

    // cache originals once (position, size, angle)
    static std::vector<float> ox, oy, ow, oh, oang;
    if (ox.size() != n) {
        ox.resize(n); oy.resize(n); ow.resize(n); oh.resize(n); oang.resize(n);
        for (size_t i = 0; i < n; ++i) {
            const auto& s = sprites[i];
            ox[i] = s.x; oy[i] = s.y; ow[i] = s.w; oh[i] = s.h; oang[i] = s.angle;
        }
    }

    // read keys this frame
    auto& keys = input_obj.GET_keyboard();
    bool nowCW = false, nowCCW = false, doReset = false;
    for (size_t i = 0; i < keys.size(); ++i) {
        Input_action a = std::get<2>(keys[i]);
        if (a == Input_action::rotate_clockwise)     nowCW = true;
        if (a == Input_action::rotate_anticlockwise) nowCCW = true;
        if (a == Input_action::reset_transform)      doReset = true;
    }

    // reset (X)
    if (doReset) {
        reset_selection_transforms(render_obj, sel, cw, ccw, ox, oy, ow, oh, oang);
        // (don’t early-return; allow J/K edges to be seen this frame too)
    }

    // J/K edge detection
    static bool prevCW = false, prevCCW = false;

    if (sel == -2) {
        // ALL: press J or K again to stop all
        bool allCW = (n > 0), allCCW = (n > 0);
        for (size_t i = 0; i < n; ++i) {
            allCW &= (cw[i] && !ccw[i]);
            allCCW &= (ccw[i] && !cw[i]);
        }
        if (nowCW && !prevCW) {
            if (allCW) { std::fill(cw.begin(), cw.end(), false); std::fill(ccw.begin(), ccw.end(), false); }
            else { std::fill(cw.begin(), cw.end(), true); std::fill(ccw.begin(), ccw.end(), false); }
        }
        if (nowCCW && !prevCCW) {
            if (allCCW) { std::fill(cw.begin(), cw.end(), false); std::fill(ccw.begin(), ccw.end(), false); }
            else { std::fill(ccw.begin(), ccw.end(), true); std::fill(cw.begin(), cw.end(), false); }
        }
    }
    else if (sel >= 0 && (size_t)sel < n) {
        // SINGLE: toggle only that sprite
        if (nowCW && !prevCW) { bool on = !cw[(size_t)sel];  cw[(size_t)sel] = on; if (on) ccw[(size_t)sel] = false; }
        if (nowCCW && !prevCCW) { bool on = !ccw[(size_t)sel]; ccw[(size_t)sel] = on; if (on) cw[(size_t)sel] = false; }
    }
    // NONE: don’t change toggles (keys locked)

    prevCW = nowCW;
    prevCCW = nowCCW;

    // apply rotation
    const float step = 0.1f;
    for (size_t i = 0; i < n; ++i) {
        if (cw[i])  sprites[i].angle -= step;
        if (ccw[i]) sprites[i].angle += step;

        if (sprites[i].angle >= 360.0f) sprites[i].angle -= 360.0f;
        else if (sprites[i].angle < 0.0f) sprites[i].angle += 360.0f;
    }
}


