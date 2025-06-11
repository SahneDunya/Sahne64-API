#include "sahne.h"

// Standart C++ kütüphaneleri (Sahne64 üzerinde veya uyumlu bir şekilde implemente edildiği varsayılır)
#include <iostream> // std::cout, std::cerr, std::endl
#include <vector>   // std::vector
#include <string>   // std::string
#include <cstring>  // strlen (veya C++20 string::length)
#include <chrono>   // std::chrono::duration, std::chrono::milliseconds
#include <cstdint>  // uint*_t, int*_t (güvenlik için)
#include <cstdio>   // fprintf (fallback için)


// Yeni bir görevde çalışacak örnek fonksiyon (basitçe çıkış yapar)
// Çekirdeğin (Karnal64'ü kullanan) bu kod için bir yürütülebilir kaynak handle'ı sağlayabildiği varsayılır.
// C++'ta statik üye fonksiyon veya serbest (free) fonksiyon C uyumlu olabilir.
extern "C" void child_task_entry_cpp(void* arg) {
    // Argüman pointer'ı kullanılabilir
     int exit_code = (arg == nullptr) ? 0 : *static_cast<int*>(arg);

    std::cout << "Child Task (C++): Started, will exit with code 42." << std::endl;

    // Görevi bir çıkış kodu ile sonlandır
    sahne_task_exit(42); // Görev 42 koduyla sonlanacak
    // Buradan sonrası çalışmaz
}


