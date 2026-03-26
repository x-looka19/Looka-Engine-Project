package com.looka.engine;

import android.content.Context;
import android.util.Log;

/**
 * NativeBridge - يربط بين كود Java وكود C++
 * جميع الدوال هنا تستدعي دوال C++ عبر JNI
 */
public class NativeBridge {
    
    private static final String TAG = "LookaEngine";
    
    // تحميل مكتبة C++
    static {
        try {
            System.loadLibrary("looka_engine");
            Log.i(TAG, "Native library loaded successfully");
        } catch (UnsatisfiedLinkError e) {
            Log.e(TAG, "Failed to load native library: " + e.getMessage());
        }
    }
    
    // ============= دوال التحكم الرئيسية =============
    
    /**
     * تهيئة المحرك
     * @param context سياق التطبيق
     */
    public static native void initEngine(Context context);
    
    /**
     * التحقق من وجود صلاحيات الرووت
     * @return true إذا كان الجهاز يحتوي على Root
     */
    public static native boolean checkRootAccess();
    
    /**
     * تشغيل المحرك
     * @return true إذا تم التشغيل بنجاح
     */
    public static native boolean startEngine();
    
    /**
     * إيقاف المحرك
     */
    public static native void stopEngine();
    
    /**
     * الحصول على عدد الإطارات في الثانية (FPS)
     * @return عدد الإطارات
     */
    public static native int getCurrentFPS();
    
    /**
     * الحصول على عدد الأعداء المكتشفين
     * @return عدد الأعداء
     */
    public static native int getEnemyCount();
    
    // ============= دوال التحكم في ESP =============
    
    /**
     * تفعيل/إيقاف ESP بشكل عام
     * @param enable true = تفعيل, false = إيقاف
     */
    public static native void enableESP(boolean enable);
    
    /**
     * تفعيل/إيقاف مربعات ESP
     * @param enable true = تفعيل, false = إيقاف
     */
    public static native void enableESPBox(boolean enable);
    
    /**
     * تفعيل/إيقاف شريط الدم
     * @param enable true = تفعيل, false = إيقاف
     */
    public static native void enableHealthBar(boolean enable);
    
    /**
     * تفعيل/إيقاف عرض المسافة
     * @param enable true = تفعيل, false = إيقاف
     */
    public static native void enableDistance(boolean enable);
    
    /**
     * تفعيل/إيقاف وضع الأعداء (يخفي كل عناصر ESP دفعة واحدة)
     * @param enable true = تفعيل, false = إيقاف
     */
    public static native void setEnemySituation(boolean enable);
    
    // ============= دوال التحكم في التصويب =============
    
    /**
     * تفعيل/إيقاف مساعد التصويب (Aim Assist)
     * @param enable true = تفعيل, false = إيقاف
     */
    public static native void enableAimAssist(boolean enable);
    
    /**
     * تفعيل/إيقاف التصويب التلقائي (Hard Aimbot)
     * @param enable true = تفعيل, false = إيقاف
     */
    public static native void enableHardcoreAimbot(boolean enable);
    
    /**
     * تفعيل/إيقاف تعزيز النترات (Flick Boost)
     * @param enable true = تفعيل, false = إيقاف
     */
    public static native void enableFlickBoost(boolean enable);
    
    /**
     * ضبط نعومة التصويب (0 = فوري, 10 = بطيء)
     * @param smoothness قيمة النعومة (0-10)
     */
    public static native void setAimSmoothness(float smoothness);
    
    /**
     * ضبط مجال الرؤية للتصويب (FOV)
     * @param fov قيمة المجال (0-180)
     */
    public static native void setAimFOV(float fov);
    
    // ============= دوال التحكم في التخفي =============
    
    /**
     * تفعيل/إيقاف وضع الشبح (إخفاء العملية)
     * @param enable true = تفعيل, false = إيقاف
     */
    public static native void enableGhostMode(boolean enable);
    
    /**
     * تفعيل/إيقاف الوضع الآمن (منع تصوير الشاشة)
     * @param enable true = تفعيل, false = إيقاف
     */
    public static native void enableSafeOverlay(boolean enable);
    
    /**
     * تفعيل/إيقاف الحماية متعددة الأشكال
     * @param enable true = تفعيل, false = إيقاف
     */
    public static native void enablePolymorphic(boolean enable);
    
    // ============= دوال التحكم في الأداء =============
    
    /**
     * ضبط حجم الذاكرة المؤقتة
     * @param sizeMB الحجم بالميجابايت (4, 8, 16, 32)
     */
    public static native void setCacheSize(int sizeMB);
    
    /**
     * ضبط وضع التحديث
     * @param mode 0=تلقائي, 1=معركة (120 FPS), 2=قائمة (30 FPS)
     */
    public static native void setRefreshMode(int mode);
    
    /**
     * مسح الذاكرة المؤقتة
     */
    public static native void clearCache();
    
    // ============= دوال التحديث والإحصائيات =============
    
    /**
     * الحصول على نسبة نجاح الذاكرة المؤقتة
     * @return نسبة النجاح (0-100)
     */
    public static native float getCacheHitRate();
    
    /**
     * الحصول على سرعة مسح الأنماط
     * @return السرعة بالميجابايت/ثانية
     */
    public static native float getScanSpeed();
    
    /**
     * الحصول على وضع التشغيل الحالي
     * @return 0=رووت, 1=بدون رووت
     */
    public static native int getCurrentMode();
    
    /**
     * الحصول على حالة المحرك
     * @return 0=متوقف, 1=يعمل, 2=خطأ
     */
    public static native int getEngineState();
    
    // ============= دوال النافذة العائمة =============
    
    /**
     * إظهار النافذة العائمة
     */
    public static native void showFloatingWindow();
    
    /**
     * إخفاء النافذة العائمة
     */
    public static native void hideFloatingWindow();
    
    /**
     * تحديث إحصائيات النافذة العائمة
     * @param fps عدد الإطارات
     * @param enemies عدد الأعداء
     * @param modeText نص الوضع
     */
    public static native void updateFloatingStats(int fps, int enemies, String modeText);
    
    // ============= دوال مساعدة =============
    
    /**
     * الحصول على إصدار المحرك
     * @return نص الإصدار
     */
    public static native String getEngineVersion();
    
    /**
     * الحصول على آخر خطأ
     * @return نص الخطأ
     */
    public static native String getLastError();
    
    /**
     * إعادة ضبط المحرك
     */
    public static native void resetEngine();
}