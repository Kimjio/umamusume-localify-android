package com.kimjio.umamusumelocalify.settings.preference;

import android.content.ContentResolver;
import android.net.Uri;
import android.util.ArraySet;

import androidx.annotation.Nullable;
import androidx.preference.PreferenceDataStore;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.io.Writer;
import java.util.Set;

public class JsonPreferenceDataStore extends PreferenceDataStore {
    private final ContentResolver contentResolver;

    private Uri path;

    private JSONObject jsonObject;

    public JsonPreferenceDataStore(ContentResolver contentResolver, Uri path) {
        super();
        this.contentResolver = contentResolver;
        this.path = path;
        init();
    }

    private void init() {
        try (InputStream is = contentResolver.openInputStream(path)) {
            StringBuilder builder = new StringBuilder();
            BufferedReader reader = new BufferedReader(new InputStreamReader(is));

            String line;
            while ((line = reader.readLine()) != null) {
                builder.append(line);
            }
            jsonObject = new JSONObject(builder.toString());
        } catch (IOException | JSONException e) {
            e.printStackTrace();
            jsonObject = new JSONObject();
        }
    }

    private void writeJson() {
        try (OutputStream os = contentResolver.openOutputStream(path, "wt")) {
            Writer writer = new PrintWriter(os);
            writer.write(jsonObject.toString(4) + "\n");
            writer.flush();
        } catch (JSONException | IOException e) {
            e.printStackTrace();
        }
    }

    public void setPath(Uri path) {
        this.path = path;
        this.init();
    }

    @Override
    public void putString(String key, @Nullable String value) {
        try {
            jsonObject.put(key, value);
            writeJson();
        } catch (JSONException e) {
            throw new RuntimeException(e);
        }
    }

    @Override
    public void putStringSet(String key, @Nullable Set<String> values) {
        try {
            if (values != null) {
                jsonObject.put(key, new JSONArray(values.toArray(new String[]{})));
            } else {
                jsonObject.put(key, null);
            }
            writeJson();
        } catch (JSONException e) {
            throw new RuntimeException(e);
        }
    }

    @Override
    public void putInt(String key, int value) {
        try {
            jsonObject.put(key, value);
            writeJson();
        } catch (JSONException e) {
            throw new RuntimeException(e);
        }
    }

    @Override
    public void putLong(String key, long value) {
        try {
            jsonObject.put(key, value);
            writeJson();
        } catch (JSONException e) {
            throw new RuntimeException(e);
        }
    }

    @Override
    public void putFloat(String key, float value) {
        try {
            jsonObject.put(key, Double.parseDouble(String.valueOf(value)));
            writeJson();
        } catch (JSONException e) {
            throw new RuntimeException(e);
        }
    }

    @Override
    public void putBoolean(String key, boolean value) {
        try {
            jsonObject.put(key, value);
            writeJson();
        } catch (JSONException e) {
            throw new RuntimeException(e);
        }
    }

    @Nullable
    @Override
    public String getString(String key, @Nullable String defValue) {
        try {
            return jsonObject.getString(key);
        } catch (JSONException ignore) {
            return defValue;
        }
    }

    @Nullable
    @Override
    public Set<String> getStringSet(String key, @Nullable Set<String> defValues) {
        try {
            Set<String> set = new ArraySet<>();
            JSONArray array = jsonObject.getJSONArray(key);
            for (int i = 0; i < array.length(); i++) {
                set.add(array.getString(i));
            }
            return set;
        } catch (JSONException e) {
            return defValues;
        }
    }

    @Override
    public int getInt(String key, int defValue) {
        try {
            return jsonObject.getInt(key);
        } catch (JSONException e) {
            return defValue;
        }
    }

    @Override
    public long getLong(String key, long defValue) {
        try {
            return jsonObject.getLong(key);
        } catch (JSONException e) {
            return defValue;
        }
    }

    @Override
    public float getFloat(String key, float defValue) {
        try {
            return (float) jsonObject.getDouble(key);
        } catch (JSONException e) {
            return defValue;
        }
    }

    @Override
    public boolean getBoolean(String key, boolean defValue) {
        try {
            return jsonObject.getBoolean(key);
        } catch (JSONException e) {
            return defValue;
        }
    }
}
