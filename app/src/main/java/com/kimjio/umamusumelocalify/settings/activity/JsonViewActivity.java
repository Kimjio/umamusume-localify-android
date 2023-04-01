package com.kimjio.umamusumelocalify.settings.activity;

import android.app.Dialog;
import android.content.res.Configuration;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.LayerDrawable;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.WindowManager;
import android.widget.Toast;

import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.annotation.IntRange;
import androidx.annotation.NonNull;
import androidx.core.view.OnApplyWindowInsetsListener;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.documentfile.provider.DocumentFile;
import androidx.lifecycle.ViewModel;
import androidx.lifecycle.ViewModelProvider;
import androidx.recyclerview.widget.RecyclerView;

import com.fasterxml.jackson.core.type.TypeReference;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.google.android.material.dialog.MaterialAlertDialogBuilder;
import com.google.android.material.progressindicator.CircularProgressIndicatorSpec;
import com.google.android.material.progressindicator.DeterminateDrawable;
import com.google.android.material.progressindicator.IndeterminateDrawable;
import com.kimjio.umamusumelocalify.settings.R;
import com.kimjio.umamusumelocalify.settings.databinding.JsonViewActivityBinding;

import org.json.JSONException;
import org.json.JSONObject;
import org.msgpack.jackson.dataformat.MessagePackMapper;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Map;
import java.util.Optional;
import java.util.concurrent.atomic.AtomicInteger;

public class JsonViewActivity extends BaseActivity<JsonViewActivityBinding> {
    public static final String EXTRA_DATETIME = "datetime";
    public static final String EXTRA_TYPE = "type";
    public static final String EXTRA_POSITION = "position";

    private DocumentFile documentFile;

    private MenuItem saveMenu;

    private Thread saveThread;

