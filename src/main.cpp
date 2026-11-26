// DiamondClicks by diamondsoup
// While you hold the jump button, every frame becomes a real click (autoclicker)
// instead of one long hold. Click delay and gamemodes are configurable.

#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include <algorithm>

using namespace geode::prelude;

namespace {
	constexpr int JUMP_BUTTON = 1; // PlayerButton::Jump

	struct State {
		bool held[2] = {false, false};    // is the user physically holding (0 = player1 side, 1 = player2 side)
		bool pressed[2] = {false, false}; // what the game currently thinks (after our injected clicks)
		int counter[2] = {0, 0};          // frames since hold started / last cycle
		bool injecting = false;           // true while we call handleButton ourselves
	};

	State g_state;

	void resetState() {
		g_state = State();
	}

	bool modeAllowed(PlayerObject* p) {
		auto* mod = Mod::get();
		if (p->m_isShip)   return mod->getSettingValue<bool>("mode-ship");
		if (p->m_isBird)   return mod->getSettingValue<bool>("mode-ufo");
		if (p->m_isBall)   return mod->getSettingValue<bool>("mode-ball");
		if (p->m_isDart)   return mod->getSettingValue<bool>("mode-wave");
		if (p->m_isRobot)  return mod->getSettingValue<bool>("mode-robot");
		if (p->m_isSpider) return mod->getSettingValue<bool>("mode-spider");
		if (p->m_isSwing)  return mod->getSettingValue<bool>("mode-swing");
		return mod->getSettingValue<bool>("mode-cube");
	}

	void inject(GJBaseGameLayer* layer, bool down, bool isPlayer1) {
		g_state.injecting = true;
		layer->handleButton(down, JUMP_BUTTON, isPlayer1);
		g_state.injecting = false;
	}

	void tick(GJBaseGameLayer* layer) {
		auto* mod = Mod::get();
		bool enabled = mod->getSettingValue<bool>("enabled");
		int delay = static_cast<int>(std::max<int64_t>(1, mod->getSettingValue<int64_t>("click-delay")));

		for (int i = 0; i < 2; ++i) {
			bool isP1 = (i == 0);
			PlayerObject* player = isP1 ? layer->m_player1 : layer->m_player2;

			// no player or dead -> forget everything about this side
			if (!player || player->m_isDead) {
				g_state.held[i] = false;
				g_state.pressed[i] = false;
				g_state.counter[i] = 0;
				continue;
			}

			if (!g_state.held[i]) {
				g_state.counter[i] = 0;
				continue;
			}

			bool active = enabled && modeAllowed(player);
			if (!active) {
				// not our business right now: make sure the game state matches the real hold
				if (!g_state.pressed[i]) {
					inject(layer, true, isP1);
					g_state.pressed[i] = true;
				}
				g_state.counter[i] = 0;
				continue;
			}

			int c = ++g_state.counter[i];

			if (delay == 1) {
				// a fresh click on every single frame: release + press
				if (g_state.pressed[i]) inject(layer, false, isP1);
				inject(layer, true, isP1);
				g_state.pressed[i] = true;
			} else {
				int phase = c % delay;
				if (phase == 0) {
					// click: press (re-press if it was somehow still down)
					if (g_state.pressed[i]) inject(layer, false, isP1);
					inject(layer, true, isP1);
					g_state.pressed[i] = true;
				} else if (phase == 1 && g_state.pressed[i]) {
					// end of the 1-frame tap
					inject(layer, false, isP1);
					g_state.pressed[i] = false;
				}
			}
		}
	}
}

class $modify(DCGameLayer, GJBaseGameLayer) {
	// remember what the user is really doing with the jump button
	void handleButton(bool down, int button, bool isPlayer1) {
		if (!g_state.injecting && button == JUMP_BUTTON) {
			int i = isPlayer1 ? 0 : 1;
			g_state.held[i] = down;
			g_state.pressed[i] = down;
			g_state.counter[i] = 0;
		}
		GJBaseGameLayer::handleButton(down, button, isPlayer1);
	}

	void processCommands(float dt, bool isHalfTick, bool isLastTick) {
		tick(this);
		GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
	}
};

class $modify(DCPlayLayer, PlayLayer) {
	void resetLevel() {
		resetState();
		PlayLayer::resetLevel();
	}

	void pauseGame(bool p0) {
		resetState();
		PlayLayer::pauseGame(p0);
	}

	void onQuit() {
		resetState();
		PlayLayer::onQuit();
	}
};
