/*
 * SPDX-FileCopyrightText: Copyright 2023-2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cassert>
#include <list>
#include <ostream>
#include <string_view>
#include <vector>

#include "header.hpp"
#include "internal_logging.hpp"

namespace mlsdk::vgflib {

constexpr std::string_view rdStateToStr(std::ios::iostate state) {
    if ((state & std::ios::eofbit) != 0) {
        return "eof";
    }
    if ((state & std::ios::failbit) != 0) {
        return "fail";
    }
    if ((state & std::ios::badbit) != 0) {
        return "bad";
    }
    return "good";
}

class SectionIndexTable {
  public:
    class SectionIndex : public SectionEntry {
      public:
        /// Construct a section index table entry
        SectionIndex(uint64_t size, uint64_t alignment) : SectionEntry(0, size), alignment_(alignment) {
            assert(alignment_ > 0);
        }

        /// Returned that the computed offset matches the required alignment
        bool IsAligned() const { return offset % alignment_ == 0; }

        /// Returns the offset to the end of payload data
        uint64_t EndOfData() const { return offset + size; }

        /// Returns the offset to the next section. (data + padding)
        uint64_t NextOffset() const { return EndOfData() + padding_; }

        /// Update the padding by using the alignment requirement of the next section
        void UpdatePadding(const SectionIndex &next) {
            padding_ = next.alignment_ - ((((EndOfData() - 1) % next.alignment_) + 1));
        }

        /// Update the offset by computing from the previous section
        void UpdateOffset(const SectionIndex &prev) { offset = prev.NextOffset(); }

        /// Write the section and check offsets
        bool Write(std::basic_ostream<char> &file, void *data) const {
            file.write(reinterpret_cast<const char *>(data), static_cast<std::streamsize>(size));
            if (file.fail()) {
                logging::error("Failed to write section index, rdstate: " + std::string(rdStateToStr(file.rdstate())));
                return false;
            }
            if (padding_ != 0) {
                std::vector<char> padArray(padding_, 0);
                file.write(padArray.data(), static_cast<std::streamsize>(padArray.size()));
                if (file.fail()) {
                    logging::error("Failed to write section index padding, rdstate: " +
                                   std::string(rdStateToStr(file.rdstate())));
                    return false;
                }
            }
            return true;
        }

        uint64_t GetSize() const { return size; }
        uint64_t GetOffset() const { return offset; }
        uint64_t GetPadding() const { return padding_; }
        uint64_t GetAlignment() const { return alignment_; }

      private:
        uint64_t alignment_ = 1; //< Alignment requirement of the section
        uint64_t padding_ = 0;   //< Padding size (after the data)
    };

    /// Add a new section and return a reference to it
    const SectionIndex &AddSection(uint64_t size, uint64_t alignment = 1) {
        assert(alignment != 0);
        sections_.emplace_back(size, alignment);
        return sections_.back();
    }

    /// Walk sections updating padding and offsets
    void Update() {
        auto current = sections_.begin();
        for (size_t i = 0; i < sections_.size() - 1; i++) {
            SectionIndex &section = *current;
            SectionIndex &next = *(++current);

            section.UpdatePadding(next);
            next.UpdateOffset(section);

            assert(section.IsAligned());
        }
    }

  private:
    std::list<SectionIndex> sections_;
};

} // namespace mlsdk::vgflib
