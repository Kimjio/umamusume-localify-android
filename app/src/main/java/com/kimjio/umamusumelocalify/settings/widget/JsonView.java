package com.kimjio.umamusumelocalify.settings.widget;

import android.animation.Animator;
import android.animation.ObjectAnimator;
import android.content.Context;
import android.content.res.ColorStateList;
import android.content.res.Configuration;
import android.os.Parcel;
import android.os.Parcelable;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.customview.view.AbsSavedState;
import androidx.databinding.DataBindingUtil;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.kimjio.umamusumelocalify.settings.R;
import com.kimjio.umamusumelocalify.settings.databinding.JsonItemBinding;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.stream.Collectors;
import java.util.stream.Stream;

public class JsonView extends RecyclerView {
    private boolean enableSaveInstanceState = true;

    public JsonView(@NonNull Context context) {
        this(context, null);
    }

    public JsonView(@NonNull Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
        init(context);
    }

    public JsonView(@NonNull Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init(context);
    }

    private void init(Context context) {
        setClipChildren(false);
        setLayoutManager(new LinearLayoutManager(context) {
            private int horizontalOffset = 0;

            @Override
            public boolean canScrollVertically() {
                return true;
            }

            @Override
            public boolean canScrollHorizontally() {
                return true;
            }

            @Override
            public int scrollHorizontallyBy(int dx, RecyclerView.Recycler recycler, RecyclerView.State state) {
                if (getChildCount() == 0) return 0;

                View maxWidthChild = getChildAt(0);
                if (maxWidthChild == null) {
                    return 0;
                }
                for (int i = 0; i < getChildCount(); i++) {
                    View child = getChildAt(i);
                    if (child != null && maxWidthChild.getMeasuredWidth() < child.getMeasuredWidth()) {
                        maxWidthChild = child;
                    }
                }
                int rowWidth = maxWidthChild.getMeasuredWidth();

                // if the row is narrower than the RecyclerView's width, don't scroll
                if (rowWidth <= getWidth()) {
                    if (horizontalOffset > 0) {
                        if (dx > 0) {
                            return 0;
                        }

                        horizontalOffset += dx;
                        if (horizontalOffset < 0) {
                            horizontalOffset = 0;
                            offsetChildrenHorizontal(horizontalOffset);
                        } else {
                            offsetChildrenHorizontal(-dx);
                        }
                        return dx;
                    }
                    return 0;
                }

                int remainingPxRight = rowWidth - getWidth() - horizontalOffset;
                // cut scrolling short if we're about to reach end of the row
                int scrolled = dx;
                if (dx > remainingPxRight) {
                    scrolled = remainingPxRight;
                } else if (dx < -horizontalOffset) {
                    return -horizontalOffset;
                }

                horizontalOffset += scrolled;
                offsetChildrenHorizontal(-scrolled);
                return scrolled;
            }

            @Override
            public void layoutDecorated(@NonNull View child, int left, int top, int right, int bottom) {
                super.layoutDecorated(child, left - horizontalOffset, top, right - horizontalOffset, bottom);
            }

            @Override
            public void layoutDecoratedWithMargins(@NonNull View child, int left, int top, int right, int bottom) {
                super.layoutDecoratedWithMargins(child, left - horizontalOffset, top, right - horizontalOffset, bottom);
            }
        });
    }

    public boolean isEnableSaveInstanceState() {
        return enableSaveInstanceState;
    }

    public void setEnableSaveInstanceState(boolean enableSaveInstanceState) {
        this.enableSaveInstanceState = enableSaveInstanceState;
    }

    public void setJSONObject(JSONObject object) {
        JsonAdapter jsonAdapter = (JsonAdapter) getAdapter();
        if (jsonAdapter == null) {
            jsonAdapter = new JsonAdapter();
            setAdapter(jsonAdapter);
        }
        jsonAdapter.setJSONObject(object);
        onAdapterInit();
    }

    public void setJSONArray(JSONArray array) {
        JsonAdapter jsonAdapter = (JsonAdapter) getAdapter();
        if (jsonAdapter == null) {
            jsonAdapter = new JsonAdapter();
            setAdapter(jsonAdapter);
        }
        jsonAdapter.setJSONArray(array);
        onAdapterInit();
    }

    public void expandAll() {
        if (getAdapter() instanceof JsonAdapter) {
            ((JsonAdapter) getAdapter()).expandAll();
        }
    }

    public void collapseAll() {
        if (getAdapter() instanceof JsonAdapter) {
            ((JsonAdapter) getAdapter()).collapseAll();
        }
    }