    private final ActivityResultLauncher<String> saveMsgPack = registerForActivityResult(new ActivityResultContracts.CreateDocument("application/octet-stream"), result -> {
        if (result != null) {
            CircularProgressIndicatorSpec spec = new CircularProgressIndicatorSpec(this, null, 0, R.style.Widget_Material3_CircularProgressIndicator_ExtraSmall);
            DeterminateDrawable<CircularProgressIndicatorSpec> drawable = DeterminateDrawable.createCircularDrawable(this, spec);
            drawable.setVisible(true, true, true);
            if (saveMenu != null) {
                float density = getResources().getDisplayMetrics().density;

                LayerDrawable layerDrawable = new LayerDrawable(new Drawable[]{saveMenu.getIcon(), drawable});
                int drawableSize = (int) (24f * density);
                layerDrawable.setLayerSize(0, drawableSize, drawableSize);
                layerDrawable.setLayerSize(1, drawableSize, drawableSize);
                saveMenu.setIcon(layerDrawable);
                saveMenu.setEnabled(false);
            }
            saveThread = new Thread(() -> {
                try (InputStream is = getContentResolver().openInputStream(documentFile.getUri())) {
                    long size = documentFile.length();
                    try (OutputStream os = getContentResolver().openOutputStream(result, "wt")) {
                        final AtomicInteger i = new AtomicInteger(0);
                        byte[] buf = new byte[1024];
                        int length;
                        while ((length = is.read(buf)) > 0) {
                            if (Thread.interrupted()) {
                                os.flush();
                                return;
                            }
                            os.write(buf);
                            i.addAndGet(length);
                            if (!isDestroyed() && !isFinishing()) {
                                runOnUiThread(() -> drawable.setLevel(((int) Math.floor(i.floatValue() * 100 / size)) * 100));
                            }
                        }
                        os.flush();
                        if (!isDestroyed() && !isFinishing()) {
                            runOnUiThread(() -> {
                                Toast.makeText(this, R.string.save_ok, Toast.LENGTH_SHORT).show();
                                drawable.hideNow();
                                saveMenu.setEnabled(true);
                                saveMenu.setIcon(((LayerDrawable) saveMenu.getIcon()).getDrawable(0));
                            });
                        }
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                }
            });
            saveThread.start();
        }
    });

    private JsonViewModel viewModel;

    private final ActivityResultLauncher<String> saveJson = registerForActivityResult(new ActivityResultContracts.CreateDocument("application/json"), result -> {
        if (result != null) {
            CircularProgressIndicatorSpec spec = new CircularProgressIndicatorSpec(this, null, 0, R.style.Widget_Material3_CircularProgressIndicator_ExtraSmall);
            IndeterminateDrawable<CircularProgressIndicatorSpec> drawable = IndeterminateDrawable.createCircularDrawable(this, spec);
            drawable.setVisible(true, true, true);
            if (saveMenu != null) {
                float density = getResources().getDisplayMetrics().density;

                int drawableSize = (int) (24f * density);
                LayerDrawable layerDrawable = new LayerDrawable(new Drawable[]{saveMenu.getIcon(), drawable});
                layerDrawable.setLayerSize(0, drawableSize, drawableSize);
                layerDrawable.setLayerSize(1, drawableSize, drawableSize);
                saveMenu.setIcon(layerDrawable);
                saveMenu.setEnabled(false);
            }
            saveThread = new Thread(() -> {
                try (OutputStream os = getContentResolver().openOutputStream(result, "wt")) {
                    String json = viewModel.jsonObject.toString(4) + "\n";
                    os.write(json.getBytes());
                    os.flush();
                    if (!isDestroyed() && !isFinishing()) {
                        runOnUiThread(() -> {
                            Toast.makeText(this, R.string.save_ok, Toast.LENGTH_SHORT).show();
                            drawable.hideNow();
                            saveMenu.setEnabled(true);
                            saveMenu.setIcon(((LayerDrawable) saveMenu.getIcon()).getDrawable(0));
                        });
                    }
                } catch (JSONException | IOException e) {
                    e.printStackTrace();
                }
            });
            saveThread.start();
        }
    });

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setSupportActionBar(binding.appBar);
        requireSupportActionBar().setDisplayHomeAsUpEnabled(true);

        binding.appBarLayout.setBackgroundColor(backgroundColor);

        if (!getIntent().hasExtra(EXTRA_PATH)) {
            finishAfterTransition();
            return;
        }

        if (getIntent().hasExtra(EXTRA_DATETIME)) {
            setTitle(getIntent().getStringExtra(EXTRA_DATETIME));
        }

        if (getIntent().hasExtra(EXTRA_TYPE)) {
            requireSupportActionBar().setSubtitle(getIntent().getIntExtra(EXTRA_TYPE, 0) == 0 ? R.string.request : R.string.response);
        }

        viewModel = new ViewModelProvider(this).get(JsonViewModel.class);

        OnApplyWindowInsetsListener listener = (v, insets) -> {
            int bottomInsets = insets.getInsets(WindowInsetsCompat.Type.navigationBars()).bottom;
            binding.json.setPaddingRelative(0, 0, 0, bottomInsets);
            ViewCompat.setOnApplyWindowInsetsListener(binding.getRoot(), null);
            return insets;
        };

        ViewCompat.setOnApplyWindowInsetsListener(binding.getRoot(), listener);

        binding.json.setEnableSaveInstanceState(false);

        if (viewModel.adapter != null) {
            binding.json.setAdapter(viewModel.adapter);
        } else {
            Uri path = getIntent().getParcelableExtra(EXTRA_PATH);
            documentFile = DocumentFile.fromSingleUri(this, path);
            if (documentFile != null && documentFile.exists()) {
                new Thread(() -> {
                    try (InputStream is = getContentResolver().openInputStream(documentFile.getUri())) {
                        ObjectMapper mapper = new MessagePackMapper();
                        Map<String, Object> unpacked = mapper.readValue(is, new TypeReference<>() {
                        });
                        viewModel.jsonObject = new JSONObject(unpacked);
                        runOnUiThread(() -> {
                            boolean isNight = (getResources().getConfiguration().uiMode & Configuration.UI_MODE_NIGHT_MASK) == Configuration.UI_MODE_NIGHT_YES;
                            binding.json.setJSONObject(viewModel.jsonObject);
                            viewModel.adapter = binding.json.getAdapter();
                        });
                    } catch (IOException e) {
                        e.printStackTrace();
                        runOnUiThread(this::finishAfterTransition);
                    }
                }).start();
            } else {
                finishAfterTransition();
            }
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (saveThread != null && saveThread.isAlive()) {
            saveThread.interrupt();
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.menu_json, menu);
        saveMenu = menu.findItem(R.id.menu_save);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(@NonNull MenuItem item) {
        if (item.getItemId() == R.id.menu_expand_all) {
            binding.json.expandAll();
            return true;
        }
        if (item.getItemId() == R.id.menu_collapse_all) {
            binding.json.collapseAll();
            return true;
        }
        if (item.getItemId() == R.id.menu_save) {
            Dialog dialog = new MaterialAlertDialogBuilder(this, R.style.ThemeOverlay_Material3_Dialog_Centered).setIcon(R.drawable.ic_save).setTitle(R.string.save_format_select).setItems(R.array.save_formats, (d, which) -> {
                saveFile(which);
            }).create();
            float density = getResources().getDisplayMetrics().density;
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                dialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_BLUR_BEHIND);
                dialog.getWindow().getAttributes().setBlurBehindRadius((int) (density * 16.0f));
            }
            dialog.show();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    private void saveFile(@IntRange(from = 0, to = 1) int type) {
        switch (type) {
            case 0:
                saveJson.launch(Optional.ofNullable(documentFile.getName()).orElse("").replace(".msgpack", ".json"));
                break;
            case 1:
                saveMsgPack.launch(documentFile.getName());
                break;
        }
    }

    public static class JsonViewModel extends ViewModel {
        private JSONObject jsonObject;

        private RecyclerView.Adapter<?> adapter;
    }
}
