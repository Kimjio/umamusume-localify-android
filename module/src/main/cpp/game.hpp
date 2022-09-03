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

    enum class Store {
        Google,
        // Ex. OneStore, MyCard...
        Other
    };

    inline auto currentGameRegion = Region::UNKNOWN;
    inline auto currentGameStore = Store::Google;

    inline std::string GamePackageName = "jp.co.cygames.umamusume";
    inline std::string GamePackageNameKor = "com.kakaogames.umamusume";
    inline std::string GamePackageNameTwnGoogle = "com.komoe.kmumamusumegp";
    inline std::string GamePackageNameTwnMyCard = "com.komoe.kmumamusumemc";

    static bool IsPackageNameEqualsByGameRegion(const char *pkgNm, Region gameRegion) {
        std::string pkgNmStr = std::string(pkgNm);
        if (pkgNmStr.empty()) {
            return false;
        }
        switch (gameRegion) {
            case Region::JAP:
                if (pkgNmStr == GamePackageName) {
                    currentGameRegion = Region::JAP;
                    currentGameStore = Store::Google;
                    return true;
                }
                break;
            case Region::KOR:
                if (pkgNmStr == GamePackageNameKor) {
                    currentGameRegion = Region::KOR;
                    currentGameStore = Store::Google;
                    return true;
                }
                break;
            case Region::TWN:
                if (pkgNmStr == GamePackageNameTwnGoogle) {
                    currentGameRegion = Region::TWN;
                    currentGameStore = Store::Google;
                    return true;
                } else if (pkgNmStr == GamePackageNameTwnMyCard) {
                    currentGameRegion = Region::TWN;
                    currentGameStore = Store::Other;
                    return true;
                }
                break;
            case Region::UNKNOWN:
            default:
                break;
        }
        return false;
    }

    static std::string GetPackageNameByGameRegionAndGameStore(Region gameRegion, Store gameStore) {
        if (gameRegion == Region::JAP)
            return GamePackageName;
        if (gameRegion == Region::KOR)
            return GamePackageNameKor;
        if (gameRegion == Region::TWN) {
            if (gameStore == Store::Other) {
                return GamePackageNameTwnMyCard;
            }
            return GamePackageNameTwnGoogle;
        }
        return "";
    }

    static std::string GetCurrentPackageName() {
        return GetPackageNameByGameRegionAndGameStore(currentGameRegion, currentGameStore);
    }

    static Region CheckPackageNameByDataPath() {
        if (access(
                std::string("/data/data/")
                        .append(GetPackageNameByGameRegionAndGameStore(Region::JAP, Store::Google)).append(
                        "/cache").data(),
                F_OK) == 0) {
            return Region::JAP;
        }
        if (access(
                std::string("/data/data/")
                        .append(GetPackageNameByGameRegionAndGameStore(Region::KOR, Store::Google)).append(
                        "/cache").data(),
                F_OK) == 0) {
            return Region::KOR;
        }
        if (access(
                std::string("/data/data/")
                        .append(GetPackageNameByGameRegionAndGameStore(Region::TWN, Store::Google)).append(
                        "/cache").data(),
                F_OK) == 0) {
            return Region::TWN;
        }
        if (access(
                std::string("/data/data/")
                        .append(GetPackageNameByGameRegionAndGameStore(Region::TWN, Store::Other)).append(
                        "/cache").data(),
                F_OK) == 0) {
            currentGameStore = Store::Other;
            return Region::TWN;
        }

        return Region::UNKNOWN;
    }
}

#endif
