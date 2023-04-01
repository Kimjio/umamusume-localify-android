package com.kimjio.umamusumelocalify.settings.activity;

import android.content.res.Configuration;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.os.Build;
import android.os.Bundle;
import android.view.MenuItem;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;

import androidx.annotation.ColorInt;
import androidx.annotation.LayoutRes;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;
import androidx.databinding.DataBindingUtil;
import androidx.databinding.ViewDataBinding;

import com.kimjio.umamusumelocalify.settings.ColorUtils;
import com.kimjio.umamusumelocalify.settings.R;

import java.lang.reflect.ParameterizedType;
import java.util.Objects;
import java.util.function.Consumer;


public abstract class BaseActivity<VB extends ViewDataBinding> extends AppCompatActivity {
    public static final String EXTRA_PACKAGE_NAME = "package_name";

    public static final String EXTRA_PATH = "path";

    public static final String EXTRA_CONFIG_PATH = "config_path";

    protected VB binding;

    @ColorInt
    protected int backgroundColor;

    private final Consumer<Boolean> crossWindowBlurEnabledListener = enabled -> {
        if (enabled) {
            getWindow().setBackgroundDrawable(new ColorDrawable(backgroundColor));
        }
    };

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        binding = DataBindingUtil.setContentView(this, getLayoutRes());
        binding.setLifecycleOwner(this);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            getWindow().setDecorFitsSystemWindows(false);
        } else {
            View decorView = getWindow().getDecorView();
            decorView.setSystemUiVisibility(decorView.getSystemUiVisibility() | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION | View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
        }

        if (enableBlurOnCreate()) {
            enableBlur();
        }

        backgroundColor = ((ColorDrawable) getWindow().getDecorView().getBackground()).getColor();

        if ((getResources().getConfiguration().uiMode & Configuration.UI_MODE_NIGHT_MASK) == Configuration.UI_MODE_NIGHT_YES) {
            backgroundColor = ColorUtils.argb(0.5f, Color.red(backgroundColor), Color.green(backgroundColor), Color.blue(backgroundColor));
        } else {
            backgroundColor = ColorUtils.argb(0.7f, Color.red(backgroundColor), Color.green(backgroundColor), Color.blue(backgroundColor));
        }

        Window window = getWindow();
        window.setBackgroundDrawable(new ColorDrawable(backgroundColor));

        float density = getResources().getDisplayMetrics().density;

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            window.getAttributes().setBlurBehindRadius((int) (density * 20.0f));
            window.getDecorView().addOnAttachStateChangeListener(
                    new View.OnAttachStateChangeListener() {
                        @Override
                        public void onViewAttachedToWindow(View v) {
                            getWindowManager().addCrossWindowBlurEnabledListener(
                                    crossWindowBlurEnabledListener);
                        }

                        @Override
                        public void onViewDetachedFromWindow(View v) {
                            getWindowManager().removeCrossWindowBlurEnabledListener(
                                    crossWindowBlurEnabledListener);
                        }
                    });
        }
    }

    public void enableBlur() {
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_SHOW_WALLPAPER | WindowManager.LayoutParams.FLAG_BLUR_BEHIND);
    }

    public void disableBlur() {
        getWindow().clearFlags(WindowManager.LayoutParams.FLAG_SHOW_WALLPAPER | WindowManager.LayoutParams.FLAG_BLUR_BEHIND);
    }

    protected boolean enableBlurOnCreate() {
        return true;
    }

    @NonNull
    public ActionBar requireSupportActionBar() {
        ActionBar actionBar = getSupportActionBar();
        if (actionBar == null)
            throw new IllegalArgumentException("ActionBar does not set for this Activity");
        return actionBar;
    }

    @LayoutRes
    private int getLayoutRes() {
        String[] split = ((Class<?>) ((ParameterizedType) Objects.requireNonNull(getClass().getGenericSuperclass())).getActualTypeArguments()[0])
                .getSimpleName().replace("Binding", "").split("(?<=.)(?=\\p{Upper})");
        StringBuilder name = new StringBuilder();

        for (int i = 0; i < split.length; i++) {
            name.append(split[i].toLowerCase());
            if (i != split.length - 1)
                name.append("_");
        }

        try {
            return R.layout.class.getField(name.toString()).getInt(R.layout.class);
        } catch (IllegalAccessException | NoSuchFieldException e) {
            e.printStackTrace();
        }

        return 0;
    }

    @Override
    public boolean onOptionsItemSelected(@NonNull MenuItem item) {
        boolean result = false;
        if (item.getItemId() == android.R.id.home) {
            finishAfterTransition();
            result = true;
        }
        return result || super.onOptionsItemSelected(item);
    }
}