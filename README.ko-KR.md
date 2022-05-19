# umamusume-localify-android
[umamusume-localify](https://github.com/GEEKiDoS/umamusume-localify) 안드로이드 포팅 버전

## 사용법
1. v24 이상의 [Magisk](https://github.com/topjohnwu/Magisk) 설치 후 Zygisk 를 활성화합니다.
2. Magisk 에서 모듈을 설치합니다.
3. `/sdcard/Android/data/jp.co.cygames.umamusume/` 에 `config.json` 파일, `dicts` 에서 참조하는 번역 파일을 넣습니다.

## 참고사항
작업 경로는 `/sdcard/Android/data/jp.co.cygames.umamusume/` 입니다.

## 설정
- `enableLogger` 덮어쓰지 않은 문자열을 `data.txt` 에 출력합니다. (`true` / `false`, 기본값: `false`)
- `dumpStaticEntries` 하드코딩된 문자열을 `data.txt` 에 출력합니다. (`true` / `false`, 기본값: `false`)
  - `enableLogger` 가 활성화돼야 합니다.
- `dumpDbEntries` DB 데이터를 `original_data/` 에 출력합니다. (`true` / `false`, 기본값: `false`)
- `maxFps` 최대 프레임 설정 (`-1` = 기본값 / `0` = 무제한 / `n > 0` = n까지 제한, 기본값: `-1`)
- `replaceFont` 게임 내 폰트를 사용하지 않고 유니티 내장 폰트를 사용합니다. (`true` / `false`, 기본값: `true`)
- `dicts` 번역 파일 목록 (`/sdcard/Android/data/jp.co.cygames.umamusume/` 이후의 번역 파일 경로 목록)

## 알려진 문제
`maxFps` 를 무제한으로 설정 시 게임의 프레임이 60FPS 에서 반토막난 30FPS 으로 재생됩니다.

## 빌드
1. Download the source code
2. Use Android Studio to run the gradle task `:module:assembleRelease` to compile, the zip package will be generated in the `out` folder