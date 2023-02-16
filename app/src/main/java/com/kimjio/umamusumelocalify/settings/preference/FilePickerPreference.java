package com.kimjio.umamusumelocalify.settings.preference;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.res.TypedArray;
import android.net.Uri;
import android.os.Build;
import android.os.Parcel;
import android.os.Parcelable;
import android.provider.DocumentsContract;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.preference.Preference;

import com.kimjio.umamusumelocalify.settings.Constants;
import com.kimjio.umamusumelocalify.settings.R;
import com.kimjio.umamusumelocalify.settings.activity.MainActivity;

public class FilePickerPreference extends Preference implements IActivityResultPreference {
    private static final String MIME_OCTET_STREAM = "application/octet-stream";

    private String mText;

    private String mMimeType;

    public FilePickerPreference(@NonNull Context context, @Nullable AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
        setSummaryProvider(SimpleSummaryProvider.getInstance());
        final TypedArray a = context.obtainStyledAttributes(attrs, R.styleable.FilePickerPreference, defStyleAttr, defStyleRes);
        mMimeType = a.getString(R.styleable.FilePickerPreference_android_mimeType);
        if (mMimeType == null) {
            mMimeType = MIME_OCTET_STREAM;
        }
        a.recycle();
    }

    public FilePickerPreference(@NonNull Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        this(context, attrs, defStyleAttr, 0);
    }

    public FilePickerPreference(@NonNull Context context, @Nullable AttributeSet attrs) {
        this(context, attrs, R.attr.preferenceStyle);
    }

    public FilePickerPreference(@NonNull Context context) {
        this(context, null);
    }

    @Override
    protected void onClick() {
        Activity activity = getActivity();
        if (activity != null) {
            Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT)
                    .setType(
                            Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q ?
                                    mMimeType :
                                    "application/octet-stream");
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                String currentPackageName = getContext().getSharedPreferences(Constants.PREF_NAME, Context.MODE_PRIVATE)
                        .getString(MainActivity.KEY_LAST_SELECTED_PACKAGE, null);
                intent.putExtra(DocumentsContract.EXTRA_INITIAL_URI, DocumentsContract.buildDocumentUri(
                        "com.android.externalstorage.documents",
                        "primary:Android/data/" + currentPackageName));
            }
            activity.startActivityForResult(intent, Math.abs(getKey().hashCode()));
        }
    }

    @Nullable
    public String getText() {
        return mText;
    }

    public void setText(@Nullable String text) {
        final boolean wasBlocking = shouldDisableDependents();

        mText = text;

        persistString(text);

        final boolean isBlocking = shouldDisableDependents();
        if (isBlocking != wasBlocking) {
            notifyDependencyChange(isBlocking);
        }

        notifyChanged();
    }

    @Override
    protected Object onGetDefaultValue(@NonNull TypedArray a, int index) {
        return a.getString(index);
    }

    @Override
    protected void onSetInitialValue(Object defaultValue) {
        setText(getPersistedString((String) defaultValue));
    }

    @Override
    public boolean shouldDisableDependents() {
        return TextUtils.isEmpty(mText) || super.shouldDisableDependents();
    }

    @Nullable
    @Override
    protected Parcelable onSaveInstanceState() {
        final Parcelable superState = super.onSaveInstanceState();
        if (isPersistent()) {
            return superState;
        }

        final SavedState myState = new SavedState(superState);
        myState.mText = getText();
        return myState;
    }

    @Override
    protected void onRestoreInstanceState(@Nullable Parcelable state) {
        if (state == null || !state.getClass().equals(SavedState.class)) {
            super.onRestoreInstanceState(state);
            return;
        }

        SavedState myState = (SavedState) state;
        super.onRestoreInstanceState(myState.getSuperState());
        setText(myState.mText);
    }

    public void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        if (requestCode == Math.abs(getKey().hashCode()) &&
                resultCode == Activity.RESULT_OK &&
                data != null) {
            Uri path = data.getData();
            String currentPackageName = getContext().getSharedPreferences(Constants.PREF_NAME, Context.MODE_PRIVATE)
                    .getString(MainActivity.KEY_LAST_SELECTED_PACKAGE, null);
            if (!path.getPath().contains("Android/data/" + currentPackageName)) {
                Toast.makeText(getContext(), R.string.error_file_not_in_data_folder, Toast.LENGTH_SHORT).show();
                return;
            }
            if (path.getPath().contains("config.json")) {
                Toast.makeText(getContext(), R.string.error_is_config, Toast.LENGTH_SHORT).show();
                return;
            }
            String[] split = path.getPath().split(currentPackageName + "/");
            setText(split[split.length - 1]);
        }
    }

    private static class SavedState extends BaseSavedState {
        public static final Creator<SavedState> CREATOR =
                new Creator<>() {
                    @Override
                    public FilePickerPreference.SavedState createFromParcel(Parcel in) {
                        return new FilePickerPreference.SavedState(in);
                    }

                    @Override
                    public FilePickerPreference.SavedState[] newArray(int size) {
                        return new FilePickerPreference.SavedState[size];
                    }
                };

        String mText;

        SavedState(Parcel source) {
            super(source);
            mText = source.readString();
        }

        SavedState(Parcelable superState) {
            super(superState);
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            super.writeToParcel(dest, flags);
            dest.writeString(mText);
        }
    }

    public static final class SimpleSummaryProvider implements SummaryProvider<FilePickerPreference> {

        private static SimpleSummaryProvider sSimpleSummaryProvider;

        private SimpleSummaryProvider() {
        }

        @NonNull
        public static FilePickerPreference.SimpleSummaryProvider getInstance() {
            if (sSimpleSummaryProvider == null) {
                sSimpleSummaryProvider = new FilePickerPreference.SimpleSummaryProvider();
            }
            return sSimpleSummaryProvider;
        }

        @SuppressLint("PrivateResource")
        @Nullable
        @Override
        public CharSequence provideSummary(@NonNull FilePickerPreference preference) {
            if (TextUtils.isEmpty(preference.getText())) {
                return (preference.getContext().getString(R.string.not_set));
            } else {
                return preference.getText();
            }
        }
    }
}
