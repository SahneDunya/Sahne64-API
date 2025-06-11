import core.stdc.stdint;
import core.stdc.stddef;
import core.stdc.stdio; // printf, fprintf için
import core.string;     // toStringz için
import core.time;       // core.time.Duration için


// sahne.h başlığındaki tüm tipleri, sabitleri ve fonksiyonları D'ye bind et
// Bu, extern(C) bloğu içinde yapılır.
extern(C) {
    // --- Temel Tipler ---
    alias uint64_t sahne_handle_t;
    alias uint64_t sahne_task_id_t;
    alias int32_t sahne_error_t;

    // --- Başarı/Hata Kodları ---
    enum SAHNE_SUCCESS = 0;
    // sahne.h'deki diğer SAHNE_ERROR_* sabitleri buraya eklenmeli
    enum SAHNE_ERROR_OUT_OF_MEMORY = 1;
    enum SAHNE_ERROR_INVALID_PARAMETER = 3;
    enum SAHNE_ERROR_INVALID_HANDLE = 13;
    enum SAHNE_ERROR_RESOURCE_NOT_FOUND = 4;
    enum SAHNE_ERROR_PERMISSION_DENIED = 5;
    enum SAHNE_ERROR_RESOURCE_BUSY = 6;
    enum SAHNE_ERROR_INTERRUPTED = 7;
    enum SAHNE_ERROR_NO_MESSAGE = 8;
    enum SAHNE_ERROR_INVALID_OPERATION = 9;
    enum SAHNE_ERROR_NOT_SUPPORTED = 10;
    enum SAHNE_ERROR_UNKNOWN_SYSCALL = 11;
    enum SAHNE_ERROR_TASK_CREATION_FAILED = 12;
    enum SAHNE_ERROR_HANDLE_LIMIT_EXCEEDED = 14;
    enum SAHNE_ERROR_NAMING_ERROR = 15;
    enum SAHNE_ERROR_COMMUNICATION_ERROR = 16;
    enum SAHNE_ERROR_WOULD_BLOCK = 17;
    enum SAHNE_ERROR_DISCONNECTED = 18;


    // --- Sistem Çağrı Numaraları ---
    enum SAHNE_SYSCALL_MEMORY_ALLOCATE = 1;
    // ... diğer SYSCALL_* sabitleri sahne.h'den buraya kopyalanmalı ...
    enum SAHNE_SYSCALL_RESOURCE_SEEK = 103;
    enum SAHNE_SYSCALL_RESOURCE_STAT = 104;
    enum SAHNE_SYSCALL_TASK_WAIT = 105;
    enum SAHNE_SYSCALL_CHANNEL_CREATE = 106;
    enum SAHNE_SYSCALL_CHANNEL_CONNECT = 107;
    enum SAHNE_SYSCALL_CHANNEL_SEND = 108;
    enum SAHNE_SYSCALL_CHANNEL_RECEIVE = 109;
    enum SAHNE_SYSCALL_POLL = 110;


    // --- Kaynak Modları ---
    enum SAHNE_MODE_READ = (1 << 0);
    // ... diğer MODE_* sabitleri sahne.h'den buraya kopyalanmalı ...
    enum SAHNE_MODE_WRITE = (1 << 1);
    enum SAHNE_MODE_CREATE = (1 << 2);
    enum SAHNE_MODE_NONBLOCK = (1 << 5);


    // --- Kernel Info Türleri ---
    enum SAHNE_KERNEL_INFO_VERSION_MAJOR = 1;
    // ... diğer KERNEL_INFO_* sabitleri sahne.h'den buraya kopyalanmalı ...
    enum SAHNE_KERNEL_INFO_TOTAL_MEMORY_BYTES = 6;


    // --- Yeni Eklenen Yapılar ve Enum Karşılıkları ---

    // resource::SeekFrom enum karşılıkları için sabitler
    enum SAHNE_SEEK_SET = 0;
    enum SAHNE_SEEK_CUR = 1;
    enum SAHNE_SEEK_END = 2;

    // resource::ResourceStatus struct karşılığı
    struct ResourceStatus_t {
        uint64_t size;
        uint32_t type_flags;
        uint32_t link_count;
        uint32_t reserved;
        // TODO: Diğer alanlar sahne.h'deki ResourceStatus_t ile senkron olmalı
    }

    // poll::PollEventFlags enum karşılığı ve sabitleri
    alias uint32_t PollEventFlags_t;
    enum SAHNE_POLL_NONE = 0;
    enum SAHNE_POLL_READABLE = (1 << 0);
    enum SAHNE_POLL_WRITABLE = (1 << 1);
    enum SAHNE_POLL_ERROR = (1 << 2);
    enum SAHNE_POLL_DISCONNECTED = (1 << 3);
    // TODO: Diğer PollEventFlags sabitleri


    // poll::PollEntry struct karşılığı
    struct PollEntry_t {
        sahne_handle_t handle;
        PollEventFlags_t events_in;
        PollEventFlags_t events_out;
    }


    // --- Fonksiyon Bildirimleri (sahne.h'deki tüm fonksiyonlar) ---

    // Düşük Seviye Syscall Arayüzü
    int64_t sahne_raw_syscall(uint64_t number, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);

    // Bellek Yönetimi
    sahne_error_t sahne_mem_allocate(size_t size, void** out_ptr);
    sahne_error_t sahne_mem_release(void* ptr, size_t size);
    sahne_error_t sahne_mem_create_shared(size_t size, sahne_handle_t* out_handle);
    sahne_error_t sahne_mem_map_shared(sahne_handle_t handle, size_t offset, size_t size, void** out_ptr);
    sahne_error_t sahne_mem_unmap_shared(void* addr, size_t size);

    // Görev Yönetimi
    sahne_error_t sahne_task_spawn(sahne_handle_t code_handle, const uint8_t* args_ptr, size_t args_len, const sahne_handle_t* initial_handles_ptr, size_t initial_handles_len, sahne_task_id_t* out_task_id);
    void sahne_task_exit(int32_t code); // D'de 'noreturn' belirtilmez
    sahne_error_t sahne_task_current_id(sahne_task_id_t* out_task_id);
    sahne_error_t sahne_task_sleep(uint64_t milliseconds);
    sahne_error_t sahne_thread_create(void function(void*), size_t stack_size, void* arg, uint64_t* out_thread_id); // D'de C function pointer tipi
    void sahne_thread_exit(int32_t code); // D'de 'noreturn'
    sahne_error_t sahne_task_yield();
    sahne_error_t sahne_task_wait_for_exit(sahne_task_id_t task_id, int32_t* out_exit_code); // Yeni

    // Kaynak Yönetimi
    sahne_error_t sahne_resource_acquire(const uint8_t* id_ptr, size_t id_len, uint32_t mode, sahne_handle_t* out_handle);
    sahne_error_t sahne_resource_read(sahne_handle_t handle, uint8_t* buffer_ptr, size_t buffer_len, size_t* out_bytes_read);
    sahne_error_t sahne_resource_write(sahne_handle_t handle, const uint8_t* buffer_ptr, size_t buffer_len, size_t* out_bytes_written);
    sahne_error_t sahne_resource_release(sahne_handle_t handle);
    sahne_error_t sahne_resource_control(sahne_handle_t handle, uint64_t request, uint64_t arg, int64_t* out_result);
    sahne_error_t sahne_resource_seek(sahne_handle_t handle, uint64_t whence, int64_t offset, uint64_t* out_new_offset); // Yeni
    sahne_error_t sahne_resource_stat(sahne_handle_t handle, ResourceStatus_t* out_status); // Yeni

    // Çekirdek Etkileşimi
    sahne_error_t sahne_kernel_get_info(uint32_t info_type, uint64_t* out_value);
    sahne_error_t sahne_kernel_get_time(uint64_t* out_time);

    // Senkronizasyon
    sahne_error_t sahne_sync_lock_create(sahne_handle_t* out_handle);
    sahne_error_t sahne_sync_lock_acquire(sahne_handle_t handle);
    sahne_error_t sahne_sync_lock_release(sahne_handle_t handle);

    // Mesajlaşma / IPC (Kanallar)
    sahne_error_t sahne_channel_create(sahne_handle_t* out_channel_handle); // Yeni
    sahne_error_t sahne_channel_connect(const uint8_t* channel_id_ptr, size_t channel_id_len, sahne_handle_t* out_channel_handle); // Yeni
    sahne_error_t sahne_channel_send(sahne_handle_t channel_handle, const uint8_t* message_ptr, size_t message_len); // Yeni
    sahne_error_t sahne_channel_receive(sahne_handle_t channel_handle, uint8_t* buffer_ptr, size_t buffer_len, size_t* out_bytes_received); // Yeni

    // Eski Mesajlaşma (Task ID ile) - isteğe bağlı olarak kaldırılabilir
    sahne_error_t sahne_msg_send(sahne_task_id_t target_task, const uint8_t* message_ptr, size_t message_len);
    sahne_error_t sahne_msg_receive(uint8_t* buffer_ptr, size_t buffer_len, size_t* out_bytes_received);

    // Polling / Olay Yönetimi
    int64_t sahne_poll(PollEntry_t* entries, size_t num_entries, int64_t timeout_ms); // Yeni (dönüş tipi int64_t, hata için negatif)


    // TODO: Diğer tüm fonksiyon bildirimleri sahne.h'den buraya kopyalanmalı
}


