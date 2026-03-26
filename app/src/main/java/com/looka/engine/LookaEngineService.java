package com.looka.engine;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Binder;
import android.os.Build;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import androidx.core.app.NotificationCompat;

public class LookaEngineService extends Service {
    
    private static final String CHANNEL_ID = "looka_engine_channel";
    private static final int NOTIFICATION_ID = 1001;
    
    private static LookaEngineService instance;
    private final IBinder binder = new LocalBinder();
    private boolean isEngineRunning = false;
    private Handler mainHandler;
    
    static {
        System.loadLibrary("looka_engine");
    }
    
    public class LocalBinder extends Binder {
        LookaEngineService getService() {
            return LookaEngineService.this;
        }
    }
    
    public static LookaEngineService getInstance() {
        return instance;
    }
    
    public void initialize(Context context) {
        instance = this;
        mainHandler = new Handler(Looper.getMainLooper());
        initNative(context);
    }
    
    @Override
    public void onCreate() {
        super.onCreate();
        createNotificationChannel();
        startForeground(NOTIFICATION_ID, createNotification());
    }
    
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return START_STICKY;
    }
    
    @Override
    public IBinder onBind(Intent intent) {
        return binder;
    }
    
    private void createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel channel = new NotificationChannel(
                CHANNEL_ID,
                "Looka Engine",
                NotificationManager.IMPORTANCE_LOW
            );
            channel.setDescription("Looka Engine is running in background");
            NotificationManager manager = getSystemService(NotificationManager.class);
            manager.createNotificationChannel(channel);
        }
    }
    
    private Notification createNotification() {
        return new NotificationCompat.Builder(this, CHANNEL_ID)
            .setContentTitle("Looka Engine")
            .setContentText("Engine is ready")
            .setSmallIcon(R.drawable.ic_launcher)
            .setPriority(NotificationCompat.PRIORITY_LOW)
            .build();
    }
    
    // Native methods
    public native void initNative(Context context);
    public native boolean checkRootAccess();
    public native boolean startEngine();
    public native void stopEngine();
    public native int getCurrentFPS();
    public native void enableESP(boolean enable);
    public native void enableAimAssist(boolean enable);
    public native void enableHardcoreAimbot(boolean enable);
    public native void enableFlickBoost(boolean enable);
    
    @Override
    public void onDestroy() {
        stopEngine();
        super.onDestroy();
    }
}