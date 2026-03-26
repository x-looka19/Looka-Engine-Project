package com.looka.engine;

import android.animation.Animator;
import android.animation.AnimatorListenerAdapter;
import android.animation.ValueAnimator;
import android.content.Context;
import android.graphics.PixelFormat;
import android.os.Build;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.LinearLayout;
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
    private int lastX, lastY;
    private int screenWidth, screenHeight;
    
    public FloatingControlPanel(Context context) {
        this.context = context;
        this.windowManager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
        createFloatingView();
    }
    
    private void createFloatingView() {
        // Inflate layout
        LayoutInflater inflater = LayoutInflater.from(context);
        floatingView = inflater.inflate(R.layout.floating_window, null);
        
        // Setup panel card
        panelCard = floatingView.findViewById(R.id.panelCard);
        
        // Setup buttons and switches
        setupControls();
        
        // Setup drag functionality
        setupDragListener();
        
        // Setup window parameters
        screenWidth = windowManager.getDefaultDisplay().getWidth();
        screenHeight = windowManager.getDefaultDisplay().getHeight();
        
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            layoutParams = new WindowManager.LayoutParams(
                WindowManager.LayoutParams.WRAP_CONTENT,
                WindowManager.LayoutParams.WRAP_CONTENT,
                WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY,
                WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE,
                PixelFormat.TRANSLUCENT
            );
        } else {
            layoutParams = new WindowManager.LayoutParams(
                WindowManager.LayoutParams.WRAP_CONTENT,
                WindowManager.LayoutParams.WRAP_CONTENT,
                WindowManager.LayoutParams.TYPE_PHONE,
                WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE,
                PixelFormat.TRANSLUCENT
            );
        }
        
        layoutParams.gravity = Gravity.TOP | Gravity.START;
        layoutParams.x = 50;
        layoutParams.y = 200;
    }
    
    private void setupControls() {
        // ESP Controls
        Switch enemySituationSwitch = floatingView.findViewById(R.id.enemySituationSwitch);
        Switch espBoxSwitch = floatingView.findViewById(R.id.espBoxSwitch);
        Switch healthBarSwitch = floatingView.findViewById(R.id.healthBarSwitch);
        
        // Aim Controls
        Switch aimAssistSwitch = floatingView.findViewById(R.id.aimAssistSwitch);
        Switch hardAimbotSwitch = floatingView.findViewById(R.id.hardAimbotSwitch);
        Switch flickBoostSwitch = floatingView.findViewById(R.id.flickBoostSwitch);
        
        // Stealth Controls
        Switch ghostModeSwitch = floatingView.findViewById(R.id.ghostModeSwitch);
        Switch safeOverlaySwitch = floatingView.findViewById(R.id.safeOverlaySwitch);
        
        // Performance Controls
        Switch cacheSwitch = floatingView.findViewById(R.id.cacheSwitch);
        Switch refreshSwitch = floatingView.findViewById(R.id.refreshSwitch);
        
        // Minimize button
        Button minimizeButton = floatingView.findViewById(R.id.minimizeButton);
        minimizeButton.setOnClickListener(v -> toggleMinimize());
        
        // Set listeners
        enemySituationSwitch.setOnCheckedChangeListener((buttonView, isChecked) -> {
            LookaEngineService.getInstance().enableESP(isChecked);
        });
        
        aimAssistSwitch.setOnCheckedChangeListener((buttonView, isChecked) -> {
            LookaEngineService.getInstance().enableAimAssist(isChecked);
        });
        
        hardAimbotSwitch.setOnCheckedChangeListener((buttonView, isChecked) -> {
            LookaEngineService.getInstance().enableHardcoreAimbot(isChecked);
        });
        
        flickBoostSwitch.setOnCheckedChangeListener((buttonView, isChecked) -> {
            LookaEngineService.getInstance().enableFlickBoost(isChecked);
        });
    }
    
    private void setupDragListener() {
        floatingView.setOnTouchListener(new View.OnTouchListener() {
            private int initialX, initialY;
            private float initialTouchX, initialTouchY;
            
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                switch (event.getAction()) {
                    case MotionEvent.ACTION_DOWN:
                        initialX = layoutParams.x;
                        initialY = layoutParams.y;
                        initialTouchX = event.getRawX();
                        initialTouchY = event.getRawY();
                        return true;
                        
                    case MotionEvent.ACTION_MOVE:
                        layoutParams.x = initialX + (int) (event.getRawX() - initialTouchX);
                        layoutParams.y = initialY + (int) (event.getRawY() - initialTouchY);
                        
                        // Keep within screen bounds
                        if (layoutParams.x < 0) layoutParams.x = 0;
                        if (layoutParams.x > screenWidth - floatingView.getWidth()) 
                            layoutParams.x = screenWidth - floatingView.getWidth();
                        if (layoutParams.y < 0) layoutParams.y = 0;
                        if (layoutParams.y > screenHeight - floatingView.getHeight()) 
                            layoutParams.y = screenHeight - floatingView.getHeight();
                        
                        windowManager.updateViewLayout(floatingView, layoutParams);
                        return true;
                        
                    case MotionEvent.ACTION_UP:
                        return true;
                }
                return false;
            }
        });
    }
    
    private void toggleMinimize() {
        if (isMinimized) {
            expand();
        } else {
            minimize();
        }
    }
    
    private void minimize() {
        // Animate to minimized state
        ValueAnimator animator = ValueAnimator.ofInt(panelCard.getHeight(), 60);
        animator.addUpdateListener(animation -> {
            int val = (int) animation.getAnimatedValue();
            panelCard.getLayoutParams().height = val;
            panelCard.requestLayout();
        });
        animator.setDuration(200);
        animator.start();
        
        // Hide content
        ScrollView content = floatingView.findViewById(R.id.scrollContent);
        content.setVisibility(View.GONE);
        
        isMinimized = true;
    }
    
    private void expand() {
        // Show content
        ScrollView content = floatingView.findViewById(R.id.scrollContent);
        content.setVisibility(View.VISIBLE);
        
        // Animate to expanded state
        ValueAnimator animator = ValueAnimator.ofInt(panelCard.getHeight(), 
            WindowManager.LayoutParams.WRAP_CONTENT);
        animator.addUpdateListener(animation -> {
            int val = (int) animation.getAnimatedValue();
            panelCard.getLayoutParams().height = val;
            panelCard.requestLayout();
        });
        animator.setDuration(200);
        animator.start();
        
        isMinimized = false;
    }
    
    public void show() {
        if (!isVisible) {
            windowManager.addView(floatingView, layoutParams);
            isVisible = true;
        }
    }
    
    public void hide() {
        if (isVisible) {
            windowManager.removeView(floatingView);
            isVisible = false;
        }
    }
    
    public void updateStats(int fps, int targets, int mode) {
        TextView fpsText = floatingView.findViewById(R.id.fpsValue);
        TextView targetsText = floatingView.findViewById(R.id.targetsValue);
        TextView modeText = floatingView.findViewById(R.id.modeValue);
        
        fpsText.setText(String.valueOf(fps));
        targetsText.setText(String.valueOf(targets));
        
        if (mode == 0) {
            modeText.setText("ULTRA");
            modeText.setTextColor(android.graphics.Color.parseColor("#FFD700"));
        } else {
            modeText.setText("STANDARD");
            modeText.setTextColor(android.graphics.Color.WHITE);
        }
    }
    
    public boolean isVisible() {
        return isVisible;
    }
}