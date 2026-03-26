package com.looka.engine;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ScrollView;
import android.widget.Switch;
import android.widget.TextView;

import androidx.cardview.widget.CardView;

public class FloatingControlPanel {
    
    private Context context;
    private WindowManager windowManager;
    private WindowManager.LayoutParams layoutParams;
    private View floatingView;
    private CardView panelCard;
    private boolean isVisible = false;
    private boolean isMinimized = false;
    
    // Callbacks
    private Runnable onEnemySituation;
    private Runnable onAimAssist;
    private Runnable onHardcoreAimbot;
    private Runnable onFlickBoost;
    private Runnable onEspBox;
    private Runnable onHealthBar;
    private Runnable onGhostMode;
    private Runnable onSafeOverlay;
    
    public FloatingControlPanel(Context context) {
        this.context = context;
        this.windowManager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
        createFloatingView();
    }
    
    private void createFloatingView() {
        // Inflate layout
        floatingView = LayoutInflater.from(context).inflate(R.layout.floating_window, null);
        
        // Get references
        panelCard = floatingView.findViewById(R.id.panelCard);
        
        // Setup switches
        Switch enemySituationSwitch = floatingView.findViewById(R.id.enemySituationSwitch);
        Switch espBoxSwitch = floatingView.findViewById(R.id.espBoxSwitch);
        Switch healthBarSwitch = floatingView.findViewById(R.id.healthBarSwitch);
        Switch aimAssistSwitch = floatingView.findViewById(R.id.aimAssistSwitch);
        Switch hardAimbotSwitch = floatingView.findViewById(R.id.hardAimbotSwitch);
        Switch flickBoostSwitch = floatingView.findViewById(R.id.flickBoostSwitch);
        Switch ghostModeSwitch = floatingView.findViewById(R.id.ghostModeSwitch);
        Switch safeOverlaySwitch = floatingView.findViewById(R.id.safeOverlaySwitch);
        
        // Minimize button
        Button minimizeButton = floatingView.findViewById(R.id.minimizeButton);
        minimizeButton.setOnClickListener(v -> toggleMinimize());
        
        // Set listeners
        if (enemySituationSwitch != null) {
            enemySituationSwitch.setOnCheckedChangeListener((buttonView, isChecked) -> {
                if (onEnemySituation != null) onEnemySituation.run();
            });
        }
        
        if (aimAssistSwitch != null) {
            aimAssistSwitch.setOnCheckedChangeListener((buttonView, isChecked) -> {
                if (onAimAssist != null) onAimAssist.run();
            });
        }
        
        if (hardAimbotSwitch != null) {
            hardAimbotSwitch.setOnCheckedChangeListener((buttonView, isChecked) -> {
                if (onHardcoreAimbot != null) onHardcoreAimbot.run();
            });
        }
        
        if (flickBoostSwitch != null) {
            flickBoostSwitch.setOnCheckedChangeListener((buttonView, isChecked) -> {
                if (onFlickBoost != null) onFlickBoost.run();
            });
        }
        
        if (espBoxSwitch != null) {
            espBoxSwitch.setOnCheckedChangeListener((buttonView, isChecked) -> {
                if (onEspBox != null) onEspBox.run();
            });
        }
        
        if (healthBarSwitch != null) {
            healthBarSwitch.setOnCheckedChangeListener((buttonView, isChecked) -> {
                if (onHealthBar != null) onHealthBar.run();
            });
        }
        
        if (ghostModeSwitch != null) {
            ghostModeSwitch.setOnCheckedChangeListener((buttonView, isChecked) -> {
                if (onGhostMode != null) onGhostMode.run();
            });
        }
        
        if (safeOverlaySwitch != null) {
            safeOverlaySwitch.setOnCheckedChangeListener((buttonView, isChecked) -> {
                if (onSafeOverlay != null) onSafeOverlay.run();
            });
        }
        
        // Setup window parameters
        layoutParams = new WindowManager.LayoutParams(
            WindowManager.LayoutParams.WRAP_CONTENT,
            WindowManager.LayoutParams.WRAP_CONTENT,
            WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY,
            WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE,
            android.graphics.PixelFormat.TRANSLUCENT
        );
    }
    
    private void toggleMinimize() {
        if (isMinimized) {
            expand();
        } else {
            minimize();
        }
    }
    
    private void minimize() {
        if (panelCard != null) {
            panelCard.setVisibility(View.GONE);
            // Show minimized view
        }
        isMinimized = true;
    }
    
    private void expand() {
        if (panelCard != null) {
            panelCard.setVisibility(View.VISIBLE);
        }
        isMinimized = false;
    }
    
    public void show() {
        if (!isVisible && floatingView != null) {
            windowManager.addView(floatingView, layoutParams);
            isVisible = true;
        }
    }
    
    public void hide() {
        if (isVisible && floatingView != null) {
            windowManager.removeView(floatingView);
            isVisible = false;
        }
    }
    
    public void updateStats(int fps, int enemies, String modeText) {
        TextView fpsText = floatingView.findViewById(R.id.fpsValue);
        TextView targetsText = floatingView.findViewById(R.id.targetsValue);
        TextView modeView = floatingView.findViewById(R.id.modeValue);
        
        if (fpsText != null) fpsText.setText(String.valueOf(fps));
        if (targetsText != null) targetsText.setText(String.valueOf(enemies));
        if (modeView != null) modeView.setText(modeText);
    }
    
    public boolean isVisible() {
        return isVisible;
    }
    
    // Setters for callbacks
    public void setOnEnemySituation(Runnable callback) { this.onEnemySituation = callback; }
    public void setOnAimAssist(Runnable callback) { this.onAimAssist = callback; }
    public void setOnHardcoreAimbot(Runnable callback) { this.onHardcoreAimbot = callback; }
    public void setOnFlickBoost(Runnable callback) { this.onFlickBoost = callback; }
    public void setOnEspBox(Runnable callback) { this.onEspBox = callback; }
    public void setOnHealthBar(Runnable callback) { this.onHealthBar = callback; }
    public void setOnGhostMode(Runnable callback) { this.onGhostMode = callback; }
    public void setOnSafeOverlay(Runnable callback) { this.onSafeOverlay = callback; }
}
