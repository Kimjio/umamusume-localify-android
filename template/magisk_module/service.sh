#!/system/bin/sh
while [ "$(getprop sys.boot_completed | tr -d '\r')" != "1" ]; do sleep 1; done
sleep 1

ABI=$(getprop ro.product.cpu.abi)

if [ "$ABI" = "x86" ]; then
  if [ -d "/data/data/jp.co.cygames.umamusume" ]; then
    touch /data/data/jp.co.cygames.umamusume/armeabi-v7a.so
    mount -o bind /data/adb/modules/zygisk_umamusume_localify/zygisk/armeabi-v7a.so /data/data/jp.co.cygames.umamusume/armeabi-v7a.so
  fi
  if [ -d "/data/data/com.kakaogames.umamusume" ]; then
    touch /data/data/com.kakaogames.umamusume/armeabi-v7a.so
    mount -o bind /data/adb/modules/zygisk_umamusume_localify/zygisk/armeabi-v7a.so /data/data/com.kakaogames.umamusume/armeabi-v7a.so
  fi
elif [ "$ABI" = "x86_64" ]; then
  if [ -d "/data/data/jp.co.cygames.umamusume" ]; then
    touch /data/data/jp.co.cygames.umamusume/arm64-v8a.so
    touch /data/data/jp.co.cygames.umamusume/armeabi-v7a.so
    mount -o bind /data/adb/modules/zygisk_umamusume_localify/zygisk/arm64-v8a.so /data/data/jp.co.cygames.umamusume/arm64-v8a.so
    mount -o bind /data/adb/modules/zygisk_umamusume_localify/zygisk/armeabi-v7a.so /data/data/jp.co.cygames.umamusume/armeabi-v7a.so
  fi
  if [ -d "/data/data/com.kakaogames.umamusume" ]; then
    touch /data/data/com.kakaogames.umamusume/arm64-v8a.so
    touch /data/data/com.kakaogames.umamusume/armeabi-v7a.so
    mount -o bind /data/adb/modules/zygisk_umamusume_localify/zygisk/arm64-v8a.so /data/data/com.kakaogames.umamusume/arm64-v8a.so
    mount -o bind /data/adb/modules/zygisk_umamusume_localify/zygisk/armeabi-v7a.so /data/data/com.kakaogames.umamusume/armeabi-v7a.so
  fi
fi
