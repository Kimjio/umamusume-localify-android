package com.kimjio.umamusumelocalify.settings.activity;

import android.animation.Animator;
import android.animation.ObjectAnimator;
import android.app.Activity;
import android.content.Intent;
import android.graphics.Canvas;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Parcel;
import android.os.Parcelable;
import android.util.DisplayMetrics;
import android.util.Log;
import android.util.TypedValue;
import android.view.HapticFeedbackConstants;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;

import androidx.annotation.NonNull;
import androidx.appcompat.content.res.AppCompatResources;
import androidx.coordinatorlayout.widget.CoordinatorLayout;
import androidx.core.content.ContextCompat;
import androidx.core.view.OnApplyWindowInsetsListener;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.databinding.DataBindingUtil;
import androidx.documentfile.provider.DocumentFile;
import androidx.lifecycle.ViewModel;
import androidx.lifecycle.ViewModelProvider;
import androidx.recyclerview.widget.ItemTouchHelper;
import androidx.recyclerview.widget.RecyclerView;

import com.google.android.material.color.MaterialColors;
import com.google.android.material.snackbar.Snackbar;
import com.kimjio.umamusumelocalify.settings.BuildConfig;
import com.kimjio.umamusumelocalify.settings.FileUtils;
import com.kimjio.umamusumelocalify.settings.R;
import com.kimjio.umamusumelocalify.settings.databinding.MsgPackItemBinding;
import com.kimjio.umamusumelocalify.settings.databinding.MsgPackListActivityBinding;

import java.lang.ref.WeakReference;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.stream.Collectors;
import java.util.stream.Stream;

import it.xabaras.android.recyclerview.swipedecorator.RecyclerViewSwipeDecorator;

public class MsgPackListActivity extends BaseActivity<MsgPackListActivityBinding> {
    private static final String TAG = "MsgPackListActivity";

    private MsgPackListViewModel viewModel;

