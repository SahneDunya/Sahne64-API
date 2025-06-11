#![no_std] // Standart kütüphaneye ihtiyaç duymuyoruz, minimal OS/runtime için

// Geliştirme sırasında kullanılmayan kodlar/uyarılar için
#![allow(dead_code)]
#![allow(unused_variables)]

// Mimariye özel sistem çağrı numaraları (Sahne64 terminolojisi)
// Bunlar, çekirdekteki sistem çağrısı işleyicisinin (Karnal64'ü kullanan) beklediği numaralarla eşleşmelidir.
#[cfg(any(target_arch = "riscv64", target_arch = "aarch64", target_arch = "x86_64", target_arch = "sparc64", target_arch = "openrisc", target_arch = "powerpc64", target_arch = "loongarch64", target_arch = "elbrus", target_arch = "mips64"))]
pub mod arch {
    // Daha önceki sistem çağrıları
    pub const SYSCALL_MEMORY_ALLOCATE: u64 = 1;  // Bellek tahsis et
    pub const SYSCALL_MEMORY_RELEASE: u64 = 2;   // Bellek serbest bırak
    pub const SYSCALL_TASK_SPAWN: u64 = 3;       // Yeni bir görev (task) başlat
    pub const SYSCALL_TASK_EXIT: u64 = 4;        // Mevcut görevi sonlandır
    pub const SYSCALL_RESOURCE_ACQUIRE: u64 = 5; // Bir kaynağa erişim tanıtıcısı (Handle) al
    pub const SYSCALL_RESOURCE_READ: u64 = 6;    // Kaynaktan oku (Handle ile)
    pub const SYSCALL_RESOURCE_WRITE: u64 = 7;   // Kaynağa yaz (Handle ile)
    pub const SYSCALL_RESOURCE_RELEASE: u64 = 8; // Kaynak tanıtıcısını serbest bırak
    pub const SYSCALL_GET_TASK_ID: u64 = 9;      // Mevcut görev ID'sini al
    pub const SYSCALL_TASK_SLEEP: u64 = 10;      // Görevi uyut
    pub const SYSCALL_LOCK_CREATE: u64 = 11;     // Kilit (Lock) oluştur
    pub const SYSCALL_LOCK_ACQUIRE: u64 = 12;    // Kilidi al (Bloklayabilir)
    pub const SYSCALL_LOCK_RELEASE: u64 = 13;    // Kilidi bırak
    pub const SYSCALL_THREAD_CREATE: u64 = 14;   // Yeni bir iş parçacığı (thread) oluştur
    pub const SYSCALL_THREAD_EXIT: u64 = 15;     // Mevcut iş parçacığını sonlandır
    pub const SYSCALL_GET_SYSTEM_TIME: u64 = 16; // Sistem saatini al
    pub const SYSCALL_SHARED_MEM_CREATE: u64 = 17; // Paylaşımlı bellek alanı oluştur (Handle döner)
    pub const SYSCALL_SHARED_MEM_MAP: u64 = 18;   // Paylaşımlı belleği adres alanına eşle (Handle ile)
    pub const SYSCALL_SHARED_MEM_UNMAP: u64 = 19; // Paylaşımlı bellek eşlemesini kaldır
    pub const SYSCALL_MESSAGE_SEND: u64 = 20;    // Başka bir göreve mesaj gönder (Task ID veya Handle ile)
    pub const SYSCALL_MESSAGE_RECEIVE: u64 = 21; // Mesaj al (Bloklayabilir)
    pub const SYSCALL_GET_KERNEL_INFO: u64 = 100; // Çekirdek bilgisi al
    pub const SYSCALL_TASK_YIELD: u64 = 101;     // CPU'yu başka bir göreve devret
    pub const SYSCALL_RESOURCE_CONTROL: u64 = 102;// Kaynağa özel kontrol komutu gönder (Handle ile)

    // --- Yeni Özellikler İçin Sistem Çağrı Numaraları ---
    pub const SYSCALL_RESOURCE_SEEK: u64 = 103;   // Kaynakta konumlan (Handle ile)
    pub const SYSCALL_RESOURCE_STAT: u64 = 104;   // Kaynak durumu al (Handle ile)
    pub const SYSCALL_TASK_WAIT: u64 = 105;       // Bir görevin sonlanmasını bekle
    pub const SYSCALL_CHANNEL_CREATE: u64 = 106;  // Mesaj kanalı oluştur (Handle döner)
    pub const SYSCALL_CHANNEL_CONNECT: u64 = 107; // Mevcut kanala bağlan (Handle döner)
    pub const SYSCALL_CHANNEL_SEND: u64 = 108;    // Kanal üzerinden mesaj gönder (Handle ile)
    pub const SYSCALL_CHANNEL_RECEIVE: u64 = 109; // Kanal üzerinden mesaj al (Handle ile)
    pub const SYSCALL_POLL: u64 = 110;            // Birden çok handle üzerinde olay bekle
    // Yeni eklenen Kernel Info türleri için de sabitler tanımlanabilir (kernel::KERNEL_INFO_*)

}

/// Sahne64 Kaynak Tanıtıcısı (Handle).
/// Kaynaklara (dosyalar, soketler, bellek bölgeleri, kilitler, kanallar vb.) erişmek için kullanılır.
/// Çekirdek tarafında (Karnal64'te) KHandle olarak karşılık bulur, ama kullanıcı alanında sadece bir u64'tür.
#[derive(Debug, Copy, Clone, PartialEq, Eq, Hash)]
#[repr(transparent)] // Bellekte sadece u64 olarak yer kaplar
pub struct Handle(u64);

impl Handle {
    /// Geçersiz veya boş bir Handle oluşturur.
    pub const fn invalid() -> Self {
        Handle(0) // Veya çekirdeğin belirlediği başka bir geçersiz değer (genellikle 0 veya -1)
    }

    /// Handle'ın geçerli olup olmadığını kontrol eder.
    pub fn is_valid(&self) -> bool {
        self.0 != Self::invalid().0 // Çekirdeğin 0'ı geçersiz kabul ettiği varsayılır
    }

    /// Handle'ın içindeki ham değeri alır (dikkatli kullanılmalı!).
    pub(crate) fn raw(&self) -> u64 {
        self.0
    }
}

