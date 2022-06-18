#!/system/bin/sh
while [ "$(getprop sys.boot_completed | tr -d '\r')" != "1" ]; do sleep 1; done
sleep 1

API=@API@

MODULE_ID=@MODULE_ID@

MODULE_LIB_NAME=@MODULE_LIB_NAME@

ABI=$(getprop ro.product.cpu.abi)

if [ $ABI = "x86" ]; then
  if [ -d "/data/data/jp.co.cygames.umamusume" ]; then
    touch /data/data/jp.co.cygames.umamusume/armeabi-v7a.so
    mount -o bind "/data/adb/modules/${API}_${MODULE_ID}/lib/armeabi-v7a/lib${MODULE_LIB_NAME}.so" /data/data/jp.co.cygames.umamusume/armeabi-v7a.so
  fi
  if [ -d "/data/data/com.kakaogames.umamusume" ]; then
    touch /data/data/com.kakaogames.umamusume/armeabi-v7a.so
    mount -o bind "/data/adb/modules/${API}_${MODULE_ID}/lib/armeabi-v7a/lib${MODULE_LIB_NAME}.so" /data/data/com.kakaogames.umamusume/armeabi-v7a.so
  fi
elif [ $ABI = "x86_64" ]; then
  if [ -d "/data/data/jp.co.cygames.umamusume" ]; then
    touch /data/data/jp.co.cygames.umamusume/arm64-v8a.so
    touch /data/data/jp.co.cygames.umamusume/armeabi-v7a.so
    mount -o bind "/data/adb/modules/${API}_${MODULE_ID}/lib/arm64-v8a/lib${MODULE_LIB_NAME}.so" /data/data/jp.co.cygames.umamusume/arm64-v8a.so
    mount -o bind "/data/adb/modules/${API}_${MODULE_ID}/lib/armeabi-v7a/lib${MODULE_LIB_NAME}.so" /data/data/jp.co.cygames.umamusume/armeabi-v7a.so
  fi
  if [ -d "/data/data/com.kakaogames.umamusume" ]; then
    touch /data/data/com.kakaogames.umamusume/arm64-v8a.so
    touch /data/data/com.kakaogames.umamusume/armeabi-v7a.so
    mount -o bind "/data/adb/modules/${API}_${MODULE_ID}/lib/arm64-v8a/lib${MODULE_LIB_NAME}.so" /data/data/com.kakaogames.umamusume/arm64-v8a.so
    mount -o bind "/data/adb/modules/${API}_${MODULE_ID}/lib/armeabi-v7a/lib${MODULE_LIB_NAME}.so" /data/data/com.kakaogames.umamusume/armeabi-v7a.so
  fi
fi
