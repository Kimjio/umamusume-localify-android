package com.kimjio.umamusumelocalify.settings.activity;

import android.net.Uri;
import android.os.Bundle;

import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentManager;

import com.kimjio.umamusumelocalify.settings.R;
import com.kimjio.umamusumelocalify.settings.databinding.SettingsActivityBinding;
import com.kimjio.umamusumelocalify.settings.fragment.SettingsFragment;

public class SettingsActivity extends BaseActivity<SettingsActivityBinding> {

    public static final String EXTRA_TITLE = "title";
    public static final String EXTRA_FRAGMENT_NAME = "fragment_name";
    public static final String EXTRA_FRAGMENT_ARGS = "fragment_args";

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setSupportActionBar(binding.appBar);
        requireSupportActionBar().setDisplayHomeAsUpEnabled(true);

        binding.appBarLayout.setBackgroundColor(backgroundColor);

        if (!getIntent().hasExtra(EXTRA_PACKAGE_NAME) ||
                !getIntent().hasExtra(EXTRA_PATH) ||
                !getIntent().hasExtra(EXTRA_CONFIG_PATH)) {
            finishAfterTransition();
            return;
        }

        if (getIntent().hasExtra(EXTRA_TITLE)) {
            setTitle(getIntent().getStringExtra(EXTRA_TITLE));
        }

        String packageName = getIntent().getStringExtra(EXTRA_PACKAGE_NAME);
        Uri path = getIntent().getParcelableExtra(EXTRA_PATH);
        Uri configPath = getIntent().getParcelableExtra(EXTRA_CONFIG_PATH);

        if (!getIntent().hasExtra(EXTRA_FRAGMENT_NAME)) {
            finishAfterTransition();
            return;
        }

        String fragmentName = getIntent().getStringExtra(EXTRA_FRAGMENT_NAME);

        Bundle bundle = getIntent().getBundleExtra(EXTRA_FRAGMENT_ARGS);
        if (bundle == null) {
            bundle = new Bundle();
        }
        bundle.putString(SettingsFragment.KEY_PACKAGE_NAME, packageName);
        bundle.putParcelable(SettingsFragment.KEY_INITIAL_PATH, configPath);
        bundle.putParcelable(SettingsFragment.KEY_INITIAL_PARENT_PATH, path);

        FragmentManager fragmentManager = getSupportFragmentManager();
        Fragment fragment = fragmentManager.getFragmentFactory().instantiate(getClassLoader(), fragmentName);
        fragment.setArguments(bundle);

        fragmentManager.beginTransaction().replace(R.id.frame, fragment).commit();
    }
}
