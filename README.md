# umamusume-localify-android
Android port of [umamusume-localify](https://github.com/GEEKiDoS/umamusume-localify)

[한국어](README.ko-KR.md)

## How to use
1. Install [Magisk](https://github.com/topjohnwu/Magisk) v24 or later and enable Zygisk
2. Install module in Magisk
3. Put the `config.json` file in `/sdcard/android/data/jp.co.cygames.umamusume/` and the translation file referenced in `dicts`.

## Note
Working directory is `/sdcard/Android/data/jp.co.cygames.umamusume/` 

(Korean version is `/sdcard/Android/data/com.kakaogames.umamusume/`)

## Config
- `enableLogger` Outputs an unoverwritten string to `data.txt` (`true` / `false`, Default: `false`)
- `dumpStaticEntries` Outputs a hard-coded string to `data.txt` (`true` / `false`, Default: `false`)
  - `enableLogger` must be enabled.
- `dumpDbEntries` Outputs DB data to `original_data/` (`true` / `false`, Default: `false`)
- `maxFps` Max FPS settings (`-1` = Default / `0` = Unlimited / `n > 0` = Limit to n, Default: `-1`)
- `uiAnimationScale` Change UI animation scale (0 < ~, Default: `1`)
- `replaceToBuiltinFont` (`replaceFont`) Use Unity builtin fonts instead of in-game fonts. (`true` / `false`, Default: `true`)
- `replaceToCustomFont` Use custom fonts instead of in-game fonts. (`true` / `false`, Default: `false`)
- `fontAssetBundlePath` Font asset bundle path (ex: `custom_font/GyeonggiTitle_Medium/font`)
- `fontAssetName` Font asset name (ex: `GyeonggiTitle_Medium.fontsettings`)
- `dicts` Translation file list (List of translation file paths since `/sdcard/Android/data/jp.co.cygames.umamusume/`)

## Known issue
When `maxFps` is set to unlimited, the frame of the game is played from 60 FPS to 30 FPS, which is halved. (Not a typical 30FPS)

## Build
1. Download the source code
2. Use Android Studio to run the gradle task `:module:assembleRelease` to compile, the zip package will be generated in the `out` folder
