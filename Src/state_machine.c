#include "state_machine.h"

// Yardımcı Fonksiyon: Hataları kontrol eder
static FaultCode_t CheckForErrors(StateMachine_t *sm) {
    // 1. Dış Hatalar (BMS Hatası vb.)
    if (sm->inputs.externalFault != FAULT_NONE) {
        return sm->inputs.externalFault;
    }
    
    // 2. Güvenlik Devresi (SDC) Koptuysa
    if (!sm->inputs.sdcClosed) {
        return FAULT_SDC_OPEN;
    }
    
    // =========================================================================
    // FS KURALI: EV 5.7 (FREN VE GAZ ÇAKIŞMASI - APPS PLAUSIBILITY)
    // Sürücü sert frene basarken (Örn: >30 Bar) aynı anda gaza basıyorsa (Örn: >%25)
    // Motora giden güç ANINDA kesilmelidir. (Hata durumuna düşürülür)
    // =========================================================================
    if (sm->inputs.brakePressure > 30 && sm->inputs.appsPercent > 25) {
        // Not: Kurala göre gaz pedalı %5'in altına düşene kadar bu hata KALICI olmalıdır.
        // Şimdilik sadece hatayı tetikleme şartını yazıyoruz.
        return FAULT_BRAKE_THROTTLE;
    }
    
    return FAULT_NONE;
}

// Durum Makinesini Başlatır
void SM_Init(StateMachine_t *sm) {
    sm->currentState = STATE_INIT;
    sm->previousState = STATE_INIT;
    sm->rtdTimerMs = 0;
    sm->isRtdTimerActive = false;
    
    // Çıktıları güvenli değerlere çek
    sm->outputs.inverterEnable = false;
    sm->outputs.rtdBuzzerOn = false;
    sm->outputs.torqueCommand = 0;
    sm->outputs.activeFault = FAULT_NONE;
}

// Durum Makinesi Ana Döngüsü (Örn: Her 10ms'de bir çağrılır)
void SM_Update(StateMachine_t *sm, uint32_t deltaTimeMs) {
    // 1. ÖNCE HATA KONTROLÜ (En yüksek öncelik)
    FaultCode_t currentFault = CheckForErrors(sm);
    
    if (currentFault != FAULT_NONE) {
        sm->currentState = STATE_FAULT;
        sm->outputs.activeFault = currentFault;
    }

    // Zamanlayıcıyı güncelle
    if (sm->isRtdTimerActive) {
        sm->rtdTimerMs += deltaTimeMs;
    }

    // Önceki durumu kaydet (Değişimleri yakalamak için)
    sm->previousState = sm->currentState;

    // 2. DURUM GEÇİŞLERİ (State Machine Mantığı)
    switch (sm->currentState) {
        
        case STATE_INIT:
            // Donanım testleri burada yapılır. Şimdilik direkt geçiyoruz.
            sm->currentState = STATE_LV_READY;
            break;

        case STATE_LV_READY:
            // Tork ve Inverter kapalı kalmalı
            sm->outputs.inverterEnable = false;
            sm->outputs.torqueCommand = 0;
            
            // TS (Tractive System) Voltajı 60V'u aştıysa aktif duruma geç
            if (sm->inputs.tsVoltage > 60) {
                sm->currentState = STATE_TS_ACTIVE;
            }
            break;

        case STATE_TS_ACTIVE:
            sm->outputs.inverterEnable = false;
            sm->outputs.torqueCommand = 0;
            
            // Güvenlik: Araç geriye düşerse (TS < 60V) LV_READY'ye dön
            if (sm->inputs.tsVoltage < 60) {
                sm->currentState = STATE_LV_READY;
                break;
            }
            
            // =========================================================================
            // FS KURALI: EV 4.12.1 (SÜRÜŞE HAZIR OLMA - READY TO DRIVE SARTLARI)
            // Araba sadece ve sadece şu şartlar aynı anda sağlanırsa çalışır:
            // 1) Sürücü frene belirli bir güçle basıyor olmalı (> %15)
            // 2) Start butonuna basılmış olmalı.
            // =========================================================================
            if (sm->inputs.brakePressure > 15 && sm->inputs.startButtonPressed) {
                sm->currentState = STATE_RTD_TRANSITION;
                
                // Zamanlayıcıyı sıfırla ve Buzzer'ı öttür!
                sm->rtdTimerMs = 0;
                sm->isRtdTimerActive = true;
                sm->outputs.rtdBuzzerOn = true; 
            }
            break;

        case STATE_RTD_TRANSITION:
            // =========================================================================
            // FS KURALI: EV 4.12.3 (RTD SESLİ UYARI - BUZZER)
            // Sürüş moduna geçmeden hemen önce, etraftaki mekanikerleri uyarmak için
            // 1 saniye ile 3 saniye arası kesintisiz zil (buzzer) çalmalıdır.
            // Biz burada tam 2 saniye (2000 ms) çalacak şekilde ayarladık.
            // =========================================================================
            if (sm->rtdTimerMs >= 2000) {
                sm->outputs.rtdBuzzerOn = false;  // Sesi kapat
                sm->isRtdTimerActive = false;     // Sayacı durdur
                sm->currentState = STATE_DRIVING; // ARTIK MOTOR DÖNEBİLİR!
            }
            break;

        case STATE_DRIVING:
            // SÜRÜŞ MODU! Inverter aktif edilir ve APPS değerine göre tork verilir.
            sm->outputs.inverterEnable = true;
            
            // Tork hesaplama (Örnek: %100 gaz = 32000 Nm komut)
            // Gerçek projede burada Torque Vectoring algoritmaları çalışacaktır.
            sm->outputs.torqueCommand = (sm->inputs.appsPercent * 32000) / 100;
            
            // Sürücü tekrar Start butonuna basarsa aracı kapat (TS_ACTIVE'e dön)
            if (sm->inputs.startButtonPressed) {
                sm->currentState = STATE_TS_ACTIVE;
            }
            break;

        case STATE_FAULT:
            // HATA DURUMU! Aracı kilitle.
            sm->outputs.inverterEnable = false;
            sm->outputs.torqueCommand = 0;
            sm->outputs.rtdBuzzerOn = false;
            sm->isRtdTimerActive = false;
            
            // Hata giderildiyse ve Reset butonuna basıldıysa geri dön
            if (CheckForErrors(sm) == FAULT_NONE && sm->inputs.resetButtonPressed) {
                sm->outputs.activeFault = FAULT_NONE;
                sm->currentState = STATE_LV_READY;
            }
            break;
    }
    
    // Son durumu çıktılara yansıt (Bu veriler CAN üzerinden HMI'a basılacak)
    sm->outputs.currentState = sm->currentState;
}