/// Sahne64 Görev (Task) Tanımlayıcısı.
/// Süreç (process) yerine kullanılır. Çekirdek tarafında KTaskId olarak karşılık bulur.
#[derive(Debug, Copy, Clone, PartialEq, Eq, Hash)]
#[repr(transparent)]
pub struct TaskId(u64);

impl TaskId {
    /// Geçersiz bir TaskId oluşturur.
    pub const fn invalid() -> Self {
        TaskId(0) // Veya çekirdeğin belirlediği başka bir geçersiz değer
    }

    /// TaskId'nin geçerli olup olmadığını kontrol eder.
    pub fn is_valid(&self) -> bool {
        self.0 != Self::invalid().0
    }

    /// TaskId'nin içindeki ham değeri alır (dikkatli kullanılmalı!).
    pub(crate) fn raw(&self) -> u64 {
        self.0
    }
}


// Sahne64 Hata Türleri (Çekirdekten dönen negatif i64'lerin kullanıcı alanı temsili)
// Bunlar, çekirdekteki KError enum'unun sistem çağrısı işleyicisi tarafından
// i64'e dönüştürülmüş hallerine karşılık gelir.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum SahneError {
    OutOfMemory,          // Yetersiz bellek
    InvalidAddress,       // Geçersiz bellek adresi
    InvalidParameter,     // Fonksiyona geçersiz parametre verildi
    ResourceNotFound,     // Belirtilen kaynak bulunamadı (örn. isimle ararken)
    PermissionDenied,     // İşlem için yetki yok
    ResourceBusy,         // Kaynak şu anda meşgul (örn. kilitli dosya, dolu kuyruk)
    Interrupted,          // İşlem bir sinyal veya başka bir olayla kesildi
    NoMessage,            // Beklenen mesaj yok (non-blocking receive)
    InvalidOperation,     // Kaynak üzerinde geçersiz işlem denendi (örn. okunamaz kaynağı okumak)
    NotSupported,         // İşlem veya özellik desteklenmiyor (örn. bilinmeyen syscall)
    UnknownSystemCall,    // Çekirdek bilinmeyen sistem çağrısı numarası aldı (Alternatif NotSupported)
    TaskCreationFailed,   // Yeni görev (task) oluşturulamadı
    InvalidHandle,        // Geçersiz veya süresi dolmuş Handle kullanıldı
    HandleLimitExceeded,  // Süreç başına düşen Handle limiti aşıldı
    NamingError,          // Kaynak isimlendirme ile ilgili hata (örn. geçersiz karakterler)
    CommunicationError,   // Mesajlaşma veya IPC hatası (genel)
    WouldBlock,           // İşlem şu anda bloke olacak ama NonBlocking işaretli
    Disconnected,         // IPC kanalı/kaynak bağlantısı kapandı
    // ... Çekirdekten (Karnal64) gelebilecek yeni hata türleri buraya eklenebilir ...
}

// Sistem çağrısı arayüzü (çekirdeğe geçiş mekanizması)
// Bu, Sahne64'ün çekirdekle konuşmasının tek yoludur.
// Argümanlar ve dönüş değeri ABI'ye (Application Binary Interface) uymalıdır.
// Karnal64'ü kullanan çekirdeğin sistem çağrısı işleyicisi bu ABI'yi bekler.
extern "sysv64" {
    fn syscall(number: u64, arg1: u64, arg2: u64, arg3: u64, arg4: u64, arg5: u64) -> i64;
}

// Hata Kodu Çevirimi Yardımcı Fonksiyonu
// Çekirdekten (sistem çağrısı işleyicisinden) dönen negatif i64'leri SahneError'a çevirir.
// Bu eşleşme, çekirdekteki KError -> i64 dönüşümü ile uyumlu olmalıdır.
fn map_kernel_error(code: i64) -> SahneError {
    match code {
        -1 => SahneError::PermissionDenied,
        -2 => SahneError::ResourceNotFound,
        -3 => SahneError::InvalidParameter, // veya KError::InvalidArgument
        -4 => SahneError::Interrupted,
        -9 => SahneError::InvalidHandle, // veya KError::BadHandle
        -11 => SahneError::ResourceBusy, // veya KError::Busy
        -12 => SahneError::OutOfMemory,
        -14 => SahneError::InvalidAddress, // veya KError::BadAddress
        -17 => SahneError::NamingError, // veya KError::AlreadyExists (duruma göre)
        -38 => SahneError::NotSupported,
        -61 => SahneError::NoMessage,
        -101 => SahneError::WouldBlock, // Yeni hata kodu eşleşmesi (örnek)
        -102 => SahneError::Disconnected, // Yeni hata kodu eşleşmesi (örnek)
        // TODO: Çekirdekteki (Karnal64) KError enum'undaki diğer değerler buraya eşlenmeli
        _ => SahneError::UnknownSystemCall, // Eşlenmemiş veya bilinmeyen negatif kod
    }
}

// Başarılı i64 sonucunu istenen T türüne çevirme ve Result döndürme yardımcısı
// Çekirdek başarılı olduğunda pozitif veya sıfır bir değer döndürür, bu değer
// sistem çağrısına göre bir pointer, ID, boyut veya özel bir değer olabilir.
fn map_kernel_ok_result<T>(result: i64) -> Result<T, SahneError>
where
    T: TryFrom<u64>, // T, u64'ten dönüştürülebilir olmalı
    <T as TryFrom<u64>>::Error: core::fmt::Debug, // Dönüşüm hatası debug basılabilir olmalı
{
    if result < 0 {
        // Should not happen if called after checking result >= 0, but for safety:
        Err(map_kernel_error(result))
    } else {
        // Güvenlik: Çekirdekten dönen u64 değerinin T türüne güvenli dönüşümü
        T::try_from(result as u64).map_err(|_| SahneError::InvalidOperation) // Dönüşüm başarısız olursa
    }
}


// --- Güncellenmiş ve Yeni Modüller/Fonksiyonlar ---

// Bellek yönetimi modülü (Paylaşımlı bellek dahil)
pub mod memory {
    use super::{SahneError, arch, syscall, map_kernel_error, map_kernel_ok_result, Handle};
    use core::ptr::NonNull; // Non-null pointer için daha güvenli temsil

