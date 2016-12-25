package utils.bindsimple;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

/**
 * Simple view bind for library support
 *
 * Created by kirozhao on 16/2/29.
 */
@Retention(RetentionPolicy.RUNTIME)
public @interface BindView {

    int value() default 0;

    String id() default "";
}
