/*
    Initial author: Convery (tcn@ayria.se)
    Started: 18-03-2019
    License: MIT
*/

#pragma once
#include <cstdint>
#include <cstdio>
#include <memory>
#include <vector>
#include <string>

class Archive
{
    #pragma pack(push, 1)
    struct Header_t
    {
        uint32_t Directoryoffset;
        uint32_t Reserved;
    };
    struct Entry_t
    {
        uint32_t Filesize;
        uint32_t Dataoffset;
        std::unique_ptr<char[]> Filename;
    };
    struct Directory_t
    {
        uint32_t Entrycount;
        std::unique_ptr<Entry_t[]> Entries;
    };
    #pragma pack(pop)

    Header_t Header{};
    Directory_t Listing{};
    std::FILE *Filehandle{};

    // Internal modifiers.
    bool getListing()
    {
        do
        {
            // Sanity-checking.
            if (!Filehandle) break;

            // Ensure that we are reading from the start.
            if (std::fseek(Filehandle, 0, SEEK_SET)) break;

            // Initialize the header.
            if (sizeof(Header_t) != std::fread(&Header, 1, sizeof(Header_t), Filehandle)) break;

            // Jump to the directory offset.
            if (std::fseek(Filehandle, Header.Directoryoffset, SEEK_SET)) break;

            // Get the count.
            if (sizeof(Directory_t::Entrycount) != std::fread(&Listing.Entrycount, 1, sizeof(Directory_t::Entrycount), Filehandle)) break;

            // Pre-allocate the entries.
            Listing.Entries = std::make_unique<Entry_t[]>(Listing.Entrycount);

            // Read all the entries.
            for (uint32_t i = 0; i < Listing.Entrycount; ++i)
            {
                std::vector<char> Filename;
                Entry_t Localentry{};
                char Temporary{};

                // Perform as separate reads in-case of corruption.
                if (sizeof(Entry_t::Filesize) != std::fread(&Localentry.Filesize, 1, sizeof(Entry_t::Filesize), Filehandle)) goto LABEL_ERROR;
                if (sizeof(Entry_t::Dataoffset) != std::fread(&Localentry.Filesize, 1, sizeof(Entry_t::Dataoffset), Filehandle)) goto LABEL_ERROR;

                // Pre-allocate the filename.
                Filename.reserve(64);

                // Read in the filename.
                while (sizeof(char) == std::fread(&Temporary, 1, sizeof(char), Filehandle) && Temporary != '\0')
                    Filename.push_back(Temporary);

                // Move the data over to the entry, (std::copy should become memmove for POD types).
                Localentry.Filename = std::make_unique<char[]>(Filename.size() + sizeof(char));
                std::copy(Filename.begin(), Filename.end(), Localentry.Filename.get());

                // Move it into the listing.
                Listing.Entries[i] = std::move(Localentry);
            }

            // Return success.
            return true;

        } while (false);

        LABEL_ERROR:
        // Return error.
        return false;
    }
    std::basic_string<uint8_t> readFile(uint32_t Offset, uint32_t Size)
    {
        std::basic_string<uint8_t> Buffer;

        // Sanity-checking and pre-allocation.
        if (!Filehandle) return {};
        else Buffer.reserve(Size);

        // Move to the data.
        std::fseek(Filehandle, Offset, SEEK_SET);

        // NOTE(tcn): The standard allows casting away const here.
        // Read as a single block, let the OS figure out the chunk-size.
        if (1 != std::fread((uint8_t *)Buffer.data(), Size, 1, Filehandle)) return {};

        return Buffer;
    }

    public:
    bool Open(const std::string &Filename)
    {
        if (Filehandle) std::fclose(Filehandle);
        Filehandle = std::fopen(Filename.c_str(), "rb+");

        if (Filehandle) return getListing();
        else return false;
    }

    // Boolean result for the operation.
    std::pair<std::basic_string<uint8_t>, bool> readFile(const std::string &Filename)
    {
        for (uint32_t i = 0; i < Listing.Entrycount; ++i)
        {
            if (std::strstr(Listing.Entries[i].Filename.get(), Filename.c_str()))
            {
                return std::make_pair(readFile(Listing.Entries[i].Dataoffset, Listing.Entries[i].Filesize), true);
            }
        }

        return {};
    }

    Archive(const std::string &Filename)
    {
        // Try to read the archive, else create a new one.
        if(!Open(Filename)) Filehandle = std::fopen(Filename.c_str(), "wb+");
    }

};