    /// Belirtilen boyutta bellek ayırır.
    /// Başarılı olursa, ayrılan belleğe non-null işaretçi döner.
    pub fn allocate(size: usize) -> Result<NonNull<u8>, SahneError> {
        let result = unsafe {
            syscall(arch::SYSCALL_MEMORY_ALLOCATE, size as u64, 0, 0, 0, 0)
        };
        if result < 0 {
            Err(map_kernel_error(result))
        } else {
            // Çekirdekten dönen adres (u64) non-null pointer'a çevrilir.
            // Çekirdeğin asla null döndürmediği varsayılır (hata durumunda negatif döner).
            NonNull::new(result as *mut u8).ok_or(SahneError::InternalError) // 0 adresi için hata ver (genellikle kernel 0 döndürmez)
        }
    }

    /// Daha önce `allocate` ile ayrılmış bir belleği serbest bırakır.
    pub fn release(ptr: NonNull<u8>, size: usize) -> Result<(), SahneError> {
        let result = unsafe {
            syscall(arch::SYSCALL_MEMORY_RELEASE, ptr.as_ptr() as u64, size as u64, 0, 0, 0)
        };
        if result < 0 {
            Err(map_kernel_error(result))
        } else {
            Ok(())
        }
    }

    /// Belirtilen boyutta paylaşımlı bellek alanı oluşturur ve bir Handle döner.
    /// Bu Handle başka görevlerle paylaşılabilir.
    pub fn create_shared(size: usize) -> Result<Handle, SahneError> {
        let result = unsafe {
            syscall(arch::SYSCALL_SHARED_MEM_CREATE, size as u64, 0, 0, 0, 0)
        };
        if result < 0 {
            Err(map_kernel_error(result))
        } else {
            Ok(Handle(result as u64)) // Başarı durumunda handle değeri döner
        }
    }

    /// Paylaşımlı bellek Handle'ını mevcut görevin adres alanına eşler.
    /// offset ve size, paylaşımlı bellek alanının hangi kısmının eşleneceğini belirler.
    pub fn map_shared(handle: Handle, offset: usize, size: usize) -> Result<NonNull<u8>, SahneError> {
          if !handle.is_valid() {
              return Err(SahneError::InvalidHandle);
          }
        let result = unsafe {
            syscall(arch::SYSCALL_SHARED_MEM_MAP, handle.raw(), offset as u64, size as u64, 0, 0)
        };
        if result < 0 {
            Err(map_kernel_error(result))
        } else {
            // Çekirdekten dönen eşlenmiş adres (u64)
            NonNull::new(result as *mut u8).ok_or(SahneError::InternalError)
        }
    }

    /// Eşlenmiş paylaşımlı bellek alanını adres alanından kaldırır.
    pub fn unmap_shared(addr: NonNull<u8>, size: usize) -> Result<(), SahneError> {
        let result = unsafe {
            syscall(arch::SYSCALL_SHARED_MEM_UNMAP, addr.as_ptr() as u64, size as u64, 0, 0, 0)
        };
        if result < 0 {
            Err(map_kernel_error(result))
        } else {
            Ok(())
        }
    }
}

// Görev (Task) yönetimi modülü (Süreç yerine)
pub mod task {
    use super::{SahneError, arch, syscall, map_kernel_error, map_kernel_ok_result, Handle, TaskId};
    use core::time::Duration;

    /// Yeni bir görev (task) başlatır.
    /// Çalıştırılacak kod bir Handle ile temsil edilir (örn. bir yürütülebilir kaynak Handle'ı).
    /// Argümanlar opak bir byte dizisi olarak geçirilir.
    /// Başarılı olursa, yeni görevin TaskId'sini döner.
    ///
    /// # Argümanlar
    /// * `code_handle`: Çalıştırılacak kodu içeren kaynağın Handle'ı.
    /// * `args`: Göreve başlangıçta iletilecek argüman verisi (genellikle ana fonksiyona geçirilir).
    /// * `initial_handles`: (Yeni Özellik) Yeni göreve başlangıçta verilecek Handle'ların listesi (örn. stdin/stdout).
    ///    Bu Handle'lar çekirdek tarafından yeni görevin handle tablosuna kopyalanır.
    ///    syscall argüman sınırlaması nedeniyle bu Handle'lar başka bir kaynak (örn. paylaşımlı bellek) üzerinden geçirilebilir veya
    ///    farklı bir syscall/mekanizma kullanılabilir. Basitlik için argüman olarak ekleyelim, syscall'da nasıl ele alınacağı çekirdeğe bağlı.
    pub fn spawn(code_handle: Handle, args: &[u8], initial_handles: &[Handle]) -> Result<TaskId, SahneError> {
          if !code_handle.is_valid() {
              return Err(SahneError::InvalidHandle);
          }
        let args_ptr = args.as_ptr() as u64;
        let args_len = args.len() as u64;

        // Handle listesini syscall üzerinden geçirmek zor olabilir.
        // Ya Handle sayısı sınırlanır, ya bir pointer/uzunluk geçirilir (başka syscall ile ayarlanmış
        // paylaşımlı bellek gibi bir alanda Handle listesi tutulur) ya da başka bir IPC mekanizması kullanılır.
        // Basitlik için, syscall'ın sadece args ve handle listesiyle ilgili bilgiyi (belki handle sayısını) aldığını varsayalım.
        // Gerçek implementasyonda handle listesinin nasıl geçirileceği ABI'nin parçasıdır.
        let handles_ptr = initial_handles.as_ptr() as u64; // Ham handle listesi pointer'ı
        let handles_len = initial_handles.len() as u64;   // Handle listesi uzunluğu

        let result = unsafe {
            syscall(arch::SYSCALL_TASK_SPAWN, code_handle.raw(), args_ptr, args_len, handles_ptr, handles_len)
        };
        if result < 0 {
            Err(map_kernel_error(result))
        } else {
            Ok(TaskId(result as u64)) // Başarı durumunda yeni task ID döner
        }
    }

    /// Mevcut görevi belirtilen çıkış koduyla sonlandırır. Bu fonksiyon geri dönmez.
    pub fn exit(code: i32) -> ! {
        unsafe {
            syscall(arch::SYSCALL_TASK_EXIT, code as u64, 0, 0, 0, 0);
        }
        // Çekirdek görevi sonlandırmalı, buraya dönülmemeli.
        // Eğer dönerse, muhtemelen bir hata olmuştur veya emulate ediliyordur.
        loop { core::hint::spin_loop(); } // İşlemciyi meşgul etmeden bekle
    }

