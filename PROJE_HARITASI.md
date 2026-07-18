# 🗺️ FS2026 VCU - PROJE HARİTASI (KOD İNDEKSİ)

**Son Güncelleme:** 18 Temmuz 2026

> Bu dosya, projedeki **her** fonksiyonun, kuralın ve verinin tam olarak hangi dosyada
> ve hangi satırda olduğunu gösteren bir "İçindekiler" sayfasıdır.
> Bir şeyi değiştirmek istediğinizde önce buraya bakın, doğru dosya ve satırı bulun.

---

## 📁 DOSYA YAPISI

```text
FS2026_VCU_Software/
│
├── Inc/                            ← TANIMLAMALAR (Header Dosyaları)
│   ├── FS2026_CAN_Dictionary.h     ← CAN Bus Sözlüğü (Mesaj ID'leri ve Paket Yapıları)
│   └── state_machine.h             ← Durum Makinesi Tanımları (Enum, Struct, Prototip)
│
├── Src/                            ← ASIL KODLAR (Source Dosyaları)
│   └── state_machine.c             ← Durum Makinesi Algoritması (Beyin)
│
├── README.md                       ← Proje açıklaması
└── PROJE_HARITASI.md               ← ★ Şu an okuduğunuz dosya ★
```

---

## 🔌 CAN SÖZLÜĞÜ (Cihazların Konuşma Dili)

**Dosya:** `Inc/FS2026_CAN_Dictionary.h`

### Mesaj ID'leri (Kim hangi numarayla konuşuyor?)
| Ne? | Dosya Yolu | Satır |
|-----|-----------|-------|
| VCU → İnverter (Tork komutu) `0x100` | `Inc/FS2026_CAN_Dictionary.h` | **Satır 14** |
| VCU → Tüm Ağ (Araç durumu) `0x101` | `Inc/FS2026_CAN_Dictionary.h` | **Satır 15** |
| VCU → Tüm Ağ (Hata bildirimi) `0x102` | `Inc/FS2026_CAN_Dictionary.h` | **Satır 16** |
| BMS → VCU (Voltaj ve Akım) `0x200` | `Inc/FS2026_CAN_Dictionary.h` | **Satır 17** |
| BMS → VCU, HMI (Pil durumu) `0x201` | `Inc/FS2026_CAN_Dictionary.h` | **Satır 18** |
| İnverter → VCU (Motor RPM, Sıcaklık) `0x300` | `Inc/FS2026_CAN_Dictionary.h` | **Satır 19** |
| İnverter → VCU (Motor hataları) `0x301` | `Inc/FS2026_CAN_Dictionary.h` | **Satır 20** |
| STM Telemetri → VCU (IMU/GPS) `0x400` | `Inc/FS2026_CAN_Dictionary.h` | **Satır 21** |

### Veri Paketleri (8 Byte'ın İçinde Ne Var?)
| Paket Adı | Ne İçeriyor? | Dosya Yolu | Satır |
|-----------|-------------|-----------|-------|
| `CAN_VCU_Control_t` | Tork isteği, İnverter aç/kapa, İleri/Geri | `Inc/FS2026_CAN_Dictionary.h` | **Satır 29-34** |
| `CAN_VCU_Status_t` | Araç durumu, Gaz %, Fren, Direksiyon açısı | `Inc/FS2026_CAN_Dictionary.h` | **Satır 37-44** |
| `CAN_BMS_VoltCurr_t` | Toplam HV Voltajı ve Akımı | `Inc/FS2026_CAN_Dictionary.h` | **Satır 47-51** |
| `CAN_BMS_Status_t` | Pil yüzdesi (SOC), Maks sıcaklık, Hata kodu | `Inc/FS2026_CAN_Dictionary.h` | **Satır 54-60** |
| `CAN_INV_Dynamics_t` | Motor devri (RPM), Motor/İnverter sıcaklığı | `Inc/FS2026_CAN_Dictionary.h` | **Satır 63-68** |
| `CAN_STM_IMU_t` | İvme X/Y, Yaw Rate, GPS durumu | `Inc/FS2026_CAN_Dictionary.h` | **Satır 71-77** |

---

## 🚗 DURUM MAKİNESİ (Araç Beyni)

### Araç Durumları (Araç şu an ne modda?)
**Dosya:** `Inc/state_machine.h`, **Satır 12-19**

| Durum | Değer | Açıklama |
|-------|-------|----------|
| `STATE_INIT` | 0 | Karta elektrik geldi, donanım kuruluyor |
| `STATE_LV_READY` | 1 | 12V hazır, sensörler çalışıyor, HV kapalı |
| `STATE_TS_ACTIVE` | 2 | Yüksek Gerilim (>60V) açıldı, sürüş bekleniyor |
| `STATE_RTD_TRANSITION` | 3 | Buzzer çalıyor (2 saniye) |
| `STATE_DRIVING` | 4 | MOTOR DÖNÜYOR - Sürüş aktif |
| `STATE_FAULT` | 5 | HATA! Motor kilitli, güç kesildi |

### Hata Kodları (Neden hata verdi?)
**Dosya:** `Inc/state_machine.h`, **Satır 22-28**