    @Override
    protected Parcelable onSaveInstanceState() {
        if (!enableSaveInstanceState) {
            return super.onSaveInstanceState();
        }
        SavedState state = new SavedState(super.onSaveInstanceState());
        if (getAdapter() instanceof JsonAdapter) {
            state.jsonItemList = ((JsonAdapter) getAdapter()).itemList;
        }
        return state;
    }

    @Override
    protected void onRestoreInstanceState(Parcelable state) {
        if (!(state instanceof SavedState)) {
            super.onRestoreInstanceState(state);
            return;
        }

        if (!((SavedState) state).enableSaveInstanceState) {
            super.onRestoreInstanceState(state);
            return;
        }

        if (getAdapter() == null) {
            setAdapter(new JsonAdapter());
        }

        if (getAdapter() instanceof JsonAdapter) {
            JsonAdapter adapter = (JsonAdapter) getAdapter();
            adapter.itemList.addAll(((SavedState) state).jsonItemList);
            adapter.notifyItemInserted(adapter.itemList.size());
        }
        super.onRestoreInstanceState(((SavedState) state).getSuperState());
    }

    private void onAdapterInit() {
        getViewTreeObserver().addOnPreDrawListener(new ViewTreeObserver.OnPreDrawListener() {
            @Override
            public boolean onPreDraw() {
                getViewTreeObserver().removeOnPreDrawListener(this);

                Animator animator = ObjectAnimator.ofFloat(JsonView.this, "alpha", 0, 1f);
                animator.setDuration(500L);
                animator.start();

                return true;
            }
        });

    }

    private static class SavedState extends AbsSavedState {
        public static final Creator<SavedState> CREATOR = new Creator<>() {
            @Override
            public SavedState createFromParcel(Parcel in) {
                return new SavedState(in);
            }

            @Override
            public SavedState[] newArray(int size) {
                return new SavedState[size];
            }
        };

        boolean enableSaveInstanceState;

        List<JsonItem<?>> jsonItemList = new ArrayList<>();

        protected SavedState(@NonNull Parcelable superState) {
            super(superState);
        }

        protected SavedState(@NonNull Parcel in) {
            super(in);
            enableSaveInstanceState = in.readInt() == 1;
            in.readTypedList(jsonItemList, JsonItem.CREATOR);
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            super.writeToParcel(dest, flags);
            dest.writeInt(enableSaveInstanceState ? 1 : 0);
            dest.writeTypedList(jsonItemList);
        }
    }

    private static class JsonItem<T> implements Parcelable {
        public static final Creator<JsonItem<?>> CREATOR = new Creator<>() {
            @Override
            public JsonItem<?> createFromParcel(Parcel in) {
                String type = in.readString();
                if ("object".equals(type)) {
                    return new JsonObjectItem(in);
                }
                if ("array".equals(type)) {
                    return new JsonArrayItem(in);
                }
                return new JsonItem<>(in);
            }

            @Override
            public JsonItem<?>[] newArray(int size) {
                return new JsonItem<?>[size];
            }
        };
        final int step;
        final String name;
        final T value;
        final NameType nameType;

        public JsonItem(int step, String name, T value, NameType nameType) {
            this.step = step;
            this.name = name;
            if (value == JSONObject.NULL) {
                this.value = null;
            } else {
                this.value = value;
            }
            this.nameType = nameType;
        }

        @SuppressWarnings("unchecked")
        protected JsonItem(Parcel in) {
            step = in.readInt();
            name = in.readString();
            T t = (T) in.readValue(getClass().getClassLoader());
            if (t instanceof NullValue) {
                value = null;
            } else {
                value = t;
            }
            nameType = (NameType) in.readValue(getClass().getClassLoader());
        }

        @Override
        public int describeContents() {
            return 0;
        }

        @Override
        public void writeToParcel(@NonNull Parcel dest, int flags) {
            dest.writeString(getType());
            dest.writeInt(step);
            dest.writeString(name);
            dest.writeValue(Objects.requireNonNullElseGet(value, NullValue::new));
            dest.writeValue(nameType);
        }

        protected String getType() {
            return "value";
        }

        public boolean isExpandable() {
            return false;
        }

        enum NameType {
            OBJECT_KEY, ARRAY_INDEX, BRACKET,
        }

        private static final class NullValue implements Serializable {
            private static final long serialVersionUID = 1;
        }
    }

    private static abstract class JsonGroupItem<T> extends JsonItem<T> {

        final List<JsonItem<?>> children = new ArrayList<>();

        boolean expanded = false;

        public JsonGroupItem(int step, String name, T value, NameType nameType) {
            super(step, name, value, nameType);
        }

