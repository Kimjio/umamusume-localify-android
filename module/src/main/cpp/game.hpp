//
// Created by Perfare on 2020/7/4.
//

#ifndef UMAMUSUMELOCALIFYANDROID_GAME_HPP
#define UMAMUSUMELOCALIFYANDROID_GAME_HPP

#define GamePackageName "jp.co.cygames.umamusume"
#define GamePackageNameKor "com.kakaogames.umamusume"

enum class GameRegion {
    UNKNOWN,
    JAP,
    KOR
};

static GameRegion gameRegion = GameRegion::UNKNOWN;

namespace {
    std::string GetCurrentPackageName() {
        if (gameRegion == GameRegion::JAP)
            return GamePackageName;
        if (gameRegion == GameRegion::KOR)
            return GamePackageNameKor;
        return nullptr;
    }
}

#endif //UMAMUSUMELOCALIFYANDROID_GAME_HPP
