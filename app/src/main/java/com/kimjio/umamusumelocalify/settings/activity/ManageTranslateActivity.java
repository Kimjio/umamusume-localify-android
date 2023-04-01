package com.kimjio.umamusumelocalify.settings.activity;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.provider.DocumentsContract;
import android.util.ArraySet;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Toast;

import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.coordinatorlayout.widget.CoordinatorLayout;
import androidx.core.view.OnApplyWindowInsetsListener;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.databinding.DataBindingUtil;
import androidx.documentfile.provider.DocumentFile;
import androidx.preference.PreferenceDataStore;
import androidx.recyclerview.widget.RecyclerView;

import com.kimjio.umamusumelocalify.settings.R;
import com.kimjio.umamusumelocalify.settings.databinding.ManageTranslateActivityBinding;
import com.kimjio.umamusumelocalify.settings.databinding.TranslateItemBinding;
import com.kimjio.umamusumelocalify.settings.preference.JsonPreferenceDataStore;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.io.Writer;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Set;
import java.util.stream.Collectors;

public class ManageTranslateActivity extends BaseActivity<ManageTranslateActivityBinding> {
    private String packageName;

    private PreferenceDataStore dataStore;

    private ActivityResultLauncher<String[]> openMultipleDocuments;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setSupportActionBar(binding.appBar);
        requireSupportActionBar().setDisplayHomeAsUpEnabled(true);

        binding.appBarLayout.setBackgroundColor(backgroundColor);

        if (getIntent().hasExtra(EXTRA_PACKAGE_NAME)) {
            packageName = getIntent().getStringExtra(EXTRA_PACKAGE_NAME);
        } else {
            finishAfterTransition();
            return;
        }

        if (!getIntent().hasExtra(EXTRA_PATH) || !getIntent().hasExtra(EXTRA_CONFIG_PATH)) {
            finishAfterTransition();
            return;
        }

        openMultipleDocuments = registerForActivityResult(new OpenMultipleDocuments(packageName), list -> {
            binding.progress.setVisibility(View.VISIBLE);
            new Thread(() -> {
                List<DocumentFile> files = list.stream().map(item -> DocumentFile.fromSingleUri(this, item))
                        .filter(Objects::nonNull)
                        .collect(Collectors.toList());
                for (DocumentFile item :
                        files) {
                    if (!Objects.requireNonNull(item).getUri().getPath().contains("Android/data/" + packageName)) {
                        runOnUiThread(() -> {
                            binding.progress.setVisibility(View.GONE);
                            Toast.makeText(this, R.string.error_file_not_in_data_folder, Toast.LENGTH_SHORT).show();
                        });
                        return;
                    }
                    if (Objects.equals(item.getName(), "config.json")) {
                        runOnUiThread(() -> {
                            binding.progress.setVisibility(View.GONE);
                            Toast.makeText(this, R.string.error_is_config, Toast.LENGTH_SHORT).show();
                        });
                        return;
                    }
                    try (InputStream is = getContentResolver().openInputStream(item.getUri())) {
                        StringBuilder builder = new StringBuilder();
                        BufferedReader reader = new BufferedReader(new InputStreamReader(is));

                        String line;
                        while ((line = reader.readLine()) != null) {
                            builder.append(line);
                        }
                        JSONObject object = new JSONObject(builder.toString());
                        boolean enabled = true;
                        if (object.has("_enabled")) {
                            enabled = object.getBoolean("_enabled");
                        }

                        String supportVersion = null;
                        if (object.has("_supportVersion")) {
                            supportVersion = object.getString("_supportVersion");
                        }
                        TranslateFile file = new TranslateFile(item.getName(), item.getUri(), supportVersion, enabled, item.canWrite(), false);
                        Set<String> set = Objects.requireNonNull(dataStore.getStringSet("dicts", new ArraySet<>()));
                        String[] split = file.path.getPath().split(packageName + "/");
                        boolean added = set.add(split[split.length - 1]);
                        dataStore.putStringSet("dicts", set);
                        runOnUiThread(() -> {
                            binding.emptyLayout.setVisibility(View.GONE);
                            if (binding.recyclerView.getAdapter() != null && added) {
                                binding.recyclerView.setVisibility(View.VISIBLE);
                                ((TranslateAdapter) binding.recyclerView.getAdapter()).add(file);
                            }
                        });
                    } catch (IOException | JSONException e) {
                        e.printStackTrace();
                    }
                }
                runOnUiThread(() -> binding.progress.setVisibility(View.GONE));
            }).start();
        });

        Uri configPath = getIntent().getParcelableExtra(EXTRA_CONFIG_PATH);
        Uri path = getIntent().getParcelableExtra(EXTRA_PATH);

        dataStore = new JsonPreferenceDataStore(getContentResolver(), configPath);

