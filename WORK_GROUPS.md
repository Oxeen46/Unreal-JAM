# İş alanları

Bu dosya, 3D Unreal oyunu için işleri genel başlıklarda toplar. İşleri ekip seçip kendine atar; kişi ataması veya sabit grup dağılımı yapılmamıştır. Tasarıma bağlı bir başlığın listede olması, oyunda kesin bulunacağı anlamına gelmez.

## 1. Oyun tasarımı ve içerik

- Oyuncunun amacı, temel döngü ve kazanma/kaybetme koşulları.
- Birkaç filmin kapsamı, içerikleri ve birbirleriyle bağlantısı.
- Senaryo/film sisteminin yöntemi ve verileri; yöntem henüz seçilmedi.
- Metinler, varsa diyaloglar ve ipuçları; İngilizce oyun içeriği.
- Kapsam dışı işler ve açık tasarım soruları.

Oyun fikrini ekip 1-MEVCUT_FIKIR_VE_KARARLAR.md içine elle yazar; ajan tasarımı kendiliğinden doldurmaz.

## 2. Oyuncu, oynanış ve karakterler

- Oyuncu hareketi, kamera, kontroller ve etkileşim.
- Kullanılabilir nesneler; tasarımda varsa toplama, taşıma ve envanter.
- Tasarımda varsa saldırı, can/hasar ve çatışma dengesi.
- Tasarımda varsa düşman/NPC davranışı, algılama ve navigasyon.
- Karakter modelleri, rig, animasyonlar ve C++ bağlantıları.

Ortak etkileşim/hasar arayüzleri önceden anlaşılır; aynı header veya sınıfı iki kişi eşzamanlı değiştirmez.

## 3. Dünya ve görsel içerik

- Bölüm bloklaması, keşif alanları, yönlendirme ve yerleşim.
- Çevre modelleri, UV, dokular, materyaller, collision ve LOD.
- Işıklandırma, atmosfer, post-process ve görsel tutarlılık.
- Tasarımda varsa dünya değişimleri ve bölüm/film görsel geçişleri.
- Hazır C++ aktörlerinin ve içeriklerin ana haritaya yerleştirilmesi.

Ana haritayı yalnız sorumlusu düzenler. Diğer kişiler kendi test haritalarında çalışır. Model/materyal gibi içerikler için de dosya sahipliği belirlenir.

## 4. Oyun sistemleri ve arayüz

- Oyun başlangıcı, bitişi, ilerleme ve film/bölüm geçişleri.
- Tasarımda varsa kayıt, kontrol noktası ve oturum verileri.
- C++ veri yapıları, ortak arayüzler ve sistemler arası olaylar.
- Menü, HUD, ayarlar, giriş odağı ve okunabilirlik.
- Tasarımda varsa altyazı ve diğer erişilebilirlik ihtiyaçları.

Oyun mantığı ve UI C++ yaklaşımında kalır; Blueprint tabanlı sistem kurulmaz. Ortak veri tiplerinin sahibi ve kullanan sistemler belirlenir.

## 5. Ses, müzik ve efektler

- Ses efektleri, ortam sesleri ve müzik.
- Görsel efektler ve oyuncuya geri bildirim.
- Ses/efektlerin oyun olaylarıyla C++ üzerinden bağlantısı.
- Ses seviyeleri, zamanlama ve performans kontrolü.
- Bulunan/üretilen içeriklerin kaynak, lisans ve atıf kaydı.

İçerik üreticisi ile tetikleyen sistemin sahibi birlikte çalışır; kaynak dosyası ve import edilmiş içerik tutarlı teslim edilir.

## 6. Teknik temel, test ve teslim

- Tek ortak C++ projesinin oluşturulması; modül ve derleme düzeni.
- Input, collision, ortak Config, .uproject ve gerekli eklenti ayarları.
- Derleme, sistem testleri, tam oynanış ve performans kontrolü.
- Dosya referansları, LFS içeriği, paketleme ve temiz ortamda çalıştırma.
- PR inceleme, main entegrasyonu, video ve jam teslim hazırlığı.

Herkes kendi işini test eder. Ufuk main'e birleştirir; bu rol bütün eksikleri düzeltme veya bütün ortak dosyaları sahiplenme görevi değildir. Jam başvurusunu seçilmiş bir insan yapar.

## Sahiplik tablosu

Seçilen alan veya alt işi aşağıya yazın. İki kişi aynı başlığı paylaşabilir; ancak kesin dosya/klasör sınırları ayrı olmalı. Proje oluşmadan yol/sınıf adları uydurulmaz.

| Alan / alt iş | Sorumlu | Dosya / klasör sınırı | Bağlantı kurulacak kişi / sistem |
|---|---|---|---|
| Ana harita | Atanmadı | Proje oluşunca belirlenecek | İçerik ve aktör yerleşimi |
| Ortak proje/derleme ayarları | Atanmadı | .uproject, ortak Config, modül/target ayarları | Bütün sistemler |
| Ortak C++ arayüzleri | Atanmadı | Header/sınıf bazında belirlenecek | Arayüzü kullanan kişiler |
| Main'e birleştirme | Ufuk | İnsan tarafından PR birleştirme rolü | Teslim eden kişi |
| Diğer alanlar / alt işler | Atanmadı | Seçim sonrası yazılacak | İlgili alan sahipleri |

Başkasının alanında değişiklik gerekiyorsa sahibine iletin. Ayrı branch aynı dosyadaki çakışmayı önlemez; bütün dosyaların tek sorumlusu olmalıdır.

Kişisel karar/yapılan iş kayıtları, bu tablodaki iş seçimi ve dosya sınırları kesinleştikten sonra başlar. Genel oyun kararları kişisel dosyalara yazılmaz; ortak fikir dosyasının ayrı karar tablosunda tutulur.
