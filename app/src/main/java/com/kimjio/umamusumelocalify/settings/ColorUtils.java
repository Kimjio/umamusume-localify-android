package com.kimjio.umamusumelocalify.settings;

public final class ColorUtils {
    public static int argb(float alpha, int red, int green, int blue) {
        return ((int) (alpha * 255.0f + 0.5f) << 24) |
                (red << 16) |
                (green << 8) |
                blue;
    }
}
