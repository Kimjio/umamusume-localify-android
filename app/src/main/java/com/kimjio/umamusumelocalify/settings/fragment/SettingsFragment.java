package com.kimjio.umamusumelocalify.settings.fragment;


import android.app.Dialog;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.documentfile.provider.DocumentFile;
import androidx.fragment.app.DialogFragment;
import androidx.preference.EditTextPreference;
import androidx.preference.Preference;
import androidx.preference.PreferenceDataStore;
import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.PreferenceGroup;
import androidx.preference.PreferenceManager;
import androidx.preference.SeekBarPreference;
import androidx.preference.TwoStatePreference;

import com.google.android.material.dialog.MaterialAlertDialogBuilder;
import com.kimjio.umamusumelocalify.settings.ModuleUtils;
import com.kimjio.umamusumelocalify.settings.R;
import com.kimjio.umamusumelocalify.settings.activity.ManageTranslateActivity;
import com.kimjio.umamusumelocalify.settings.preference.FilePickerPreference;
import com.kimjio.umamusumelocalify.settings.preference.JsonPreferenceDataStore;
import com.kimjio.umamusumelocalify.settings.preference.MaterialEditTextPreferenceDialogFragmentCompat;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.Objects;

public class SettingsFragment extends PreferenceFragmentCompat {
    public static final String KEY_INITIAL_PATH = "initial_path";
    public static final String KEY_INITIAL_PARENT_PATH = "initial_parent_path";
    public static final String KEY_PACKAGE_NAME = "package_name";

    private String packageName;

    private Uri path;

