# İş alanları ve sahiplik

Bu dosya, 3D Unreal üretimindeki işleri seçilebilir alanlarda toplar; günlük görev listesi değildir. Sabit dört grup yoktur. Aşağıdaki işlerin bulunması oyunda zorunlu oldukları anlamına gelmez. Tasarıma bağlı işler MEVCUT_FIKIR.md içinde doğrulandıktan sonra seçilir.

## Emek ölçeği ve seçim

Puanlar ajan tahminidir, ekip kararı veya saat/gün taahhüdü değildir: **1 çok düşük**, **2 düşük**, **3 orta**, **4 yüksek**, **5 çok yüksek**. Alt iş puanı tek bir sınırlı uygulama içindir; alan puanı toplamın aritmetik toplamı değildir. İçerik sayısı, yeni teknoloji, entegrasyon ve test yükü arttıkça yeniden değerlendirin. AI kullanımı puanı kendiliğinden düşürmez.

Her kişi birden fazla alan veya alanın sınırları ayrılmış alt işlerini seçebilir. Önce zorunlu işleri, sonra tasarımın gerektirdiklerini seçin. Tek kişiye hem bütün ortak dosyaları hem birleştirme hem içerik üretimini yüklemeyin. Herkes kendi işinin testini üstlenir.

| Alan | Alt işler ve tahmini emek | Alan yükü / gerekçe | Bağlantı ve sınır |
|---|---|---|---|
| 01 — Tasarım ve kapsam | Ana döngü ve kazanma/kaybetme (3); birkaç filmin içerik sınırı (4); sistem bağlantıları ve kapsam dışı işler (3) | 4 — karar ve içerik bağımlılığı yüksek | Tasarımı ekip elle yazar. Ajan fikir dosyasını kendiliğinden doldurmaz. |
| 02 — Teknik temel | Tek ortak C++ projesi (2); modül/derleme ve eklenti gereksinimleri (3); input/collision/ortak Config (3); temiz bilgisayarda açılış (3) | 4 — herkesi etkileyen ortak dosyalar | Proje adı/şablonu seçilmeden oluşturulmaz. Ortak ayarlar tek sorumluya ait. |
| 03 — Oyuncu ve kamera | Hareket (3); kamera ve görüş (3); kontrol/etkileşim hissi (3); durum geçişleri (3) | 4 — bütün oyunun kullanım hissi | C++ sınıfları; animasyon ve bölümle arayüz anlaşması gerekir. Bakış açısı henüz seçilmiş değil. |
| 04 — Etkileşim ve nesneler | Etkileşim arayüzü (2); kullanılabilir nesneler (3); taşıma/toplama/envanter varsa (4); kayıtla bağlantı varsa (3) | 3–4 — nesne çeşitliliğine bağlı | Ortak arayüzün tek sahibi; oyuncu ve dünya aynı header'ı ayrı ayrı değiştirmez. |
| 05 — Oyun akışı ve ilerleme | Başlatma/bitirme (2); oturum durumları (3); film/bölüm geçişi (4); kontrol noktası/kayıt varsa (4) | 4 — farklı sistemlerin birleşimi | GameMode/GameInstance ve ortak veri yapıları için tekil sahiplik. |
| 06 — Filme özgü sistemler | Film verileri (3); seçilen senaryo oluşturma yöntemi (5); içerik üretim hattı (4); sonuç/ilerleme bağlantısı (4) | 5 — yöntem ve kapsam henüz açık | Hazır seçenek/parça birleştirme/karma yöntem seçilmedi. Hiçbirini uygulanacak karar saymayın. |
| 07 — Anlatı ve bilgi sunumu | Metinler (3); bilgi/ipuçları varsa (3); konuşma/altyazı varsa (3); içerik tutarlılığı ve İngilizce teslim (3) | 4 — film sayısıyla büyür | Senaryo ayrıntıları henüz onaylanmadı. UI, ses ve film sistemine bağlanır. |
| 08 — Çatışma ve hasar, varsa | Saldırı (3); can/hasar (3); tepki ve dengeleme (3); silah/mermi çeşitleri varsa (4) | 4 — animasyon ve test maliyeti | Çatışma kesin oyun kapsamı değildir. Oyuncu/düşman ortak sözleşmesi gerekir. |
| 09 — Düşman/NPC, varsa | Algılama (3); C++ davranış/durum makinesi (4); navigasyon (3); saldırı/etkileşim ve çeşitler (4) | 5 — davranış ve ortam birlikte test edilir | Blueprint davranış sistemi kurulmaz. Dünya, animasyon ve varsa çatışmaya bağlanır. |
| 10 — Bölüm ve keşif | Bloklama/ölçek (3); yol ve yönlendirme (3); içerik yerleşimi (4); ana harita entegrasyonu (4) | 5 — bütün içerik burada buluşur | Ana harita tek kişide. Diğerleri kişisel test haritalarında çalışır. |
| 11 — Çevre ve 3D içerik | Modeller/UV (4); kaplama ve materyal (3); collision/LOD (3); import ve ölçek/pivot kontrolü (3) | 4–5 — içerik adedi belirleyici | Dış/AI kaynak kaydı gerekir. Kaynak dosyası ve import edilmiş asset için sahiplik belirleyin. |
| 12 — Karakter ve animasyon | Model/rig (4); animasyon üretimi/retarget (4); C++ oynatma/geçiş bağlantısı (4); root motion/çarpışma kontrolü (3) | 5 — teknik ve görsel bağımlılık | Blueprint tabanlı animasyon sistemi kurulmaz. Animasyonun C++ ile bağlanma yöntemi ayrıca belirlenir. |
| 13 — Görsel dünya sunumu | Işık/pozlama (3); materyal görünümü (3); atmosfer/post-process (3); dünya değişimi varsa (4) | 4 — performans ve sanat dengesi | Ana haritaya yalnız sahibi uygular. Dünya değişimi zorunlu mekanik değildir. |
| 14 — Arayüz ve erişilebilirlik | C++ arayüz altyapısı (3); HUD/menüler (3); ayarlar ve giriş odağı (3); ölçek/okunabilirlik/renk ve altyazı (3) | 4 — birçok durum/ekran testi | Widget Blueprint sistemi kurulmaz; C++ UI yöntemi uygulamadan önce seçilir. |
| 15 — Ses ve müzik | Ses üretimi/seçimi (3); müzik/ambiyans (3); C++ tetikleme/karışım (3); ses seviyesi ve lisans kontrolü (2) | 3–4 — çeşit ve lisansa bağlı | Seslerin kaynak kaydı; ses kodu ve içerik sahipleri arasında tetikleme sözleşmesi. |
| 16 — Efekt ve geri bildirim | Görsel efektler (3); vurma/etkileşim geri bildirimi varsa (3); kamera/ekran tepkileri (2); performans (3) | 3–4 — sistem olaylarına bağlı | Niagara/materyal içeriği kullanılabilir; oyun mantığı C++ kalır. |
| 17 — Test ve performans | Sistem testleri (3); baştan sona oynanış (4); profil/bellek/yükleme (4); bozuk referans ve paket testi (3) | 5 — her alanı keser | Ayrı kişinin testi, üreticinin test sorumluluğunu kaldırmaz. Performans hedefleri henüz seçilmedi. |
| 18 — Entegrasyon ve sürüm | Arayüz sözleşmeleri (3); teslim incelemesi (3); main birleştirme (3); paketleme ve temiz kurulum (4) | 4–5 — tekrar eden ortak yük | Main birleştirme Ufuk'ta; diğer alt işler ve dosya sahipliği otomatik olarak Ufuk'a atanmaz. |
| 19 — Jam teslimi ve kayıt | Kurala uygunluk (2); asset/atıf listesi (2); 30–60 sn video (3); itch.io paketi ve insan teslimi (3) | 3–4 — son gün sıkışma riski | Resmî Team Owner henüz seçilmedi. Ufuk'un merge görevi bu rolü otomatik vermez. |

