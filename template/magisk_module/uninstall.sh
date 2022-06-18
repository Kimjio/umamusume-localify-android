ABI=$(getprop ro.product.cpu.abi)

if [ $ABI = "x86" ]; then
  if [ -d "/data/data/jp.co.cygames.umamusume" ]; then
    umount -f /data/data/jp.co.cygames.umamusume/armeabi-v7a.so
    rm /data/data/jp.co.cygames.umamusume/armeabi-v7a.so
  fi
  if [ -d "/data/data/com.kakaogames.umamusume" ]; then
    umount -f /data/data/com.kakaogames.umamusume/armeabi-v7a.so
    rm /data/data/com.kakaogames.umamusume/armeabi-v7a.so
  fi
elif [ $ABI = "x86_64" ]; then
  if [ -d "/data/data/jp.co.cygames.umamusume" ]; then
    umount -f /data/data/jp.co.cygames.umamusume/arm64-v8a.so
    umount -f /data/data/jp.co.cygames.umamusume/armeabi-v7a.so
    rm /data/data/jp.co.cygames.umamusume/arm64-v8a.so
    rm /data/data/jp.co.cygames.umamusume/armeabi-v7a.so
  fi
  if [ -d "/data/data/com.kakaogames.umamusume" ]; then
    umount -f /data/data/com.kakaogames.umamusume/arm64-v8a.so
    umount -f /data/data/com.kakaogames.umamusume/armeabi-v7a.so
    rm /data/data/com.kakaogames.umamusume/arm64-v8a.so
    rm /data/data/com.kakaogames.umamusume/armeabi-v7a.so
  fi
fi

# Don't modify anything after this
if [ -f $INFO ]; then
  while read LINE; do
    if [ "$(echo -n $LINE | tail -c 1)" == "~" ]; then
      continue
    elif [ -f "$LINE~" ]; then
      mv -f $LINE~ $LINE
    else
      rm -f $LINE
      while true; do
        LINE=$(dirname $LINE)
        [ "$(ls -A $LINE 2>/dev/null)" ] && break 1 || rm -rf $LINE
      done
    fi
  done < $INFO
  rm -f $INFO
fi
