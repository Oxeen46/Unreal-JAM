# Ekip rehberi

Bu rehber, AI kullansın veya kullanmasın her ekip üyesinin kurulum, çalışma, sorun çözme ve teslim adımlarını açıklar. 🟢 normal adım, 🟡 kontrol/dikkat, 🔴 durup koordinasyon anlamındadır. Teknik çalışma düzeni bu rehberdedir; oyun kararları Kararlar_ve_Yapilanlar klasöründe, iş ve dosya sahipliği WORK_GROUPS.md içindedir.

## Teknik esaslar ve hesaplar

- Ortak motor Unreal Engine **5.8.4**, oyun programlaması **yalnız C++**. Blueprint programlama veya Blueprint tabanlı sistem kurulmaz; model, materyal, animasyon, harita ve ses içerikleri kullanılabilir.
- Safa: `Oxeen46` → `Safa`; Ufuk: `HorizonTheBestVideoGameDeveloperEver` → `Ufuk`; Canberk: `baylenin` → `Canberk`; Mustafa: `Tesbim` → `Mustafa`. Hesaplar GitHub Collaborators ekranından doğrulandı.
- Herkes kendi bilgisayarında üretip test eder. AI kullanmayan kişi bu rehberi aynı şekilde izler; kimin AI kullanmadığı atanmış değildir.
- GitHub repo adresi: https://github.com/Oxeen46/Unreal-JAM. Public görünürlük kendiliğinden değiştirilmez.
- **Ajanlar main’e dokunmaz:** main üzerinde dosya değiştirme, commit, push veya merge yapmaz. main’i kaynak olarak okumak veya origin/main’i kendi isim branch’ine almak serbesttir. Ufuk’un birleştirme rolünü ajanının main’i değiştirme izni saymayın.

## Branch sistemi nasıl çalışır?

1. **Branch kişisel çalışma kopyanızdır.** Kendi branch’inizdeki commit/push, main’i değiştirmez.
2. **PR bir birleştirme talebidir.** GitHub’da kaynak kendi branch’iniz, hedef main olacak şekilde açılır. İşinizi ve testlerinizi anlatırsınız.
3. **Ufuk kontrol eder ve insan olarak birleştirir.** PR uygunsa main’e Merge yapar; eksikse sahibine geri iletir. Başkasının eksik işini tamamlamak onun görevi değildir. Ufuk kendi geliştirmesini de Ufuk branch’inde yapar.
4. **Diğerleri yeni main’i kendi branch’ine alır.** Bu alma işlemi sizin branch’inizi günceller, main’e yazmaz.
5. **Okuma ve yazma farklıdır.** Public repoda bütün branch’ler okunabilir. Kişiye özel yazma kısıtı için GitHub ruleset kurulup doğrulanmalıdır; belgeye kural yazmak sunucu kilidi değildir.
6. **Hedef koruma düzeni:** kişi branch’inde yalnız ilgili kişi ve yönetici Safa yazabilsin; main’i PR yoluyla yalnız Ufuk birleştirsin. Safa repo sahibi olarak kuralları sonradan değiştirebilsin. Force push ve silme kapalı kalsın. Bu korumalar henüz uygulanmadı; mevcut erişim çözülünce GitHub seçenekleriyle doğrulanacak.
7. Safa kuralları **Settings → Rulesets** bölümünden gözden geçirip değiştirebilir. Sunucu kısıtları kurulana kadar herkes çalışma kuralına uymalı; “başkasının branch’ine teknik olarak yazamam” varsayımı yapmamalı.

## 1. Şu anda ne hazır, ne bekliyor?

1. 🟢 Belgeler ve sürümlenebilir ignore/LFS kuralları hazırlandı. Oyun projesi henüz yok; bugün `.uproject` bulamamak hata değildir.
2. 🟡 Yerelde main, Safa, Ufuk, Canberk ve Mustafa branch’leri var. Güncel düzenleme Safa branch’indedir. GitHub isim branch’leri ve korumalar eklentinin 403 erişim hatası nedeniyle henüz kurulmadı. Commit/push Safa’ya bırakılmıştır.
3. 🔴 Proje adı, başlangıç şablonu, proje oluşturacak kişi, hedef platform, resmî Team Owner ve dosya sahipleri seçilmeden ilgili uygulamaya başlamayın. AI kullanmayan kişinin kim olduğu da burada varsayılmaz.

