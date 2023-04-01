package com.kimjio.umamusumelocalify.settings.app;

import android.app.Application;
import android.content.res.Configuration;

import androidx.annotation.OptIn;
import androidx.window.core.ExperimentalWindowApi;
import androidx.window.embedding.SplitController;

import com.kimjio.umamusumelocalify.settings.R;

public class SettingsApplication extends Application {
    private static final String TAG = "SettingsApplication";

    @OptIn(markerClass = ExperimentalWindowApi.class)
    @Override
    public void onCreate() {
        super.onCreate();
        if (getResources().getConfiguration().isLayoutSizeAtLeast(Configuration.SCREENLAYOUT_SIZE_LARGE)) {
            SplitController.initialize(this, R.xml.split_configuration);
        }
    }
}