    private Uri parentPath;

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        if (getArguments() != null) {
            packageName = getArguments().getString(KEY_PACKAGE_NAME);
            path = getArguments().getParcelable(KEY_INITIAL_PATH);
            parentPath = getArguments().getParcelable(KEY_INITIAL_PARENT_PATH);
        }
        super.onCreate(savedInstanceState);
    }

    @Override
    public void onCreatePreferences(@Nullable Bundle savedInstanceState, @Nullable String rootKey) {
        PreferenceDataStore dataStore = new JsonPreferenceDataStore(requireContext().getContentResolver(), requireArguments().getParcelable(KEY_INITIAL_PATH));
        getPreferenceManager().setPreferenceDataStore(dataStore);
        setPreferencesFromResource(R.xml.preference, rootKey);
        DocumentFile file = DocumentFile.fromSingleUri(requireContext(), path);
        if (file != null) {
            getPreferenceScreen().setEnabled(file.canWrite());
            if (!getPreferenceScreen().isEnabled() && Build.VERSION.SDK_INT >= 33) {
                Dialog dialog = new MaterialAlertDialogBuilder(requireContext())
                        .setIcon(R.drawable.ic_error)
                        .setTitle(R.string.error)
                        .setMessage(R.string.error_readonly)
                        .setPositiveButton(android.R.string.ok, null)
                        .create();
                float density = getResources().getDisplayMetrics().density;
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                    dialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_BLUR_BEHIND);
                    dialog.getWindow().getAttributes().setBlurBehindRadius((int) (density * 16.0f));
                }
                dialog.show();
            }
        } else {
            getPreferenceScreen().setEnabled(false);
        }

        /*SwitchPreference restoreNotificationPreference = Objects.requireNonNull(findPreference("restoreNotification"));
        restoreNotificationPreference.setDefaultValue(Constants.PKG_KOR.equals(packageName));
        restoreNotificationPreference.setVisible(Constants.PKG_KOR.equals(packageName));
        dataStore.putBoolean(restoreNotificationPreference.getKey(), Constants.PKG_KOR.equals(packageName));*/

        String moduleVersion = ModuleUtils.getModuleVersion();
        if (moduleVersion != null && !moduleVersion.isBlank()) {
            Preference versionPreference = Objects.requireNonNull(findPreference("version"));
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

        boolean enableLogger = dataStore.getBoolean("enableLogger", false);

        dumpStaticEntriesPreference.setEnabled(enableLogger);
        dumpDbEntriesPreference.setEnabled(enableLogger);

        enableLoggerPreference.setOnPreferenceChangeListener((preference, newValue) -> {
            boolean value = (boolean) newValue;
            dumpStaticEntriesPreference.setEnabled(value);
            dumpDbEntriesPreference.setEnabled(value);
            return true;
        });

        maxFpsPreference.setMax(Math.round(requireActivity().getWindowManager().getDefaultDisplay().getRefreshRate()));

        boolean replaceToCustomFont = dataStore.getBoolean("replaceToCustomFont", false);

        replaceToBuiltinFontPreference.setEnabled(!replaceToCustomFont);
        fontAssetBundlePathPreference.setEnabled(replaceToCustomFont);
        fontAssetNamePreference.setEnabled(replaceToCustomFont);

        replaceToCustomFontPreference.setOnPreferenceChangeListener((preference, newValue) -> {
            boolean value = (boolean) newValue;
            replaceToBuiltinFontPreference.setEnabled(!value);
            fontAssetBundlePathPreference.setEnabled(value);
            fontAssetNamePreference.setEnabled(value);
            return true;
        });

        Preference manageTranslate = Objects.requireNonNull(findPreference("manageTranslate"));
        manageTranslate.setOnPreferenceClickListener(p -> {
            requireContext().startActivity(new Intent(requireContext(), ManageTranslateActivity.class)
                    .putExtra(ManageTranslateActivity.EXTRA_PACKAGE_NAME, packageName)
                    .putExtra(ManageTranslateActivity.EXTRA_PATH, parentPath)
                    .putExtra(ManageTranslateActivity.EXTRA_CONFIG_PATH, path));
            return true;
        });
    }

    @NonNull
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = super.onCreateView(inflater, container, savedInstanceState);

        ViewCompat.setOnApplyWindowInsetsListener(requireActivity().getWindow().getDecorView(), (v, insets) -> {
            getListView().setPaddingRelative(0, 0, 0, insets.getInsets(WindowInsetsCompat.Type.navigationBars()).bottom);
            return insets;
        });

        return view;
    }

    @Override
    public void onDisplayPreferenceDialog(@NonNull Preference preference) {
        if (preference instanceof EditTextPreference) {
            final DialogFragment f;
            f = MaterialEditTextPreferenceDialogFragmentCompat.newInstance(preference.getKey());
            f.setTargetFragment(this, 0);
            f.show(getParentFragmentManager(), "androidx.preference.PreferenceFragment.DIALOG");
        } else {
            super.onDisplayPreferenceDialog(preference);
        }
    }

    public void changeDataSource(String packageName, Uri parentPath, Uri path) {
        this.packageName = packageName;
        this.parentPath = parentPath;
        this.path = path;
        /*Preference restoreNotificationPreference = Objects.requireNonNull(findPreference("restoreNotification"));
        restoreNotificationPreference.setDefaultValue(Constants.PKG_KOR.equals(packageName));
        restoreNotificationPreference.setVisible(Constants.PKG_KOR.equals(packageName));*/
        getPreferenceManager().setPreferenceDataStore(new JsonPreferenceDataStore(requireContext().getContentResolver(), this.path));
        getPreferenceScreen().notifyDependencyChange(false);
        updatePreference(getPreferenceScreen());
    }

    private void updatePreference(PreferenceGroup preference) {
        try {
            Method method = Preference.class.getDeclaredMethod("onAttachedToHierarchy", PreferenceManager.class);
            method.setAccessible(true);
            Method method1 = Preference.class.getDeclaredMethod("notifyChanged");
            method1.setAccessible(true);
            Field field = TwoStatePreference.class.getDeclaredField("mChecked");
            field.setAccessible(true);

            for (int i = 0; i < preference.getPreferenceCount(); i++) {
                Preference childPreference = preference.getPreference(i);
                if (childPreference instanceof PreferenceGroup) {
                    updatePreference((PreferenceGroup) childPreference);
                } else {
                    if (!childPreference.getClass().equals(Preference.class)) {
                        method.invoke(childPreference, getPreferenceManager());
                        method1.invoke(childPreference);
                    }
                    if (childPreference instanceof TwoStatePreference) {
                        Preference.OnPreferenceChangeListener listener = childPreference.getOnPreferenceChangeListener();
                        if (listener != null) {
                            listener.onPreferenceChange(childPreference, field.getBoolean(childPreference));
                        }
                    }
                }
            }
        } catch (ReflectiveOperationException | IllegalArgumentException e) {
            e.printStackTrace();
        }
    }
}
