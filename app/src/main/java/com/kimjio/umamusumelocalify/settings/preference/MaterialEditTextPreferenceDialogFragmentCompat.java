package com.kimjio.umamusumelocalify.settings.preference;

import android.annotation.SuppressLint;
import android.app.Dialog;
import android.content.DialogInterface;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.view.WindowManager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.preference.DialogPreference;
import androidx.preference.EditTextPreferenceDialogFragmentCompat;
import androidx.preference.PreferenceDialogFragmentCompat;

import com.google.android.material.dialog.MaterialAlertDialogBuilder;
import com.kimjio.umamusumelocalify.settings.R;

import java.lang.reflect.Field;
import java.lang.reflect.Method;

public class MaterialEditTextPreferenceDialogFragmentCompat extends EditTextPreferenceDialogFragmentCompat {

    @NonNull
    public static MaterialEditTextPreferenceDialogFragmentCompat newInstance(String key) {
        final MaterialEditTextPreferenceDialogFragmentCompat
                fragment = new MaterialEditTextPreferenceDialogFragmentCompat();
        final Bundle b = new Bundle(1);
        b.putString(ARG_KEY, key);
        fragment.setArguments(b);
        return fragment;
    }

    @SuppressLint("RestrictedApi")
    @NonNull
    @Override
    public Dialog onCreateDialog(@Nullable Bundle savedInstanceState) {
        CharSequence dialogTitle;
        CharSequence positiveButtonText;
        CharSequence negativeButtonText;
        BitmapDrawable dialogIcon = null;

        final String key = requireArguments().getString(ARG_KEY);
        if (savedInstanceState == null) {
            DialogPreference preference = getPreference();
            dialogTitle = preference.getDialogTitle();
            positiveButtonText = preference.getPositiveButtonText();
            negativeButtonText = preference.getNegativeButtonText();

            final Drawable icon = preference.getDialogIcon();
            if (icon == null || icon instanceof BitmapDrawable) {
                dialogIcon = (BitmapDrawable) icon;
            } else {
                final Bitmap bitmap = Bitmap.createBitmap(icon.getIntrinsicWidth(),
                        icon.getIntrinsicHeight(), Bitmap.Config.ARGB_8888);
                final Canvas canvas = new Canvas(bitmap);
                icon.setBounds(0, 0, canvas.getWidth(), canvas.getHeight());
                icon.draw(canvas);
                dialogIcon = new BitmapDrawable(getResources(), bitmap);
            }
        } else {
            dialogTitle = savedInstanceState.getCharSequence("PreferenceDialogFragment.title");
            positiveButtonText = savedInstanceState.getCharSequence("PreferenceDialogFragment.positiveText");
            negativeButtonText = savedInstanceState.getCharSequence("PreferenceDialogFragment.negativeText");
            final Bitmap bitmap = savedInstanceState.getParcelable("PreferenceDialogFragment.icon");
            if (bitmap != null) {
                dialogIcon = new BitmapDrawable(getResources(), bitmap);
            }
        }

        try {
            Field mWhichButtonClickedField = PreferenceDialogFragmentCompat.class.getDeclaredField("mWhichButtonClicked");
            mWhichButtonClickedField.setAccessible(true);
            mWhichButtonClickedField.setInt(this, DialogInterface.BUTTON_NEGATIVE);
        } catch (Exception e) {
            e.printStackTrace();
        }

        final MaterialAlertDialogBuilder builder = new MaterialAlertDialogBuilder(requireContext(), R.style.ThemeOverlay_Material3_Dialog_Centered)
                .setTitle(dialogTitle)
                .setIcon(dialogIcon)
                .setPositiveButton(positiveButtonText, this)
                .setNegativeButton(negativeButtonText, this);

        View contentView = onCreateDialogView(requireContext());
        if (contentView != null) {
            onBindDialogView(contentView);
            builder.setView(contentView);
        }

        onPrepareDialogBuilder(builder);

        // Create the dialog
        final Dialog dialog = builder.create();
        if (needInputMethod()) {
            try {
                Method method = PreferenceDialogFragmentCompat.class.getDeclaredMethod("requestInputMethod", Dialog.class);
                method.setAccessible(true);
                method.invoke(this, dialog);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        float density = getResources().getDisplayMetrics().density;

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            dialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_BLUR_BEHIND);
            dialog.getWindow().getAttributes().setBlurBehindRadius((int) (density * 16.0f));
        }

        return dialog;
    }
}
