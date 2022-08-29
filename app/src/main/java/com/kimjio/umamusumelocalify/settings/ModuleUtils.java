package com.kimjio.umamusumelocalify.settings;

import androidx.annotation.Nullable;

public final class ModuleUtils {
    static {
        System.loadLibrary("app");
    }

    @Nullable
    public static native String getModuleVersion();
}