        protected JsonGroupItem(Parcel in) {
            super(in);
            in.readTypedList(children, CREATOR);
            expanded = in.readInt() == 1;
        }

        @Override
        public void writeToParcel(@NonNull Parcel dest, int flags) {
            super.writeToParcel(dest, flags);
            dest.writeTypedList(children);
            dest.writeInt(expanded ? 1 : 0);
        }

        @Override
        public boolean isExpandable() {
            return !children.isEmpty();
        }

        public List<JsonItem<?>> getExpandedItems() {
            List<JsonItem<?>> expandedList = new ArrayList<>();
            children.forEach(jsonItem -> {
                if (expanded) {
                    expandedList.add(jsonItem);
                    if (jsonItem instanceof JsonGroupItem) {
                        expandedList.addAll(((JsonGroupItem<?>) jsonItem).getExpandedItems());
                    }
                }
            });
            return expandedList;
        }

        public List<JsonItem<?>> getFlattenItems(boolean child) {
            List<JsonItem<?>> flatten = new ArrayList<>();
            if (!child) {
                flatten.add(this);
            }
            children.forEach(jsonItem -> {
                flatten.add(jsonItem);
                if (jsonItem instanceof JsonGroupItem) {
                    flatten.addAll(((JsonGroupItem<?>) jsonItem).getFlattenItems(true));
                }
            });
            return flatten;
        }
    }

    private static class JsonObjectItem extends JsonGroupItem<String> {
        public static final Creator<JsonObjectItem> CREATOR = new Creator<>() {
            @Override
            public JsonObjectItem createFromParcel(Parcel in) {
                return new JsonObjectItem(in);
            }

            @Override
            public JsonObjectItem[] newArray(int size) {
                return new JsonObjectItem[size];
            }
        };

        public JsonObjectItem(int step, String name, JSONObject value, NameType nameType) {
            super(step, name, value.toString(), nameType);
            if (value.length() > 0) {
                children.add(new JsonItem<>(step + 1, "{", null, NameType.BRACKET));
                value.keys().forEachRemaining(s -> {
                    Object o = value.opt(s);
                    if (o instanceof JSONObject) {
                        children.add(new JsonObjectItem(step + 2, s, (JSONObject) o, NameType.OBJECT_KEY));
                    } else if (o instanceof JSONArray) {
                        children.add(new JsonArrayItem(step + 2, s, (JSONArray) o, NameType.OBJECT_KEY));
                    } else {
                        children.add(new JsonItem<>(step + 2, s, o, NameType.OBJECT_KEY));
                    }
                });
                children.add(new JsonItem<>(step + 1, "}", null, NameType.BRACKET));
            }
        }

        protected JsonObjectItem(Parcel in) {
            super(in);
        }

        @Override
        protected String getType() {
            return "object";
        }
    }

    private static class JsonArrayItem extends JsonGroupItem<String> {
        public static final Creator<JsonArrayItem> CREATOR = new Creator<>() {
            @Override
            public JsonArrayItem createFromParcel(Parcel in) {
                return new JsonArrayItem(in);
            }

            @Override
            public JsonArrayItem[] newArray(int size) {
                return new JsonArrayItem[size];
            }
        };

        public JsonArrayItem(int step, String name, JSONArray value, NameType nameType) {
            super(step, name, value.toString(), nameType);
            if (value.length() > 0) {
                children.add(new JsonItem<>(step + 1, "[", null, NameType.BRACKET));
                for (int i = 0; i < value.length(); i++) {
                    Object o = value.opt(i);
                    if (o instanceof JSONObject) {
                        children.add(new JsonObjectItem(step + 2, Integer.toString(i), (JSONObject) o, NameType.ARRAY_INDEX));
                    } else if (o instanceof JSONArray) {
                        children.add(new JsonArrayItem(step + 2, Integer.toString(i), (JSONArray) o, NameType.ARRAY_INDEX));
                    } else {
                        children.add(new JsonItem<>(step + 2, Integer.toString(i), o, NameType.ARRAY_INDEX));
                    }
                }
                children.add(new JsonItem<>(step + 1, "]", null, NameType.BRACKET));
            }
        }

        protected JsonArrayItem(Parcel in) {
            super(in);
        }

        @Override
        protected String getType() {
            return "array";
        }
    }

    private static class JsonAdapter extends RecyclerView.Adapter<JsonViewHolder> {
        private final List<JsonItem<?>> itemList = new ArrayList<>();

