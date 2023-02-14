package com.kimjio.umamusumelocalify.settings.preference;

import android.content.Context;
import android.util.AttributeSet;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.preference.DropDownPreference;

import com.kimjio.umamusumelocalify.settings.R;

public class IntegerDropDownPreference extends DropDownPreference {
    public IntegerDropDownPreference(@NonNull Context context) {
        this(context, null);
    }

    public IntegerDropDownPreference(@NonNull Context context, @Nullable AttributeSet attrs) {
        this(context, attrs, R.attr.dropdownPreferenceStyle);
    }

    public IntegerDropDownPreference(@NonNull Context context, @Nullable AttributeSet attrs, int defStyle) {
        this(context, attrs, defStyle, 0);
    }

    public IntegerDropDownPreference(@NonNull Context context, @Nullable AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
    }

    @Override
    protected boolean persistString(String value) {
        if ("null".equals(value)) {
            super.persistString(null);
            return true;
        }
        int intValue = Integer.parseInt(value);
        return persistInt(intValue);
    }

    @Override
    protected String getPersistedString(String defaultReturnValue) {
        int intValue;

        if (defaultReturnValue != null) {
            int intDefaultReturnValue = Integer.parseInt(defaultReturnValue);
            intValue = getPersistedInt(intDefaultReturnValue);
        } else {
            if (getPersistedInt(0) == getPersistedInt(1)) {
                intValue = getPersistedInt(0);
            } else {
                return "null";
            }
        }

        return Integer.toString(intValue);
    }
}
