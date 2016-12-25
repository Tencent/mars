package utils.bindsimple;

import android.app.Activity;
import android.content.res.Resources;
import android.view.View;

import java.lang.reflect.Field;

/**
 * Simple runtime binding
 * <p/>
 * Created by kirozhao on 16/2/29.
 */
public class BindSimple {

    private final String TAG = "BindSimple";

    public static void bind(Activity context) {

        final Resources res = context.getResources();
        final Field[] fields = context.getClass().getDeclaredFields();

        for (Field f : fields) {
            Class<?> type = f.getType();

            // View Subclass ?
            if (!View.class.isAssignableFrom(type)) {
                continue;
            }

            // Find views by id
            try {
                BindView anno = f.getAnnotation(BindView.class);
                if (anno == null) {
                    continue;
                }

                f.setAccessible(true);

                if (anno.value() != 0) {
                    f.set(context, context.findViewById(anno.value()));

                } else if (anno.id() != null) {
                    int id = res.getIdentifier(anno.id(), "id", context.getPackageName());
                    if (id != 0) {
                        f.set(context, context.findViewById(id));
                    }
                }

            } catch (IllegalAccessException e) {
                e.printStackTrace();
            }
        }
    }
}
