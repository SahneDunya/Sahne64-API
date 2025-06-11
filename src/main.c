#include "sahne.h"

// Standart C kütüphaneleri (Sahne64 üzerinde implemente edildiği varsayılır veya uyumludur)
#include <stdio.h>  // printf, fprintf
#include <string.h> // strlen
#include <stdlib.h> // EXIT_SUCCESS, EXIT_FAILURE (veya kendi çıkış kodları)
// task_sleep için unistd.h'ye gerek yok, sahne_task_sleep kullanıyoruz

// Yeni bir görevde çalışacak örnek fonksiyon (basitçe çıkış yapar)
// Çekirdeğin (Karnal64'ü kullanan) bu kod için bir yürütülebilir kaynak handle'ı sağlayabildiği varsayılır.
// Bu fonksiyonun adresinin veya derlenmiş halinin çekirdeğe nasıl iletilip handle alındığı
// işletim sisteminin başlatma/yükleme sürecine bağlıdır.
void child_task_entry(void* arg) {
    // Argüman pointer'ı kullanılabilir
     int exit_code = (arg == NULL) ? 0 : *(int*)arg;

    printf("Child Task: Started, will exit with code 42.\n");

    // Görevi bir çıkış kodu ile sonlandır
    sahne_task_exit(42); // Görev 42 koduyla sonlanacak
    // Buradan sonrası çalışmaz
}


