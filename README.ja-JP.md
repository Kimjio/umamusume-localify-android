# umamusume-localify-android
Android port of [umamusume-localify](https://github.com/GEEKiDoS/umamusume-localify)

[韓国語](README.ko-KR.md)

## 使用方法
1. [Magisk](https://github.com/topjohnwu/Magisk) v24またはそれ以降をインストールし、Zygiskを有効化してください。
2. [Shamiko](https://github.com/LSPosed/LSPosed.github.io/releases)をインストールし、SafetyNetによる対策を回避する設定をしてください。 (DenyListを有効化するとモジュールが動作しません)
3. Magiskにumamusume-localify-androidのモジュールをインストールしてください。
4. `config.json` ファイルを `/sdcard/android/data/jp.co.cygames.umamusume/` `dicts`に参照されている翻訳ファイルを配置します。
## 設定アプリ
jsonファイルを編集せずに設定可能なアプリを作りました。

[GitHubのリリース](https://github.com/Kimjio/umamusume-localify-android/releases)または[Playストア](https://play.google.com/store/apps/details?id=com.kimjio.umamusumelocalify.settings)からダウンロードができます。

## 備考
日本語版ウマ娘のディレクトリは `/sdcard/Android/data/jp.co.cygames.umamusume/`

(韓国語版ウマ娘のディレクトリは `/sdcard/Android/data/com.kakaogames.umamusume/`)

になります。

## 設定
- `enableLogger` 書き換えられていない文字列を`data.txt`に出力します。 (`true` / `false` デフォルト: `false`)
- `dumpStaticEntries` `data.txt`にハードコードされた文字列を出力します。 (`true` / `false` デフォルト: `false`)
  - `enableLogger`を有効化する必要があります。
- `dumpDbEntries` DBのデータを`original_data/`に出力します。 (`true` / `false` デフォルト: `false`)
- `maxFps` 最大のFPS値を設定します。 (`-1` = デフォルト / `0` = 無制限 / `n > 0` = 指定した値に制限 デフォルト: `-1`)
- `uiAnimationScale` UIアニメーションスケールの変更をします。 (0 < ~ デフォルト: `1`)
  - 注意: `0`に設定をするとソフトロックがかかります。
- `uiUseSystemResolution` システムの解像度でUIをレンダリングします。 (`true` / `false` デフォルト: `false`)
- `replaceToBuiltinFont` (`replaceFont`) ゲーム内のフォントではなくUnityの内蔵フォントを使用します。 (`true` / `false` デフォルト: `false`)
- `replaceToCustomFont` ゲーム内のフォントをカスタムフォントに変更します。 (`true` / `false` デフォルト: `false`)
- `fontAssetBundlePath` フォントアセットバンドルパスを指定します。 (例: `custom_font/GyeonggiTitle_Medium/font`)
- `fontAssetName` フォントアセット名 (例: `GyeonggiTitle_Medium.fontsettings`)
- `graphicsQuality` あらかじめ設定されているグラフィック設定を変更します。 (`-1` ~ `4` デフォルト: `-1`)
  - `-1`: アプリの設定に従う
  - `0`: `Toon1280`、アンチエイリアシング オフ
  - `1`: `Toon1280x2`、アンチエイリアシング x2
  - `2`: `Toon1280x4`、アンチエイリアシング x4
  - `3`: `ToonFull`、アンチエイリアシング x8
- `antiAliasing` アンチエイリアシングの設定を変更します。 (`-1`、 `0`、 `2`、 `4`、 `8` デフォルト: `-1`)
  - `-1`: グラフィックの設定に従う
  - `0`: アンチエイリアシング OFF
  - `2`: アンチエイリアシング x2
  - `4`: アンチエイリアシング x4
  - `8`: アンチエイリアシング x8
- `forceLandscape` すべての画面表示を強制的に横にします。 (`true` / `false` デフォルト: `false`)
- `forceLandscapeUiScale` 強制横画面表示のUIスケールを設定します。 (0 < ~ デフォルト: `1.8`)
- `uiLoadingShowOrientationGuide` 画面を回転させた時に「読み込み中」の表示をさせるか設定をします。 (`true` / `false` デフォルト: `true`)
- `replaceAssetsPath` ダウンロードをしたアセットをゲーム内で置き換えるためのアセットが入ったフォルダを指定します。
  - 置換をするアセットは元のアセットファイルと同じハッシュ名である必要があります。
  - 例: `2FOXNDZ5H52B3E4JMXVAJ4FRMDE3PX7Q` (ホームフッターのテクスチャとスクリプトを含むアセット (Android))
- `dicts` 翻訳ファイルリスト (翻訳ファイルのパス `/sdcard/Android/data/jp.co.cygames.umamusume/`)

## 既知の問題
`maxFps`を無制限にするとゲームのフレームレートが60FPSから30FPSになってしまう。 (一般的な30FPSではありません)

## ビルド
1. ソースコードをダウンロードします。
2. Android Studioを使用し、gradle task `:module:assembleRelease`を実行しコンパイルをする事で`out`フォルダ内にzipファイルが生成されます。