    /// Mevcut görevin TaskId'sini alır.
    pub fn current_id() -> Result<TaskId, SahneError> {
        let result = unsafe {
            syscall(arch::SYSCALL_GET_TASK_ID, 0, 0, 0, 0, 0)
        };
        if result < 0 {
            Err(map_kernel_error(result))
        } else {
            Ok(TaskId(result as u64)) // Başarı durumunda mevcut task ID döner
        }
    }

    /// Mevcut görevi belirtilen süre kadar uyutur.
    /// duration: Uyutulacak süre. Çekirdek hassasiyeti desteklemeyebilir.
    pub fn sleep(duration: Duration) -> Result<(), SahneError> {
        let milliseconds = duration.as_millis(); // Süreyi milisaniyeye çevir
        // Çekirdeğin u64 milisaniye kabul ettiği varsayılır. Daha yüksek hassasiyet gerekirse ABI değişir.
        let result = unsafe {
            syscall(arch::SYSCALL_TASK_SLEEP, milliseconds as u64, 0, 0, 0, 0)
        };
        if result < 0 {
            Err(map_kernel_error(result))
        } else {
            Ok(())
        }
    }

    /// Yeni bir iş parçacığı (thread) oluşturur.
    /// İş parçacıkları aynı görevin adres alanını paylaşır.
    /// `entry_point_fn`: Yeni iş parçacığının başlangıç fonksiyonu.
    /// `stack_size`: Yeni iş parçacığı için ayrılacak yığın boyutu.
    /// `arg`: Başlangıç fonksiyonuna geçirilecek argüman.
    /// Başarılı olursa, yeni iş parçacığının ID'sini döner. (ThreadId struct'ı daha temiz olabilir)
    pub fn create_thread(entry_point_fn: fn(*mut c_void), stack_size: usize, arg: *mut c_void) -> Result<u64, SahneError> {
        // Fonksiyon işaretçisi u64 olarak geçirilir. Güvenli değil, ama düşük seviye ABI'de yaygın.
        let entry_point_u64 = entry_point_fn as u64;
        let arg_u64 = arg as u64; // Argüman pointer'ı u64 olarak geçirilir.
        let result = unsafe {
            syscall(arch::SYSCALL_THREAD_CREATE, entry_point_u64, stack_size as u64, arg_u64, 0, 0)
        };
        if result < 0 {
            Err(map_kernel_error(result))
        } else {
            Ok(result as u64) // Thread ID (u64)
        }
    }

    /// Mevcut iş parçacığını belirtilen çıkış koduyla sonlandırır. Bu fonksiyon geri dönmez.
    pub fn exit_thread(code: i32) -> ! {
        unsafe {
            syscall(arch::SYSCALL_THREAD_EXIT, code as u64, 0, 0, 0, 0);
        }
        loop { core::hint::spin_loop(); }
    }

    /// CPU'yu gönüllü olarak başka bir çalıştırılabilir göreve/iş parçacığına bırakır.
    pub fn yield_now() -> Result<(), SahneError> {
        let result = unsafe {
            syscall(arch::SYSCALL_TASK_YIELD, 0, 0, 0, 0, 0)
        };
        if result < 0 {
            Err(map_kernel_error(result))
        } else {
            Ok(())
        }
    }

    /// (Yeni Özellik) Belirtilen görevin sonlanmasını bekler.
    /// Çekirdek, görevin sonlandığında çağıran görevi uyandırır.
    /// `task_id`: Beklenecek görevin ID'si.
    /// Başarı durumunda görevin çıkış kodunu döner.
    pub fn wait_for_exit(task_id: TaskId) -> Result<i32, SahneError> {
        if !task_id.is_valid() {
             return Err(SahneError::InvalidParameter);
        }
        let result = unsafe {
             syscall(arch::SYSCALL_TASK_WAIT, task_id.raw(), 0, 0, 0, 0)
        };
        // Çekirdek, başarı durumunda çıkış kodunu (i32) i64'e dönüştürüp dönebilir.
        // Hata durumunda negatif kerror_t döner.
        if result < 0 {
             Err(map_kernel_error(result))
        } else {
             // Başarı: Çekirdek çıkış kodunu döndürdü. i64 -> i32 dönüşümü.
             // Taşma olasılığı var mı? (i32 değerinin i64'e sığması beklenir)
             Ok(result as i32)
        }
    }
}

// Kaynak yönetimi modülü (Dosya sistemi yerine, Seek ve Stat eklendi)
pub mod resource {
    use super::{SahneError, arch, syscall, map_kernel_error, map_kernel_ok_result, Handle};
    use core::ffi::c_void;
    use core::ptr::NonNull;

    // Kaynak açma/edinme modları için Sahne64'e özgü bayraklar (Karnal64 modları ile eşleşmeli)
    pub const MODE_READ: u32 = 1 << 0;    // Kaynaktan okuma yeteneği iste
    pub const MODE_WRITE: u32 = 1 << 1;   // Kaynağa yazma yeteneği iste
    pub const MODE_CREATE: u32 = 1 << 2;  // Kaynak yoksa oluşturulsun
    pub const MODE_EXCLUSIVE: u32 = 1 << 3; // Kaynak zaten varsa hata ver (CREATE ile kullanılır)
    pub const MODE_TRUNCATE: u32 = 1 << 4; // Kaynak açılırken içeriğini sil (varsa ve yazma izni varsa)
    pub const MODE_NONBLOCK: u32 = 1 << 5; // İşlemleri bloke etme (read/write/receive)
    // ... Sahne64'e özel diğer modlar (örn. Append, Exec, Device vb.)

    /// Sahne64'e özgü bir kaynak adı veya tanımlayıcısı. Çekirdek (Karnal64) bunu işler.
    pub type ResourceId<'a> = &'a str; // Karnal64 tarafında u8 slice olarak alınır, UTF8 doğrulaması yapılabilir.

