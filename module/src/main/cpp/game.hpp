#pragma once

#ifndef UMAMUSUMELOCALIFYANDROID_GAME_HPP
#define UMAMUSUMELOCALIFYANDROID_GAME_HPP

#include <string>

#define Unity2019 "2019.4."s
#define Unity2020 "2020.3."s

using namespace std;

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

    inline auto GamePackageName = "jp.co.cygames.umamusume"s;
    inline auto GamePackageNameKor = "com.kakaogames.umamusume"s;
    inline auto GamePackageNameTwnGoogle = "com.komoe.kmumamusumegp"s;
    inline auto GamePackageNameTwnMyCard = "com.komoe.kmumamusumemc"s;

    static bool IsPackageNameEqualsByGameRegion(const char *pkgNm, Region gameRegion) {
        string pkgNmStr = string(pkgNm);
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

    static string GetPackageNameByGameRegionAndGameStore(Region gameRegion, Store gameStore) {
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

    static string GetCurrentPackageName() {
        return GetPackageNameByGameRegionAndGameStore(currentGameRegion, currentGameStore);
    }

    static Region CheckPackageNameByDataPath() {
        if (access(
                "/data/data/"s
                        .append(GetPackageNameByGameRegionAndGameStore(Region::JAP,
                                                                       Store::Google)).append(
                        "/cache").data(),
                F_OK) == 0) {
            return Region::JAP;
        }
        if (access(
                "/data/data/"s
                        .append(GetPackageNameByGameRegionAndGameStore(Region::KOR,
                                                                       Store::Google)).append(
                        "/cache").data(),
                F_OK) == 0) {
            return Region::KOR;
        }
        if (access(
                "/data/data/"s
                        .append(GetPackageNameByGameRegionAndGameStore(Region::TWN,
                                                                       Store::Google)).append(
                        "/cache").data(),
                F_OK) == 0) {
            return Region::TWN;
        }
        if (access(
                "/data/data/"s
                        .append(GetPackageNameByGameRegionAndGameStore(Region::TWN,
                                                                       Store::Other)).append(
                        "/cache").data(),
                F_OK) == 0) {
            currentGameStore = Store::Other;
            return Region::TWN;
        }

        return Region::UNKNOWN;
    }
}

#endif
