# Safa — kesin kararlar

Bu dosya Safa'nın kesinleştirdiği kararları tutar; diğer kişilerin ajanları değiştirmez. Başlangıç kayıtları 12 Kasım 2026 devir özeti ve sonraki düzeltmelerinden aktarılmıştır; diğer kişilerin ayrı onayı sayılmaz.

## S-001 — Teknik temel — 2026-11-12
- **Karar:** Ekip Safa, Ufuk, Canberk, Mustafa. Unreal Engine 5.8.4; oyun programlaması yalnız C++. Blueprint programlama/Blueprint tabanlı sistem kurulmayacak. Model, materyal, animasyon, harita, ses gibi içerikler kullanılabilir.
- **Gerekçe:** Ortak teknik yaklaşım.
- **Etkilenen alan:** Bütün üretim ve içerik bağlantıları.
- **Yerine geçtiği karar:** Yok.

## S-002 — Git ve teslim — 2026-11-12
- **Karar:** Hedef branch'ler main, Safa, Ufuk, Canberk, Mustafa. Herkes kendi bilgisayarında ve isim branch'inde çalışıp test eder. Ufuk main'e birleştirir; eksikleri teslim eden düzeltir. İsim branch'lerinin oluşturulduğu varsayılmaz.
- **Gerekçe:** Üretim ve birleştirme sorumluluklarını ayırmak.
- **Etkilenen alan:** Git akışı.
- **Yerine geçtiği karar:** Görev başına feature branch önerisi uygulanmayacak.

## S-003 — Belgeler ve kayıtlar — 2026-11-12
- **Karar:** README, AGENTS, START_PROMPT, WORK_GROUPS, EKIP_REHBERI, JAM_RULES, ASSETS ve decisions altındaki mevcut fikir ile dört kişisel karar dosyası kullanılacak. TASKS, ayrı WORKFLOW/STANDARDS veya zorunlu günlük/devir raporu olmayacak. Kesin kararlar ayrıca yaz denmeden ilgili kişinin dosyasına kaydedilir; öneriler kaydedilmez.
- **Gerekçe:** Sade, tekrarsız düzen; aktif konuşmalar WhatsApp/Discord'da.
- **Etkilenen alan:** Belgeler ve ajanlar.
- **Yerine geçtiği karar:** Eski docs/PROJECT_BRIEF, docs/TASKS gibi öneriler.

## S-004 — Tasarımın sınırı — 2026-11-12
- **Karar:** Jam 2026 Epic AI MegaJam; kullanıcının bildirdiği tema Director’s Cut. Nihai oyunda birkaç film olacak. Ekip oyun fikrini MEVCUT_FIKIR.md içine elle yazacak; eski fikir konuşmaları aktarılmayacak. Senaryo oluşturma yöntemi açık.
- **Gerekçe:** Önerileri kesin tasarım gibi göstermemek.
- **Etkilenen alan:** Tasarım, içerik ve iş seçimi.
- **Yerine geçtiği karar:** Tek filmle ilk döngü önerisi nihai kapsam kararı değildir.

## S-005 — İş alanları ve sahiplik — 2026-11-12
- **Karar:** Sabit dört grup olmayacak. 3D Unreal üretim işleri başlıklar ve alt işlerde listelenecek; göreli emekleri derecelendirilecek. Ekip işleri seçip kendine atayacak. Her dosyanın tek sorumlusu olacak; ana harita ve ortak proje ayarlarının sahipleri ayrıca belirlenecek.
- **Gerekçe:** Gerçek kapsama göre dengeleme ve çakışma riskini azaltma.
- **Etkilenen alan:** WORK_GROUPS ve tüm kaynak/içerik dosyaları.
- **Yerine geçtiği karar:** A–D taslağı ve dört sabit grup gereksinimi.

## S-006 — Başlangıç sırası ve oturum yetkisi — 2026-11-12
- **Karar:** Önce belgeler, sonra mevcut dosyalar incelenerek ignore/LFS düzeni hazırlanacak. Proje adı ve şablonu açık; ortak Unreal projesini daha sonra tek kişi oluşturup açılış/derleme testi yapacak. Bu oturumda hazırlık dosyaları oluşturulabilir; commit/push kullanıcıya ait. Erişim engelinde durulacak.
- **Gerekçe:** Belge hazırlığını oyun üretiminden ayırmak.
- **Etkilenen alan:** Kurulum. Unreal projesi, eklenti kurulumu, motor değişikliği ve oyun geliştirme bu izne dahil değil.
- **Yerine geçtiği karar:** Önceki salt okunur sınır yalnız burada izin verilen hazırlıklar için kaldırıldı.
