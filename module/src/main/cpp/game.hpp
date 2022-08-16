#pragma once

#ifndef UMAMUSUMELOCALIFYANDROID_GAME_HPP
#define UMAMUSUMELOCALIFYANDROID_GAME_HPP

#define Unity2019 "2019.4.21f1"
#define Unity2019Twn "2019.4.19f1"
#define Unity2020 "2020.3.24f1"

namespace Game {
    enum class Region {
        UNKNOWN,
        JAP,
        KOR,
        TWN,
    };

    inline auto currentGameRegion = Region::UNKNOWN;

    inline std::string GamePackageName = "jp.co.cygames.umamusume";
    inline std::string GamePackageNameKor = "com.kakaogames.umamusume";
    inline std::string GamePackageNameTwn = "com.komoe.kmumamusumegp";

    static bool IsPackageNameEqualsByGameRegion(const char *pkgNm, Region gameRegion) {
        std::string pkgNmStr = std::string(pkgNm);
        if (pkgNmStr.empty()) {
            return false;
        }
        switch (gameRegion) {
            case Region::JAP:
                if (pkgNmStr == GamePackageName) {
                    currentGameRegion = Region::JAP;
                    return true;
                }
                break;
            case Region::KOR:
                if (pkgNmStr == GamePackageNameKor) {
                    currentGameRegion = Region::KOR;
                    return true;
                }
                break;
            case Region::TWN:
                if (pkgNmStr == GamePackageNameTwn) {
                    currentGameRegion = Region::TWN;
                    return true;
                }
                break;
            case Region::UNKNOWN:
            default:
                break;
        }
        return false;
    }

    static std::string GetPackageNameByGameRegion(Region gameRegion) {
        if (gameRegion == Region::JAP)
            return GamePackageName;
        if (gameRegion == Region::KOR)
            return GamePackageNameKor;
        if (gameRegion == Region::TWN)
            return GamePackageNameTwn;
        return "";
    }

    static std::string GetCurrentPackageName() {
        return GetPackageNameByGameRegion(currentGameRegion);
    }

    static Region CheckPackageNameByDataPath() {
        if (access(
                std::string("/data/data/")
                        .append(GetPackageNameByGameRegion(Region::JAP)).append(
                        "/cache").data(),
                F_OK) == 0) {
            return Region::JAP;
        }
        if (access(
                std::string("/data/data/")
                        .append(GetPackageNameByGameRegion(Region::KOR)).append(
                        "/cache").data(),
                F_OK) == 0) {
            return Region::KOR;
        }
        if (access(
                std::string("/data/data/")
                        .append(GetPackageNameByGameRegion(Region::TWN)).append(
                        "/cache").data(),
                F_OK) == 0) {
            return Region::TWN;
        }

        return Region::UNKNOWN;
    }
}

#endif
