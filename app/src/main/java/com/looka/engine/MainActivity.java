package com.looka.engine;

import android.animation.Animator;
import android.animation.AnimatorListenerAdapter;
import android.animation.ObjectAnimator;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.provider.Settings;
import android.view.View;
import android.view.animation.AccelerateDecelerateInterpolator;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;
import androidx.appcompat.app.AppCompatActivity;
import androidx.cardview.widget.CardView;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

public class MainActivity extends AppCompatActivity {
    
    private static final int REQUEST_OVERLAY_PERMISSION = 1001;
    private static final int REQUEST_NOTIFICATION_PERMISSION = 1002;
    
    private TextView statusText;
    private TextView modeText;
    private TextView fpsText;
    private ProgressBar loadingProgress;
    private Button startButton;
    private Button stopButton;
    private Button settingsButton;
    private CardView statusCard;
    
    private LookaEngineService engineService;
    private Handler mainHandler;
    private boolean isEngineRunning = false;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
        mainHandler = new Handler(Looper.getMainLooper());
        
        initViews();
        initEngine();
        checkPermissions();
        setupClickListeners();
        
        // Animate entrance
        animateEntrance();
    }
    
    private void initViews() {
        statusText = findViewById(R.id.statusText);
        modeText = findViewById(R.id.modeText);
        fpsText = findViewById(R.id.fpsText);
        loadingProgress = findViewById(R.id.loadingProgress);
        startButton = findViewById(R.id.startButton);
        stopButton = findViewById(R.id.stopButton);
        settingsButton = findViewById(R.id.settingsButton);
        statusCard = findViewById(R.id.statusCard);
    }
    
    private void initEngine() {
        engineService = LookaEngineService.getInstance();
        engineService.initialize(this);
        
        // Check root status
        boolean hasRoot = engineService.checkRootAccess();
        updateModeDisplay(hasRoot);
    }
    
    private void checkPermissions() {
        // Check overlay permission for Android 6+
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            if (!Settings.canDrawOverlays(this)) {
                requestOverlayPermission();
            }
        }
        
        // Check notification permission for Android 13+
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            if (ContextCompat.checkSelfPermission(this, 
                android.Manifest.permission.POST_NOTIFICATIONS) != PackageManager.PERMISSION_GRANTED) {
                requestNotificationPermission();
            }
        }
    }
    
    private void requestOverlayPermission() {
        new AlertDialog.Builder(this)
            .setTitle("Permission Required")
            .setMessage("Looka Engine needs overlay permission to display floating control panel")
            .setPositiveButton("Grant", (dialog, which) -> {
                Intent intent = new Intent(Settings.ACTION_MANAGE_OVERLAY_PERMISSION,
                    Uri.parse("package:" + getPackageName()));
                startActivityForResult(intent, REQUEST_OVERLAY_PERMISSION);
            })
            .setNegativeButton("Cancel", null)
            .show();
    }
    
    private void requestNotificationPermission() {
        ActivityCompat.requestPermissions(this,
            new String[]{android.Manifest.permission.POST_NOTIFICATIONS},
            REQUEST_NOTIFICATION_PERMISSION);
    }
    
    private void setupClickListeners() {
        startButton.setOnClickListener(v -> startEngine());
        stopButton.setOnClickListener(v -> stopEngine());
        settingsButton.setOnClickListener(v -> showSettingsDialog());
    }
    
    private void startEngine() {
        if (isEngineRunning) return;
        
        startButton.setEnabled(false);
        loadingProgress.setVisibility(View.VISIBLE);
        
        new Thread(() -> {
            boolean success = engineService.startEngine();
            
            mainHandler.post(() -> {
                loadingProgress.setVisibility(View.GONE);
                startButton.setEnabled(true);
                
                if (success) {
                    isEngineRunning = true;
                    statusText.setText("🟢 ACTIVE");
                    statusText.setTextColor(getColor(R.color.success_green));
                    startButton.setVisibility(View.GONE);
                    stopButton.setVisibility(View.VISIBLE);
                    
                    Toast.makeText(MainActivity.this, 
                        "Looka Engine started successfully", Toast.LENGTH_SHORT).show();
                    
                    // Start monitoring FPS
                    startFPSMonitoring();
                } else {
                    statusText.setText("🔴 ERROR");
                    statusText.setTextColor(getColor(R.color.error_red));
                    Toast.makeText(MainActivity.this, 
                        "Failed to start engine. Check root access.", Toast.LENGTH_LONG).show();
                }
            });
        }).start();
    }
    
    private void stopEngine() {
        if (!isEngineRunning) return;
        
        new Thread(() -> {
            engineService.stopEngine();
            
            mainHandler.post(() -> {
                isEngineRunning = false;
                statusText.setText("⚪ STOPPED");
                statusText.setTextColor(getColor(R.color.text_secondary));
                startButton.setVisibility(View.VISIBLE);
                stopButton.setVisibility(View.GONE);
                fpsText.setText("--- FPS");
                
                Toast.makeText(MainActivity.this, 
                    "Engine stopped", Toast.LENGTH_SHORT).show();
            });
        }).start();
    }
    
    private void startFPSMonitoring() {
        new Thread(() -> {
            while (isEngineRunning) {
                int fps = engineService.getCurrentFPS();
                mainHandler.post(() -> {
                    fpsText.setText(String.format("%d FPS", fps));
                });
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    break;
                }
            }
        }).start();
    }
    
    private void updateModeDisplay(boolean hasRoot) {
        if (hasRoot) {
            modeText.setText("🚀 ULTRA MODE");
            modeText.setTextColor(getColor(R.color.ultra_gold));
            statusText.setText("⚪ READY");
        } else {
            modeText.setText("📱 STANDARD MODE");
            modeText.setTextColor(getColor(R.color.text_secondary));
            statusText.setText("⚪ READY");
            
            // Show notification for non-root users
            showNonRootNotification();
        }
    }
    
    private void showNonRootNotification() {
        new AlertDialog.Builder(this)
            .setTitle("Standard Mode Active")
            .setMessage("Root access not detected.\n\n" +
                       "Ultra Mode features (120 FPS ESP, Zero Latency Aimbot) are locked.\n\n" +
                       "Standard Mode features are still available.")
            .setPositiveButton("OK", null)
            .setNegativeButton("Learn More", (dialog, which) -> {
                // Open guide for rooting
                Intent intent = new Intent(Intent.ACTION_VIEW, 
                    Uri.parse("https://looka-engine.com/root-guide"));
                startActivity(intent);
            })
            .show();
    }
    
    private void showSettingsDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        View dialogView = getLayoutInflater().inflate(R.layout.dialog_settings, null);
        
        // Populate settings
        // ... setup settings UI
        
        builder.setView(dialogView)
            .setTitle("Settings")
            .setPositiveButton("Save", (dialog, which) -> {
                // Save settings
                saveSettings();
            })
            .setNegativeButton("Cancel", null)
            .show();
    }
    
    private void saveSettings() {
        // Save user preferences
        Toast.makeText(this, "Settings saved", Toast.LENGTH_SHORT).show();
    }
    
    private void animateEntrance() {
        statusCard.setAlpha(0f);
        statusCard.setTranslationY(50f);
        statusCard.animate()
            .alpha(1f)
            .translationY(0f)
            .setDuration(500)
            .setInterpolator(new AccelerateDecelerateInterpolator())
            .start();
    }
    
    @Override
    protected void onResume() {
        super.onResume();
        if (engineService != null) {
            boolean hasRoot = engineService.checkRootAccess();
            updateModeDisplay(hasRoot);
        }
    }
    
    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (isEngineRunning) {
            stopEngine();
        }
    }
}
