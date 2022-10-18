package com.kimjio.umamusumelocalify;

import android.app.Notification;
import android.content.Context;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.Icon;
import android.util.Log;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Member;
import java.lang.reflect.Method;

@SuppressWarnings("unused")
public final class Hooker {
    private static final String TAG = "UmamusumeLocalify[Java]";
    private static final String KEY_LARGE_ICON = "largeIcon";

    public Method backup;

    private Member target;
    private Method replacement;
    private Object owner = null;

    @SuppressWarnings("JavaJniMissingFunction")
    private native Method doHook(Member original, Method callback);

    @SuppressWarnings("JavaJniMissingFunction")
    private native boolean doUnhook(Member target);

    public Object callback(Object[] args) throws InvocationTargetException, IllegalAccessException {
        MethodCallback methodCallback = new MethodCallback(backup, args);
        return replacement.invoke(owner, methodCallback);
    }

    public boolean unhook() {
        return doUnhook(target);
    }

    public static Hooker hook(Member target, Method replacement, Object owner) {
        Hooker hooker = new Hooker();
        try {
            Method callbackMethod = Hooker.class.getDeclaredMethod("callback", Object[].class);
            Method result = hooker.doHook(target, callbackMethod);
            if (result == null) return null;
            hooker.backup = result;
            hooker.target = target;
            hooker.replacement = replacement;
            hooker.owner = owner;
        } catch (NoSuchMethodException ignored) {
        }
        return hooker;
    }

    Notification build(MethodCallback callback) throws InvocationTargetException, IllegalAccessException {
        Notification.Builder thiz = (Notification.Builder) callback.args[0];
        Icon largeIcon = null;
        String largeIconPath = thiz.getExtras().getString(KEY_LARGE_ICON);
        try {
            Field contextField = thiz.getClass().getField("mContext");
            Context context = (Context) contextField.get(thiz);
            largeIcon = Icon.createWithResource(context, findResourceIdInContextByName(context, largeIconPath));
        } catch (Exception ignore) {
        }
        if (largeIconPath != null && largeIcon == null) {
            Bitmap bitmap = BitmapFactory.decodeFile(largeIconPath);
            if (bitmap != null) {
                largeIcon = Icon.createWithBitmap(bitmap);
            }
        }
        thiz.setLargeIcon(largeIcon);
        return (Notification) callback.backup.invoke(thiz);
    }

    private int findResourceIdInContextByName(Context context, String name) {
        if (name == null)
            return 0;

        try {
            Resources res = context.getResources();
            if (res != null) {
                int id = res.getIdentifier(name, "mipmap", context.getPackageName());
                if (id == 0)
                    return res.getIdentifier(name, "drawable", context.getPackageName());
                else
                    return id;
            }
            return 0;
        } catch (Resources.NotFoundException e) {
            return 0;
        }
    }

    public static void load() throws NoSuchMethodException, SecurityException {
        Hooker.hook(
                Notification.Builder.class.getDeclaredMethod("build"),
                Hooker.class.getDeclaredMethod("build", MethodCallback.class),
                new Hooker());
        Log.i(TAG, "Notification$Builder.build() hooked");
    }
}
