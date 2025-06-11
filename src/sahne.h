#ifndef SAHNE_H
#define SAHNE_H

#include <stdint.h> // For uint*_t, int*_t
#include <stddef.h> // For size_t

#ifdef __cplusplus
extern "C" {
#endif

// --- Temel Tipler ---
// Çekirdek tarafındaki (Karnal64) dahili tiplerin (khandle_t, ktid_t vb.) kullanıcı alanı temsilidir.
// Boyutları çekirdektekilerle eşleşmelidir.
typedef uint64_t sahne_handle_t;
typedef uint64_t sahne_task_id_t;
typedef int32_t sahne_error_t; // Hata kodları için (negatif veya pozitif olabilir, SahneError enumuna karşılık gelir)

// --- Başarı/Hata Kodları ---
// Rust SahneError enumuna karşılık gelen C tamsayı sabitleri (genellikle pozitif, 0 başarı)
#define SAHNE_SUCCESS 0

// Rust SahneError eşleşmeleri (sahne64.rs map_kernel_error ile uyumlu olmalı)
#define SAHNE_ERROR_OUT_OF_MEMORY 1
#define SAHNE_ERROR_INVALID_ADDRESS 2
#define SAHNE_ERROR_INVALID_PARAMETER 3
#define SAHNE_ERROR_RESOURCE_NOT_FOUND 4
#define SAHNE_ERROR_PERMISSION_DENIED 5
#define SAHNE_ERROR_RESOURCE_BUSY 6
#define SAHNE_ERROR_INTERRUPTED 7
#define SAHNE_ERROR_NO_MESSAGE 8
#define SAHNE_ERROR_INVALID_OPERATION 9
#define SAHNE_ERROR_NOT_SUPPORTED 10
#define SAHNE_ERROR_UNKNOWN_SYSCALL 11
#define SAHNE_ERROR_TASK_CREATION_FAILED 12
#define SAHNE_ERROR_INVALID_HANDLE 13
#define SAHNE_ERROR_HANDLE_LIMIT_EXCEEDED 14
#define SAHNE_ERROR_NAMING_ERROR 15
#define SAHNE_ERROR_COMMUNICATION_ERROR 16
#define SAHNE_ERROR_WOULD_BLOCK 17 // Yeni hata türü
#define SAHNE_ERROR_DISCONNECTED 18 // Yeni hata türü
// ... sahne64.rs'deki SahneError enumundaki diğer hata kodları buraya eklenmeli ...

// --- Sistem Çağrı Numaraları (sahne64.rs arch modülünden) ---
// Bunlar, çekirdeğin beklediği ABI numaralarıdır.
#define SAHNE_SYSCALL_MEMORY_ALLOCATE 1
#define SAHNE_SYSCALL_MEMORY_RELEASE 2
#define SAHNE_SYSCALL_TASK_SPAWN 3
#define SAHNE_SYSCALL_TASK_EXIT 4
#define SAHNE_SYSCALL_RESOURCE_ACQUIRE 5
#define SAHNE_SYSCALL_RESOURCE_READ 6
#define SAHNE_SYSCALL_RESOURCE_WRITE 7
#define SAHNE_SYSCALL_RESOURCE_RELEASE 8
#define SAHNE_SYSCALL_GET_TASK_ID 9
#define SAHNE_SYSCALL_TASK_SLEEP 10
#define SAHNE_SYSCALL_LOCK_CREATE 11
#define SAHNE_SYSCALL_LOCK_ACQUIRE 12
#define SAHNE_SYSCALL_LOCK_RELEASE 13
#define SAHNE_SYSCALL_THREAD_CREATE 14
#define SAHNE_SYSCALL_THREAD_EXIT 15
#define SAHNE_SYSCALL_GET_SYSTEM_TIME 16
#define SAHNE_SYSCALL_SHARED_MEM_CREATE 17
#define SAHNE_SYSCALL_SHARED_MEM_MAP 18
#define SAHNE_SYSCALL_SHARED_MEM_UNMAP 19
#define SAHNE_SYSCALL_MESSAGE_SEND 20
#define SAHNE_SYSCALL_MESSAGE_RECEIVE 21
#define SAHNE_SYSCALL_GET_KERNEL_INFO 100
#define SAHNE_SYSCALL_TASK_YIELD 101
#define SAHNE_SYSCALL_RESOURCE_CONTROL 102
// --- Yeni Eklenen Sistem Çağrı Numaraları ---
#define SAHNE_SYSCALL_RESOURCE_SEEK 103
#define SAHNE_SYSCALL_RESOURCE_STAT 104
#define SAHNE_SYSCALL_TASK_WAIT     105
#define SAHNE_SYSCALL_CHANNEL_CREATE 106
#define SAHNE_SYSCALL_CHANNEL_CONNECT 107
#define SAHNE_SYSCALL_CHANNEL_SEND 108
#define SAHNE_SYSCALL_CHANNEL_RECEIVE 109
#define SAHNE_SYSCALL_POLL          110


// --- Kaynak Modları (sahne64.rs resource modülünden) ---
#define SAHNE_MODE_READ (1 << 0)
#define SAHNE_MODE_WRITE (1 << 1)
#define SAHNE_MODE_CREATE (1 << 2)
#define SAHNE_MODE_EXCLUSIVE (1 << 3)
#define SAHNE_MODE_TRUNCATE (1 << 4)
#define SAHNE_MODE_NONBLOCK (1 << 5) // Yeni mod


// --- Kernel Info Türleri (sahne64.rs kernel modülünden) ---
#define SAHNE_KERNEL_INFO_VERSION_MAJOR 1
#define SAHNE_KERNEL_INFO_VERSION_MINOR 2
#define SAHNE_KERNEL_INFO_BUILD_ID 3
#define SAHNE_KERNEL_INFO_UPTIME_SECONDS 4
#define SAHNE_KERNEL_INFO_ARCHITECTURE 5
#define SAHNE_KERNEL_INFO_TOTAL_MEMORY_BYTES 6 // Yeni info türü
#define SAHNE_KERNEL_INFO_FREE_MEMORY_BYTES 7  // Yeni info türü


// --- Yeni Eklenen Yapılar ve Enum Karşılıkları ---

// resource::SeekFrom enum'ının C karşılığı için sabitler
#define SAHNE_SEEK_SET 0 // Başlangıçtan (Start)
#define SAHNE_SEEK_CUR 1 // Mevcut konumdan (Current)
#define SAHNE_SEEK_END 2 // Sondan (End)

// resource::ResourceStatus struct'ının C karşılığı (repr(C) uyumlu)
typedef struct ResourceStatus_t {
    uint64_t size;       // Kaynak boyutu
    uint32_t type_flags; // Kaynak tipi ve özellik bayrakları
    uint32_t link_count; // Kaynağa bağlı link sayısı (filesystemler için)
    uint32_t reserved;   // Hizalanma/gelecekte kullanım
    // TODO: Diğer alanlar (sahne64.rs'deki ResourceStatus ile senkron tutulmalı)
} ResourceStatus_t;

// poll::PollEventFlags enum'ının C karşılığı için sabitler
typedef uint32_t PollEventFlags_t;
#define SAHNE_POLL_NONE       0
#define SAHNE_POLL_READABLE   (1 << 0)
#define SAHNE_POLL_WRITABLE   (1 << 1)
#define SAHNE_POLL_ERROR      (1 << 2)
#define SAHNE_POLL_DISCONNECTED (1 << 3)
// TODO: Diğer PollEventFlags sabitleri

// poll::PollEntry struct'ının C karşılığı (repr(C) uyumlu)
typedef struct PollEntry_t {
    sahne_handle_t handle;         // Beklenecek handle
    PollEventFlags_t events_in;    // Beklenen olaylar (input)
    PollEventFlags_t events_out;   // Gerçekleşen olaylar (output, çekirdek doldurur)
} PollEntry_t;


// --- Düşük Seviye Syscall Arayüzü (İsteğe bağlı, genellikle sarmalanır) ---
// Ham sistem çağrısı arayüzü - genellikle uygulamalar tarafından doğrudan kullanımı önerilmez.
// Çekirdekten dönen ham i64 değeri (pozitif/sıfır başarı değeri, negatif hata kodu).
int64_t sahne_raw_syscall(uint64_t number, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);


// --- Bellek Yönetimi ---
/**
 * Belirtilen boyutta bellek tahsis eder.
 * @param size Tahsis edilecek bellek boyutu.
 * @param out_ptr Başarı durumunda tahsis edilen adresi saklamak için çıkış parametresi.
 * @return SAHNE_SUCCESS başarı durumunda, aksi halde bir hata kodu.
 */
sahne_error_t sahne_mem_allocate(size_t size, void** out_ptr);

/**
 * Daha önce `sahne_mem_allocate` ile tahsis edilmiş bir bellek bölgesini serbest bırakır.
 * @param ptr Bellek bölgesinin adresi.
 * @param size Bellek bölgesinin boyutu.
 * @return SAHNE_SUCCESS başarı durumunda, aksi halde bir hata kodu.
 */
sahne_error_t sahne_mem_release(void* ptr, size_t size);

/**
 * Paylaşımlı bellek alanı oluşturur.
 * @param size Paylaşımlı bellek alanının boyutu.
 * @param out_handle Başarı durumunda handle saklamak için çıkış parametresi.
 * @return SAHNE_SUCCESS başarı durumunda, aksi halde bir hata kodu.
 */
sahne_error_t sahne_mem_create_shared(size_t size, sahne_handle_t* out_handle);

/**
 * Paylaşımlı bellek handle'ını mevcut görevin adres alanına eşler.
 * @param handle Paylaşımlı bellek alanının handle'ı.
 * @param offset Paylaşımlı bellek içindeki ofset.
 * @param size Eşlenecek alanın boyutu.
 * @param out_ptr Başarı durumunda eşlenen adresi saklamak için çıkış parametresi.
 * @return SAHNE_SUCCESS başarı durumunda, aksi halde bir hata kodu.
 */
sahne_error_t sahne_mem_map_shared(sahne_handle_t handle, size_t offset, size_t size, void** out_ptr);

/**
 * Eşlenmiş paylaşımlı bellek alanını mevcut görevin adres alanından kaldırır.
 * @param addr Eşlenmiş adres.
 * @param size Eşlenmiş alanın boyutu.
 * @return SAHNE_SUCCESS başarı durumunda, aksi halde bir hata kodu.
 */
sahne_error_t sahne_mem_unmap_shared(void* addr, size_t size);


// --- Görev Yönetimi ---
/**
 * Yeni bir görev (task) başlatır.
 * @param code_handle Çalıştırılabilir kodu içeren kaynağın handle'ı.
 * @param args_ptr Göreve iletilecek argüman verisine pointer.
 * @param args_len Argüman verisi uzunluğu.
 * @param initial_handles_ptr Yeni göreve verilecek Handle listesine pointer (ABI'ye bağlı olarak nasıl geçirileceği değişir).
 * @param initial_handles_len Handle listesi uzunluğu.
 * @param out_task_id Başarı durumunda yeni Task ID saklamak için çıkış parametresi.
 * @return SAHNE_SUCCESS başarı durumunda, aksi halde bir hata kodu.
 */
sahne_error_t sahne_task_spawn(sahne_handle_t code_handle, const uint8_t* args_ptr, size_t args_len, const sahne_handle_t* initial_handles_ptr, size_t initial_handles_len, sahne_task_id_t* out_task_id);


/**
 * Mevcut görevi belirtilen çıkış koduyla sonlandırır. Geri dönmez.
 * @param code Çıkış kodu.
 */
void sahne_task_exit(int32_t code) __attribute__((noreturn));

/**
 * Mevcut görevin Task ID'sini alır.
 * @param out_task_id Başarı durumunda mevcut Task ID saklamak için çıkış parametresi.
 * @return SAHNE_SUCCESS başarı durumunda, aksi halde bir hata kodu.
 */
sahne_error_t sahne_task_current_id(sahne_task_id_t* out_task_id);

/**
 * Mevcut görevi belirtilen süre kadar uyutur.
 * @param milliseconds Uyutulacak süre (milisaniye).
 * @return SAHNE_SUCCESS başarı durumunda, aksi halde bir hata kodu.
 */
sahne_error_t sahne_task_sleep(uint64_t milliseconds);

/**
 * Yeni bir iş parçacığı (thread) oluşturur.
 * @param entry_point_fn Yeni iş parçacığının başlangıç fonksiyon pointer'ı.
 * @param stack_size Yeni iş parçacığı için yığın boyutu.
 * @param arg Başlangıç fonksiyonuna geçirilecek argüman pointer'ı.
 * @param out_thread_id Başarı durumunda yeni thread ID saklamak için çıkış parametresi.
 * @return SAHNE_SUCCESS başarı durumunda, aksi halde bir hata kodu.
 */
sahne_error_t sahne_thread_create(void (*entry_point_fn)(void*), size_t stack_size, void* arg, uint64_t* out_thread_id); // C fonksiyon pointer tipini kullan


/**
 * Mevcut iş parçacığını belirtilen çıkış koduyla sonlandırır. Geri dönmez.
 * @param code Çıkış kodu.
 */
void sahne_thread_exit(int32_t code) __attribute__((noreturn));

/**
 * CPU'yu gönüllü olarak başka bir çalıştırılabilir göreve/iş parçacığına bırakır.
 * @return SAHNE_SUCCESS başarı durumunda, aksi halde bir hata kodu.
 */
sahne_error_t sahne_task_yield(void);

/**
 * (Yeni) Belirtilen görevin sonlanmasını bekler.
 * @param task_id Beklenecek görevin ID'si.
 * @param out_exit_code Başarı durumunda görevin çıkış kodunu saklamak için çıkış parametresi.
 * @return SAHNE_SUCCESS başarı durumunda, aksi halde bir hata kodu.
 */
sahne_error_t sahne_task_wait_for_exit(sahne_task_id_t task_id, int32_t* out_exit_code);


// --- Kaynak Yönetimi ---
/**
 * Belirtilen ID'ye sahip bir kaynağa erişim handle'ı edinir.
 * @param id_ptr Kaynak ID (byte dizisi) pointer'ı.
 * @param id_len Kaynak ID uzunluğu.
 * @param mode Erişim modları (SAHNE_MODE_* bayrakları).
 * @param out_handle Başarı durumunda edinilen handle saklamak için çıkış parametresi.
 * @return SAHNE_SUCCESS başarı durumunda, aksi halde bir hata kodu.
 */
sahne_error_t sahne_resource_acquire(const uint8_t* id_ptr, size_t id_len, uint32_t mode, sahne_handle_t* out_handle);

/**
 * Belirtilen Handle ile temsil edilen kaynaktan veri okur.
 * @param handle Kaynağın handle'ı.
 * @param buffer_ptr Okuma tamponu pointer'ı.
 * @param buffer_len Tamponun boyutu.
 * @param out_bytes_read Başarı durumunda okunan byte sayısını saklamak için çıkış parametresi.
 * @return SAHNE_SUCCESS başarı durumunda, aksi halde bir hata kodu. Okunan byte sayısı *out_bytes_read'e yazılır.
 */
sahne_error_t sahne_resource_read(sahne_handle_t handle, uint8_t* buffer_ptr, size_t buffer_len, size_t* out_bytes_read);

/**
 * Belirtilen Handle ile temsil edilen kaynağa veri yazar.
 * @param handle Kaynağın handle'ı.
 * @param buffer_ptr Yazılacak veri pointer'ı.
 * @param buffer_len Yazılacak veri uzunluğu.
 * @param out_bytes_written Başarı durumunda yazılan byte sayısını saklamak için çıkış parametresi.
 * @return SAHNE_SUCCESS başarı durumunda, aksi halde bir hata kodu. Yazılan byte sayısı *out_bytes_written'a yazılır.
 */
sahne_error_t sahne_resource_write(sahne_handle_t handle, const uint8_t* buffer_ptr, size_t buffer_len, size_t* out_bytes_written);

/**
 * Belirtilen Handle'ı serbest bırakır.
 * @param handle Serbest bırakılacak handle.
 * @return SAHNE_SUCCESS başarı durumunda, aksi halde bir hata kodu.
 */
sahne_error_t sahne_resource_release(sahne_handle_t handle);

/**
 * Kaynağa özel kontrol komutu gönderir.
 * @param handle Kaynağın handle'ı.
 * @param request Komut kodu.
 * @param arg Komut argümanı.
 * @param out_result Başarı durumunda komutun sonucunu saklamak için çıkış parametresi.
 * @return SAHNE_SUCCESS başarı durumunda, aksi halde bir hata kodu. Komutun sonucu *out_result'a yazılır.
 */
sahne_error_t sahne_resource_control(sahne_handle_t handle, uint64_t request, uint64_t arg, int64_t* out_result);

/**
 * (Yeni) Belirtilen Handle ile temsil edilen kaynak içinde konumlanır.
 * @param handle Kaynağın handle'ı.
 * @param whence Konumlanma noktası (SAHNE_SEEK_SET, SAHNE_SEEK_CUR, SAHNE_SEEK_END).
 * @param offset Konumlanma noktasına göre ofset.
 * @param out_new_offset Başarı durumunda yeni ofseti saklamak için çıkış parametresi.
 * @return SAHNE_SUCCESS başarı durumunda, aksi halde bir hata kodu. Yeni ofset *out_new_offset'a yazılır.
 */
sahne_error_t sahne_resource_seek(sahne_handle_t handle, uint64_t whence, int64_t offset, uint64_t* out_new_offset);

/**
 * (Yeni) Belirtilen Handle ile temsil edilen kaynağın durum bilgilerini (stat) alır.
 * @param handle Kaynağın handle'ı.
 * @param out_status Başarı durumunda durum bilgisini saklamak için çıkış parametresi (ResourceStatus_t yapısı).
 * @return SAHNE_SUCCESS başarı durumunda, aksi halde bir hata kodu. Durum bilgisi *out_status'a kopyalanır.
 */
sahne_error_t sahne_resource_stat(sahne_handle_t handle, ResourceStatus_t* out_status);


// --- Çekirdek Etkileşimi ---
/**
 * Çekirdekten belirli bir bilgiyi alır.
 * @param info_type Talep edilen bilgi türü (SAHNE_KERNEL_INFO_*).
 * @param out_value Başarı durumunda bilgi değerini saklamak için çıkış parametresi.
 * @return SAHNE_SUCCESS başarı durumunda, aksi halde bir hata kodu. Bilgi değeri *out_value'ya yazılır.
 */
sahne_error_t sahne_kernel_get_info(uint32_t info_type, uint64_t* out_value);

/**
 * Sistem saatini alır.
 * @param out_time Başarı durumunda sistem zamanını saklamak için çıkış parametresi.
 * @return SAHNE_SUCCESS başarı durumunda, aksi halde bir hata kodu. Sistem zamanı *out_time'a yazılır.
 */
sahne_error_t sahne_kernel_get_time(uint64_t* out_time);


// --- Senkronizasyon ---
/**
 * Yeni bir kilit (Lock) kaynağı oluşturur.
 * @param out_handle Başarı durumunda kilit handle'ı saklamak için çıkış parametresi.
 * @return SAHNE_SUCCESS başarı durumunda, aksi halde bir hata kodu. Kilit handle'ı *out_handle'a yazılır.
 */
sahne_error_t sahne_sync_lock_create(sahne_handle_t* out_handle);

/**
 * Kilidi almaya çalışır.
 * @param handle Kilidin handle'ı.
 * @return SAHNE_SUCCESS başarı durumunda, aksi halde bir hata kodu.
 */
sahne_error_t sahne_sync_lock_acquire(sahne_handle_t handle);

/**
 * Kilidi serbest bırakır.
 * @param handle Kilidin handle'ı.
 * @return SAHNE_SUCCESS başarı durumunda, aksi halde bir hata kodu.
 */
sahne_error_t sahne_sync_lock_release(sahne_handle_t handle);


// --- Mesajlaşma / IPC (Handle tabanlı kanallar) ---
/**
 * (Yeni) Yeni bir mesaj kanalı kaynağı oluşturur.
 * @param out_channel_handle Başarı durumunda kanal handle'ı saklamak için çıkış parametresi.
 * @return SAHNE_SUCCESS başarı durumunda, aksi halde bir hata kodu. Kanal handle'ı *out_channel_handle'a yazılır.
 */
sahne_error_t sahne_channel_create(sahne_handle_t* out_channel_handle);

/**
 * (Yeni) Mevcut bir mesaj kanalına ID'si üzerinden bağlanır.
 * @param channel_id_ptr Kanal ID pointer'ı (byte dizisi).
 * @param channel_id_len Kanal ID uzunluğu.
 * @param out_channel_handle Başarı durumunda yeni kanal handle'ı saklamak için çıkış parametresi (bağlantı handle'ı).
 * @return SAHNE_SUCCESS başarı durumunda, aksi halde bir hata kodu. Yeni handle *out_channel_handle'a yazılır.
 */
sahne_error_t sahne_channel_connect(const uint8_t* channel_id_ptr, size_t channel_id_len, sahne_handle_t* out_channel_handle);

/**
 * (Yeni) Belirtilen kanal handle'ı üzerinden mesaj gönderir.
 * @param channel_handle Mesajın gönderileceği kanalın handle'ı.
 * @param message_ptr Gönderilecek mesaj verisi pointer'ı.
 * @param message_len Mesaj verisi uzunluğu.
 * @return SAHNE_SUCCESS başarı durumunda, aksi halde bir hata kodu.
 */
sahne_error_t sahne_channel_send(sahne_handle_t channel_handle, const uint8_t* message_ptr, size_t message_len);

/**
 * (Yeni) Belirtilen kanal handle'ı üzerinden mesaj alır.
 * @param channel_handle Mesajın alınacağı kanalın handle'ı.
 * @param buffer_ptr Mesajın kopyalanacağı tampon pointer'ı.
 * @param buffer_len Tamponun boyutu.
 * @param out_bytes_received Başarı durumunda alınan byte sayısını saklamak için çıkış parametresi.
 * @return SAHNE_SUCCESS başarı durumunda (alınan byte sayısı *out_bytes_received'a yazılır), SAHNE_ERROR_NO_MESSAGE non-blocking durumda mesaj yoksa, aksi halde başka bir hata kodu.
 */
sahne_error_t sahne_channel_receive(sahne_handle_t channel_handle, uint8_t* buffer_ptr, size_t buffer_len, size_t* out_bytes_received);

// NOT: Eski Task ID'ye doğrudan mesaj gönderme/alma fonksiyonları (sahne_msg_send, sahne_msg_receive)
// yeni kanallarla birlikte kullanım durumuna göre burada kalabilir veya kaldırılabilir.
// Şu anki güncellemeyle bunlar da korunmuş oldu.


// --- Polling / Olay Yönetimi ---
/**
 * (Yeni) Birden çok handle üzerinde belirli olayların gerçekleşmesini bekler.
 * @param entries Beklenecek handle'ları ve olayları içeren PollEntry_t dizisi pointer'ı.
 * @param num_entries Dizideki entry sayısı.
 * @param timeout_ms Ne kadar bekleneceği (milisaniye cinsinden). -1 sonsuz bekleme. 0 non-blocking.
 * @return Başarı durumunda olay gerçekleşen entry sayısını (int64_t >= 0), aksi halde negatif bir hata kodu (kerror_t) döner.
 * Çekirdek, 'entries' dizisindeki 'events_out' alanlarını günceller.
 */
int64_t sahne_poll(PollEntry_t* entries, size_t num_entries, int64_t timeout_ms);


#ifdef __cplusplus
} // extern "C"
#endif

#endif // SAHNE_H