        public void setJSONObject(JSONObject object) {
            if (!itemList.isEmpty()) {
                notifyItemRangeRemoved(0, itemList.size());
                itemList.clear();
            }
            itemList.add(new JsonItem<>(0, "{", null, JsonItem.NameType.BRACKET));
            object.keys().forEachRemaining(s -> {
                Object o = object.opt(s);
                if (o instanceof JSONObject) {
                    itemList.add(new JsonObjectItem(1, s, (JSONObject) o, JsonItem.NameType.OBJECT_KEY));
                } else if (o instanceof JSONArray) {
                    itemList.add(new JsonArrayItem(1, s, (JSONArray) o, JsonItem.NameType.OBJECT_KEY));
                } else {
                    itemList.add(new JsonItem<>(1, s, o, JsonItem.NameType.OBJECT_KEY));
                }
            });
            itemList.add(new JsonItem<>(0, "}", null, JsonItem.NameType.BRACKET));
            notifyItemRangeInserted(0, itemList.size());
        }

        public void setJSONArray(JSONArray array) {
            if (!itemList.isEmpty()) {
                notifyItemRangeRemoved(0, itemList.size());
                itemList.clear();
            }
            itemList.add(new JsonItem<>(0, "[", null, JsonItem.NameType.BRACKET));
            for (int i = 0; i < array.length(); i++) {
                Object o = array.opt(i);
                if (o instanceof JSONObject) {
                    itemList.add(new JsonObjectItem(1, Integer.toString(i), (JSONObject) o, JsonItem.NameType.ARRAY_INDEX));
                } else if (o instanceof JSONArray) {
                    itemList.add(new JsonArrayItem(1, Integer.toString(i), (JSONArray) o, JsonItem.NameType.ARRAY_INDEX));
                } else {
                    itemList.add(new JsonItem<>(1, Integer.toString(i), o, JsonItem.NameType.ARRAY_INDEX));
                }
            }
            itemList.add(new JsonItem<>(0, "]", null, JsonItem.NameType.BRACKET));
            notifyItemRangeInserted(0, itemList.size());
        }

        public void expandAll() {
            int beforeSize = itemList.size();
            List<JsonItem<?>> flatten = getRootItemList().stream().flatMap(jsonItem -> {
                if (jsonItem instanceof JsonGroupItem) {
                    return ((JsonGroupItem<?>) jsonItem).getFlattenItems(false).stream();
                }
                return Stream.of(jsonItem);
            }).collect(Collectors.toList());
            itemList.clear();
            itemList.addAll(flatten);
            for (JsonItem<?> item : itemList) {
                if (item instanceof JsonGroupItem) {
                    ((JsonGroupItem<?>) item).expanded = true;
                }
            }
            notifyItemRangeRemoved(itemList.size(), beforeSize);
            notifyItemRangeChanged(0, itemList.size());
        }

        public void collapseAll() {
            List<JsonItem<?>> rootIndex = getRootItemList();
            int beforeSize = itemList.size();
            for (JsonItem<?> item : itemList) {
                if (item instanceof JsonGroupItem) {
                    ((JsonGroupItem<?>) item).expanded = false;
                }
            }
            itemList.clear();
            itemList.addAll(rootIndex);
            notifyItemRangeRemoved(itemList.size(), beforeSize);
            notifyItemRangeChanged(0, itemList.size());
        }

        private List<JsonItem<?>> getRootItemList() {
            return itemList.stream().filter(jsonItem -> jsonItem.step <= 1).collect(Collectors.toList());
        }

        @NonNull
        @Override
        public JsonViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
            return new JsonViewHolder(LayoutInflater.from(parent.getContext()).inflate(R.layout.json_item, parent, false));
        }