        OnApplyWindowInsetsListener listener = (v, insets) -> {
            int bottomInsets = insets.getInsets(WindowInsetsCompat.Type.navigationBars()).bottom;
            CoordinatorLayout.LayoutParams params = (CoordinatorLayout.LayoutParams) binding.addButton.getLayoutParams();
            params.bottomMargin += bottomInsets;
            binding.addButton.measure(View.MeasureSpec.UNSPECIFIED, View.MeasureSpec.UNSPECIFIED);
            binding.recyclerView.setPaddingRelative(0, 0, 0, params.bottomMargin + binding.addButton.getMeasuredHeight());
            ViewCompat.setOnApplyWindowInsetsListener(binding.getRoot(), null);
            return insets;
        };

        ViewCompat.setOnApplyWindowInsetsListener(binding.getRoot(), listener);

        Set<String> dictSet = Objects.requireNonNull(dataStore.getStringSet("dicts", new ArraySet<>()));
        new Thread(() -> {
            List<TranslateFile> files = dictSet.stream()
                    .map(item -> {
                        DocumentFile file = DocumentFile.fromSingleUri(this, Uri.parse(path.toString() + "%2F" + item.replaceAll("/", "%2F")));
                        if (file != null) return Map.of("origPath", item, "file", file);
                        return Map.of("origPath", item);
                    })
                    .map(map -> {
                        if (map.containsKey("file")) {
                            DocumentFile item = (DocumentFile) Objects.requireNonNull(map.get("file"));
                            if (item.exists()) {
                                try (InputStream is = getContentResolver().openInputStream(item.getUri())) {
                                    StringBuilder builder = new StringBuilder();
                                    BufferedReader reader = new BufferedReader(new InputStreamReader(is));

                                    String line;
                                    while ((line = reader.readLine()) != null) {
                                        builder.append(line);
                                    }
                                    JSONObject object = new JSONObject(builder.toString());
                                    boolean enabled = true;
                                    if (object.has("_enabled")) {
                                        enabled = object.getBoolean("_enabled");
                                    }

                                    String supportVersion = null;
                                    if (object.has("_supportVersion")) {
                                        supportVersion = object.getString("_supportVersion");
                                    }
                                    return new TranslateFile(item.getName(), item.getUri(), supportVersion, enabled, item.canWrite(), false);
                                } catch (IOException | JSONException e) {
                                    e.printStackTrace();
                                    return new TranslateFile(item.getName(), item.getUri(), null, false, false, true);
                                }
                            } else {
                                return new TranslateFile(Objects.requireNonNull(map.get("origPath")).toString(), null, null, false, false, true);
                            }
                        }
                        return null;
                    })
                    .filter(Objects::nonNull)
                    .collect(Collectors.toList());
            runOnUiThread(() -> {
                if (files.isEmpty()) {
                    binding.emptyLayout.setVisibility(View.VISIBLE);
                    binding.recyclerView.setVisibility(View.GONE);
                } else {
                    binding.emptyLayout.setVisibility(View.GONE);
                }
                binding.recyclerView.setAdapter(new TranslateAdapter(this, files, this::toggleFileEnabled, this::onItemRemoved));
                binding.progress.setVisibility(View.GONE);
            });
        }).start();

