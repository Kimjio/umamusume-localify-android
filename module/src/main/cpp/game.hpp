#ifndef UMAMUSUMELOCALIFYANDROID_GAME_HPP
#define UMAMUSUMELOCALIFYANDROID_GAME_HPP

#define GamePackageName "jp.co.cygames.umamusume"
#define GamePackageNameKor "com.kakaogames.umamusume"

#define Unity2019 "2019.4.21f1"
#define Unity2020 "2020.3.24f1"

enum class GameRegion {
    UNKNOWN,
    JAP,
    KOR
};

extern GameRegion gameRegion;

namespace {
    std::string GetCurrentPackageName() {
        if (gameRegion == GameRegion::JAP)
            return GamePackageName;
        if (gameRegion == GameRegion::KOR)
            return GamePackageNameKor;
        return "";
    }
}

#endif //UMAMUSUMELOCALIFYANDROID_GAME_HPP