        @Override
        public void onBindViewHolder(@NonNull JsonViewHolder holder, int position) {
            boolean isNight = (holder.itemView.getResources().getConfiguration().uiMode & Configuration.UI_MODE_NIGHT_MASK) == Configuration.UI_MODE_NIGHT_YES;
            float density = holder.itemView.getResources().getDisplayMetrics().density;
            JsonItem<?> item = itemList.get(position);
            holder.binding.left.setText(item.name);
            if (item.nameType == JsonItem.NameType.OBJECT_KEY) {
                holder.binding.leftPrefix.setText("\"");
                holder.binding.leftSuffix.setText("\"");
            } else {
                holder.binding.leftPrefix.setText(null);
                holder.binding.leftSuffix.setText(null);
            }
            if (position == 0 || position == getItemCount() - 1) {
                holder.itemView.setPadding(Math.round(density * 8), 0, 0, 0);
            } else {
                holder.itemView.setPadding(Math.round(density * 16) * item.step, 0, 0, 0);
            }
            if (!(item instanceof JsonGroupItem)) {
                holder.binding.rightPrefix.setText(null);
                holder.binding.rightSuffix.setText(null);
                if (item.value == null) {
                    holder.binding.right.setTextColor(isNight ? 0xffEC7600 : 0xff1232AC);
                    if (item.nameType != JsonItem.NameType.BRACKET) {
                        holder.binding.right.setText(R.string.json_null);
                    } else {
                        holder.binding.right.setText(null);
                    }
                } else {
                    if (item.value instanceof String) {
                        holder.binding.rightPrefix.setText("\"");
                        holder.binding.rightSuffix.setText("\"");
                        holder.binding.right.setTextColor(isNight ? 0xff6E875A : 0xff377B2A);
                    } else if (item.value instanceof Number) {
                        holder.binding.rightPrefix.setText(null);
                        holder.binding.rightSuffix.setText(null);
                        holder.binding.right.setTextColor(isNight ? 0xff6897BB : 0xff284FE2);
                    } else {
                        holder.binding.rightPrefix.setText(null);
                        holder.binding.rightSuffix.setText(null);
                        holder.binding.right.setTextColor(isNight ? 0xffEC7600 : 0xff1232AC);
                    }
                    holder.binding.right.setText(item.value.toString());
                }
            } else {
                if (((JsonGroupItem<?>) item).children.isEmpty()) {
                    if (item instanceof JsonObjectItem) {
                        holder.binding.rightPrefix.setText("{");
                        holder.binding.rightSuffix.setText("}");
                    }
                    if (item instanceof JsonArrayItem) {
                        holder.binding.rightPrefix.setText("[");
                        holder.binding.rightSuffix.setText("]");
                    }
                    holder.binding.right.setText(" ");
                } else {
                    holder.binding.rightPrefix.setText(null);
                    holder.binding.rightSuffix.setText(null);
                    holder.binding.right.setText(null);
                }
            }
            holder.binding.right.requestLayout();
            if (item.nameType == JsonItem.NameType.ARRAY_INDEX) {
                holder.binding.left.setTextColor(isNight ? 0xff6897BB : 0xff284FE2);
            } else if (item.nameType == JsonItem.NameType.BRACKET) {
                holder.binding.left.setTextColor(isNight ? 0xffE8E2B7 : 0xff000000);
            } else {
                holder.binding.left.setTextColor(isNight ? 0xff6E875A : 0xff377B2A);
            }
            if (item.isExpandable()) {
                holder.binding.expand.setVisibility(VISIBLE);
            } else {
                holder.binding.expand.setVisibility(INVISIBLE);
            }
            holder.binding.expand.setImageTintList(ColorStateList.valueOf(isNight ? 0xffE8E2B7 : 0xff000000));
            if (item instanceof JsonGroupItem && ((JsonGroupItem<?>) item).expanded) {
                holder.binding.expand.setRotation(180);
            } else {
                holder.binding.expand.setRotation(0);
            }
            holder.itemView.setOnClickListener(v -> {
                if (item.isExpandable() && item instanceof JsonGroupItem) {
                    JsonGroupItem<?> groupItem = (JsonGroupItem<?>) item;
                    int currentPos = itemList.indexOf(item);
                    if (!groupItem.expanded) {
                        groupItem.expanded = true;
                        itemList.addAll(currentPos + 1, groupItem.children);
                        ObjectAnimator animator = ObjectAnimator.ofFloat(holder.binding.expand, "rotation", 0, 180f);
                        animator.setDuration(200);
                        animator.start();
                        notifyItemRangeInserted(currentPos + 1, groupItem.children.size());
                    } else {
                        List<JsonItem<?>> expanded = groupItem.getExpandedItems();
                        for (JsonItem<?> jsonItem : expanded) {
                            if (jsonItem instanceof JsonGroupItem) {
                                ((JsonGroupItem<?>) jsonItem).expanded = false;
                            }
                        }
                        groupItem.expanded = false;
                        Animator animator = ObjectAnimator.ofFloat(holder.binding.expand, "rotation", 180f, 0);
                        animator.setDuration(200);
                        animator.start();
                        itemList.subList(currentPos + 1, currentPos + 1 + expanded.size()).clear();
                        notifyItemRangeRemoved(currentPos + 1, expanded.size());
                    }
                }
            });
        }

        @Override
        public int getItemCount() {
            return itemList.size();
        }
    }

    private static class JsonViewHolder extends RecyclerView.ViewHolder {
        public JsonItemBinding binding;

        public JsonViewHolder(@NonNull View itemView) {
            super(itemView);
            binding = DataBindingUtil.bind(itemView);
        }
    }
}