// Yeni bir görevde çalışacak örnek fonksiyon (D)
// Çekirdeğin (Karnal64'ü kullanan) bu kod için bir yürütülebilir kaynak handle'ı sağlayabildiği varsayılır.
// C uyumlu olması için extern(C) gerekebilir eğer çekirdek böyle bir fonksiyon pointer'ı bekliyorsa.
extern(C) void child_task_entry_d(void* arg) {
    // Argüman pointer'ı kullanılabilir
     int exit_code = (arg is null) ? 0 : *cast(int*)arg;

    printf("Child Task (D): Started, will exit with code 42.\n");

    // Görevi bir çıkış kodu ile sonlandır
    sahne_task_exit(42); // Görev 42 koduyla sonlanacak
    // Buradan sonrası çalışmaz
}


void main() {
    sahne_task_id_t task_id;
    sahne_error_t err;

    printf("Sahne64 D Program Starting (Extended API)...\n");

    // Mevcut görev ID'sini al
    err = sahne_task_current_id(&task_id);
    if (err == SAHNE_SUCCESS) {
        printf("Current Task ID: %llu\n", task_id); // D'de %llu uint64_t için kullanılır
    } else {
        fprintf(stderr, "Failed to get Task ID, error: %d\n", err);
    }

    // Bellek tahsisi (mevcut kod - D idiomları ile)
    void* allocated_mem = null;
    size_t mem_size = 1024;
    err = sahne_mem_allocate(mem_size, &allocated_mem);
    if (err == SAHNE_SUCCESS) {
        printf("Allocated %zu bytes at %p\n", mem_size, allocated_mem);
        if (allocated_mem !is null) {
             *cast(uint8_t*)allocated_mem = 42; // Örnek kullanım
        }
        // Belleği serbest bırak (sonra yapalım)
         err = sahne_mem_release(allocated_mem, mem_size);
        // ... hata kontrolü ...
    } else {
        fprintf(stderr, "Memory allocation failed, error: %d\n", err);
    }


    // --- Yeni Özellik: Kaynakta Konumlanma ve Durum Alma (Seek & Stat) ---
    sahne_handle_t seekable_file_handle = 0;
    string file_res_name = "sahne://app_data/log_d.txt"; // D örneği için farklı isim
    uint32_t file_mode = SAHNE_MODE_READ | SAHNE_MODE_WRITE | SAHNE_MODE_CREATE;

    printf("\n--- Kaynak Seek ve Stat Örneği (D) ---\n");
    err = sahne_resource_acquire(cast(const uint8_t*)toStringz(file_res_name), file_res_name.length, file_mode, &seekable_file_handle);
    if (err == SAHNE_SUCCESS) {
        printf("Acquired seekable resource '%s', Handle: %llu\n", toStringz(file_res_name), seekable_file_handle);

        uint64_t current_pos = 0;
        // Mevcut konumu al
        err = sahne_resource_seek(seekable_file_handle, SAHNE_SEEK_CUR, 0, &current_pos);
        if (err == SAHNE_SUCCESS) {
            printf("Initial position: %llu\n", current_pos);
        } else {
            fprintf(stderr, "Failed to get initial position, error: %d\n", err);
        }

        // Konumu 100 byte ileri al (başlangıçtan itibaren)
        err = sahne_resource_seek(seekable_file_handle, SAHNE_SEEK_SET, 100, &current_pos);
         if (err == SAHNE_SUCCESS) {
            printf("Seeked to position 100. New position: %llu\n", current_pos);
        } else {
            fprintf(stderr, "Failed to seek, error: %d\n", err);
        }

        // Kaynak durumunu al (Stat)
        ResourceStatus_t file_status; // D'de C yapısı kullanılır
        err = sahne_resource_stat(seekable_file_handle, &file_status);
        if (err == SAHNE_SUCCESS) {
            printf("Resource Stat:\n");
            printf("  Size: %llu bytes\n", file_status.size);
            printf("  Type/Flags: %x\n", file_status.type_flags); // Hex yazdırma
            printf("  Link Count: %u\n", file_status.link_count);
            // TODO: Diğer stat alanlarını yazdır
        } else {
            fprintf(stderr, "Failed to get resource status, error: %d\n", err);
        }

        // Handle'ı serbest bırak
        err = sahne_resource_release(seekable_file_handle);
        if (err == SAHNE_SUCCESS) {
             printf("Released seekable resource handle.\n");
        } else {
             fprintf(stderr, "Failed to release seekable resource handle, error: %d\n", err);
        }

    } else {
        fprintf(stderr, "Failed to acquire seekable resource '%s', error: %d\n", toStringz(file_res_name), err);
    }


    // --- Yeni Özellik: Görev Başlatma ve Sonlanmasını Bekleme (Spawn & Wait) ---
    sahne_handle_t child_code_handle = 0;
    sahne_task_id_t child_task_id;
    int32_t child_exit_code = -1;

    // Çocuk görev kodu için varsayımsal handle (child_task_entry_d fonksiyonunu çalıştırdığı varsayılır)
    child_code_handle.raw = 125; // Farklı bir varsayımsal değer

    printf("\n--- Görev Başlatma ve Bekleme Örneği (D) ---\n");
    // Task spawn çağrısı (D), argümanlar ve başlangıç handle listesiyle (şimdilik boş)
    err = sahne_task_spawn(child_code_handle, null, 0, null, 0, &child_task_id); // null yerine NULL kullanabilirsiniz
    if (err == SAHNE_SUCCESS) {
        printf("Child Task started with ID: %llu\n", child_task_id);

        // Başlatılan görevin sonlanmasını bekle
        printf("Waiting for child task %llu to exit...\n", child_task_id);
        err = sahne_task_wait_for_exit(child_task_id, &child_exit_code);

        if (err == SAHNE_SUCCESS) {
            printf("Child Task %llu exited with code: %d\n", child_task_id, child_exit_code);
        } else {
            fprintf(stderr, "Failed to wait for child task %llu, error: %d\n", child_task_id, err);
        }

    } else {
        fprintf(stderr, "Failed to spawn child task, error: %d\n", err);
    }


    // --- Yeni Özellik: Mesajlaşma Kanalları (D) ---
    sahne_handle_t channel_tx_handle = 0;

    printf("\n--- Mesajlaşma Kanalı Örneği (D) ---\n");
    // Yeni bir mesaj kanalı oluştur
    err = sahne_channel_create(&channel_tx_handle);
    if (err == SAHNE_SUCCESS) {
        printf("Message Channel created, Handle: %llu\n", channel_tx_handle);

        // Oluşturulan kanala mesaj gönder
        string msg_to_send = "Hello Channel D!"; // D string
        err = sahne_channel_send(channel_tx_handle, cast(const uint8_t*)msg_to_send.ptr, msg_to_send.length);
        if (err == SAHNE_SUCCESS) {
            printf("Sent message '%s' on channel %llu\n", toStringz(msg_to_send), channel_tx_handle);

            // Mesajı kanaldan al (Bloklayıcı olabilir)
            ubyte[64] received_buffer; // D static array
            size_t bytes_received = 0;
            printf("Attempting to receive message on channel %llu...\n", channel_tx_handle);
            err = sahne_channel_receive(channel_tx_handle, received_buffer.ptr, received_buffer.length, &bytes_received);
            if (err == SAHNE_SUCCESS) {
                // D array slice ve toStringz kullan
                printf("Received %zu bytes on channel %llu: '%.*s'\n", bytes_received, channel_tx_handle, cast(int)bytes_received, cast(const char*)received_buffer.ptr);
                // Veya D string'e çevirip yazdırabilirsiniz
                 printf("Received %zu bytes... '%s'\n", bytes_received, toStringz(cast(string)received_buffer[0..bytes_received]));

            } else if (err == SAHNE_ERROR_NO_MESSAGE) {
                printf("No message available on channel %llu (if non-blocking) or timed out.\n", channel_tx_handle);
            } else {
                fprintf(stderr, "Failed to receive message on channel %llu, error: %d\n", channel_tx_handle, err);
            }

        } else {
            fprintf(stderr, "Failed to send message on channel %llu, error: %d\n", channel_tx_handle, err);
        }

        // Kanal handle'ını serbest bırak
        err = sahne_resource_release(channel_tx_handle);
        if (err == SAHNE_SUCCESS) {
             printf("Released channel handle.\n");
        } else {
             fprintf(stderr, "Failed to release channel handle, error: %d\n", err);
        }

    } else {
        fprintf(stderr, "Failed to create message channel, error: %d\n", err);
    }


    // --- Yeni Özellik: Polling (D) ---
    sahne_handle_t console_read_handle = 0;
    sahne_handle_t dummy_event_handle = 0;

    // Konsol okuma handle'ını edin (varsayalım stdin handle'ı edinilebilir ve non-blocking yapılabilir)
    string stdin_res_name = "sahne://device/console/stdin";
    err = sahne_resource_acquire(cast(const uint8_t*)toStringz(stdin_res_name), stdin_res_name.length, SAHNE_MODE_READ | SAHNE_MODE_NONBLOCK, &console_read_handle);
    if (err != SAHNE_SUCCESS) {
         fprintf(stderr, "Warning: Failed to acquire console read handle for poll example, error: %d. Using dummy handle.\n", err);
         console_read_handle.raw = 99; // Varsayımsal dummy handle
    } else {
        printf("\nAcquired console read handle %llu for polling.\n", console_read_handle);
    }

    // Başka bir dummy olay handle'ı
    dummy_event_handle.raw = 100;

    printf("\n--- Polling Örneği (D) ---\n");

    // Poll edilecek entry dizisi oluştur (D static array kullan)
    PollEntry_t[2] poll_entries; // D static array

    // Entry 1: Konsol okuma handle'ı
    poll_entries[0].handle = console_read_handle;
    poll_entries[0].events_in = SAHNE_POLL_READABLE;
    poll_entries[0].events_out = SAHNE_POLL_NONE;

    // Entry 2: Dummy olay handle'ı
    poll_entries[1].handle = dummy_event_handle;
    poll_entries[1].events_in = SAHNE_POLL_ERROR | SAHNE_POLL_DISCONNECTED;
    poll_entries[1].events_out = SAHNE_POLL_NONE;

    // Poll çağrısı yap (Örn: 2 saniye timeout ile)
    auto timeout_duration = Duration.seconds(2); // D Duration
    int64_t timeout_ms = cast(int64_t)timeout_duration.total!(msecs); // int64_t milisaniyeye çevir
    printf("Polling on %zu handles with %lldms timeout...\n", poll_entries.length, timeout_ms); // D array .length

    int64_t num_ready = sahne_poll(poll_entries.ptr, poll_entries.length, timeout_ms); // D array .ptr ve .length kullan

    if (num_ready < 0) {
        // Hata durumunda negatif kerror_t değeri döner
        fprintf(stderr, "Poll failed, error: %lld\n", num_ready); // Ham hata kodunu yazdır
        // İsterseniz SahneError'a çevirip yazdırabilirsiniz
         sahne_error_t poll_err = cast(sahne_error_t)-num_ready; // Basit çevirim, tam map_kernel_error D'de yazılmalı
         fprintf(stderr, "Poll failed, error: %d (SahneError code)\n", poll_err);

    } else {
        // Başarı durumunda (>=0) olay gerçekleşen handle sayısı döner
        printf("Poll returned. %lld handle(s) ready.\n", num_ready);
        if (num_ready > 0) {
            // Hangi handle'ların hazır olduğunu kontrol et
            foreach (i; 0..poll_entries.length) { // D foreach döngüsü
                if (poll_entries[i].events_out != SAHNE_POLL_NONE) {
                    printf("  Handle %llu ready with events: %x\n", poll_entries[i].handle, poll_entries[i].events_out); // Hex yazdırma %x

                    // Gerçekleşen olay türlerine göre işlem yapabilirsiniz
                    if ((poll_entries[i].events_out & SAHNE_POLL_READABLE) != 0) { // Bitwise AND
                         printf("    -> Readable!\n");
                         // Okuma işlemini dene (non-blocking okuma burada uygun olabilir)
                          ubyte[16] temp_buf;
                          size_t temp_read = 0;
                          sahne_resource_read(poll_entries[i].handle, temp_buf.ptr, temp_buf.length, &temp_read);
                         // ... işlem sonucu kontrol et ...
                    }
                     if ((poll_entries[i].events_out & SAHNE_POLL_WRITABLE) != 0) {
                         printf("    -> Writable!\n");
                         // Yazma işlemini dene
                     }
                    // Diğer olay türlerini kontrol et...
                }
            }
        }
    }

    // Polling için edinilen handle'ı serbest bırak (eğer acquire edildiyse)
    if (console_read_handle.raw != 99) { // Sadece gerçekten acquire edildiyse
       sahne_resource_release(console_read_handle);
    }

    // Tahsis edilen belleği serbest bırak (main'in başındaki allocate için)
    if (allocated_mem !is null) {
         err = sahne_mem_release(allocated_mem, mem_size);
         if (err == SAHNE_SUCCESS) {
             printf("\nReleased initial allocated memory.\n");
         } else {
             fprintf(stderr, "\nFailed to release initial memory, error: %d\n", err);
         }
    }


    printf("\nSahne64 D Program Exiting (Extended API).\n");
    // Görevi normal çıkış koduyla sonlandır
    sahne_task_exit(0);
    // Buradan sonrası çalışmaz
     return 0; // D'de void main geri dönmez
}