## Atama ve dosya sınırları

Aşağıdaki tablo atama yapılana kadar boş kalır. Bir alanı iki kişi paylaşıyorsa dosya/alt klasör sınırlarını ayrı satırlarda yazın. Kişi değişikliği açık koordinasyonla yapılır; dosya taşıyarak sahiplik değiştirmeyin.

| Alan / alt iş | Sorumlu | Sahip olunan kesin dosya/klasörler | Diğer alanlarla arayüz / bağlantı | Durum |
|---|---|---|---|---|
| Ana harita | Atanmadı | Proje oluşunca belirlenecek | İçerik ve C++ aktörlerinin yerleşimi | Seçim bekliyor |
| Ortak ayarlar | Atanmadı | `.uproject`, ortak `Config/`, `*.Build.cs`, `*.Target.cs`, ortak modül ve eklenti ayarları | Input, collision, derleme, eklentiler | Seçim bekliyor |
| Ortak C++ arayüzleri | Atanmadı | Proje oluşunca header/sınıf bazında belirlenecek | Çağrılar, veri tipleri, olaylar | Seçim bekliyor |
| Entegrasyon / main birleştirme | Ufuk | PR birleştirme rolü; dosya sahipliği vermez | Teslim eden herkes kendi işini test eder | Rol kesin |
| Diğer işler | Atanmadı | Seçim ve proje yapısı sonrası doldurulacak | İlgili alan sahipleri | Seçim bekliyor |

Kişisel test haritası ve kaynak alanları, proje oluşturulurken örneğin `Content/Tests/<Kisi>/` gibi ayrı yollarla belirlenebilir; bu yol şu anda kurulmuş veya kesinleştirilmiş değildir. Ana harita, ortak header'lar, sınıflar, modül ayarları ve ortak assetler sahibi belirlenmeden düzenlenmez. İş yükleri seçim sonrasında birlikte yeniden dengelenir; sıfır çakışma veya eşit yük garantisi yoktur.