    private final ItemTouchHelper touchHelper = new ItemTouchHelper(new ItemTouchHelper.SimpleCallback(0, ItemTouchHelper.START | ItemTouchHelper.END) {
        final int KEY_SWIPE_HAPTIC = R.id.swipe_haptic;

        @Override
        public boolean onMove(@NonNull RecyclerView recyclerView, @NonNull RecyclerView.ViewHolder viewHolder, @NonNull RecyclerView.ViewHolder target) {
            return false;
        }

        @Override
        public void onChildDraw(@NonNull Canvas c, @NonNull RecyclerView recyclerView, @NonNull RecyclerView.ViewHolder viewHolder, float dX, float dY, int actionState, boolean isCurrentlyActive) {
            new RecyclerViewSwipeDecorator.Builder(c, recyclerView, viewHolder, dX, dY, actionState, isCurrentlyActive)
                    .addActionIcon(R.drawable.ic_delete)
                    .setActionIconTint(ContextCompat.getColor(MsgPackListActivity.this, R.color.error))
                    .setIconHorizontalMargin(TypedValue.COMPLEX_UNIT_DIP, 24)
                    .create()
                    .decorate();

            if (actionState == ItemTouchHelper.ACTION_STATE_SWIPE) {
                int width;
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                    width = getWindowManager().getCurrentWindowMetrics().getBounds().width();
                } else {
                    DisplayMetrics metrics = new DisplayMetrics();
                    getWindowManager().getDefaultDisplay().getMetrics(metrics);
                    width = metrics.widthPixels;
                }
                float percent = Math.abs(dX) * 100 / width;

                Boolean swipeHaptic = (Boolean) viewHolder.itemView.getTag(KEY_SWIPE_HAPTIC);

                if (percent > 50f && (swipeHaptic == null || !swipeHaptic)) {
                    viewHolder.itemView.setTag(KEY_SWIPE_HAPTIC, true);
                    if (isCurrentlyActive) {
                        viewHolder.itemView.performHapticFeedback(HapticFeedbackConstants.CLOCK_TICK);
                    }
                } else if (swipeHaptic != null && swipeHaptic && percent < 50f) {
                    viewHolder.itemView.setTag(KEY_SWIPE_HAPTIC, false);
                    if (isCurrentlyActive) {
                        viewHolder.itemView.performHapticFeedback(HapticFeedbackConstants.CLOCK_TICK);
                    }
                }
            }

            super.onChildDraw(c, recyclerView, viewHolder, dX, dY, actionState, isCurrentlyActive);
        }

        @Override
        public boolean isLongPressDragEnabled() {
            return false;
        }

        @Override
        public void onSwiped(@NonNull RecyclerView.ViewHolder viewHolder, int direction) {
            int position = viewHolder.getBindingAdapterPosition();

            Object item = viewModel.adapter.filtered.get(position);

            int groupPosition = viewModel.adapter.files.indexOf(item);
            AtomicInteger parentPosition = new AtomicInteger(-1);
            AtomicInteger childPosition = new AtomicInteger(-1);

            if (item instanceof MsgPackFile) {
                for (int i = position - 1; i >= 0; i--) {
                    Object upperItem = viewModel.adapter.filtered.get(i);
                    if (upperItem instanceof MsgPackGroup) {
                        for (int j = 0; j < viewModel.adapter.files.size(); j++) {
                            Object object = viewModel.adapter.files.get(j);
                            if (object instanceof MsgPackGroup) {
                                parentPosition.set(j);
                                break;
                            }
                        }
                        MsgPackGroup group = (MsgPackGroup) viewModel.adapter.files.get(parentPosition.get());
                        childPosition.set(group.files.indexOf(item));
                        if (childPosition.get() > -1) {
                            group.files.remove(childPosition.get());
                        }
                        break;
                    }
                }
            }

            if (groupPosition > -1) {
                viewModel.adapter.files.remove(groupPosition);
            }
            viewModel.adapter.notifyItemRemoved(position);
            viewModel.adapter.setFilter(viewModel.filter);

            Snackbar.make(binding.getRoot(), R.string.deleted, Snackbar.LENGTH_SHORT)
                    .addCallback(new Snackbar.Callback() {
                        @Override
                        public void onDismissed(Snackbar transientBottomBar, int event) {
                            if (event != Snackbar.Callback.DISMISS_EVENT_ACTION) {
                                new Thread(() -> {
                                    if (item instanceof MsgPackGroup) {
                                        boolean result = true;
                                        for (MsgPackFile file : ((MsgPackGroup) item).filtered) {
                                            DocumentFile documentFile = DocumentFile.fromSingleUri(MsgPackListActivity.this, file.path);
                                            if (documentFile != null) {
                                                result = result && documentFile.delete();
                                            }
                                        }
                                        if (!result && BuildConfig.DEBUG) {
                                            Log.w(TAG, "Delete failed group: ".concat(((MsgPackGroup) item).date));
                                        }
                                    } else if (item instanceof MsgPackFile) {
                                        boolean result = false;
                                        DocumentFile documentFile = DocumentFile.fromSingleUri(MsgPackListActivity.this, ((MsgPackFile) item).path);
                                        if (documentFile != null) {
                                            result = documentFile.delete();
                                        }
                                        if (!result && BuildConfig.DEBUG) {
                                            Log.w(TAG, "Delete failed: ".concat(((MsgPackFile) item).fileName));
                                        }
                                    }
                                }).start();
                            }
                        }
                    })
                    .setAction(R.string.undo, v -> {
                        if (groupPosition > 0) {
                            viewModel.adapter.files.add(groupPosition, item);
                        }
                        if (parentPosition.get() > -1 && childPosition.get() > -1 &&
                                item instanceof MsgPackFile) {
                            ((MsgPackGroup) viewModel.adapter.files.get(parentPosition.get())).files.add(childPosition.get(), (MsgPackFile) item);
                        }
                        viewModel.adapter.setFilter(viewModel.filter);
                    })
                    .show();
        }
    });

    private Uri path;

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

        viewModel = new ViewModelProvider(this).get(MsgPackListViewModel.class);

        path = getIntent().getParcelableExtra(EXTRA_PATH);

        OnApplyWindowInsetsListener listener = (v, insets) -> {
            int bottomInsets = insets.getInsets(WindowInsetsCompat.Type.navigationBars()).bottom;
            binding.recyclerView.setPaddingRelative(0, 0, 0, bottomInsets);
            CoordinatorLayout.LayoutParams params = (CoordinatorLayout.LayoutParams) binding.topFab.getLayoutParams();
            params.bottomMargin += bottomInsets;
            ViewCompat.setOnApplyWindowInsetsListener(binding.getRoot(), null);
            return insets;
        };

        ViewCompat.setOnApplyWindowInsetsListener(binding.getRoot(), listener);

        binding.recyclerView.addOnScrollListener(new RecyclerView.OnScrollListener() {
            @Override
            public void onScrolled(@NonNull RecyclerView recyclerView, int dx, int dy) {
                if (recyclerView.computeVerticalScrollOffset() > 0) {
                    binding.topFab.show();
                } else {
                    binding.topFab.hide();
                }
            }
        });

        binding.topFab.hide();
        binding.topFab.setOnClickListener(v -> {
            binding.topFab.hide();
            binding.recyclerView.scrollToPosition(0);
        });

        binding.refreshLayout.setProgressBackgroundColorSchemeColor(MaterialColors.getColor(binding.getRoot(), R.attr.colorSurface));
        binding.refreshLayout.setColorSchemeColors(MaterialColors.getColor(binding.getRoot(), R.attr.colorPrimary), MaterialColors.getColor(binding.getRoot(), R.attr.colorSecondary), MaterialColors.getColor(binding.getRoot(), R.attr.colorTertiary));
        binding.refreshLayout.setOnRefreshListener(this::initFiles);

        if (viewModel.adapter != null) {
            binding.refreshLayout.setRefreshing(false);
            binding.recyclerView.setAdapter(viewModel.adapter);
            touchHelper.attachToRecyclerView(binding.recyclerView);
            binding.progress.hide();
            binding.progressText.setVisibility(View.INVISIBLE);
        } else if (viewModel.groups != null) {
            binding.refreshLayout.setRefreshing(false);
            if (viewModel.groups.isEmpty()) {
                binding.emptyLayout.setVisibility(View.VISIBLE);
                binding.recyclerView.setVisibility(View.GONE);
            } else {
                binding.emptyLayout.setVisibility(View.GONE);
                viewModel.adapter = new MsgPackAdapter(this, viewModel.groups);
                binding.recyclerView.setAdapter(viewModel.adapter);
                if (viewModel.filter != null) {
                    viewModel.adapter.setFilter(viewModel.filter);
                }
                touchHelper.attachToRecyclerView(binding.recyclerView);
            }
            binding.progress.hide();
            binding.progressText.setVisibility(View.INVISIBLE);
        } else {
            initFiles();
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.menu_msgpack_list, menu);
        if (viewModel.filter != null) {
            switch (viewModel.filter) {
                case ALL:
                    menu.findItem(R.id.menu_filter_all).setChecked(true);
                    break;
                case RESPONSE:
                    menu.findItem(R.id.menu_filter_response).setChecked(true);
                    break;
                case REQUEST:
                    menu.findItem(R.id.menu_filter_request).setChecked(true);
                    break;
            }
        }
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(@NonNull MenuItem item) {
        Filter filter = null;
        if (item.getItemId() == R.id.menu_filter_all) {
            item.setChecked(!item.isChecked());
            filter = Filter.ALL;
        }
        if (item.getItemId() == R.id.menu_filter_response) {
            item.setChecked(!item.isChecked());
            filter = Filter.RESPONSE;
        }
        if (item.getItemId() == R.id.menu_filter_request) {
            item.setChecked(!item.isChecked());
            filter = Filter.REQUEST;
        }
        if (filter != null) {
            viewModel.filter = filter;
            if (viewModel.adapter != null) {
                viewModel.adapter.setFilter(filter);
            }
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    private void initFiles() {
        if (viewModel.initThread == null || !viewModel.initThread.isAlive()) {
            binding.emptyLayout.setVisibility(View.GONE);
            binding.progress.show();
            binding.progress.setIndeterminate(true);
            binding.progressText.setText(null);
            binding.progressText.setVisibility(View.VISIBLE);
            binding.progressText.setAlpha(1f);
            binding.recyclerView.setAdapter(null);
            viewModel.adapter = null;
            viewModel.groups = Collections.emptyList();
            viewModel.initThread = new InitThread(viewModel, this);
            viewModel.initThread.start();
        } else {
            viewModel.initThread.setViewModel(viewModel);
            viewModel.initThread.setActivity(this);
            binding.progress.setIndeterminate(false);
            binding.progress.setMax(viewModel.filesLength);
        }
    }

    private enum Filter {
        ALL, RESPONSE, REQUEST
    }

    private static class MsgPackGroup implements Parcelable {
        public static final Creator<MsgPackGroup> CREATOR = new Creator<>() {
            @Override
            public MsgPackGroup createFromParcel(Parcel in) {
                return new MsgPackGroup(in);
            }

            @Override
            public MsgPackGroup[] newArray(int size) {
                return new MsgPackGroup[size];
            }
        };
        public final String date;
        public final List<MsgPackFile> files;
        public List<MsgPackFile> filtered;
        public boolean expanded = false;

        public MsgPackGroup(String date, List<MsgPackFile> files) {
            this.date = date;
            this.files = files;
            this.filtered = new ArrayList<>(files);
        }

        protected MsgPackGroup(Parcel in) {
            date = in.readString();
            files = in.createTypedArrayList(MsgPackFile.CREATOR);
            filtered = in.createTypedArrayList(MsgPackFile.CREATOR);
            expanded = in.readInt() == 1;
        }

        public MsgPackGroup setExpanded(boolean expanded) {
            this.expanded = expanded;
            return this;
        }

        public MsgPackGroup setFiltered(List<MsgPackFile> filtered) {
            this.filtered = filtered;
            return this;
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            dest.writeString(date);
            dest.writeTypedList(files);
            dest.writeTypedList(filtered);
            dest.writeInt(expanded ? 1 : 0);
        }

        @Override
        public int describeContents() {
            return 0;
        }
    }

    private static class MsgPackFile implements Parcelable {
        public static final Creator<MsgPackFile> CREATOR = new Creator<>() {
            @Override
            public MsgPackFile createFromParcel(Parcel in) {
                return new MsgPackFile(in);
            }

            @Override
            public MsgPackFile[] newArray(int size) {
                return new MsgPackFile[size];
            }
        };
        public final String fileName;
        public final Uri path;
        public final long timestamp;
        public final long length;

        public MsgPackFile(String fileName, Uri path, long length) {
            this.fileName = fileName;
            this.path = path;
            if (fileName != null) {
                String fileNameWithoutExtension = fileName.split(".msgpack")[0];
                timestamp = Long.parseLong(fileNameWithoutExtension.substring(0, fileNameWithoutExtension.length() - 1));
            } else {
                timestamp = -1;
            }
            this.length = length;
        }

        protected MsgPackFile(Parcel in) {
            fileName = in.readString();
            path = in.readParcelable(Uri.class.getClassLoader());
            timestamp = in.readLong();
            length = in.readLong();
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            dest.writeString(fileName);
            dest.writeParcelable(path, flags);
            dest.writeLong(timestamp);
            dest.writeLong(length);
        }

        @Override
        public int describeContents() {
            return 0;
        }
    }

    private static class MsgPackAdapter extends RecyclerView.Adapter<MsgPackViewHolder> {
        private final WeakReference<Activity> activity;
        private final List<Object> files;

        private final List<Object> filtered;

        public MsgPackAdapter(Activity activity, List<Object> files) {
            this.activity = new WeakReference<>(activity);
            this.files = files;
            this.filtered = new ArrayList<>(files);
        }

        @NonNull
        @Override
        public MsgPackViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
            return new MsgPackViewHolder(LayoutInflater.from(parent.getContext()).inflate(R.layout.msg_pack_item, parent, false));
        }

        @Override
        public void onBindViewHolder(@NonNull MsgPackViewHolder holder, int position) {
            Object obj = filtered.get(position);
            if (obj instanceof MsgPackFile) {
                MsgPackFile file = (MsgPackFile) obj;
                if (file.timestamp > 0) {
                    holder.binding.fileName.setText(new SimpleDateFormat("yyyy-MM-dd HH:mm:ss", Locale.getDefault()).format(new Date(file.timestamp)));
                }
                holder.binding.type.setVisibility(View.VISIBLE);
                holder.binding.type.setText(file.fileName.endsWith("R.msgpack") ? R.string.response : R.string.request);
                holder.binding.icon.setImageDrawable(AppCompatResources.getDrawable(holder.itemView.getContext(), file.fileName.endsWith("R.msgpack") ? R.drawable.ic_response : R.drawable.ic_request));
                holder.binding.fileLength.setText(FileUtils.getHumanReadableByte(file.length));
                holder.binding.expand.setVisibility(View.GONE);
                holder.itemView.setOnClickListener(v -> activity.get().startActivity(new Intent(activity.get(), JsonViewActivity.class).putExtra(EXTRA_PATH, file.path).putExtra(JsonViewActivity.EXTRA_POSITION, position).putExtra(JsonViewActivity.EXTRA_DATETIME, holder.binding.fileName.getText()).putExtra(JsonViewActivity.EXTRA_TYPE, file.fileName.endsWith("R.msgpack") ? 1 : 0)));
            } else if (obj instanceof MsgPackGroup) {
                holder.binding.fileName.setText(((MsgPackGroup) obj).date);
                holder.binding.type.setVisibility(View.GONE);
                holder.binding.type.setText(null);
                holder.binding.icon.setImageDrawable(AppCompatResources.getDrawable(holder.itemView.getContext(), R.drawable.ic_date_range));
                holder.binding.fileLength.setText(String.valueOf(((MsgPackGroup) obj).filtered.size()));
                if (((MsgPackGroup) obj).expanded) {
                    holder.binding.expand.setRotation(180);
                } else {
                    holder.binding.expand.setRotation(0);
                }
                holder.binding.expand.setVisibility(View.VISIBLE);
                holder.itemView.setOnClickListener(v -> {
                    int currentPos = filtered.indexOf(obj);
                    MsgPackGroup group = (MsgPackGroup) obj;
                    if (!group.expanded) {
                        group.expanded = true;
                        filtered.addAll(currentPos + 1, group.filtered);
                        ObjectAnimator animator = ObjectAnimator.ofFloat(holder.binding.expand, "rotation", 0, 180f);
                        animator.setDuration(200);
                        animator.start();
                        notifyItemRangeInserted(currentPos + 1, group.filtered.size());
                    } else {
                        group.expanded = false;
                        Animator animator = ObjectAnimator.ofFloat(holder.binding.expand, "rotation", 180f, 0);
                        animator.setDuration(200);
                        animator.start();
                        filtered.subList(currentPos + 1, currentPos + 1 + group.filtered.size()).clear();
                        notifyItemRangeRemoved(currentPos + 1, group.filtered.size());
                    }
                });
            }
        }

        @Override
        public int getItemCount() {
            return filtered.size();
        }

        public void setFilter(Filter filter) {
            List<Object> list = new ArrayList<>();
            if (!filtered.isEmpty()) {
                list.addAll(filtered);
                notifyItemRangeRemoved(0, filtered.size());
                filtered.clear();
            }
            switch (filter) {
                case ALL:
                    filtered.addAll(files.stream().flatMap(obj -> {
                        if (obj instanceof MsgPackGroup && ((MsgPackGroup) obj).expanded) {
                            ((MsgPackGroup) obj).setFiltered(((MsgPackGroup) obj).files);
                            List<Object> filtered = new ArrayList<>(((MsgPackGroup) obj).filtered);
                            filtered.add(0, obj);
                            return filtered.stream();
                        }
                        return Stream.of(obj);
                    }).collect(Collectors.toList()));
                    break;
                case RESPONSE:
                    filtered.addAll(list.stream().map(obj -> {
                        if (obj instanceof MsgPackGroup) {
                            return new MsgPackGroup(((MsgPackGroup) obj).date, ((MsgPackGroup) obj).files).setExpanded(((MsgPackGroup) obj).expanded).setFiltered(((MsgPackGroup) obj).files.stream().filter(file -> file.fileName.endsWith("R.msgpack")).collect(Collectors.toList()));
                        }
                        return null;
                    }).filter(Objects::nonNull).flatMap(obj -> {
                        if (obj != null && obj.expanded) {
                            List<Object> filtered = new ArrayList<>(obj.filtered);
                            filtered.add(0, obj);
                            return filtered.stream();
                        }
                        return Stream.of(obj);
                    }).collect(Collectors.toList()));
                    break;
                case REQUEST:
                    filtered.addAll(list.stream().map(obj -> {
                        if (obj instanceof MsgPackGroup) {
                            return new MsgPackGroup(((MsgPackGroup) obj).date, ((MsgPackGroup) obj).files).setExpanded(((MsgPackGroup) obj).expanded).setFiltered(((MsgPackGroup) obj).files.stream().filter(file -> file.fileName.endsWith("Q.msgpack")).collect(Collectors.toList()));
                        }
                        return null;
                    }).filter(Objects::nonNull).flatMap(obj -> {
                        if (obj != null && obj.expanded) {
                            List<Object> filtered = new ArrayList<>(obj.filtered);
                            filtered.add(0, obj);
                            return filtered.stream();
                        }
                        return Stream.of(obj);
                    }).collect(Collectors.toList()));
                    break;
            }
            notifyItemRangeInserted(0, filtered.size());
        }
    }

    private static class MsgPackViewHolder extends RecyclerView.ViewHolder {
        public MsgPackItemBinding binding;

        public MsgPackViewHolder(@NonNull View itemView) {
            super(itemView);
            binding = DataBindingUtil.bind(itemView);
        }
    }

    private static class InitThread extends Thread implements Runnable {
        MsgPackListViewModel viewModel;
        WeakReference<MsgPackListActivity> activity;

        public InitThread(MsgPackListViewModel viewModel, MsgPackListActivity activity) {
            this.viewModel = viewModel;
            this.activity = new WeakReference<>(activity);
        }

        public void setViewModel(MsgPackListViewModel viewModel) {
            this.viewModel = viewModel;
        }

        public void setActivity(MsgPackListActivity activity) {
            this.activity = new WeakReference<>(activity);
        }

        @Override
        public void run() {
            DocumentFile root = DocumentFile.fromTreeUri(activity.get(), Uri.parse(activity.get().path.toString()));
            if (root != null) {
                DocumentFile folder = root.findFile("msgpack_dump");
                if (folder != null) {
                    DocumentFile[] fileArray = folder.listFiles();
                    viewModel.filesLength = fileArray.length;
                    if (activity.get() != null) {
                        activity.get().binding.getRoot().post(() -> {
                            activity.get().binding.progress.setIndeterminate(false);
                            activity.get().binding.progress.setMax(viewModel.filesLength);
                            activity.get().binding.progress.setProgressCompat(0, false);
                        });
                    }
                    AtomicInteger i = new AtomicInteger(0);
                    Map<String, List<MsgPackFile>> filesMap = Arrays.stream(fileArray).map((documentFile -> {
                        if (activity.get() != null) {
                            activity.get().binding.getRoot().post(() -> {
                                activity.get().binding.progress.setProgressCompat(i.addAndGet(1), false);
                                activity.get().binding.progressText.setText(String.format(Locale.getDefault(), "%d / %d", i.get(), fileArray.length));
                            });
                        }
                        return new MsgPackFile(documentFile.getName(), documentFile.getUri(), documentFile.length());
                    })).filter(file -> Objects.nonNull(file.fileName)).sorted((a, b) -> (int) (a.timestamp - b.timestamp)).collect(Collectors.groupingBy(file -> new SimpleDateFormat("yyyy-MM-dd", Locale.getDefault()).format(new Date(file.timestamp))));
                    viewModel.groups = filesMap.entrySet().stream().map(entry -> new MsgPackGroup(entry.getKey(), entry.getValue())).sorted(Comparator.comparing(a -> a.date)).collect(Collectors.toList());
                }
            }
            if (activity.get() != null) {
                activity.get().binding.getRoot().post(() -> {
                    if (viewModel.groups.isEmpty()) {
                        activity.get().binding.emptyLayout.setVisibility(View.VISIBLE);
                        activity.get().binding.recyclerView.setVisibility(View.GONE);
                    } else {
                        activity.get().binding.emptyLayout.setVisibility(View.GONE);
                    }
                    activity.get().binding.refreshLayout.setRefreshing(false);
                    viewModel.adapter = new MsgPackAdapter(activity.get(), viewModel.groups);
                    activity.get().binding.recyclerView.setAdapter(viewModel.adapter);
                    if (viewModel.filter != null) {
                        viewModel.adapter.setFilter(viewModel.filter);
                    }
                    activity.get().touchHelper.attachToRecyclerView(activity.get().binding.recyclerView);

                    activity.get().binding.recyclerView.getViewTreeObserver().addOnPreDrawListener(new ViewTreeObserver.OnPreDrawListener() {
                        @Override
                        public boolean onPreDraw() {
                            activity.get().binding.recyclerView.getViewTreeObserver().removeOnPreDrawListener(this);

                            Animator animator = ObjectAnimator.ofFloat(activity.get().binding.recyclerView, "alpha", 0, 1f);
                            animator.setDuration(500L);
                            animator.start();

                            return true;
                        }
                    });

                    Animator animator = ObjectAnimator.ofFloat(activity.get().binding.progressText, "alpha", 1f, 0);
                    animator.setDuration(500L);
                    animator.addListener(new Animator.AnimatorListener() {
                        @Override
                        public void onAnimationStart(@NonNull Animator animation) {

                        }

                        @Override
                        public void onAnimationEnd(@NonNull Animator animation) {
                            activity.get().binding.progressText.setVisibility(View.INVISIBLE);
                        }

                        @Override
                        public void onAnimationCancel(@NonNull Animator animation) {

                        }

                        @Override
                        public void onAnimationRepeat(@NonNull Animator animation) {

                        }
                    });
                    animator.start();
                    activity.get().binding.progress.hide();
                });
            }
        }
    }

    public static class MsgPackListViewModel extends ViewModel {
        private InitThread initThread;

        private List<Object> groups;

        private int filesLength;

        private Filter filter = Filter.ALL;

        private MsgPackAdapter adapter;
    }
}
