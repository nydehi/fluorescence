
#include "cursormanager.hpp"

namespace uome {
namespace ui {

CursorManager::CursorManager(Config& config, boost::shared_ptr<CL_DisplayWindow> window) :
        warMode_(false) {

    unsigned int artIdStart = config["ui.cursor-artid-start"].as<unsigned int>();
    unsigned int artIdStartWarMode = config["ui.cursor-artid-start-warmode"].as<unsigned int>();

    for (unsigned int i = 0; i < CursorType::COUNT; ++i) {
        setCursorImage(i, artIdStart + i, false, window);
        setCursorImage(i, artIdStartWarMode + i, true, window);
    }
}

void CursorManager::setWarMode(bool value) {
    warMode_ = value;
    updateCursor();
}

void CursorManager::setCursor(unsigned int id) {
    currentCursorId_ = id;
    updateCursor();
}

void CursorManager::setCursorImage(unsigned int id, unsigned int artId, bool warMode, boost::shared_ptr<CL_DisplayWindow> window) {
    if (warMode) {
        id += CursorType::COUNT;
    }
    cursorImages_[id].set(id, artId, window);
}

void CursorManager::updateCursor() {
    unsigned int cursorIndex = currentCursorId_;
    if (warMode_) {
        cursorIndex += CursorType::COUNT;
    }

    cursorImages_[cursorIndex].activate();
}

}
}