    /// Belirtilen ID'ye sahip bir kaynağa erişim Handle'ı edinir.
    /// `id`: Kaynağı tanımlayan Sahne64'e özgü tanımlayıcı.
    /// `mode`: Kaynağa nasıl erişileceğini belirten bayraklar (MODE_*).
    pub fn acquire(id: ResourceId, mode: u32) -> Result<Handle, SahneError> {
        let id_ptr = id.as_ptr() as u64;
        let id_len = id.len() as u64;
        let result = unsafe {
            syscall(arch::SYSCALL_RESOURCE_ACQUIRE, id_ptr, id_len, mode as u64, 0, 0)
        };
        if result < 0 {
            Err(map_kernel_error(result))
        } else {
            Ok(Handle(result as u64)) // Başarı durumunda handle değeri döner
        }
    }

    /// Belirtilen Handle ile temsil edilen kaynaktan veri okur.
    /// Okunan byte sayısını döner.
    pub fn read(handle: Handle, buffer: &mut [u8]) -> Result<usize, SahneError> {
        if !handle.is_valid() {
            return Err(SahneError::InvalidHandle);
        }
        let buffer_ptr = buffer.as_mut_ptr() as u64;
        let buffer_len = buffer.len() as u64;
        let result = unsafe {
            syscall(arch::SYSCALL_RESOURCE_READ, handle.raw(), buffer_ptr, buffer_len, 0, 0)
        };
        if result < 0 {
            Err(map_kernel_error(result)) // Hata (örn. WouldBlock, InvalidHandle, PermissionDenied)
        } else {
            Ok(result as usize) // Başarı (okunan byte sayısı)
        }
    }

    /// Belirtilen Handle ile temsil edilen kaynağa veri yazar.
    /// Yazılan byte sayısını döner.
    pub fn write(handle: Handle, buffer: &[u8]) -> Result<usize, SahneError> {
          if !handle.is_valid() {
              return Err(SahneError::InvalidHandle);
          }
        let buffer_ptr = buffer.as_ptr() as u64;
        let buffer_len = buffer.len() as u64;
        let result = unsafe {
            syscall(arch::SYSCALL_RESOURCE_WRITE, handle.raw(), buffer_ptr, buffer_len, 0, 0)
        };
        if result < 0 {
            Err(map_kernel_error(result)) // Hata (örn. WouldBlock, InvalidHandle, PermissionDenied)
        } else {
            Ok(result as usize) // Başarı (yazılan byte sayısı)
        }
    }

    /// Belirtilen Handle'ı serbest bırakır, kaynağa erişimi sonlandırır.
    pub fn release(handle: Handle) -> Result<(), SahneError> {
          if !handle.is_valid() {
              return Err(SahneError::InvalidHandle); // Zaten geçersiz handle'ı bırakmaya çalışma
          }
        let result = unsafe {
            syscall(arch::SYSCALL_RESOURCE_RELEASE, handle.raw(), 0, 0, 0, 0)
        };
        if result < 0 {
            Err(map_kernel_error(result))
        } else {
            Ok(())
        }
    }

    /// Kaynağa özel kontrol komutları göndermek için kullanılır (Unix `ioctl` benzeri).
    pub fn control(handle: Handle, request: u64, arg: u64) -> Result<i64, SahneError> {
          if !handle.is_valid() {
              return Err(SahneError::InvalidHandle);
          }
        let result = unsafe {
            syscall(arch::SYSCALL_RESOURCE_CONTROL, handle.raw(), request, arg, 0, 0)
        };
        if result < 0 {
            Err(map_kernel_error(result))
        } else {
            Ok(result) // Kontrol komutunun çekirdekten dönen ham değeri
        }
    }

    // (Yeni Özellik) SeekFrom enum'ı: Kaynak içinde konumlanma noktaları
    #[derive(Debug, Copy, Clone, PartialEq, Eq)]
    #[repr(u64)] // Çekirdeğe u64 olarak gönderilecek
    pub enum SeekFrom {
        Start(u64),   // Başlangıçtan itibaren
        Current(i64), // Mevcut konumdan itibaren (ileri/geri)
        End(i64),     // Sondan itibaren (genellikle negatif)
    }

    /// (Yeni Özellik) Belirtilen Handle ile temsil edilen kaynak içinde konumlanır.
    /// `pos`: Konumlanma noktası ve ofseti.
    /// Başarı durumunda yeni ofseti döner.
    pub fn seek(handle: Handle, pos: SeekFrom) -> Result<u64, SahneError> {
        if !handle.is_valid() {
            return Err(SahneError::InvalidHandle);
        }
        let (whence, offset) = match pos {
            SeekFrom::Start(o) => (0u64, o as i64), // Karnal64 0:Start, 1:Current, 2:End bekleyebilir
            SeekFrom::Current(o) => (1u64, o),
            SeekFrom::End(o) => (2u64, o),
        };
        let result = unsafe {
            syscall(arch::SYSCALL_RESOURCE_SEEK, handle.raw(), whence, offset as u64, 0, 0) // Offset i64 olsa da, syscall argları u64
        };
        if result < 0 {
            Err(map_kernel_error(result))
        } else {
            Ok(result as u64) // Başarı durumunda yeni ofset döner (u64)
        }
    }

    // (Yeni Özellik) ResourceStatus struct'ı: Kaynak durumu bilgileri
    // Çekirdekten dönecek veriyi tutar
    #[derive(Debug, Copy, Clone, PartialEq, Eq)]
    #[repr(C)] // C ABI uyumu (syscall ile data transferi için)
    pub struct ResourceStatus {
        pub size: u64,       // Kaynak boyutu (dosya boyutu gibi)
        pub type_flags: u32, // Kaynak tipi ve özellik bayrakları (dosya, cihaz, pipe, socket vb.)
        pub link_count: u62, // Kaynağa bağlı link sayısı (filesystemler için)
        pub reserved: u32,   // Hizalanma veya gelecekte kullanım için
        // TODO: Erişim zamanları, izinler gibi diğer stat bilgileri
    }

