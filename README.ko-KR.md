# umamusume-localify-android
[umamusume-localify](https://github.com/GEEKiDoS/umamusume-localify) 안드로이드 포팅 버전

[English](README.md)

## 사용법
1. v24 이상의 [Magisk](https://github.com/topjohnwu/Magisk) 설치 후 Zygisk 를 활성화합니다.
2. Magisk 에서 모듈을 설치합니다.
3. `/sdcard/Android/data/jp.co.cygames.umamusume/` 에 `config.json` 파일, `dicts` 에서 참조하는 번역 파일을 넣습니다.

## 참고사항
작업 경로는 `/sdcard/Android/data/jp.co.cygames.umamusume/` 입니다.

(한국어 버전은 `/sdcard/Android/data/com.kakaogames.umamusume/` 입니다.)

## 설정
- `enableLogger` 덮어쓰지 않은 문자열을 `data.txt` 에 출력합니다. (`true` / `false`, 기본값: `false`)
- `dumpStaticEntries` 하드코딩된 문자열을 `data.txt` 에 출력합니다. (`true` / `false`, 기본값: `false`)
  - `enableLogger` 가 활성화돼야 합니다.
- `dumpDbEntries` DB 데이터를 `original_data/` 에 출력합니다. (`true` / `false`, 기본값: `false`)
- `maxFps` 최대 프레임 설정 (`-1` = 기본값 / `0` = 무제한 / `n > 0` = n까지 제한, 기본값: `-1`)
- `uiAnimationScale` UI 애니메이션 배율 변경 (0 < ~, 기본값: `1`)
- `replaceToBuiltinFont` (`replaceFont`) 게임 내 폰트를 사용하지 않고 유니티 내장 폰트를 사용합니다. (`true` / `false`, 기본값: `true`)
- `replaceToCustomFont` 게임 내 폰트를 사용하지 않고 사용자 지정 폰트를 사용합니다. (`true` / `false`, Default: `false`)
- `fontAssetBundlePath` 폰트 애셋 번들 경로 (예: `custom_font/GyeonggiTitle_Medium/font`)
- `fontAssetName` 폰트 애셋 이름 (예: `GyeonggiTitle_Medium.fontsettings`)
- `dicts` 번역 파일 목록 (`/sdcard/Android/data/jp.co.cygames.umamusume/` 이후의 번역 파일 경로 목록)

## 알려진 문제
`maxFps` 를 무제한으로 설정 시 게임의 프레임이 60FPS 에서 반토막난 30FPS 으로 재생됩니다.

## 

## 빌드
1. 소스 코드를 다운 받습니다.
2. Android Studio 사용하여 gradle 작업 `:module:assembleRelease` 을 실행해서 컴파일하면, `out` 폴더에 zip 패키지가 생성됩니다.