package com.kimjio.umamusumelocalify.settings.activity;

import android.annotation.SuppressLint;
import android.app.ActivityManager;
import android.app.Dialog;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.provider.DocumentsContract;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.TextView;

import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.documentfile.provider.DocumentFile;
import androidx.preference.Preference;
import androidx.preference.PreferenceGroup;
import androidx.preference.PreferenceScreen;

import com.google.android.material.dialog.MaterialAlertDialogBuilder;
import com.kimjio.umamusumelocalify.settings.BuildConfig;
import com.kimjio.umamusumelocalify.settings.Constants;
import com.kimjio.umamusumelocalify.settings.R;
import com.kimjio.umamusumelocalify.settings.databinding.MainActivityBinding;
import com.kimjio.umamusumelocalify.settings.fragment.SettingsFragment;
import com.kimjio.umamusumelocalify.settings.preference.IActivityResultPreference;

import java.util.List;
import java.util.stream.Collectors;

public class MainActivity extends BaseActivity<MainActivityBinding> {

    public static final String KEY_LAST_SELECTED = "last_selected_index";
    public static final String KEY_LAST_SELECTED_PACKAGE = "last_selected_package";

    private SharedPreferences preferences;

    private String currentPackageName;

    private SettingsFragment fragment;

    private final ActivityResultLauncher<Uri> requestDocumentTree = registerForActivityResult(
            new ActivityResultContracts.OpenDocumentTree(), result -> {
                if (result != null) {
                    if (!result.getPath().endsWith("Android/data/" + currentPackageName)) {
                        showWrongPathDialog();
                        return;
                    }
                    getContentResolver().takePersistableUriPermission(result,
                            Intent.FLAG_GRANT_READ_URI_PERMISSION |
                                    Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
                    preferences.edit().putString(currentPackageName, result.toString()).apply();
                    initFragment(DocumentFile.fromTreeUri(this, result));
                } else {
                    binding.progress.setVisibility(View.GONE);
                    binding.permRequired.setVisibility(View.VISIBLE);
                }
            });

    private boolean createdAfterStateSaved = false;

    private boolean hasError = false;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setSupportActionBar(binding.appBar);
        requireSupportActionBar().setDisplayShowTitleEnabled(false);

        binding.appBarLayout.setBackgroundColor(backgroundColor);

        preferences = getSharedPreferences(Constants.PREF_NAME, MODE_PRIVATE);

        binding.requestButton.setOnClickListener(v -> requestFolderAccess());

        @SuppressLint("QueryPermissionsNeeded")
        List<PackageInfo> packageInfoList = getPackageManager()
                .getInstalledPackages(PackageManager.GET_META_DATA)
                .stream()
                .filter(item -> ((item.applicationInfo.flags & ApplicationInfo.FLAG_SYSTEM) == 0) &&
                        !item.packageName.equals(BuildConfig.APPLICATION_ID))
                .filter(item -> Constants.targetPackageNames.contains(item.packageName))
                .collect(Collectors.toList());

        binding.packageSpinner.setAdapter(new PackageAdapter(packageInfoList));
        int lastSelected = preferences.getInt(KEY_LAST_SELECTED, 0);
        if (lastSelected > packageInfoList.size()) {
            lastSelected = 0;
        }
        binding.packageSpinner.setSelection(lastSelected);
        binding.packageSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                setPreferences(currentPackageName = packageInfoList.get(position).packageName);
                preferences.edit().putInt(KEY_LAST_SELECTED, position).apply();
                preferences.edit().putString(KEY_LAST_SELECTED_PACKAGE, currentPackageName).apply();
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
            }
        });

        if (packageInfoList.isEmpty()) {
            binding.progress.setVisibility(View.GONE);
            binding.appNotFound.setVisibility(View.VISIBLE);
        }
    }

    @Override
    public boolean onCreateOptionsMenu(@NonNull Menu menu) {
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(@NonNull MenuItem item) {
        if (item.getItemId() == R.id.menu_restart_app && currentPackageName != null) {
            getSystemService(ActivityManager.class).killBackgroundProcesses(currentPackageName);
            Intent intent = getPackageManager().getLaunchIntentForPackage(currentPackageName);
            startActivity(intent);
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (createdAfterStateSaved && !getSupportFragmentManager().isStateSaved()) {
            createdAfterStateSaved = false;
            setPreferences(currentPackageName);
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (fragment != null && fragment.getPreferenceManager() != null) {
            PreferenceScreen preferenceScreen = fragment.getPreferenceScreen();
            if (preferenceScreen != null) {
                preferenceActivityResult(requestCode, resultCode, data, preferenceScreen);
            }
        }
    }

    private void preferenceActivityResult(int requestCode, int resultCode, @Nullable Intent data, PreferenceGroup preferenceGroup) {
        for (int i = 0; i < preferenceGroup.getPreferenceCount(); i++) {
            Preference childPreference = preferenceGroup.getPreference(i);
            if (childPreference instanceof PreferenceGroup) {
                preferenceActivityResult(requestCode, resultCode, data, (PreferenceGroup) childPreference);
            } else {
                if (childPreference instanceof IActivityResultPreference) {
                    ((IActivityResultPreference) childPreference).onActivityResult(requestCode, resultCode, data);
                }
            }
        }
    }

    private void requestFolderAccess() {
        requestDocumentTree.launch(
                DocumentsContract.buildDocumentUri(
                        "com.android.externalstorage.documents",
                        "primary:Android/data/" + currentPackageName));
    }

    private void showWrongPathDialog() {
        Dialog dialog = new MaterialAlertDialogBuilder(this)
                .setIcon(R.drawable.ic_info)
                .setTitle(R.string.error)
                .setMessage(R.string.error_wrong_path)
                .setPositiveButton(android.R.string.ok, (d, which) -> requestFolderAccess())
                .setNegativeButton(android.R.string.cancel, (d, which) -> {
                    binding.progress.setVisibility(View.GONE);
                    binding.permRequired.setVisibility(View.VISIBLE);
                })
                .create();
        float density = getResources().getDisplayMetrics().density;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            dialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_BLUR_BEHIND);
            dialog.getWindow().getAttributes().setBlurBehindRadius((int) (density * 16.0f));
        }
        dialog.show();
    }

    private void setPreferences(String packageName) {
        binding.progress.setVisibility(View.VISIBLE);
        binding.permRequired.setVisibility(View.GONE);

        String treePath = preferences.getString(packageName, null);

        if (treePath == null) {
            if (fragment != null) {
                View view = fragment.getView();
                if (view != null) {
                    view.setVisibility(View.GONE);
                }
            }
            Dialog dialog = new MaterialAlertDialogBuilder(this)
                    .setIcon(R.drawable.ic_info)
                    .setTitle(R.string.perm_required)
                    .setMessage(R.string.perm_required_message)
                    .setPositiveButton(android.R.string.ok, (d, which) -> requestFolderAccess())
                    .setNegativeButton(android.R.string.cancel, (d, which) -> {
                        binding.progress.setVisibility(View.GONE);
                        binding.permRequired.setVisibility(View.VISIBLE);
                    })
                    .create();
            float density = getResources().getDisplayMetrics().density;
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                dialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_BLUR_BEHIND);
                dialog.getWindow().getAttributes().setBlurBehindRadius((int) (density * 16.0f));
            }
            dialog.show();
        } else {
            initFragment(DocumentFile.fromTreeUri(this, Uri.parse(treePath)));
        }
    }

    private void initFragment(DocumentFile file) {
        if (file != null) {
            DocumentFile configFile = file.findFile("config.json");
            if (configFile == null) {
                configFile = file.createFile("application/json", "config.json");
                if (configFile == null) {
                    if (!hasError) {
                        hasError = true;
                        // Provider is null or revoked
                        new Handler().postDelayed(() -> initFragment(file), 1000);
                    } else {
                        // Permission revoked
                        preferences.edit().remove(currentPackageName).apply();
                        setPreferences(currentPackageName);
                    }
                } else {
                    hasError = false;
                }
            }
            if (configFile != null) {
                if (fragment == null) {
                    if (!getSupportFragmentManager().isStateSaved()) {
                        Bundle bundle = new Bundle();
                        bundle.putString(SettingsFragment.KEY_PACKAGE_NAME, currentPackageName);
                        bundle.putParcelable(SettingsFragment.KEY_INITIAL_PATH, configFile.getUri());
                        bundle.putParcelable(SettingsFragment.KEY_INITIAL_PARENT_PATH, file.getUri());
                        fragment = new SettingsFragment();
                        fragment.setArguments(bundle);
                        getSupportFragmentManager().beginTransaction().replace(R.id.frame, fragment).commit();
                        binding.progress.setVisibility(View.GONE);
                        binding.permRequired.setVisibility(View.GONE);
                    } else {
                        createdAfterStateSaved = true;
                    }
                } else {
                    fragment.changeDataSource(currentPackageName, file.getUri(), configFile.getUri());
                    binding.progress.setVisibility(View.GONE);
                    binding.permRequired.setVisibility(View.GONE);
                    View view = fragment.getView();
                    if (view != null) {
                        view.setVisibility(View.VISIBLE);
                    }
                }
            }
        } else {
            // Permission revoked
            preferences.edit().remove(currentPackageName).apply();
            setPreferences(currentPackageName);
        }
    }

    private class PackageAdapter extends BaseAdapter {
        final List<PackageInfo> packageInfoList;

        public PackageAdapter(List<PackageInfo> packageInfoList) {
            this.packageInfoList = packageInfoList;
        }

        @Override
        public int getCount() {
            return packageInfoList.size();
        }

        @Override
        public Object getItem(int position) {
            return packageInfoList.get(position);
        }

        @Override
        public long getItemId(int position) {
            if (packageInfoList.isEmpty()) {
                return 0;
            }
            return packageInfoList.get(position).packageName.hashCode();
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            if (convertView == null) {
                convertView = LayoutInflater.from(MainActivity.this).inflate(R.layout.package_item, parent, false);
            }
            TextView nameView = convertView.findViewById(R.id.name);
            TextView packageNameView = convertView.findViewById(R.id.package_name);
            nameView.setText(getPackageManager().getApplicationLabel(packageInfoList.get(position).applicationInfo));
            packageNameView.setText(packageInfoList.get(position).packageName);
            return convertView;
        }
    }


}
