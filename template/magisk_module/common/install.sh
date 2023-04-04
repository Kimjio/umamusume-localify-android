enforce_install_from_magisk_app() {
  if $BOOTMODE; then
    ui_print "- Installing from Magisk app"
  else
    ui_print "*********************************************************"
    ui_print "! Install from recovery is NOT supported"
    ui_print "! Some recovery has broken implementations, install with such recovery will finally cause Riru or Riru modules not working"
    ui_print "! Please install from Magisk app"
    abort "*********************************************************"
  fi
}

# Extract verify.sh
ui_print "- Extracting verify.sh"
unzip -o "$ZIPFILE" 'verify.sh' -d "$TMPDIR" >&2
if [ ! -f "$TMPDIR/verify.sh" ]; then
  ui_print    "*********************************************************"
  ui_print    "! Unable to extract verify.sh!"
  ui_print    "! This zip may be corrupted, please try downloading again"
  abort "*********************************************************"
fi
. $TMPDIR/verify.sh

# Extract riru.sh

# Variables provided by riru.sh:
#
# RIRU_API: API version of installed Riru, 0 if not installed
# RIRU_MIN_COMPATIBLE_API: minimal supported API version by installed Riru, 0 if not installed or version < v23.2
# RIRU_VERSION_CODE: version code of installed Riru, 0 if not installed or version < v23.2
# RIRU_VERSION_NAME: version name of installed Riru, "" if not installed or version < v23.2

if [ "$FLAVOR" == "riru" ]; then
  # Extract riru.sh
  extract "$ZIPFILE" 'riru.sh' "$TMPDIR"
  . "$TMPDIR/riru.sh"
  # Functions from riru.sh
  check_riru_version
fi

# Functions from util_functions.sh (it will be loaded by riru.sh)
enforce_install_from_magisk_app

# Check architecture
if [ "$ARCH" != "arm" ] && [ "$ARCH" != "arm64" ] && [ "$ARCH" != "x86" ] && [ "$ARCH" != "x64" ]; then
  abort "! Unsupported platform: $ARCH"
else
  ui_print "- Device platform: $ARCH"
fi

# Extract libs
ui_print "- Extracting module files"

extract "$ZIPFILE" 'module.prop' "$MODPATH"
extract "$ZIPFILE" 'system.prop' "$MODPATH"
extract "$ZIPFILE" 'uninstall.sh' "$MODPATH"
extract "$ZIPFILE" 'classes.dex' "$MODPATH"

# Riru v24+ load files from the "riru" folder in the Magisk module folder
# This "riru" folder is also used to determine if a Magisk module is a Riru module
if [ "$FLAVOR" == "zygisk" ]; then
  mkdir -p "$MODPATH/zygisk"
  if [ "$ARCH" = "arm" ] || [ "$ARCH" = "arm64" ]; then
    extract "$ZIPFILE" "lib/armeabi-v7a/lib$MODULE_LIB_NAME.so" "$MODPATH/zygisk" true
    mv "$MODPATH/zygisk/lib$MODULE_LIB_NAME.so" "$MODPATH/zygisk/armeabi-v7a.so"

    if [ "$IS64BIT" = true ]; then
      extract "$ZIPFILE" "lib/arm64-v8a/lib$MODULE_LIB_NAME.so" "$MODPATH/zygisk" true
      mv "$MODPATH/zygisk/lib$MODULE_LIB_NAME.so" "$MODPATH/zygisk/arm64-v8a.so"
    fi
  fi

  if [ "$ARCH" = "x86" ] || [ "$ARCH" = "x64" ]; then
    extract "$ZIPFILE" "lib/x86_64/lib$MODULE_LIB_NAME.so" "$MODPATH/zygisk" true
    mv "$MODPATH/zygisk/lib$MODULE_LIB_NAME.so" "$MODPATH/zygisk/x86_64.so"

    if [ "$IS64BIT" = true ]; then
      extract "$ZIPFILE" "lib/x86/lib$MODULE_LIB_NAME.so" "$MODPATH/zygisk" true
      mv "$MODPATH/zygisk/lib$MODULE_LIB_NAME.so" "$MODPATH/zygisk/x86.so"
    fi
  fi
elif [ "$FLAVOR" == "riru" ]; then
  mkdir "$MODPATH/riru"
  mkdir "$MODPATH/riru/lib"
  mkdir "$MODPATH/riru/lib64"
  if [ "$ARCH" = "arm" ] || [ "$ARCH" = "arm64" ]; then
    ui_print "- Extracting arm libraries"
    extract "$ZIPFILE" "lib/armeabi-v7a/lib$RIRU_MODULE_LIB_NAME.so" "$MODPATH/riru/lib" true

    if [ "$IS64BIT" = true ]; then
      ui_print "- Extracting arm64 libraries"
      extract "$ZIPFILE" "lib/arm64-v8a/lib$RIRU_MODULE_LIB_NAME.so" "$MODPATH/riru/lib64" true
    fi
  fi

  if [ "$ARCH" = "x86" ] || [ "$ARCH" = "x64" ]; then
    ui_print "- Extracting x86 libraries"
    extract "$ZIPFILE" "lib/x86/lib$RIRU_MODULE_LIB_NAME.so" "$MODPATH/riru/lib" true

    if [ "$IS64BIT" = true ]; then
      ui_print "- Extracting x64 libraries"
      extract "$ZIPFILE" "lib/x86_64/lib$RIRU_MODULE_LIB_NAME.so" "$MODPATH/riru/lib64" true
    fi
  fi

  if [ "$RIRU_MODULE_DEBUG" = true ]; then
    mv "$MODPATH/riru" "$MODPATH/system"
    mv "$MODPATH/system/lib/lib$RIRU_MODULE_LIB_NAME.so" "$MODPATH/system/lib/libriru_$RIRU_MODULE_LIB_NAME.so"
    mv "$MODPATH/system/lib64/lib$RIRU_MODULE_LIB_NAME.so" "$MODPATH/system/lib64/libriru_$RIRU_MODULE_LIB_NAME.so"
    if [ "$RIRU_API" -ge 26 ]; then
      mkdir -p "$MODPATH/riru/lib"
      mkdir -p "$MODPATH/riru/lib64"
      touch "$MODPATH/riru/lib/libriru_$RIRU_MODULE_LIB_NAME"
      touch "$MODPATH/riru/lib64/libriru_$RIRU_MODULE_LIB_NAME"
    else
      mkdir -p "/data/adb/riru/modules/$RIRU_MODULE_LIB_NAME"
    fi
  fi
fi
