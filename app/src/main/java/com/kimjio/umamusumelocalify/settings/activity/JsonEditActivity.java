package com.kimjio.umamusumelocalify.settings.activity;

import android.content.res.Configuration;
import android.net.Uri;
import android.os.Bundle;
import android.text.Editable;
import android.text.method.KeyListener;
import android.util.TypedValue;
import android.view.ActionMode;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.core.view.OnApplyWindowInsetsListener;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.documentfile.provider.DocumentFile;

import com.blacksquircle.ui.editorkit.plugin.delimiters.BracketsHighlightPlugin;
import com.blacksquircle.ui.editorkit.plugin.linenumbers.LineNumbersPlugin;
import com.blacksquircle.ui.editorkit.plugin.pinchzoom.PinchZoomPlugin;
import com.blacksquircle.ui.editorkit.plugin.textscroller.TextScrollerPlugin;
import com.blacksquircle.ui.editorkit.utils.EditorTheme;
import com.blacksquircle.ui.language.json.JsonLanguage;
import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.kimjio.umamusumelocalify.settings.R;
import com.kimjio.umamusumelocalify.settings.databinding.JsonEditActivityBinding;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.io.Writer;

public class JsonEditActivity extends BaseActivity<JsonEditActivityBinding> {

    public static final int RESULT_SAVED = 1;

    private Uri documentPath;

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

        OnApplyWindowInsetsListener listener = (v, insets) -> {
            int bottomInsets = insets.getInsets(WindowInsetsCompat.Type.navigationBars()).bottom;
            binding.code.setPaddingRelative(0, 0, 0, bottomInsets);
            binding.scroller.setPaddingRelative(0, 0, 0, bottomInsets);

            ViewCompat.setOnApplyWindowInsetsListener(binding.getRoot(), null);
            return insets;
        };

        ViewCompat.setOnApplyWindowInsetsListener(binding.getRoot(), listener);


        Uri path = getIntent().getParcelableExtra(EXTRA_PATH);
        DocumentFile documentFile = DocumentFile.fromSingleUri(this, path);
        if (documentFile != null && documentFile.exists()) {
            setTitle(documentFile.getName());
            new Thread(() -> {
                documentPath = documentFile.getUri();
                try (InputStream is = getContentResolver().openInputStream(documentPath)) {
                    StringBuilder builder = new StringBuilder();
                    BufferedReader reader = new BufferedReader(new InputStreamReader(is));

                    String line;
                    while ((line = reader.readLine()) != null) {
                        builder.append(line);
                    }
                    JSONObject jsonObject = new JSONObject(builder.toString());
                    runOnUiThread(() -> {
                        try {
                            boolean isNight = (getResources().getConfiguration().uiMode & Configuration.UI_MODE_NIGHT_MASK) == Configuration.UI_MODE_NIGHT_YES;

                            binding.code.setColorScheme(isNight ? EditorTheme.INSTANCE.getDARCULA() : EditorTheme.INSTANCE.getINTELLIJ_LIGHT());
                            binding.code.setBackgroundColor(binding.code.getColorScheme().getBackgroundColor());
                            binding.code.installPlugin(new PinchZoomPlugin());
                            binding.code.installPlugin(new BracketsHighlightPlugin());
                            binding.scroller.attachTo(binding.code);
                            TextScrollerPlugin textScrollerPlugin = new TextScrollerPlugin();
                            textScrollerPlugin.setScroller(binding.scroller);
                            binding.code.installPlugin(textScrollerPlugin);
                            binding.code.setTextSize(TypedValue.COMPLEX_UNIT_SP, 14);
                            LineNumbersPlugin lineNumbersPlugin = new LineNumbersPlugin();
                            binding.code.installPlugin(lineNumbersPlugin);
                            lineNumbersPlugin.setTextSize(14);
                            binding.code.setLanguage(new JsonLanguage());
                            binding.code.setTextContent(jsonObject.toString(4));

                            if (!documentFile.canWrite()) {
                                binding.code.setShowSoftInputOnFocus(false);
                                binding.code.setSoftKeyboard(false);
                                binding.code.setCursorVisible(false);
                                binding.code.setTextIsSelectable(true);
                                binding.code.setKeyListener(new KeyListener() {
                                    @Override
                                    public int getInputType() {
                                        return 0;
                                    }

                                    @Override
                                    public boolean onKeyDown(View view, Editable text, int keyCode, KeyEvent event) {
                                        return true;
                                    }

                                    @Override
                                    public boolean onKeyUp(View view, Editable text, int keyCode, KeyEvent event) {
                                        return true;
                                    }

                                    @Override
                                    public boolean onKeyOther(View view, Editable text, KeyEvent event) {
                                        return true;
                                    }

                                    @Override
                                    public void clearMetaKeyState(View view, Editable content, int states) {
                                    }
                                });
                                ActionMode.Callback callback = new ActionMode.Callback() {
                                    @Override
                                    public boolean onCreateActionMode(ActionMode mode, Menu menu) {
                                        return true;
                                    }

                                    @Override
                                    public boolean onPrepareActionMode(ActionMode mode, Menu menu) {
                                        if (menu != null) {
                                            menu.removeItem(android.R.id.cut);
                                            menu.removeItem(android.R.id.paste);
                                            menu.removeItem(android.R.id.pasteAsPlainText);
                                        }
                                        return true;
                                    }

                                    @Override
                                    public boolean onActionItemClicked(ActionMode mode, MenuItem item) {
                                        return false;
                                    }

                                    @Override
                                    public void onDestroyActionMode(ActionMode mode) {
                                    }
                                };
                                binding.code.setCustomInsertionActionModeCallback(callback);
                                binding.code.setCustomSelectionActionModeCallback(callback);
                            }
                        } catch (JSONException e) {
                            e.printStackTrace();
                        }
                    });
                } catch (JSONException | IOException e) {
                    e.printStackTrace();
                    runOnUiThread(this::finishAfterTransition);
                }
            }).start();
        } else {
            finishAfterTransition();
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.menu_json_edit, menu);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(@NonNull MenuItem item) {
        if (item.getItemId() == R.id.menu_save) {
            saveFIle();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    private void saveFIle() {
        try (OutputStream os = getContentResolver().openOutputStream(documentPath, "wt")) {
            String text = binding.code.getText().toString();
            new ObjectMapper().readTree(text);
            Writer writer = new PrintWriter(os);
            writer.write(text);
            writer.flush();
            Toast.makeText(this, R.string.save_ok, Toast.LENGTH_SHORT).show();
            setResult(RESULT_SAVED);
        } catch (JsonProcessingException e) {
            Toast.makeText(this, e.getLocalizedMessage(), Toast.LENGTH_SHORT).show();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
