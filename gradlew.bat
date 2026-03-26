@if "%DEBUG%" == "" @echo off
@rem  Gradle wrapper

java %JAVA_OPTS% -classpath "gradle\wrapper\gradle-wrapper.jar" org.gradle.wrapper.GradleWrapperMain %*