| Hata | Değer | Ne Oldu? |
|------|-------|----------|
| `FAULT_NONE` | 0 | Hata yok, her şey normal |
| `FAULT_APPS_PLAUSIBILITY` | 1 | İki gaz pedalı sensörü birbirinden %10 farklı |
| `FAULT_BRAKE_THROTTLE` | 2 | Sert fren + %25 gaz aynı anda (FS kuralı EV 5.7) |
| `FAULT_BMS` | 3 | Bataryadan hata geldi |
| `FAULT_SDC_OPEN` | 4 | Güvenlik devresi koptu (E-Stop vb.) |

### Sensör Girişleri (Koda dışarıdan ne besleniyor?)
**Dosya:** `Inc/state_machine.h`, **Satır 32-40**

| Girdi | Tip | Açıklama |
|-------|-----|----------|
| `appsPercent` | uint8 | Gaz pedalı pozisyonu (0-100%) |
| `brakePressure` | uint8 | Fren basıncı (0-255 Bar) |
| `tsVoltage` | uint16 | Batarya toplam voltajı (Volt) |
| `startButtonPressed` | bool | Start butonu basılı mı? |
| `resetButtonPressed` | bool | Hata sıfırlama butonu basılı mı? |
| `sdcClosed` | bool | Güvenlik devresi kapalı (güvenli) mi? |
| `externalFault` | FaultCode | Dışarıdan gelen hata kodu |

### Çıktılar (Kod arabaya ne emri veriyor?)
**Dosya:** `Inc/state_machine.h`, **Satır 43-49**

| Çıktı | Tip | Açıklama |
|-------|-----|----------|
| `currentState` | enum | Şu anki araç durumu |
| `rtdBuzzerOn` | bool | Buzzer çalsın mı? |
| `inverterEnable` | bool | Motor İnverteri açılsın mı? |
| `torqueCommand` | int16 | Motora kaç birim tork versin? |
| `activeFault` | FaultCode | Şu an aktif olan hata kodu |

---

## ⚙️ ALGORİTMALAR (Kod Ne Yapıyor, Nerede?)

**Dosya:** `Src/state_machine.c`

### Fonksiyonlar
| Fonksiyon | Ne İş Yapar? | Dosya Yolu | Satır |
|-----------|-------------|-----------|-------|
| `CheckForErrors()` | Her döngüde hata var mı diye kontrol eder | `Src/state_machine.c` | **Satır 4-27** |
| `SM_Init()` | Sistemi ilk açılışta güvenli değerlere getirir | `Src/state_machine.c` | **Satır 30-41** |
| `SM_Update()` | ANA DÖNGÜ: Durumlar arası geçişleri yönetir | `Src/state_machine.c` | **Satır 44-151** |

### FS Yarışma Kuralları (Kodun İçine Gömülü Kurallar)
| Kural Numarası | Kural Adı | Ne Yapıyor? | Dosya Yolu | Satır |
|----------------|-----------|------------|-----------|-------|
| **EV 5.7** | Fren-Gaz Çakışması | Fren >30 Bar + Gaz >%25 ise tork kesilir | `Src/state_machine.c` | **Satır 15-24** |
| **EV 4.12.1** | RTD Koşulları | Fren >%15 basılı + Start butonu → Buzzer | `Src/state_machine.c` | **Satır 90-103** |
| **EV 4.12.3** | RTD Buzzer Süresi | 2 saniye kesintisiz ses → Sürüş moduna geç | `Src/state_machine.c` | **Satır 107-117** |

### Durum Geçişleri (Hangi durum nereye geçiyor?)
| Geçiş | Şart | Dosya Yolu | Satır |
|--------|------|-----------|-------|
| INIT → LV_READY | Donanım kurulumu bitti | `Src/state_machine.c` | **Satır 64-67** |
| LV_READY → TS_ACTIVE | BMS voltajı > 60V | `Src/state_machine.c` | **Satır 74-77** |
| TS_ACTIVE → RTD_TRANSITION | Fren > %15 VE Start basılı | `Src/state_machine.c` | **Satır 96-103** |
| RTD_TRANSITION → DRIVING | 2 saniye Buzzer tamamlandı | `Src/state_machine.c` | **Satır 113-117** |
| DRIVING → TS_ACTIVE | Start butonuna tekrar basıldı | `Src/state_machine.c` | **Satır 129-131** |
| HERHANGİ → FAULT | Hata tespit edildi | `Src/state_machine.c` | **Satır 48-51** |
| FAULT → LV_READY | Hata gitti + Reset butonu | `Src/state_machine.c` | **Satır 142-145** |

---

## 🔧 BİR ŞEYİ DEĞİŞTİRMEK İSTİYORSAN

| Yapmak İstediğin | Git Şuraya |
|-------------------|-----------|
| Motora giden tork miktarını ayarlamak | `Src/state_machine.c` → **Satır 126** |
| Buzzer süresini değiştirmek (2 sn → 3 sn) | `Src/state_machine.c` → **Satır 113** (2000 değerini 3000 yap) |
| Fren eşik değerini değiştirmek (%15) | `Src/state_machine.c` → **Satır 96** (15 değerini değiştir) |
| Fren-Gaz çakışma eşiğini değiştirmek | `Src/state_machine.c` → **Satır 20** (30 ve 25 değerlerini değiştir) |
| Yeni bir CAN mesajı eklemek | `Inc/FS2026_CAN_Dictionary.h` → **Satır 21'in altına** yeni `#define` ekle |
| Yeni bir hata türü eklemek | `Inc/state_machine.h` → **Satır 27'nin altına** yeni `FAULT_...` ekle |
| Yeni bir sensör girişi eklemek | `Inc/state_machine.h` → **Satır 39'un altına** yeni değişken ekle |