int main() {
    sahne_task_id_t task_id;
    sahne_error_t err;

    std::cout << "Sahne64 C++ Program Starting (Extended API)..." << std::endl;

    // Mevcut görev ID'sini al
    err = sahne_task_current_id(&task_id);
    if (err == SAHNE_SUCCESS) {
        std::cout << "Current Task ID: " << static_cast<unsigned long long>(task_id) << std::endl;
    } else {
        std::cerr << "Failed to get Task ID, error: " << err << std::endl;
    }

    // Bellek tahsisi (mevcut kod - C++ idiomları ile)
    void* allocated_mem = nullptr;
    size_t mem_size = 1024;
    err = sahne_mem_allocate(mem_size, &allocated_mem);
    if (err == SAHNE_SUCCESS) {
        std::cout << "Allocated " << mem_size << " bytes at " << allocated_mem << std::endl;
        if (allocated_mem != nullptr) {
             *static_cast<uint8_t*>(allocated_mem) = 42; // Örnek kullanım
        }
        // Belleği serbest bırak (sonra yapalım)
         err = sahne_mem_release(allocated_mem, mem_size);
        // ... hata kontrolü ...
    } else {
        std::cerr << "Memory allocation failed, error: " << err << std::endl;
    }


    // --- Yeni Özellik: Kaynakta Konumlanma ve Durum Alma (Seek & Stat) ---
    sahne_handle_t seekable_file_handle = 0;
    std::string file_res_name = "sahne://app_data/log_cpp.txt"; // C++ örneği için farklı isim
    uint32_t file_mode = SAHNE_MODE_READ | SAHNE_MODE_WRITE | SAHNE_MODE_CREATE;

    std::cout << "\n--- Kaynak Seek ve Stat Örneği (C++) ---\n";
    err = sahne_resource_acquire(reinterpret_cast<const uint8_t*>(file_res_name.c_str()), file_res_name.length(), file_mode, &seekable_file_handle);
    if (err == SAHNE_SUCCESS) {
        std::cout << "Acquired seekable resource '" << file_res_name << "', Handle: " << static_cast<unsigned long long>(seekable_file_handle) << std::endl;

        uint64_t current_pos = 0;
        // Mevcut konumu al
        err = sahne_resource_seek(seekable_file_handle, SAHNE_SEEK_CUR, 0, &current_pos);
        if (err == SAHNE_SUCCESS) {
            std::cout << "Initial position: " << static_cast<unsigned long long>(current_pos) << std::endl;
        } else {
            std::cerr << "Failed to get initial position, error: " << err << std::endl;
        }

        // Konumu 100 byte ileri al (başlangıçtan itibaren)
        err = sahne_resource_seek(seekable_file_handle, SAHNE_SEEK_SET, 100, &current_pos);
         if (err == SAHNE_SUCCESS) {
            std::cout << "Seeked to position 100. New position: " << static_cast<unsigned long long>(current_pos) << std::endl;
        } else {
            std::cerr << "Failed to seek, error: " << err << std::endl;
        }

        // Kaynak durumunu al (Stat)
        ResourceStatus_t file_status; // C++'ta C yapısı kullanılır
        err = sahne_resource_stat(seekable_file_handle, &file_status);
        if (err == SAHNE_SUCCESS) {
            std::cout << "Resource Stat:\n";
            std::cout << "  Size: " << static_cast<unsigned long long>(file_status.size) << " bytes" << std::endl;
            std::cout << "  Type/Flags: 0x" << std::hex << file_status.type_flags << std::dec << std::endl; // Hex yazdırma
            std::cout << "  Link Count: " << file_status.link_count << std::endl;
            // TODO: Diğer stat alanlarını yazdır
        } else {
            std::cerr << "Failed to get resource status, error: " << err << std::endl;
        }

        // Handle'ı serbest bırak
        err = sahne_resource_release(seekable_file_handle);
        if (err == SAHNE_SUCCESS) {
             std::cout << "Released seekable resource handle." << std::endl;
        } else {
             std::cerr << "Failed to release seekable resource handle, error: " << err << std::endl;
        }

    } else {
        std::cerr << "Failed to acquire seekable resource '" << file_res_name << "', error: " << err << std::endl;
    }


    // --- Yeni Özellik: Görev Başlatma ve Sonlanmasını Bekleme (Spawn & Wait) ---
    sahne_handle_t child_code_handle = 0;
    sahne_task_id_t child_task_id;
    int32_t child_exit_code = -1;

    // Çocuk görev kodu için varsayımsal handle (child_task_entry_cpp fonksiyonunu çalıştırdığı varsayılır)
    child_code_handle.raw = 124; // Farklı bir varsayımsal değer

    std::cout << "\n--- Görev Başlatma ve Bekleme Örneği (C++) ---\n";
    // Task spawn çağrısı (C++), argümanlar ve başlangıç handle listesiyle (şimdilik boş)
    err = sahne_task_spawn(child_code_handle, nullptr, 0, nullptr, 0, &child_task_id); // nullptr yerine NULL kullanabilirsiniz
    if (err == SAHNE_SUCCESS) {
        std::cout << "Child Task started with ID: " << static_cast<unsigned long long>(child_task_id) << std::endl;

        // Başlatılan görevin sonlanmasını bekle
        std::cout << "Waiting for child task " << static_cast<unsigned long long>(child_task_id) << " to exit..." << std::endl;
        err = sahne_task_wait_for_exit(child_task_id, &child_exit_code);

        if (err == SAHNE_SUCCESS) {
            std::cout << "Child Task " << static_cast<unsigned long long>(child_task_id) << " exited with code: " << child_exit_code << std::endl;
        } else {
            std::cerr << "Failed to wait for child task " << static_cast<unsigned long long>(child_task_id) << ", error: " << err << std::endl;
        }

    } else {
        std::cerr << "Failed to spawn child task, error: " << err << std::endl;
    }


    // --- Yeni Özellik: Mesajlaşma Kanalları (C++) ---
    sahne_handle_t channel_tx_handle = 0;

    std::cout << "\n--- Mesajlaşma Kanalı Örneği (C++) ---\n";
    // Yeni bir mesaj kanalı oluştur
    err = sahne_channel_create(&channel_tx_handle);
    if (err == SAHNE_SUCCESS) {
        std::cout << "Message Channel created, Handle: " << static_cast<unsigned long long>(channel_tx_handle) << std::endl;

        // Oluşturulan kanala mesaj gönder
        std::string msg_to_send = "Hello Channel C++!";
        err = sahne_channel_send(channel_tx_handle, reinterpret_cast<const uint8_t*>(msg_to_send.c_str()), msg_to_send.length());
        if (err == SAHNE_SUCCESS) {
            std::cout << "Sent message '" << msg_to_send << "' on channel " << static_cast<unsigned long long>(channel_tx_handle) << std::endl;

            // Mesajı kanaldan al (Bloklayıcı olabilir)
            std::vector<uint8_t> received_buffer(64);
            size_t bytes_received = 0;
            std::cout << "Attempting to receive message on channel " << static_cast<unsigned long long>(channel_tx_handle) << "..." << std::endl;
            err = sahne_channel_receive(channel_tx_handle, received_buffer.data(), received_buffer.size(), &bytes_received);
            if (err == SAHNE_SUCCESS) {
                std::cout << "Received " << bytes_received << " bytes on channel " << static_cast<unsigned long long>(channel_tx_handle) << ": '" << std::string(received_buffer.begin(), received_buffer.begin() + bytes_received) << "'" << std::endl;
            } else if (err == SAHNE_ERROR_NO_MESSAGE) {
                std::cout << "No message available on channel " << static_cast<unsigned long long>(channel_tx_handle) << " (if non-blocking) or timed out." << std::endl;
            } else {
                std::cerr << "Failed to receive message on channel " << static_cast<unsigned long long>(channel_tx_handle) << ", error: " << err << std::endl;
            }

        } else {
            std::cerr << "Failed to send message on channel " << static_cast<unsigned long long>(channel_tx_handle) << ", error: " << err << std::endl;
        }

        // Kanal handle'ını serbest bırak
        err = sahne_resource_release(channel_tx_handle);
        if (err == SAHNE_SUCCESS) {
             std::cout << "Released channel handle." << std::endl;
        } else {
             std::cerr << "Failed to release channel handle, error: " << err << std::endl;
        }

    } else {
        std::cerr << "Failed to create message channel, error: " << err << std::endl;
    }


    // --- Yeni Özellik: Polling (C++) ---
    sahne_handle_t console_read_handle = 0;
    sahne_handle_t dummy_event_handle = 0;

    // Konsol okuma handle'ını edin (varsayalım stdin handle'ı edinilebilir ve non-blocking yapılabilir)
    std::string stdin_res_name = "sahne://device/console/stdin";
    err = sahne_resource_acquire(reinterpret_cast<const uint8_t*>(stdin_res_name.c_str()), stdin_res_name.length(), SAHNE_MODE_READ | SAHNE_MODE_NONBLOCK, &console_read_handle);
    if (err != SAHNE_SUCCESS) {
         std::cerr << "Warning: Failed to acquire console read handle for poll example, error: " << err << ". Using dummy handle." << std::endl;
         console_read_handle.raw = 99; // Varsayımsal dummy handle
    } else {
        std::cout << "\nAcquired console read handle " << static_cast<unsigned long long>(console_read_handle) << " for polling." << std::endl;
    }

    // Başka bir dummy olay handle'ı
    dummy_event_handle.raw = 100;

    std::cout << "\n--- Polling Örneği (C++) ---\n";

    // Poll edilecek entry dizisi oluştur (std::vector kullan)
    std::vector<PollEntry_t> poll_entries(2);

    // Entry 1: Konsol okuma handle'ı
    poll_entries[0].handle = console_read_handle;
    poll_entries[0].events_in = SAHNE_POLL_READABLE;
    poll_entries[0].events_out = SAHNE_POLL_NONE;

    // Entry 2: Dummy olay handle'ı
    poll_entries[1].handle = dummy_event_handle;
    poll_entries[1].events_in = SAHNE_POLL_ERROR | SAHNE_POLL_DISCONNECTED;
    poll_entries[1].events_out = SAHNE_POLL_NONE;

    // Poll çağrısı yap (Örn: 2 saniye timeout ile)
    auto timeout_duration = std::chrono::seconds(2);
    int64_t timeout_ms = std::chrono::duration_cast<std::chrono::milliseconds>(timeout_duration).count();
    std::cout << "Polling on " << poll_entries.size() << " handles with " << timeout_ms << "ms timeout..." << std::endl;

    int64_t num_ready = sahne_poll(poll_entries.data(), poll_entries.size(), timeout_ms); // vector::data() ve size() kullan

    if (num_ready < 0) {
        // Hata durumunda negatif kerror_t değeri döner
        std::cerr << "Poll failed, error: " << num_ready << std::endl;
        // İsterseniz SahneError'a çevirip yazdırabilirsiniz
         sahne_error_t poll_err = map_kernel_error(num_ready); // C++ tarafında map_kernel_error yazılmalı
         std::cerr << "Poll failed, error: " << poll_err << " (SahneError code)" << std::endl;

    } else {
        // Başarı durumunda (>=0) olay gerçekleşen handle sayısı döner
        std::cout << "Poll returned. " << num_ready << " handle(s) ready." << std::endl;
        if (num_ready > 0) {
            // Hangi handle'ların hazır olduğunu kontrol et
            for (size_t i = 0; i < poll_entries.size(); ++i) {
                if (poll_entries[i].events_out != SAHNE_POLL_NONE) {
                    std::cout << "  Handle " << static_cast<unsigned long long>(poll_entries[i].handle) << " ready with events: 0x" << std::hex << poll_entries[i].events_out << std::dec << std::endl;

                    // Gerçekleşen olay türlerine göre işlem yapabilirsiniz
                    if (poll_entries[i].events_out & SAHNE_POLL_READABLE) {
                         std::cout << "    -> Readable!" << std::endl;
                         // Okuma işlemini dene (non-blocking okuma burada uygun olabilir)
                          std::vector<uint8_t> temp_buf(16);
                          size_t temp_read = 0;
                          sahne_resource_read(poll_entries[i].handle, temp_buf.data(), temp_buf.size(), &temp_read);
                         // ... işlem sonucu kontrol et ...
                    }
                     if (poll_entries[i].events_out & SAHNE_POLL_WRITABLE) {
                         std::cout << "    -> Writable!" << std::endl;
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
    if (allocated_mem != nullptr) {
         err = sahne_mem_release(allocated_mem, mem_size);
         if (err == SAHNE_SUCCESS) {
             std::cout << "\nReleased initial allocated memory." << std::endl;
         } else {
             std::cerr << "\nFailed to release initial memory, error: " << err << std::endl;
         }
    }

    std::cout << "\nSahne64 C++ Program Exiting (Extended API)." << std::endl;
    // Görevi normal çıkış koduyla sonlandır
    sahne_task_exit(0);
    // Buradan sonrası çalışmaz
    return 0; // Bu satıra asla ulaşılmamalı
}