    /// (Yeni Özellik) Belirtilen Handle ile temsil edilen kaynağın durum bilgilerini (stat) alır.
    /// `status_buffer`: Çekirdekten durum bilgisinin kopyalanacağı tampon.
    /// Başarı durumunda doldurulmuş ResourceStatus struct'ını döndürmek yerine,
    /// syscall'ın tampona yazmasını sağlayıp success dönmek daha yaygındır.
    pub fn stat(handle: Handle, status_buffer: &mut ResourceStatus) -> Result<(), SahneError> {
         if !handle.is_valid() {
            return Err(SahneError::InvalidHandle);
        }
        let buffer_ptr = status_buffer as *mut ResourceStatus as u64;
        let buffer_len = core::mem::size_of::<ResourceStatus>() as u64; // Tampon boyutu struct boyutu olmalı

        let result = unsafe {
            syscall(arch::SYSCALL_RESOURCE_STAT, handle.raw(), buffer_ptr, buffer_len, 0, 0)
        };
         if result < 0 {
            Err(map_kernel_error(result))
        } else {
            // Başarı durumunda çekirdek tamponu doldurmuş demektir.
            Ok(())
        }
    }
}

// Çekirdek ile genel etkileşim modülü (Daha fazla info türü eklenebilir)
pub mod kernel {
    use super::{SahneError, arch, syscall, map_kernel_error, map_kernel_ok_result};

    // Çekirdek bilgi türleri için Sahne64'e özgü sabitler (Karnal64 info türleri ile eşleşmeli)
    pub const KERNEL_INFO_VERSION_MAJOR: u32 = 1;
    pub const KERNEL_INFO_VERSION_MINOR: u32 = 2;
    pub const KERNEL_INFO_BUILD_ID: u32 = 3;
    pub const KERNEL_INFO_UPTIME_SECONDS: u32 = 4; // Sistem çalışma süresi (saniye)
    pub const KERNEL_INFO_ARCHITECTURE: u32 = 5;   // Çalışan mimari (örn. ARCH_X86_64 sabiti dönebilir)
    pub const KERNEL_INFO_TOTAL_MEMORY_BYTES: u32 = 6; // (Yeni) Toplam fiziksel bellek
    pub const KERNEL_INFO_FREE_MEMORY_BYTES: u32 = 7;  // (Yeni) Boş fiziksel bellek
    // ... diğer Sahne64'e özgü kernel bilgileri

    /// Çekirdekten belirli bir bilgiyi alır.
    pub fn get_info(info_type: u32) -> Result<u64, SahneError> {
        let result = unsafe {
            syscall(arch::SYSCALL_GET_KERNEL_INFO, info_type as u64, 0, 0, 0, 0)
        };
          if result < 0 {
              Err(map_kernel_error(result))
          } else {
              Ok(result as u64) // Başarı durumunda bilgi değeri döner (u64)
          }
    }

    /// Sistem saatini (örneğin, epoch'tan beri geçen nanosaniye olarak) alır.
    pub fn get_time() -> Result<u64, SahneError> {
        let result = unsafe {
            syscall(arch::SYSCALL_GET_SYSTEM_TIME, 0, 0, 0, 0, 0)
        };
          if result < 0 {
              Err(map_kernel_error(result))
          } else {
              Ok(result as u64) // Başarı durumunda zaman değeri döner (u64)
          }
    }
}

// Senkronizasyon araçları modülü (Mutex -> Lock)
// Yeni kilit türleri veya try_acquire gibi fonksiyonlar eklenebilir.
pub mod sync {
    use super::{SahneError, arch, syscall, map_kernel_error, map_kernel_ok_result, Handle};

    /// Yeni bir kilit (Lock) kaynağı oluşturur ve bunun için bir Handle döner.
    pub fn lock_create() -> Result<Handle, SahneError> {
        let result = unsafe {
            syscall(arch::SYSCALL_LOCK_CREATE, 0, 0, 0, 0, 0)
        };
        if result < 0 {
            Err(map_kernel_error(result))
        } else {
            Ok(Handle(result as u64)) // Başarı durumunda handle döner
        }
    }

    /// Belirtilen Handle'a sahip kilidi almaya çalışır.
    /// Kilit başka bir thread/task tarafından tutuluyorsa, çağıran bloke olur.
    pub fn lock_acquire(lock_handle: Handle) -> Result<(), SahneError> {
          if !lock_handle.is_valid() {
              return Err(SahneError::InvalidHandle);
          }
        let result = unsafe {
            syscall(arch::SYSCALL_LOCK_ACQUIRE, lock_handle.raw(), 0, 0, 0, 0)
        };
        if result < 0 {
            Err(map_kernel_error(result))
        } else {
            Ok(()) // Başarı
        }
    }

    /// Belirtilen Handle'a sahip kilidi serbest bırakır.
    /// Kilidin çağıran thread/task tarafından tutuluyor olması gerekir.
    pub fn lock_release(lock_handle: Handle) -> Result<(), SahneError> {
          if !lock_handle.is_valid() {
              return Err(SahneError::InvalidHandle);
          }
        let result = unsafe {
            syscall(arch::SYSCALL_LOCK_RELEASE, lock_handle.raw(), 0, 0, 0, 0)
        };
        if result < 0 {
            Err(map_kernel_error(result))
        } else {
            Ok(()) // Başarı
        }
    }
    // NOT: Kilit Handle'ı resource::release ile tamamen yok edilebilir.
}

// Görevler arası iletişim (IPC) modülü (Handle tabanlı kanallar eklendi)
pub mod messaging {
    use super::{SahneError, arch, syscall, map_kernel_error, map_kernel_ok_result, TaskId, Handle, resource};
    use core::time::Duration;

    // Sahne64'te mesajlaşma kanalları veya portlar da Handle ile temsil edilebilir.
    // Önceki TaskId üzerinden doğrudan mesajlaşma basitti, şimdi Handle tabanlı kanallar ekleyelim.

    /// (Yeni Özellik) Yeni bir mesaj kanalı kaynağı oluşturur.
    /// Kanallar tek yönlü veya çift yönlü olabilir (mode bayrakları ile?).
    /// Başarı durumunda kanalın bir ucunu temsil eden bir Handle döner.
    /// Diğer ucuna bağlanmak için bu Handle'ı (veya ResourceId karşılığını) paylaşmak gerekir.
    pub fn create_channel() -> Result<Handle, SahneError> {
        // mode bayrakları eklenebilir (tek/çift yönlü, buffer boyutu vb.)
        let result = unsafe {
            syscall(arch::SYSCALL_CHANNEL_CREATE, 0, 0, 0, 0, 0)
        };
        if result < 0 {
            Err(map_kernel_error(result))
        } else {
            Ok(Handle(result as u64)) // Başarı durumunda kanal handle'ı döner
        }
    }