        binding.addButton.setOnClickListener(v ->
                openMultipleDocuments.launch(new String[]{
                        Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q ?
                                "application/json" :
                                "application/octet-stream"}));
    }

    private void toggleFileEnabled(Runnable setViewEnabled, Uri path, boolean enabled) {
        new Thread(() -> {
            try (InputStream is = getContentResolver().openInputStream(path)) {
                StringBuilder builder = new StringBuilder();
                BufferedReader reader = new BufferedReader(new InputStreamReader(is));

                String line;
                while ((line = reader.readLine()) != null) {
                    builder.append(line);
                }
                JSONObject object = new JSONObject(builder.toString());
                object.put("_enabled", enabled);
                try (OutputStream os = getContentResolver().openOutputStream(path, "wt")) {
                    Writer writer = new PrintWriter(os);
                    writer.write(object.toString(4) + "\n");
                    writer.flush();
                } catch (JSONException | IOException e) {
                    e.printStackTrace();
                }
            } catch (IOException | JSONException e) {
                e.printStackTrace();
            }
            runOnUiThread(setViewEnabled);
        }).start();
    }

    private void onItemRemoved(TranslateFile file) {
        Set<String> set = Objects.requireNonNull(dataStore.getStringSet("dicts", new ArraySet<>()));
        set.removeIf((item) -> file.path.getPath().contains(item));
        dataStore.putStringSet("dicts", set);
        if (set.isEmpty()) {
            binding.emptyLayout.setVisibility(View.VISIBLE);
            binding.recyclerView.setVisibility(View.GONE);
        }
    }

    private static class OpenMultipleDocuments extends ActivityResultContracts.OpenMultipleDocuments {
        private final String packageName;

        public OpenMultipleDocuments(String packageName) {
            this.packageName = packageName;
        }

        @NonNull
        @Override
        public Intent createIntent(@NonNull Context context, @NonNull String[] input) {
            Intent intent = super.createIntent(context, input);
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                intent.putExtra(DocumentsContract.EXTRA_INITIAL_URI, DocumentsContract.buildDocumentUri(
                        "com.android.externalstorage.documents",
                        "primary:Android/data/" + packageName));
            }
            return intent;
        }
    }

    private static class TranslateFile {
        public final String fileName;
        public final Uri path;
        public String supportVersion;
        public boolean enabled;
        public boolean canWrite;
        public boolean hasError;

        public TranslateFile(String fileName, Uri path, String supportVersion, boolean enabled, boolean canWrite, boolean hasError) {
            this.fileName = fileName;
            this.path = path;
            this.supportVersion = supportVersion;
            this.enabled = enabled;
            this.canWrite = canWrite;
            this.hasError = hasError;
        }
    }

    private static class TranslateAdapter extends RecyclerView.Adapter<TranslateViewHolder> {
        private final Context context;
        private final List<TranslateFile> files;

        private final OnCheckedChangeListener useTranslateCheckedChangeListener;

        private final OnItemRemovedListener onItemRemovedListener;

        public TranslateAdapter(
                @NonNull Context context,
                @NonNull List<TranslateFile> files,
                @NonNull OnCheckedChangeListener useTranslateCheckedChangeListener,
                @NonNull OnItemRemovedListener onItemRemovedListener
        ) {
            this.context = context;
            this.files = files;
            this.useTranslateCheckedChangeListener = useTranslateCheckedChangeListener;
            this.onItemRemovedListener = onItemRemovedListener;
        }

        public void add(TranslateFile file) {
            files.add(file);
            notifyItemInserted(files.size() - 1);
        }


        public void remove(int position) {
            if (position < 0) {
                return;
            }
            onItemRemovedListener.onItemRemoved(files.get(position));
            files.remove(position);
            notifyItemRemoved(position);
        }

        @NonNull
        @Override
        public TranslateViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
            return new TranslateViewHolder(LayoutInflater.from(parent.getContext()).inflate(R.layout.translate_item, parent, false));
        }

        @Override
        public void onBindViewHolder(@NonNull TranslateViewHolder holder, int position) {
            TranslateFile file = files.get(position);
            holder.binding.fileName.setText(file.fileName);
            String supportedVersion = file.supportVersion;
            holder.binding.supportVersion.setVisibility(supportedVersion == null ? View.GONE : View.VISIBLE);
            holder.binding.supportVersion.setText(Objects.requireNonNullElse(context.getString(R.string.support_version, supportedVersion), ""));
            if (file.hasError) {
                holder.binding.errorIcon.setVisibility(View.VISIBLE);
                holder.binding.useTranslate.setVisibility(View.GONE);
                holder.binding.supportVersion.setVisibility(View.VISIBLE);
                holder.binding.supportVersion.setText(context.getString(R.string.error_occurred));
            } else {
                holder.binding.errorIcon.setVisibility(View.GONE);
                holder.binding.useTranslate.setVisibility(View.VISIBLE);
                holder.binding.useTranslate.setChecked(file.enabled);
                holder.binding.useTranslate.setOnCheckedChangeListener((buttonView, isChecked) -> {
                    holder.itemView.setEnabled(false);
                    holder.binding.fileName.setEnabled(false);
                    holder.binding.supportVersion.setEnabled(false);
                    holder.binding.removeButton.setEnabled(false);
                    holder.binding.useTranslate.setEnabled(false);
                    useTranslateCheckedChangeListener.onCheckedChangeListener(() -> {
                        holder.itemView.setEnabled(true);
                        holder.binding.fileName.setEnabled(true);
                        holder.binding.supportVersion.setEnabled(true);
                        holder.binding.removeButton.setEnabled(true);
                        holder.binding.useTranslate.setEnabled(true);
                    }, file.path, isChecked);
                });
            }
            holder.binding.removeButton.setEnabled(true);
            holder.binding.removeButton.setOnClickListener(v -> {
                v.setEnabled(false);
                remove(files.indexOf(file));
            });
            holder.itemView.setOnClickListener(v -> holder.binding.useTranslate.toggle());
            if (!file.canWrite) {
                holder.itemView.setEnabled(false);
                holder.binding.useTranslate.setEnabled(false);
            }
        }

        @Override
        public int getItemCount() {
            return files.size();
        }

        public interface OnCheckedChangeListener {
            void onCheckedChangeListener(Runnable setViewEnabled, Uri path, boolean isChecked);
        }

        public interface OnItemRemovedListener {
            void onItemRemoved(TranslateFile removed);
        }
    }

    private static class TranslateViewHolder extends RecyclerView.ViewHolder {
        public TranslateItemBinding binding;

        public TranslateViewHolder(@NonNull View itemView) {
            super(itemView);
            binding = DataBindingUtil.bind(itemView);
        }
    }
}
