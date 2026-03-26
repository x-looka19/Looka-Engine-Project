#!/bin/sh
#
# Gradle wrapper
#

exec java $JAVA_OPTS -classpath "gradle/wrapper/gradle-wrapper.jar" org.gradle.wrapper.GradleWrapperMain "$@"
