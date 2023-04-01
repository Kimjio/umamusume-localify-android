package com.kimjio.umamusumelocalify.settings.fragment;

import android.content.Intent;
import android.os.Bundle;

import androidx.annotation.Nullable;
import androidx.preference.Preference;

import com.kimjio.umamusumelocalify.settings.R;
import com.kimjio.umamusumelocalify.settings.activity.BaseActivity;
import com.kimjio.umamusumelocalify.settings.activity.MsgPackListActivity;

import java.util.Objects;

public class MsgPackSettingsFragment extends BaseSettingsFragment {
    @Override
    public void onCreatePreferences(@Nullable Bundle savedInstanceState, @Nullable String rootKey) {
        super.onCreatePreferences(savedInstanceState, rootKey);

        Preference history = Objects.requireNonNull(findPreference("history"));
        history.setOnPreferenceClickListener(p -> {
            requireContext().startActivity(new Intent(requireContext(), MsgPackListActivity.class)
                    .putExtra(BaseActivity.EXTRA_PATH, parentPath));
            return true;
        });
    }

    @Override
    protected int getPreferenceResourceId() {
        return R.xml.preference_msgpack;
    }
}
