package com.tencent.mars.sample.core;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * Activity event connection
 *
 * Created by kirozhao on 16/3/1.
 */
@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.TYPE)
public @interface ActivityEventConnection {

    ActivityEvent.Connect connect() default ActivityEvent.Connect.None;
}