int main() {
    sahne_task_id_t task_id;
    sahne_error_t err;

    printf("Sahne64 C Program Starting (Extended API)...\n");

    // Mevcut görev ID'sini al
    err = sahne_task_current_id(&task_id);
    if (err == SAHNE_SUCCESS) {
        printf("Current Task ID: %llu\n", (unsigned long long)task_id);
    } else {
        fprintf(stderr, "Failed to get Task ID, error: %d\n", err);
    }

    // Bellek tahsisi (mevcut kod)
    void* allocated_mem = NULL;
    size_t mem_size = 1024;
    err = sahne_mem_allocate(mem_size, &allocated_mem);
    if (err == SAHNE_SUCCESS) {
        printf("Allocated %zu bytes at %p\n", mem_size, allocated_mem);
        if (allocated_mem != NULL) {
             *((uint8_t*)allocated_mem) = 42; // Örnek kullanım
        }
        // Belleği serbest bırak (sonra yapalım, diğerlerini göstermek için kalsın)
         err = sahne_mem_release(allocated_mem, mem_size);
        // ... hata kontrolü ...
    } else {
        fprintf(stderr, "Memory allocation failed, error: %d\n", err);
    }


    // --- Yeni Özellik: Kaynakta Konumlanma ve Durum Alma (Seek & Stat) ---
    // Seekable ve Statable bir kaynak varsayalım (örn. bir dosya kaynağı)
    sahne_handle_t seekable_file_handle = 0;
    const char* file_res_name = "sahne://app_data/log.txt";
    uint32_t file_mode = SAHNE_MODE_READ | SAHNE_MODE_WRITE | SAHNE_MODE_CREATE; // Oku, Yaz, Yoksa Oluştur

    printf("\n--- Kaynak Seek ve Stat Örneği ---\n");
    err = sahne_resource_acquire((const uint8_t*)file_res_name, strlen(file_res_name), file_mode, &seekable_file_handle);
    if (err == SAHNE_SUCCESS) {
        printf("Acquired seekable resource '%s', Handle: %llu\n", file_res_name, (unsigned long long)seekable_file_handle);

        uint64_t current_pos = 0;
        // Mevcut konumu al (başlangıçtan 0 ofset ile seek)
        err = sahne_resource_seek(seekable_file_handle, SAHNE_SEEK_CUR, 0, &current_pos);
        if (err == SAHNE_SUCCESS) {
            printf("Initial position: %llu\n", (unsigned long long)current_pos);
        } else {
            fprintf(stderr, "Failed to get initial position, error: %d\n", err);
        }

        // Konumu 100 byte ileri al (başlangıçtan itibaren)
        err = sahne_resource_seek(seekable_file_handle, SAHNE_SEEK_SET, 100, &current_pos);
         if (err == SAHNE_SUCCESS) {
            printf("Seeked to position 100. New position: %llu\n", (unsigned long long)current_pos);
        } else {
            fprintf(stderr, "Failed to seek, error: %d\n", err);
        }

        // Kaynak durumunu al (Stat)
        ResourceStatus_t file_status;
        err = sahne_resource_stat(seekable_file_handle, &file_status);
        if (err == SAHNE_SUCCESS) {
            printf("Resource Stat:\n");
            printf("  Size: %llu bytes\n", (unsigned long long)file_status.size);
            printf("  Type/Flags: 0x%x\n", file_status.type_flags);
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
        fprintf(stderr, "Failed to acquire seekable resource '%s', error: %d\n", file_res_name, err);
    }


    // --- Yeni Özellik: Görev Başlatma ve Sonlanmasını Bekleme (Spawn & Wait) ---
    // Yeni bir görev başlatacağız ve onun bitmesini bekleyeceğiz.

    sahne_handle_t child_code_handle = 0; // Çocuk görev kodunun handle'ı
    sahne_task_id_t child_task_id;
    int32_t child_exit_code = -1;

    // Gerçekte bu handle, 'child_task_entry' fonksiyonunun derlenmiş kodunu
    // içeren bir kaynağa (örn. bir dosyaya) aittir ve çekirdek tarafından sağlanır.
    // Basitlik için burada varsayımsal bir handle kullanıyoruz.
    // Çekirdeğin bu handle'ı işleyip 'child_task_entry' adresinde kodu çalıştırabildiği varsayılır.
    child_code_handle.raw = 123; // Varsayımsal handle değeri

    printf("\n--- Görev Başlatma ve Bekleme Örneği ---\n");
    // Task spawn çağrısı, argümanlar ve başlangıç handle listesiyle (şimdilik boş)
    err = sahne_task_spawn(child_code_handle, NULL, 0, NULL, 0, &child_task_id); // argumanlar ve handle listesi boş
    if (err == SAHNE_SUCCESS) {
        printf("Child Task started with ID: %llu\n", (unsigned long long)child_task_id.raw);

        // Başlatılan görevin sonlanmasını bekle
        printf("Waiting for child task %llu to exit...\n", (unsigned long long)child_task_id.raw);
        err = sahne_task_wait_for_exit(child_task_id, &child_exit_code);

        if (err == SAHNE_SUCCESS) {
            printf("Child Task %llu exited with code: %d\n", (unsigned long long)child_task_id.raw, child_exit_code);
        } else {
            fprintf(stderr, "Failed to wait for child task %llu, error: %d\n", (unsigned long long)child_task_id.raw, err);
        }

    } else {
        fprintf(stderr, "Failed to spawn child task, error: %d\n", err);
    }


    // --- Yeni Özellik: Mesajlaşma Kanalları ---
    sahne_handle_t channel_tx_handle = 0;
    sahne_handle_t channel_rx_handle = 0; // Genellikle connect ile alınır veya spawn argümanı ile verilir

    printf("\n--- Mesajlaşma Kanalı Örneği ---\n");
    // Yeni bir mesaj kanalı oluştur
    err = sahne_channel_create(&channel_tx_handle);
    if (err == SAHNE_SUCCESS) {
        printf("Message Channel created, Handle: %llu\n", (unsigned long long)channel_tx_handle.raw);

        // Oluşturulan kanala mesaj gönder
        const char* msg_to_send = "Hello Channel!";
        err = sahne_channel_send(channel_tx_handle, (const uint8_t*)msg_to_send, strlen(msg_to_send));
        if (err == SAHNE_SUCCESS) {
            printf("Sent message '%s' on channel %llu\n", msg_to_send, (unsigned long long)channel_tx_handle.raw);

            // NOT: Gerçek bir senaryoda, mesajı BİR BAŞKA TASK'ın
            // kanalın diğer ucundan (veya bu handle'ın kopyasından)
            // sahne_channel_receive ile alması gerekir.
            // Basitlik için burada aynı handle'dan almaya çalışalım (eğer kernel destekliyorsa loopback).

            // Mesajı kanaldan al (Bloklayıcı olabilir)
            uint8_t received_buffer[64];
            size_t bytes_received = 0;
            printf("Attempting to receive message on channel %llu...\n", (unsigned long long)channel_tx_handle.raw); // Kanalın tipi loopback değilse bloklar
            err = sahne_channel_receive(channel_tx_handle, received_buffer, sizeof(received_buffer), &bytes_received);
            if (err == SAHNE_SUCCESS) {
                printf("Received %zu bytes on channel %llu: '%.*s'\n", bytes_received, (unsigned long long)channel_tx_handle.raw, (int)bytes_received, (const char*)received_buffer);
            } else if (err == SAHNE_ERROR_NO_MESSAGE) {
                printf("No message available on channel %llu (if non-blocking) or timed out.\n", (unsigned long long)channel_tx_handle.raw);
            }
             else {
                fprintf(stderr, "Failed to receive message on channel %llu, error: %d\n", (unsigned long long)channel_tx_handle.raw, err);
            }

        } else {
            fprintf(stderr, "Failed to send message on channel %llu, error: %d\n", (unsigned long long)channel_tx_handle.raw, err);
        }

        // Kanal handle'ını serbest bırak
        err = sahne_resource_release(channel_tx_handle); // Kanal handle'ları da resource handle'ları gibi release edilir
        if (err == SAHNE_SUCCESS) {
             printf("Released channel handle.\n");
        } else {
             fprintf(stderr, "Failed to release channel handle, error: %d\n", err);
        }


    } else {
        fprintf(stderr, "Failed to create message channel, error: %d\n", err);
    }


    // --- Yeni Özellik: Polling ---
    // Birden çok handle üzerinde olay bekleme.
    // Örnek olarak: Konsol okuma handle'ı, bir kanal handle'ı, belki bir timer handle'ı (varsa)
    sahne_handle_t console_read_handle = 0; // Konsoldan okuma handle'ı varsayalım (örn. stdin)
    sahne_handle_t dummy_event_handle = 0; // Başka bir olay handle'ı varsayalım (örn. timer veya özel olay kaynağı)

    // Konsol okuma handle'ını edin (varsayalım stdin handle'ı ResourceID ile edinilebilir)
    // Gerçekte stdin/stdout/stderr handle'ları genellikle görev başlatılırken verilir.
    // Basitlik için burada edinmeye çalışalım veya varsayımsal handle kullanalım.
    const char* stdin_res_name = "sahne://device/console/stdin";
    err = sahne_resource_acquire((const uint8_t*)stdin_res_name, strlen(stdin_res_name), SAHNE_MODE_READ | SAHNE_MODE_NONBLOCK, &console_read_handle);
    if (err != SAHNE_SUCCESS) {
         fprintf(stderr, "Warning: Failed to acquire console read handle for poll example, error: %d. Using dummy handle.\n", err);
         console_read_handle.raw = 99; // Varsayımsal dummy handle
    } else {
        printf("\nAcquired console read handle %llu for polling.\n", (unsigned long long)console_read_handle.raw);
    }

    // Başka bir dummy olay handle'ı (varsayalım kernel tarafından sağlanıyor)
    dummy_event_handle.raw = 100;

    printf("\n--- Polling Örneği ---\n");

    // Poll edilecek entry dizisi oluştur
    PollEntry_t poll_entries[2];

    // Entry 1: Konsol okuma handle'ı, okunmaya hazır olmayı bekle
    poll_entries[0].handle = console_read_handle;
    poll_entries[0].events_in = SAHNE_POLL_READABLE; // READABLE olayını bekle
    poll_entries[0].events_out = SAHNE_POLL_NONE;    // Başlangıçta gerçekleşen olay yok

    // Entry 2: Dummy olay handle'ı, hata veya bağlantı kesilmesi bekle
    poll_entries[1].handle = dummy_event_handle;
    poll_entries[1].events_in = SAHNE_POLL_ERROR | SAHNE_POLL_DISCONNECTED; // ERROR veya DISCONNECTED bekle
    poll_entries[1].events_out = SAHNE_POLL_NONE; // Başlangıçta gerçekleşen olay yok

    // Poll çağrısı yap (Örn: 2 saniye timeout ile)
    int64_t timeout_ms = 2000; // 2000 milisaniye = 2 saniye
    printf("Polling on %zu handles with %lldms timeout...\n", sizeof(poll_entries) / sizeof(poll_entries[0]), timeout_ms);

    int64_t num_ready = sahne_poll(poll_entries, sizeof(poll_entries) / sizeof(poll_entries[0]), timeout_ms);

    if (num_ready < 0) {
        // Hata durumunda negatif kerror_t değeri döner
        fprintf(stderr, "Poll failed, error: %lld\n", num_ready); // Ham hata kodunu yazdır
        // İsterseniz map_kernel_error ile SahneError'a çevirip yazdırabilirsiniz
         sahne_error_t poll_err = map_kernel_error(num_ready);
         fprintf(stderr, "Poll failed, error: %d (SahneError code)\n", poll_err);

    } else {
        // Başarı durumunda (>=0) olay gerçekleşen handle sayısı döner
        printf("Poll returned. %lld handle(s) ready.\n", num_ready);
        if (num_ready > 0) {
            // Hangi handle'ların hazır olduğunu kontrol et
            for (size_t i = 0; i < sizeof(poll_entries) / sizeof(poll_entries[0]); ++i) {
                if (poll_entries[i].events_out != SAHNE_POLL_NONE) {
                    printf("  Handle %llu ready with events: 0x%x\n", (unsigned long long)poll_entries[i].handle.raw, poll_entries[i].events_out);

                    // Gerçekleşen olay türlerine göre işlem yapabilirsiniz
                    if (poll_entries[i].events_out & SAHNE_POLL_READABLE) {
                         printf("    -> Readable!\n");
                         // Okuma işlemini dene (non-blocking okuma burada uygun olabilir)
                         uint8_t temp_buf[16];
                         size_t temp_read = 0;
                          sahne_resource_read(poll_entries[i].handle, temp_buf, sizeof(temp_buf), &temp_read); // Bu çağrı da non-blocking olmalı
                         // ... işlem sonucu kontrol et ...
                    }
                     if (poll_entries[i].events_out & SAHNE_POLL_WRITABLE) {
                         printf("    -> Writable!\n");
                         // Yazma işlemini dene
                     }
                    // Diğer olay türlerini kontrol et...
                }
            }
        }
    }

    // Polling için edinilen handle'ı serbest bırak (eğer edinildiyse)
    if (console_read_handle.raw != 99) { // Sadece gerçekten acquire edildiyse
       sahne_resource_release(console_read_handle);
    }

    // Tahsis edilen belleği serbest bırak (main'in başındaki allocate için)
    if (allocated_mem != NULL) {
         err = sahne_mem_release(allocated_mem, mem_size);
         if (err == SAHNE_SUCCESS) {
             printf("\nReleased initial allocated memory.\n");
         } else {
             fprintf(stderr, "\nFailed to release initial memory, error: %d\n", err);
         }
    }


    printf("\nSahne64 C Program Exiting (Extended API).\n");
    // Görevi normal çıkış koduyla sonlandır
    sahne_task_exit(0);
    // Buradan sonrası çalışmaz
    return 0;
}
