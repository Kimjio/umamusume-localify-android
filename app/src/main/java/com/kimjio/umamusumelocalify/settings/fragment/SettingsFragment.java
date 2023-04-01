package com.kimjio.umamusumelocalify.settings.fragment;


import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.view.Display;
import android.widget.Toast;

import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.annotation.Nullable;
import androidx.preference.EditTextPreference;
import androidx.preference.Preference;
import androidx.preference.SeekBarPreference;
import androidx.preference.TwoStatePreference;

import com.kimjio.umamusumelocalify.settings.Constants;
import com.kimjio.umamusumelocalify.settings.ModuleUtils;
import com.kimjio.umamusumelocalify.settings.R;
import com.kimjio.umamusumelocalify.settings.activity.BaseActivity;
import com.kimjio.umamusumelocalify.settings.activity.JsonEditActivity;
import com.kimjio.umamusumelocalify.settings.activity.ManageTranslateActivity;
import com.kimjio.umamusumelocalify.settings.preference.FilePickerPreference;

import java.util.Objects;

public class SettingsFragment extends BaseSettingsFragment {
    private static final int TAPS_TO_MANUAL_EDIT = 7;

    private Toast mManualEditHitToast;
    private int mManualEditHitCountdown;

    private final ActivityResultLauncher<Intent> editConfig =
            registerForActivityResult(new ActivityResultContracts.StartActivityForResult(), result -> {
                if (result.getResultCode() == JsonEditActivity.RESULT_SAVED) {
                    requireActivity().recreate();
                }
            });

    @Override
    public void onStart() {
        super.onStart();
        mManualEditHitCountdown = TAPS_TO_MANUAL_EDIT;
    }

    @Override
    public void onCreatePreferences(@Nullable Bundle savedInstanceState, @Nullable String rootKey) {
        super.onCreatePreferences(savedInstanceState, rootKey);

        /*SwitchPreference restoreNotificationPreference = Objects.requireNonNull(findPreference("restoreNotification"));
        restoreNotificationPreference.setDefaultValue(Constants.PKG_KOR.equals(packageName));
        restoreNotificationPreference.setVisible(Constants.PKG_KOR.equals(packageName));
        dataStore.putBoolean(restoreNotificationPreference.getKey(), Constants.PKG_KOR.equals(packageName));*/

        TwoStatePreference restoreGallopWebviewPreference = Objects.requireNonNull(findPreference("restoreGallopWebview"));
        restoreGallopWebviewPreference.setVisible(Constants.PKG_KOR.equals(packageName));

        String moduleVersion = ModuleUtils.getModuleVersion();
        Preference versionPreference = Objects.requireNonNull(findPreference("version"));
        versionPreference.setOnPreferenceClickListener(p -> {
            if (mManualEditHitCountdown > 0) {
                mManualEditHitCountdown--;
                if (mManualEditHitCountdown == 0) {
                    mManualEditHitCountdown = TAPS_TO_MANUAL_EDIT;
                    editConfig.launch(new Intent(requireContext(), JsonEditActivity.class)
                            .putExtra(BaseActivity.EXTRA_PATH, path));
                } else if (mManualEditHitCountdown < (TAPS_TO_MANUAL_EDIT - 2)) {
                    if (mManualEditHitToast != null) {
                        mManualEditHitToast.cancel();
                    }
                    mManualEditHitToast = Toast.makeText(requireContext(),
                            requireContext().getResources().getQuantityString(
                                    R.plurals.show_manual_edit_countdown, mManualEditHitCountdown,
                                    mManualEditHitCountdown),
                            Toast.LENGTH_SHORT);
                    mManualEditHitToast.show();
                }
            }
            return true;
        });
        if (moduleVersion != null && !moduleVersion.isBlank()) {
            versionPreference.setSummary(moduleVersion);
        }

        TwoStatePreference enableLoggerPreference = Objects.requireNonNull(findPreference("enableLogger"));
        TwoStatePreference dumpStaticEntriesPreference = Objects.requireNonNull(findPreference("dumpStaticEntries"));
        TwoStatePreference dumpDbEntriesPreference = Objects.requireNonNull(findPreference("dumpDbEntries"));
        SeekBarPreference maxFpsPreference = Objects.requireNonNull(findPreference("maxFps"));

        TwoStatePreference replaceToBuiltinFontPreference = Objects.requireNonNull(findPreference("replaceToBuiltinFont"));
        TwoStatePreference replaceToCustomFontPreference = Objects.requireNonNull(findPreference("replaceToCustomFont"));
        FilePickerPreference fontAssetBundlePathPreference = Objects.requireNonNull(findPreference("fontAssetBundlePath"));
        EditTextPreference fontAssetNamePreference = Objects.requireNonNull(findPreference("fontAssetName"));
        EditTextPreference tmproFontAssetNamePreference = Objects.requireNonNull(findPreference("tmproFontAssetName"));

        boolean enableLogger = dataStore.getBoolean("enableLogger", false);

        dumpStaticEntriesPreference.setEnabled(enableLogger);
        dumpDbEntriesPreference.setEnabled(enableLogger);

        enableLoggerPreference.setOnPreferenceChangeListener((preference, newValue) -> {
            boolean value = (boolean) newValue;
            dumpStaticEntriesPreference.setEnabled(value);
            dumpDbEntriesPreference.setEnabled(value);
            return true;
        });

        Display display;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            display = requireActivity().getDisplay();
        } else {
            display = requireActivity().getWindowManager().getDefaultDisplay();
        }

        maxFpsPreference.setMax(Math.round(display.getRefreshRate()));

        boolean replaceToCustomFont = dataStore.getBoolean("replaceToCustomFont", false);

        replaceToBuiltinFontPreference.setEnabled(!replaceToCustomFont);
        fontAssetBundlePathPreference.setEnabled(replaceToCustomFont);
        fontAssetNamePreference.setEnabled(replaceToCustomFont);
        tmproFontAssetNamePreference.setEnabled(replaceToCustomFont);

        replaceToCustomFontPreference.setOnPreferenceChangeListener((preference, newValue) -> {
            boolean value = (boolean) newValue;
            replaceToBuiltinFontPreference.setEnabled(!value);
            fontAssetBundlePathPreference.setEnabled(value);
            fontAssetNamePreference.setEnabled(value);
            tmproFontAssetNamePreference.setEnabled(value);
            return true;
        });

        Preference manageTranslate = Objects.requireNonNull(findPreference("manageTranslate"));
        manageTranslate.setOnPreferenceClickListener(p -> {
            startActivity(new Intent(requireContext(), ManageTranslateActivity.class)
                    .putExtra(BaseActivity.EXTRA_PACKAGE_NAME, packageName)
                    .putExtra(BaseActivity.EXTRA_PATH, parentPath)
                    .putExtra(BaseActivity.EXTRA_CONFIG_PATH, path));
            return true;
        });
    }

    @Override
    public void changeDataSource(String packageName, Uri parentPath, Uri path) {
        /*Preference restoreNotificationPreference = Objects.requireNonNull(findPreference("restoreNotification"));
        restoreNotificationPreference.setDefaultValue(Constants.PKG_KOR.equals(packageName));
        restoreNotificationPreference.setVisible(Constants.PKG_KOR.equals(packageName));*/
        TwoStatePreference restoreGallopWebviewPreference = Objects.requireNonNull(findPreference("restoreGallopWebview"));
        restoreGallopWebviewPreference.setVisible(Constants.PKG_KOR.equals(packageName));
        super.changeDataSource(packageName, parentPath, path);
    }

    @Override
    protected int getPreferenceResourceId() {
        return R.xml.preference;
    }
}
