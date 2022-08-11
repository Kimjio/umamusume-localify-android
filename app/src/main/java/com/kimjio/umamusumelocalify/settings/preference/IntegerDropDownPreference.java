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
                throw new IllegalArgumentException("Cannot get an int without a default return value");
            }
        }

        return Integer.toString(intValue);
    }
}
