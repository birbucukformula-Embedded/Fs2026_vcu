# FS2026 VCU - İÇİNDEKİLER
# ══════════════════════════════════════════════════════════════
# Her kod değişikliğinde bu dosya güncellenecektir.
# Son Güncelleme: 18 Temmuz 2026
# ══════════════════════════════════════════════════════════════


# ┌─────────────────────────────────────────────────────────────
# │  Inc/FS2026_CAN_Dictionary.h  (CAN Bus Sözlüğü)
# ├─────────────────────────────────────────────────────────────
# │
# │   Satır 13-21 .... CAN Mesaj ID Numaraları
# │     ├── 0x100  VCU_Control    (VCU → İnverter)         :14
# │     ├── 0x101  VCU_Status     (VCU → Ekran, Telemetri) :15
# │     ├── 0x102  VCU_Faults     (VCU → Tüm Ağ)          :16
# │     ├── 0x200  BMS_Volt_Curr  (BMS → VCU)              :17
# │     ├── 0x201  BMS_Status     (BMS → VCU, Ekran)       :18
# │     ├── 0x300  INV_Dynamics   (İnverter → VCU, Ekran)  :19
# │     ├── 0x301  INV_Faults     (İnverter → VCU)         :20
# │     └── 0x400  STM_IMU        (Telemetri → VCU)        :21
# │
# │   Satır 29-34 .... CAN_VCU_Control_t   → Tork komutu, İnverter aç/kapa
# │   Satır 37-44 .... CAN_VCU_Status_t    → Araç durumu, Gaz %, Fren, SDC
# │   Satır 47-51 .... CAN_BMS_VoltCurr_t  → HV Voltaj ve Akım
# │   Satır 54-60 .... CAN_BMS_Status_t    → SOC %, Sıcaklık, Hata kodu
# │   Satır 63-68 .... CAN_INV_Dynamics_t  → Motor RPM, Sıcaklıklar
# │   Satır 71-77 .... CAN_STM_IMU_t       → İvme X/Y, Yaw Rate, GPS
# │
# └─────────────────────────────────────────────────────────────


# ┌─────────────────────────────────────────────────────────────
# │  Inc/state_machine.h  (Durum Makinesi Tanımları)
# ├─────────────────────────────────────────────────────────────
# │
# │   Satır 12-19 .... VehicleState_t (Araç Durumları)
# │     ├── 0 = STATE_INIT             :13
# │     ├── 1 = STATE_LV_READY         :14
# │     ├── 2 = STATE_TS_ACTIVE        :15
# │     ├── 3 = STATE_RTD_TRANSITION   :16
# │     ├── 4 = STATE_DRIVING          :17
# │     └── 5 = STATE_FAULT            :18
# │
# │   Satır 22-28 .... FaultCode_t (Hata Kodları)
# │     ├── 0 = FAULT_NONE                :23
# │     ├── 1 = FAULT_APPS_PLAUSIBILITY   :24
# │     ├── 2 = FAULT_BRAKE_THROTTLE      :25
# │     ├── 3 = FAULT_BMS                 :26
# │     └── 4 = FAULT_SDC_OPEN            :27
# │
# │   Satır 32-40 .... VCU_Inputs_t (Sensör Girişleri)
# │   Satır 43-49 .... VCU_Outputs_t (Araç Çıktıları)
# │   Satır 52-60 .... StateMachine_t (Ana Yapı)
# │   Satır 65-66 .... Fonksiyon Prototipleri (SM_Init, SM_Update)
# │
# └─────────────────────────────────────────────────────────────


# ┌─────────────────────────────────────────────────────────────
# │  Inc/torque_control.h & Src/torque_control.c  (Tork Hesaplamaları)
# ├─────────────────────────────────────────────────────────────
# │
# │   Satır 12-14 .... TC_CalculateTorque() Prototipleri (Header)
# │
# │   Satır 4-15  .... TC_ApplyTorqueMap()  — Ölü bölge (Deadzone) ve Gaz→Tork dönüşümü
# │   Satır 18-35 .... TC_ApplyRegen()      — Rejeneratif frenleme (Motoru jeneratör yap)
# │   Satır 38-55 .... TC_ApplySafetyLimits() — Pil sıcaksa veya şarj bitiyorsa torku kıs
# │
# │   Satır 59-74 .... TC_CalculateTorque() — ANA ÇAĞRI FONKSİYONU
# │     ├── :63      Gaza basılıyorsa → TC_ApplyTorqueMap()
# │     ├── :67      Gazdan çekildiyse → TC_ApplyRegen()
# │     └── :71      Son olarak → TC_ApplySafetyLimits() uygula ve İnverter'e gönder
# │
# └─────────────────────────────────────────────────────────────


# ┌─────────────────────────────────────────────────────────────
# │  Src/state_machine.c  (Ana Beyin Algoritması)
# ├─────────────────────────────────────────────────────────────
# │
# │   Satır 4-27  .... CheckForErrors()  — Hata kontrol fonksiyonu
# │     ├── :6-8     Dış hata kontrolü (BMS vb.)
# │     ├── :10-13   SDC (Güvenlik Devresi) kopma kontrolü
# │     └── :15-24   ★ FS KURALI EV 5.7: Fren-Gaz çakışması
# │
# │   Satır 30-41 .... SM_Init()  — Sistemi güvenli başlatma
# │
# │   Satır 44-151 ... SM_Update()  — ANA DÖNGÜ (switch-case)
# │     │
# │     ├── :64-67   STATE_INIT → LV_READY geçişi
# │     │
# │     ├── :69-78   STATE_LV_READY
# │     │   └── Voltaj > 60V ise → TS_ACTIVE            :75
# │     │
# │     ├── :80-104  STATE_TS_ACTIVE
# │     │   ├── Voltaj < 60V ise → LV_READY'ye geri dön :85
# │     │   └── ★ FS KURALI EV 4.12.1: RTD Şartları     :90-103
# │     │       └── Fren > %15 + Start basılı → Buzzer   :96
# │     │
# │     ├── :106-118 STATE_RTD_TRANSITION
# │     │   └── ★ FS KURALI EV 4.12.3: Buzzer 2sn        :107-117
# │     │       └── 2000ms dolunca → DRIVING moduna geç   :113
# │     │
# │     ├── :120-132 STATE_DRIVING (Sürüş Modu)
# │     │   ├── Tork hesaplama (APPS × 32000 / 100)       :126
# │     │   └── Start tekrar basılırsa → TS_ACTIVE         :129
# │     │
# │     └── :134-146 STATE_FAULT (Hata Durumu)
# │         ├── Tork = 0, İnverter = kapalı                :136-139
# │         └── Hata gitti + Reset basılı → LV_READY       :142
# │
# └─────────────────────────────────────────────────────────────


# ══════════════════════════════════════════════════════════════
# HIZLI ERİŞİM: BİR ŞEYİ DEĞİŞTİRMEK İSTİYORSAN
# ══════════════════════════════════════════════════════════════
#
#   Tork miktarını ayarla ............. state_machine.c :126
#   Buzzer süresini değiştir .......... state_machine.c :113  (2000 → ?)
#   RTD fren eşiğini değiştir ........ state_machine.c :96   (15 → ?)
#   Fren-Gaz çakışma eşiği ........... state_machine.c :20   (30 ve 25)
#   Yeni CAN mesajı ekle ............. FS2026_CAN_Dictionary.h :21 altına
#   Yeni hata türü ekle .............. state_machine.h :27 altına
#   Yeni sensör girişi ekle .......... state_machine.h :39 altına
#
# ══════════════════════════════════════════════════════════════