    /// (Yeni Özellik) Mevcut bir mesaj kanalına ResourceId'si (veya Handle'ı) üzerinden bağlanır.
    /// `channel_id`: Kanalı tanımlayan ResourceId veya başka bir tanımlayıcı.
    /// Başarı durumunda kanalın diğer ucuna erişim sağlayan bir Handle döner.
    pub fn connect_channel(channel_id: resource::ResourceId) -> Result<Handle, SahneError> {
        let id_ptr = channel_id.as_ptr() as u64;
        let id_len = channel_id.len() as u64;
        let result = unsafe {
            // Karnal64 tarafında bu ResourceId ile kanalı bulup,
            // çağıran göreve yeni bir Handle oluşturup vermesi gerekir.
            syscall(arch::SYSCALL_CHANNEL_CONNECT, id_ptr, id_len, 0, 0, 0)
        };
         if result < 0 {
            Err(map_kernel_error(result))
        } else {
            Ok(Handle(result as u64)) // Başarı durumunda yeni kanal handle'ı döner
        }
    }


    /// (Yeni Özellik) Belirtilen kanal handle'ı üzerinden mesaj gönderir.
    /// Kanal doluysa veya bloklama modunda değilse hata dönebilir.
    /// `channel_handle`: Mesajın gönderileceği kanalın Handle'ı.
    /// `message`: Gönderilecek veri.
    pub fn send_on_channel(channel_handle: Handle, message: &[u8]) -> Result<(), SahneError> {
        if !channel_handle.is_valid() {
            return Err(SahneError::InvalidHandle);
        }
        let msg_ptr = message.as_ptr() as u64;
        let msg_len = message.len() as u64;
        let result = unsafe {
            syscall(arch::SYSCALL_CHANNEL_SEND, channel_handle.raw(), msg_ptr, msg_len, 0, 0)
        };
        if result < 0 {
            Err(map_kernel_error(result)) // Hata (örn. WouldBlock, Disconnected, InvalidHandle)
        } else {
            Ok(()) // Başarı
        }
    }

    /// (Yeni Özellik) Belirtilen kanal handle'ı üzerinden mesaj alır.
    /// Kanal boşsa veya bloklama modunda değilse hata dönebilir.
    /// `channel_handle`: Mesajın alınacağı kanalın Handle'ı.
    /// `buffer`: Mesajın kopyalanacağı tampon.
    /// Başarı durumunda alınan mesajın byte cinsinden boyutunu döner.
    pub fn receive_on_channel(channel_handle: Handle, buffer: &mut [u8]) -> Result<usize, SahneError> {
         if !channel_handle.is_valid() {
            return Err(SahneError::InvalidHandle);
        }
        let buffer_ptr = buffer.as_mut_ptr() as u64;
        let buffer_len = buffer.len() as u64;
        let result = unsafe {
            syscall(arch::SYSCALL_CHANNEL_RECEIVE, channel_handle.raw(), buffer_ptr, buffer_len, 0, 0)
        };
        if result < 0 {
            Err(map_kernel_error(result)) // Hata (örn. WouldBlock, Disconnected, InvalidHandle)
        } else {
            Ok(result as usize) // Başarı (alınan byte sayısı)
        }
    }

    // Eski, doğrudan Task ID'ye mesaj gönderme fonksiyonları da kalabilir veya kaldırılabilir.
     Pub fn send(...)
     Pub fn receive(...) // Bu fonksiyon muhtemelen mevcut görevin varsayılan mesaj kuyruğuna bağlıdır.
    // Yeni channel API'sı daha genel ve esnektir.
}

// Yeni bir modül: Polling ve Olay Yönetimi
// Birden çok handle üzerinde eşzamanlı olarak olay (okuma, yazma, bağlantı kesilmesi vb.) bekleme mekanizması.
pub mod poll {
    use super::{SahneError, arch, syscall, map_kernel_error, map_kernel_ok_result, Handle};
    use core::time::Duration;

    // Beklenecek ve dönecek olay türleri bayrakları
    #[derive(Debug, Copy, Clone, PartialEq, Eq)]
    #[repr(u32)] // Çekirdeğe u32 olarak gönderilir/alınır
    pub enum PollEventFlags {
        NONE = 0,
        READABLE = 1 << 0,  // Okunmaya hazır (veri var veya bağlantı kapandı)
        WRITABLE = 1 << 1,  // Yazmaya hazır (buffer'da yer var)
        ERROR = 1 << 2,     // Hata durumu
        DISCONNECTED = 1 << 3, // Bağlantı kesildi
        // TODO: Diğer olay türleri (kilit açıldı, görev sonlandı vb.)
    }

    impl core::ops::BitOr for PollEventFlags {
        type Output = Self;
        fn bitor(self, other: Self) -> Self {
            unsafe { core::mem::transmute(self as u32 | other as u32) }
        }
    }

    impl core::ops::BitAnd for PollEventFlags {
         type Output = Self;
        fn bitand(self, other: Self) -> Self {
            unsafe { core::mem::transmute(self as u32 & other as u32) }
        }
    }

     impl core::ops::BitOrAssign for PollEventFlags {
        fn bitor_assign(&mut self, other: Self) {
            *self = *self | other;
        }
    }

     impl core::ops::BitAndAssign for PollEventFlags {
         fn bitand_assign(&mut self, other: Self) {
            *self = *self & other;
        }
     }


    /// (Yeni Özellik) PollEntry struct'ı: Poll çağrısı için tek bir handle girişi
    #[derive(Debug, Copy, Clone, PartialEq, Eq)]
    #[repr(C)] // C ABI uyumu (syscall ile array olarak geçecek)
    pub struct PollEntry {
        pub handle: Handle,         // Beklenecek handle
        pub events_in: PollEventFlags, // Beklenen olaylar (input)
        pub events_out: PollEventFlags, // Gerçekleşen olaylar (output, çekirdek doldurur)
    }