## 2. İlk kurulum ve ortak projeyi alma

1. 🟢 GitHub Desktop'ta kendi hesabınızla giriş yapın. Gerekli repo yazma daveti varsa kabul edin. **File → Clone repository → URL** üzerinden `https://github.com/Oxeen46/Unreal-JAM.git` adresini klonlayın. Herkes kendi yerel yolunu seçebilir.
2. 🟢 Git ve Git LFS kurulu olsun. Terminalde `git --version`, `git lfs version` ile kontrol edin. LFS kurulu değilse resmî [Git LFS](https://git-lfs.com/) kurulumunu kullanın. Klon içinde `git lfs install --local` çalıştırın; mevcut özel hook uyarısı varsa üzerine yazmaya zorlamayın, koordine edin.
3. 🟡 `.gitattributes` paylaşılır; yerel LFS hook/filtre kurulumu her bilgisayarda ayrıca gerekir. `git lfs env` ile kontrol edin. LFS kurulumunun dosya kilitlemeyi otomatik açmadığını bilin.
4. 🟢 Ekipteki sürümü kullanın; aşağıdaki teknik esasları okuyun. C++ derleme için ilgili Unreal sürümüne uygun Visual Studio C++ oyun geliştirme araçları ve Windows SDK gerekir. Sürüm uyumluluğu doğrulanmadan araç/motor yükseltmeyin.
5. 🔴 Ortak projeyi yalnız seçilen kişi, ayrıca izin verildikten sonra oluşturur. Herkes ayrı proje oluşturup birleştirmeye çalışmaz. Oluşturan kişi açılışı ve tam C++ derlemeyi test edip paylaşır; diğerleri aynı projeyi çeker.
6. 🟢 Proje paylaşıldıktan sonra pull yapın; `git lfs pull` ile LFS dosyalarını alın. Kaynaklar mevcutken Unreal proje dosyalarını yeniden üretin; IDE'de ilgili proje için **Development Editor / Win64** hedefini derleyin, ardından `.uproject` açın. Hedef adını proje oluşmadan tahmin etmeyin.
7. 🔴 “Başka sürüme dönüştür”, “eksik eklenti kur” veya “modülleri değiştir” önerilerini otomatik kabul etmeyin. Hata metniyle ortak ayar sorumlusuna danışın.

## 3. Branch seçimi

1. 🟢 Herkes kendi isim branch'inde çalışır; adları ve main birleştirme rolü aşağıdaki teknik esaslardadır. GitHub Desktop **Current Branch** bölümünden aktif branch'i kontrol edin; terminal karşılığı `git branch --show-current`.
2. 🟡 İsim branch'i yoksa, temiz çalışma ağacında güncel main'den **Branch → New branch** ile tam adını kullanarak oluşturun; gerekiyorsa **Publish branch** ile paylaşın. Bu bir kurulum adımıdır; mevcut sanmayın.
3. 🔴 Bekleyen değişiklik varken branch değiştirip dosyaları yanlış yere taşımayın. Önce 9. bölümdeki koruma adımlarını izleyin.

## 4. Günlük işe başlama

1. 🟢 Unreal ve IDE'yi kapatın; branch ve **Changes** listesini kontrol edin.
2. 🟢 **Fetch origin**, uzaktaki değişiklik bilgisini getirir; kendi başına çalışan dosyalarınızı main ile birleştirmez. Temizken kendi branch'iniz için **Pull origin** yapın.
3. 🟢 Kendi isim branch’inizde kalın ve Fetch origin yapın. Güncel main’i kendi branch’inize almak için **Branch → Merge into current branch** içinde uzak main’i (`origin/main`) seçin. Terminal karşılığı temiz çalışma ağacında `git fetch origin`, ardından `git merge origin/main` olur. Main’i kendi branch’inize alırsınız; kendi işinizi main’e göndermiş olmazsınız. Çakışmada 10. bölüme geçin.
4. 🟡 Bu işlem çakışırsa 10. bölüme geçin. Ortak geçmişi yeniden yazan rebase/force push ile çözmeye çalışmayın.
5. 🟢 Dört karar dosyasını, MEVCUT_FIKIR.md'yi, iş alanınızı ve JAM_RULES.md'yi okuyun. Kararlar yalnız push → main'e birleştirme → pull sonrası diğer kişilere ulaşır; WhatsApp mesajı otomatik repo kaydı değildir.
6. 🔴 İşin dosya sahibi belli değilse veya aynı dosyada başkası çalışıyorsa önce konuşun. Branch ayrılığı aynı dosyadaki çakışmayı önlemez.

## 5. Üretim ve sahiplik

1. 🟢 Yalnız WORK_GROUPS.md'de size atanmış dosya/klasörleri değiştirin. Orada henüz atama yoksa önce birlikte belirleyin.
2. 🔴 Başkasının dosyasında değişiklik gerekiyorsa somut isteği sahibine iletin. İzin/koordinasyon olmadan düzeltme, taşıma veya toplu yeniden adlandırma yapmayın.
3. 🟡 Ortak header, veri tipi, sınıf arayüzü, modül/target dosyaları metin olsa da çakışabilir. Fonksiyon imzası, olaylar ve veri sözleşmelerini sahipleri önceden anlaşsın.
4. 🟢 Ana haritayı yalnız sahibi düzenler. Diğerleri kendi test haritalarında çalışır; hazır C++ aktör/bileşen ve içeriklerini ana harita sahibine teslim eder. Test haritası çalıştı diye ana haritaya yerleştirilmiş sayılmaz.
5. 🔴 Input, collision, `.uproject`, ortak Config, eklenti ve derleme ayarları tek sahibindedir. Main'e birleştirme rolü, bütün bu dosyaların sahipliği demek değildir.
6. 🟡 C++ programlama kararı materyal, animasyon, model, ses veya efekt içeriklerini yasaklamaz; fakat Blueprint tabanlı oyun, UI veya animasyon sistemi kurulmaz. Gerekli bağlantı yöntemi net değilse uygulamadan önce konuşun.

## 6. Unreal içeriklerini güvenli paylaşma

1. 🟢 Model, doku, materyal, animasyon, ses, efekt ve harita için de sahip belirleyin. Ortak materyali veya skeleton'ı değiştirmek birçok kişinin içeriğini etkileyebilir.
2. 🟡 `.uasset` ve `.umap` ikili dosyalardır; Git bunları güvenilir biçimde satır satır birleştiremez. Aynı assete eşzamanlı müdahale etmeyin. Kaynak model dosyasıyla import edilmiş asseti aynı teslimde tutarlı güncelleyin.
3. 🟢 Dış/AI içerik kullanınca Assets/<Ad>.md dosyanızı güncelleyin; paylaşılabilir dosyaları Assets/<Ad>/ altına ekleyin; lisans, atıf, araç/kaynak ve proje yolu eksik kalmasın. Public repoya kaynak dosyası koyma iznini ayrıca kontrol edin.
4. 🟡 Unreal içinde taşıma/yeniden adlandırma referans ve redirector üretir. Ortak bağımlılıklar varsa sahiplerle koordine edin; redirector temizliğini herkesi etkileyen sürpriz işlem yapmayın.
5. 🟡 LFS takibini `git check-attr filter -- dosya-yolu` ile kontrol edin; takip edilen dosyada `filter: lfs` beklenir. Commit sonrası `git lfs ls-files` ile doğrulayın. Yeni uzantı için gerekliliği değerlendirin; büyük dosyayı normal Git geçmişine yanlışlıkla koymayın.
6. 🔴 LFS kotası/kimlik doğrulama hatasında durun. Ücretli plan veya satın alma başlatmayın, geçmişi dönüştürmeyin. Sayısal kota kullanmadan [güncel GitHub LFS belgelerini](https://docs.github.com/en/repositories/working-with-files/managing-large-files/about-git-large-file-storage) ve hesap kullanımını kontrol edin.

## 7. Commit, push ve gün sonu

1. 🟢 Unreal'da kaydedin, gerekli C++ derlemeyi ve ilgili oynanış testini yapın. Ortak C++/entegrasyon değişikliğinde yalnız Live Coding sonucuna güvenmeyin; editör kapalı tam derleme de yapın.
2. 🟢 **Changes** listesini tek tek inceleyin. İlgisiz dosya, özel bilgi veya üretilebilir çıktı varsa commit'e dahil etmeyin. Kaydedilmemiş editör içeriği Git'e görünmez.
3. 🟢 Oyun işi için önce kendi Kararlar_ve_Yapilanlar/<Ad>.md kaydınızı değişikliklerle birlikte hazırlayın. Birbiriyle ilişkili küçük değişiklikleri açıklayıcı mesajla kendi branch’inize commit edin. Commit yerel kayıttır; **Push origin** uzaktaki branch'e gönderir.
4. 🟡 Push'u tamamlayın ve LFS hatası olmadığını kontrol edin. Commit var ama push yoksa ekip sizin değişikliklerinizi henüz alamaz. Push da tek başına main'e birleşme değildir.
5. 🟢 Gün sonunda gerekli içerikleri kaydedin, kendi işinizi test edin ve paylaşılmaya hazır olanı push edin. Bitmemiş işi “test edildi” diye sunmayın; durumunu koordinasyon kanalında açıkça belirtin. Ayrı günlük rapor zorunlu değil.

## 8. PR ve main'e teslim

1. 🟢 Kendi branch'inizden main'e PR açın. Neyi değiştirdiğinizi, nasıl test ettiğinizi, eksikleri ve bağımlılıkları kısa anlatın.
2. 🟢 Ufuk teslimi inceler; gereken projeyi açma/derleme/oynanış kontrolünden sonra main'e birleştirir. Eksik işi üreticisine geri iletir; herkesi düzeltmek zorunda değildir.
3. 🟡 Sahiplik ihlali, çatışan karar, kırık derleme veya eksik LFS varsa merge bekler. Merge sonrası bütün ekip 4. bölümle yeni main'i kendi branch'ine alır.
4. 🟡 Kalıcı isim branch'lerini PR sonrası otomatik silmeyin. GitHub'ın “Delete branch” düğmesini rutin son adım saymayın.

## 9. Yanlış branch / kaybolma riski

1. 🔴 Yanlış branch'te çalıştığınızı fark edince push/merge yapmadan durun. Changes listesini ve aktif branch'i not edin; dosyaları silmeyin.
2. 🟡 Commit edilmemiş iş için GitHub Desktop'ın stash seçeneği kullanılabilir; hangi branch'e ait olduğunu not edin. Stash ortak yedek değildir, yereldir. Unreal'da önce kaydedin; kritik çalışma kopyasını repo dışında güvenle yedekleyin.
3. 🟡 Yanlış branch'e commit yaptıysanız commit kimliğini not edin. Doğru branch'e aktarma ve yanlış daldan geri alma yöntemini Ufuk'la belirleyin; özellikle paylaşılmış geçmişi resetlemeyin.
4. 🔴 `reset --hard`, `clean`, zorla checkout, “discard all” ve force push iş kaybettirebilir. Sorun çözme kısayolu olarak kullanmayın. Başkasının değişikliğini silmeyin.
5. 🟢 Paylaşılmış hatayı geri almak için genellikle yeni bir **revert commit** kullanılır; önce etkilenen sahiplerle koordine edin. Yerel commit edilmemiş dosyayı discard etmek geri alınabilir kabul edilmez.

## 10. Çakışma

1. 🔴 İşlemi durdurun, çakışan dosyaları ve merge yönünü belirleyin; dosya sahiplerine/Ufuk'a bildirin. Rastgele “ours/theirs” seçmeyin.
2. 🟡 Markdown/C++ çakışmasında her iki tarafın amacını koruyarak birleştirin; çakışma işaretlerini kaldırdıktan sonra metni okuyun ve ilgili derlemeyi yapın. Assets klasöründe diğer kişilerin dosyalarını koruyun.
3. 🔴 `.uasset`/`.umap` için iki sürümün otomatik birleşeceğini varsaymayın. İki çalışmayı güvenle koruyup sahibiyle hangi sürümün temel alınacağını seçin; gereken değişikliği editörde yeniden uygulayın.
4. 🟡 Çözüm net değilse merge'i iptal edip önceki duruma dönmek mümkündür; önce mevcut işin korunduğundan emin olun. Terminalde aktif merge için `git merge --abort` kullanılabilir; başarısızsa zorlamayın.
5. 🟢 Çözümden sonra derleme, referanslar ve ilgili oynanışı tekrar test edin. Main'e bozuk çözüm göndermeyin.

## 11. Açılış / derleme hataları

1. 🟡 Hata metnini, branch'i ve commit'i kaydedin. Motor sürümü, eklentiler, Visual Studio/SDK ve LFS içeriğini kontrol edin.
2. 🟡 Küçücük metin olarak `version https://git-lfs.github.com/spec/v1` içeren asset, gerçek içerik yerine pointer olabilir; `git lfs pull` ve erişimi kontrol edin.
3. 🔴 Motorun kendiliğinden sürüm yükseltmesine veya eksik modül hatasında rastgele eklenti silmeye izin vermeyin. Sahibiyle koordine edin.
4. 🟡 Yeniden üretilebilir önbellek temizliği ancak proje kapalıyken, yol doğrulanarak ve gerekli iş yedeklenerek yapılır. `Content`, `Source`, `Config` ve `.uproject` silinmez. `Saved` içinde kurtarma/autosave bulunabileceği için körlemesine temizlemeyin.
5. 🟢 Çözülemeyen hatada değişiklik sahibine ve ortak ayar sorumlusuna somut hata/son çalışan sürüm bilgisi verin; Ufuk'u bütün hataların otomatik tamircisi saymayın.

## 12. Kararlar, kapsam ve iletişim

1. 🟢 Kesin OYUN kararını Kararlar_ve_Yapilanlar/<Ad>.md dosyanıza kısa gerekçe, etkilenen alan ve varsa yerine geçtiği kayıtla ekleyin. Ajan ayrıca beklemeden kaydeder; AI kullanmıyorsanız elle yazın. Her oyun işi commit’ine aynı dosyada yapılan iş, değişen oyun dosyaları, test sonucu ve eksikleri ekleyin. GitHub ayarı veya repo düzenlemesini oyun kararı/oyun işi diye kaydetmeyin.
2. 🔴 Başka kişinin karar dosyasını düzenlemeyin. En yeni tarihli kayıt otomatik üstün değildir; çelişkiyi ilgili kişilerle çözün, uzlaşıyı kendi kaydınızda önceki kimliklere referansla belirtin.
3. 🟡 Sahiplik/ortak arayüz değişikliğinde ilgili sahiplerle; ana harita yerleşiminde harita sahibiyle; merge ve teslim çatışmasında Ufuk'la; kapsam/film tasarımında ekiple konuşun.
4. 🟢 MEVCUT_FIKIR.md oyun fikrinin tek ana kaynağıdır. İçeriğini ekip elle doldurur; ajan ancak ayrıca açık talimat alırsa değiştirir. Eski fikirleri kendiliğinden taşımaz.

## 13. Paketleme ve jam teslimi

1. 🟢 JAM_RULES.md'deki kaynağa dayalı gereklilikleri okuyun. Paketlemeyi erken test edin; yalnız editörde çalışan proje teslim değildir.
2. 🟢 Test edilmiş main sürümünden, seçilmiş platforma paket alın. Kaynak projeyi, LFS pointer dosyalarını veya geliştirme önbelleklerini oyun paketi diye yüklemeyin.
3. 🟢 Paketi indirip/çıkarıp mümkünse başka bilgisayarda çalıştırın; başlangıç, kontroller, film geçişleri, sonlanma, ses ve performansı kontrol edin. Tasarımda olmayan özellikleri varmış gibi test edilmiş yazmayın.
4. 🟢 Video, İngilizce içerik, kaynak/atıf listesi ve gereken özel başvuru bilgilerini hazırlayın. Teslim sürümünün commit kimliğini PR/teslim açıklamasında belirtmek izlenebilirlik sağlar; zorunlu ayrı rapor gerekmez.
5. 🔴 Başvuruyu seçilmiş insan yapar. Otomatik ajan teslimi yapılmaz. Gönderilen paketi koruyun; sonradan üretilen sürümün değerlendirmeye gireceğini varsaymayın.
