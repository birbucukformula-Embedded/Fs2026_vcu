# FS2026 ARAÇ İÇİ HABERLEŞME AĞI (SİSTEM MİMARİSİ)
# ══════════════════════════════════════════════════════════════
# Bu dosya aracın yazılımı geliştikçe aşama aşama güncellenecektir.
# Şu an sadece koda döktüğümüz ve aktif olan kısımları içermektedir.
# Son Güncelleme: 18 Temmuz 2026
# ══════════════════════════════════════════════════════════════

Aşağıdaki şemalar, arabadaki cihazların birbirlerine HANGİ KABLO/PROTOKOL ile bağlandığını ve HANGİ VERİLERİ gönderdiğini gösterir.


## 1. DONANIM BAĞLANTI ŞEMASI (Fiziksel)

```text
  [ GAZ PEDALI (APPS) ] ---------(Analog Voltaj)--------┐
                                                        │
  [ FREN SENSÖRÜ ] -------------(Analog Voltaj)---------┤
                                                        │
  [ START / RESET BUTONU ] -----(Dijital 1/0)-----------┤
                                                        ▼
                                           ╔══════════════════════╗
                                           ║      ANA BEYİN       ║
  [ BUZZER (RTD Ses) ] <-------(12V PWM)---║        (VCU)         ║
                                           ║   (state_machine.c)  ║
                                           ║  (torque_control.c)  ║
                                           ╚══════════════════════╝
                                                        │
                                                        │ (CAN BUS HATTI)
                                                        ▼
                                           [ İNVERTER / MOTOR SÜRÜCÜ ]
```


## 2. CAN BUS VERİ AKIŞI (Mantıksal)
Şu ana kadar sadece VCU -> İnverter yönündeki iletişim mantığı koda dökülmüştür.

### 🚗 VCU (Ana Beyin) Ne Gönderiyor?

**Kime:** İnverter
**Nasıl:** CAN Bus (ID: `0x100`)
**Ne Sıklıkla:** Henüz zamanlayıcı yazılmadı.
**Paket İçeriği (`CAN_VCU_Control_t`):**
- **Tork Komutu:** Gaz pedalı yüzdesine, güvenlik limitlerine (Örn: Motor ısınırsa) ve Regen durumuna göre VCU tarafından hesaplanır.
- **İnverter İzni (Enable):** VCU `STATE_DRIVING` moduna geçmeden İnverter'e güç açma izni vermez.
- **Yön:** İleri/Geri


### 🧠 VCU Kendi İçinde Kararları Nasıl Alıyor? (Internal Flow)

```text
 1. GİRDİLER           2. KARAR (state_machine.c)          3. ÇIKTILAR (torque_control.c)
 ───────────           ──────────────────────────          ──────────────────────────────
 - Gaz Yüzdesi     →   Eğer Fren+Gaz çakışırsa    →        HATA VER (Tork = 0)
 - Fren Basıncı    →   Fren>15 + Start basılırsa  →        BUZZER ÇAL (2 sn) -> DRIVING geç
 - TS Voltajı      →   DRIVING modundaysa         →        TORK HESAPLA -> İnverter'e gönder
```


---
> **Not:** Sistemimize BMS (Batarya), Telemetri ve Sürücü Ekranı modülleri eklendikçe bu ağ şeması genişleyecektir.