    /// (Yeni Özellik) Birden çok handle üzerinde belirli olayların gerçekleşmesini bekler.
    /// `entries`: Beklenecek handle'lar ve beklenen olayları içeren PollEntry dizisi.
    ///            Çekirdek bu diziyi güncelleyerek gerçekleşen olayları `events_out` alanına yazar.
    /// `timeout`: Ne kadar bekleneceği. None ise sonsuza kadar bekler. Some(Duration) ise belirtilen süre kadar bekler.
    /// Başarı durumunda, olay gerçekleşen (events_out != NONE) entry sayısını döner.
    pub fn poll(entries: &mut [PollEntry], timeout: Option<Duration>) -> Result<usize, SahneError> {
        let entries_ptr = entries.as_mut_ptr() as u64;
        let entries_len = entries.len() as u64;
        let timeout_ms = match timeout {
            Some(d) => d.as_millis() as i64, // ms olarak i64'e çevir
            None => -1, // Sonsuz bekleme için özel değer (Unix poll'daki -1 gibi)
        };

        // Syscall argümanları (handle listesi pointer/uzunluk, timeout)
        let result = unsafe {
            syscall(arch::SYSCALL_POLL, entries_ptr, entries_len, timeout_ms as u64, 0, 0)
        };

        if result < 0 {
            Err(map_kernel_error(result)) // Hata (örn. Interrupted, InvalidArgument)
        } else {
            // Başarı durumunda, olay gerçekleşen entry sayısı döner.
            // Çekirdek, `entries` dizisindeki `events_out` alanlarını doldurmuş olmalıdır.
            Ok(result as usize)
        }
    }
}

// --- Re-export public API ---
pub use arch;
pub use memory;
pub use task;
pub use resource;
pub use kernel;
pub use sync;
pub use messaging;
pub use poll; // Yeni polling modülü
pub use {Handle, TaskId, SahneError}; // Export Rust-idiomatic types

// C API hata tipi de Rust tarafından kullanılabilir hale getirilebilir (isteğe bağlı)
 pub use crate::sahne_error_t; // C API bindinlerini ayrı bir dosyada tutmak daha temiz olabilir

// --- C API katmanı ve no_std gerekli implementasyonlar (önceki koddan) ---
// Bu kısım C API başlığını (sahne.h) ve kullanıcı alanında no_std print!/panic!
// desteğini sağlar. Burayı önceki sağladığınız koddan koruyabiliriz.
// Sadece yeni eklenen Rust fonksiyonları için C wrapper'ları eklemek gerekir.

#[no_mangle]
pub extern "C" fn sahne_resource_seek(handle: u64, whence: u64, offset: i64) -> i64 {
    let pos = match whence {
        0 => resource::SeekFrom::Start(offset as u64), // Dikkat: i64 -> u64 taşma?
        1 => resource::SeekFrom::Current(offset),
        2 => resource::SeekFrom::End(offset),
        _ => return SahneError::InvalidParameter as i64, // Karnal64'teki eşleşen hata kodu
    };
    match resource::seek(Handle(handle), pos) {
        Ok(new_offset) => new_offset as i64,
        Err(e) => map_sahne_error_to_c(e) as i64, // map_sahne_error_to_c fonksiyonu gerekli
    }
}

// --- no_std için Gerekli Olabilecekler (önceki koddan) ---

#[cfg(not(test))]
#[panic_handler]
fn panic(_info: &core::panic::PanicInfo) -> ! { ... } // Çekirdek içi panic işleyici çağırmalı

#[cfg(not(feature = "std"))]
mod stdio_impl { ... print!, println!, eprintln! makroları ... } // Çekirdek konsol kaynağına yazmalı


// --- Örnek Kullanım (std gerektirir veya no_std ortamında özel entry point gerekir) ---

#[cfg(feature = "std")] // Test/örnek amaçlı
fn main() {
    // Güncellenmiş API fonksiyonlarını kullanın
     use crate::{task, resource, messaging, poll, ...};

    println!("Güncellenmiş Sahne64 API Örneği Başlatıldı!");

    // Kaynakta konumlanma örneği
     let file_handle = resource::acquire("sahne://test/data", resource::MODE_READ).expect("Kaynak edinilemedi");
     let initial_offset = resource::seek(file_handle, resource::SeekFrom::Current(0)).expect("Ofset alınamadı");
     println!("Başlangıç ofseti: {}", initial_offset);
     resource::seek(file_handle, resource::SeekFrom::Start(100)).expect("Konumlanamadı");
     let new_offset = resource::seek(file_handle, resource::SeekFrom::Current(0)).expect("Yeni ofset alınamadı");
     println!("Yeni ofset: {}", new_offset);
     resource::release(file_handle).expect("Kaynak bırakılamadı");

    // Kanal mesajlaşma örneği
     let tx_channel = messaging::create_channel().expect("Kanal oluşturulamadı");
     println!("Kanal oluşturuldu: {:?}", tx_channel);
     let rx_channel = messaging::connect_channel("sahne://kanal/kanal_adi").expect("Kanala bağlanılamadı"); // Önce kanalın bir ResourceId ile kaydedilmesi lazım
     messaging::send_on_channel(tx_channel, b"Merhaba Kanal!").expect("Mesaj gönderilemedi");
     let mut buffer = [0u8; 64];
     match messaging::receive_on_channel(rx_channel, &mut buffer) { ... }

    // Polling örneği (İki dummy handle üzerinde bekleyelim)
     let handle1 = Handle(1); // Varsayımsal handle
     let handle2 = Handle(2);
     let mut entries = [
         poll::PollEntry { handle: handle1, events_in: poll::PollEventFlags::READABLE, events_out: poll::PollEventFlags::NONE },
         poll::PollEntry { handle: handle2, events_in: poll::PollEventFlags::READABLE | poll::PollEventFlags::ERROR, events_out: poll::PollEventFlags::NONE },
     ];
     println!("Handle'larda olay bekleniyor (1s timeout)...");
     match poll::poll(&mut entries, Some(Duration::from_secs(1))) {
         Ok(num_ready) => {
             println!("{} handle hazır.", num_ready);
             for entry in entries.iter() {
                 if entry.events_out != poll::PollEventFlags::NONE {
                     println!("Handle {:?} hazır olaylar: {:?}", entry.handle, entry.events_out);
                 }
             }
         }
         Err(e) => eprintln!("Poll hatası: {:?}", e),
     }


     task::exit(0);
}
