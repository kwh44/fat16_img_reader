#include <iostream>
#include <fstream>
#include <bitset>

int convert(unsigned char ms_part, unsigned char ls_part) { // ms - more significant, ls - less significant
    int res = 0;
    res |= static_cast<int>(ls_part);
    res |= (static_cast<int>(ms_part) << 8);

    return res;
}


class BootSector {
public:
    int sectors_per_cluster;
    int bytes_per_sector;
    int reserved_sectors;
    int fat_number;
    int root_max_files;
    int bytes_per_root_entry = 32;
    int bytes_per_root;
    int sectors_per_fat;
    int bytes_per_fat;
    int signature;

    explicit BootSector(char *image) {
        std::ifstream img(image, std::ios::binary);

        auto *buffer = new char[512];
        img.read(buffer, 512);

        sectors_per_cluster = static_cast<int>(buffer[13]);
        bytes_per_sector = convert(buffer[12], buffer[11]);
        reserved_sectors = convert(buffer[15], buffer[14]);
        fat_number = static_cast<int>(buffer[16]);
        root_max_files = convert(buffer[18], buffer[17]);
        bytes_per_root = bytes_per_root_entry * root_max_files;
        sectors_per_fat = convert(buffer[23], buffer[22]);
        bytes_per_fat = sectors_per_fat * bytes_per_sector;
        signature = convert(buffer[511], buffer[510]);

        delete[]buffer;
    }

    void print_image_info() {
        std::cout << "Sectors per cluster: " << sectors_per_cluster << std::endl;
        std::cout << "Bytes per sector: " << bytes_per_sector << std::endl;
        std::cout << "Size of reserved area, in sectors: " << reserved_sectors << std::endl;
        std::cout << "Number of FATs: " << fat_number << std::endl;
        std::cout << "Max entries in root: " << root_max_files << std::endl;
        std::cout << "Size of root, in bytes: " << bytes_per_root << std::endl;
        std::cout << "Size of each FAT, in sectors: " << sectors_per_fat << ", in bytes: " << bytes_per_fat << std::endl;

        int correct_signature = 0xaa55;
        if (signature == correct_signature)
            std::cout << "Signature is correct" << std::endl;
        else
            std::cout << "Signature is wrong" << std::endl;
    }

    int get_root_offset() {
        return bytes_per_sector * (reserved_sectors + fat_number * sectors_per_fat);
    }
};


class Entry{
private:
    char *entry_data = nullptr;

    unsigned int file_size() {
        int ms_part = convert(entry_data[31], entry_data[30]);
        int ls_part = convert(entry_data[29], entry_data[28]);

        return (ms_part << 16) | ls_part;
    }

    std::string file_name() {
        std::string name(12, ' ');
        char empty = 0b00100000;
        int total_characters = 0;

        for (; total_characters < 8 && entry_data[total_characters] != empty; ++total_characters)
            name[total_characters] = entry_data[total_characters];

        name[total_characters++] = '.';

        for (int i = 8; i < 11 && entry_data[i] != empty; ++i)
            name[total_characters++] = entry_data[i];

        if (name[total_characters - 1] == '.')
            name[total_characters - 1] = ' ';

        return name;
    }

public:
    explicit Entry(char *data) {
        entry_data = data;
    }

    bool exists() {
        auto first_character = static_cast<unsigned char>(entry_data[0]);
        return first_character != 0x00 && first_character != 0xe5;
    }

    void print_info() {
        std::cout << "File name: " << file_name() << std::endl;
        std::cout << "First cluster number: " << convert(entry_data[27], entry_data[26]) << std::endl;
        std::cout << "File size: " << file_size() << std::endl;
        std::cout << std::endl;
    }
};


class RootDirectory{
private:
    int bytes_per_entry;
    int size;

public:
    char *data = nullptr;

    RootDirectory(char *image, BootSector &boot_sector) {
        std::ifstream img(image, std::ios::binary);

        bytes_per_entry = boot_sector.bytes_per_root_entry;
        size = boot_sector.bytes_per_root;
        data = new char[size];

        img.seekg(boot_sector.get_root_offset());
        img.read(data, boot_sector.bytes_per_root);
    }

    void print_info() {
        char *entry_data = new char[bytes_per_entry];

        std::cout << "Root directory: " << std::endl;
        for (int i = 0 ; i != size; i += bytes_per_entry) {
            Entry entry(data + i);
            if (entry.exists())
                entry.print_info();
        }

        delete[]entry_data;
    }

    ~RootDirectory() {
        delete[]data;
    }
};


int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Invalid number of program arguments." << std::endl;
        return 1;
    }

    std::ifstream img(argv[1], std::ios::binary);
    if (!img) {
        std::cout << "Can't open image." << std::endl;
        return 1;
    }

    BootSector boot_sector(argv[1]);
    boot_sector.print_image_info();

    std::cout << std::endl;

    RootDirectory rootDirectory(argv[1], boot_sector);
    rootDirectory.print_info();

    return 0;
}

