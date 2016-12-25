package com.tencent.mars.sample.wrapper;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Inherited;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * Define task property
 * <p></p>
 * Created by kirozhao on 16/2/29.
 */
@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.TYPE)
@Documented
@Inherited
public @interface TaskProperty {

    String host() default "";

    String path();

    boolean shortChannelSupport() default true;

    boolean longChannelSupport() default false;

    int cmdID() default -1;
}
