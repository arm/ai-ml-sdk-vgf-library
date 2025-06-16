/*
 * SPDX-FileCopyrightText: Copyright 2023-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cassert>
#include <list>
#include <ostream>
#include <vector>

#include "header.hpp"

namespace mlsdk::vgflib {

struct SectionIndexTable {
    struct SectionIndex : public SectionEntry {
        /// Construct a section index table entry
        SectionIndex(uint64_t size, uint64_t alignment) : SectionEntry(0, size), _alignment(alignment) {
            assert(alignment > 0);
        }

        /// Returned that the computed offset matches the required alignment
        bool IsAligned() const { return offset % _alignment == 0; }

        /// Returns the offset to the end of payload data
        uint64_t EndOfData() const { return offset + size; }

        /// Returns the offset to the next section. (data + padding)
        uint64_t NextOffset() const { return EndOfData() + _padding; }

        /// Update the padding by using the alignment requirement of the next section
        void UpdatePadding(const SectionIndex &next) {
            _padding = next._alignment - ((((EndOfData() - 1) % next._alignment) + 1));
        }

        /// Update the offset by computing from the previous section
        void UpdateOffset(const SectionIndex &prev) { offset = prev.NextOffset(); }

        /// Write the section and check offsets
        bool Write(std::basic_ostream<char> &file, void *data) const {
            file.write(reinterpret_cast<const char *>(data), static_cast<std::streamsize>(size));
            if (file.bad()) {
                return false;
            }
            if (_padding) {
                std::vector<char> padArray(_padding, 0);
                file.write(padArray.data(), static_cast<std::streamsize>(padArray.size()));
                if (file.bad()) {
                    return false;
                }
            }
            return true;
        }

        uint64_t GetSize() const { return size; }
        uint64_t GetOffset() const { return offset; }
        uint64_t GetPadding() const { return _padding; }
        uint64_t GetAlignment() const { return _alignment; }

      private:
        uint64_t _alignment = 1; //< Alignment requirement of the section
        uint64_t _padding = 0;   //< Padding size (after the data)
    };

    /// Add a new section and return a reference to it
    const SectionIndex &AddSection(uint64_t size, uint64_t alignment = 1) {
        assert(alignment != 0);
        _sections.emplace_back(size, alignment);
        return _sections.back();
    }

    /// Walk sections updating padding and offsets
    void Update() {
        auto current = _sections.begin();
        for (size_t i = 0; i < _sections.size() - 1; i++) {
            SectionIndex &section = *current;
            SectionIndex &next = *(++current);

            section.UpdatePadding(next);
            next.UpdateOffset(section);

            assert(section.IsAligned());
        }
    }

  private:
    std::list<SectionIndex> _sections;
};

} // namespace mlsdk::vgflib
