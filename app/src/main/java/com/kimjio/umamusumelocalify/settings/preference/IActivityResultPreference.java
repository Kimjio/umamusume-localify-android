package com.kimjio.umamusumelocalify.settings.preference;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.view.ContextThemeWrapper;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public interface IActivityResultPreference {

    void onActivityResult(int requestCode, int resultCode, @Nullable Intent data);

    @NonNull
    Context getContext();

    @Nullable
    default Activity getActivity() {
        Context c = getContext();
        if (c instanceof ContextThemeWrapper) {
            if (((ContextThemeWrapper) c).getBaseContext() instanceof Activity)
                return (Activity) ((ContextThemeWrapper) c).getBaseContext();
        }
        if (c instanceof Activity)
            return (Activity) c;
        return null;
    }
}
