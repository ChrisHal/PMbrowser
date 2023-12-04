/*
    Copyright 2020 - 2022 Christian R. Halaszovich

     This file is part of PMbrowser.

    PMbrowser is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PMbrowser is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with PMbrowser.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef HK_TREE_H
#define HK_TREE_H

#pragma once

#include <istream>
#include <ostream>
#include <vector>
#include <array>
#include <string>
#include <string_view>
#include <algorithm>
#include <memory>
#include <limits>
#include <type_traits>
#include <cstring>
#include <cstdint>
#include <cstddef>

namespace hkLib {

    // file offset and other constants pertaining to trees and treenodes

    enum RecordingModeType {
        InOut = 0,
        OnCell = 1,
        OutOut = 2,
        WholeCell = 3,
        CClamp = 4,
        VClamp = 5,
        NoMode = 6
    };

    // offsets into data record fields that we are interested in
    constexpr size_t stExtTrigger = 164, // in Stimulation record
        TrLabel = 4,
        TrTraceCount = 36,  // in Trace Record
        TrTraceID = 36,  // in v. 1000 this is called ID
        TrData = 40,
        TrDataPoints = 44,
        TrDataKind = 64,
        TrRecordingMode = 68,
        TrDataFormat = 70,
        TrDataScaler = 72,
        TrYUnit = 96,
        TrXInterval = 104,
        TrXStart = 112,
        TrXUnit = 120,
        TrSealResistance = 168,
        TrCSlow = 176,
        TrGSeries = 184,
        TrRsValue = 192,
        TrLinkDAChannel = 216, // int32
        TrSelfChannel = 288,
        TrInterleaveSize = 292,
        TrInterleaveSkip = 296,
        TrTrHolding = 408,
        SwLabel = 4, // for Sweep ...
        SwStimCount = 40,
        SwSweepCount = 44,
        SwTime = 48,
        SwTimer = 56,
        SeLabel = 4, //(*String32Type*)
        SeSeriesCount = 116,
        SeAmplStateFlag = 124, // flag > 0 => load local oldAmpState, otherwise load from .amp File
        SeAmplStateRef = 128, // ref  == 0 => use local oldAmpState. Caution: This is a 1-based offset!
        SeTime = 136,
        SeOldAmpState = 472,
        GrLabel = 4,
        GrGroupCount = 120,
        RoVersionName = 8, // root record
        RoStartTime = 520,
        // now from Amp records:
        RoAmplifierName = 40,
        RoAmplifier = 72, // CHAR
        RoADBoard = 73, // CHAR
        // For AmplStateRecord:
        AmplifierStateSize = 400,
        AmStateCount = 4,
        AmAmplifierState = 112;


    /// stim tree
    // from channel record
    constexpr std::size_t
        chLinkedChannel = 4, //int32
        chAdcChannel = 20, // (*INT16*)
        chAdcMode = 22, // (*BYTE*)
        chSetLastSegVmemb = 27, // BOOL (byte)
        chDacChannel = 28, // (*INT16*)
        chDacMode = 30, // (*BYTE*)
        chDacUnit = 40, // String8Type
        chHolding = 48, // LONGREAL, for CC in micro-ampere!
        // for stim recorde
        stEntryName = 4, //(*String32Type*)
        stFileName = 36,
        stDataStartSegment = 100, // (*INT32*)
        stDataStartTime = 104, // LongReal
        stNumberSweeps = 144, // int32
        stActualDacChannels = 160, // int32
        stHasLockIn = 168, // bool (8bit)
        // for segment record
        seClass = 4, // BYTE
        seVoltageIncMode = 6, // (*BYTE*)
        seDurationIncMode = 7, // (*BYTE*)
        seVoltage = 8,
        seVoltageSource = 16, // int32
        seDeltaVFactor = 20, // (*LONGREAL*)
        seDeltaVIncrement = 28, // (*LONGREAL*)
        seDuration = 36, // double
        seDurationSource = 44, // (*INT32*)
        seDeltaTFactor = 48, // (*LONGREAL*)
        seDeltaTIncrement = 56; // (*LONGREAL*)

    enum class SegmentClass {
        Constant = 0,
        Ramp,
        Continuous,
        ConstSine,
        Squarewave,
        Chirpwave
    };

    constexpr std::array<const char*, 6> SegmentClassNames{ {
        "Constant",
        "Ramp",
        "Continous",
        "ConstSine",
        "Squarewave",
        "ChirpWave"
    } };

    enum class IncrementModeType {
        ModeInc,
        ModeDec,
        ModeIncInterleaved,
        ModeDecInterleaved,
        ModeAlternate,
        ModeLogInc,
        ModeLogDec,
        ModeLogIncInterleaved,
        ModeLogDecInterleaved,
        ModeLogAlternate
    };

    constexpr std::array<const char*, 10> IncrementModeNames{ {
        "ModeInc",
        "ModeDec",
        "ModeIncInterleaved",
        "ModeDecInterleaved",
        "ModeAlternate",
        "ModeLogInc",
        "ModeLogDec",
        "ModeLogIncInterleaved",
        "ModeLogDecInterleaved",
        "ModeLogAlternate"
    } };

    // TrDataKind
    constexpr uint16_t LittleEndianBit = 1, IsLeak = 1 << 1, IsImon = 1 << 3, IsVmon = 1 << 4, ClipBit = 1 << 5;

    // TrDataFormatType
    constexpr char DFT_int16 = 0, DFT_int32 = 1, DFT_float = 2, DFT_double = 3;


    constexpr uint32_t MagicNumber = 0x54726565, SwappedMagicNumber = 0x65657254;

    struct UserParamDescr {
        enum {
            SizeName = 32,
            SizeUnit = 8,
            Size = 40
        };
        std::string_view Name, Unit;
    };
    std::ostream& operator<<(std::ostream& os, const UserParamDescr&);

    class hkTree;

    /// <summary>
    /// A node in the tree (pul., pgf, amp, etc. tree)
    /// </summary>
    struct hkTreeNode {
    private:
        template<typename T> T extractValueNoCheck(std::size_t offset) const noexcept
        {
            static_assert(std::is_arithmetic_v<T>, "must be arithmetic type");
            T t{};
            auto src = Data + offset;
            if (!isSwapped) {
                std::copy(src, src + sizeof t, reinterpret_cast<char*>(&t));
            }
            else {
                std::reverse_copy(src, src + sizeof t, reinterpret_cast<char*>(&t));
            }
            return t;
        }
    public:
        hkTreeNode() : Parent{ nullptr }, Data{ nullptr }, len{ 0 }, Children{}, level{ -1 }, isSwapped{ false } {};
        hkTreeNode(hkTreeNode&&) = default;
        hkTreeNode(const hkTreeNode&) = delete;
        hkTreeNode& operator=(const hkTreeNode&) = delete;

        /// <summary>
        /// extract a value from record data, swaps bytes if needed
        /// throws out_of_range exception
        /// if offset is too large
        /// </summary>
        /// <typeparam name="T">type of value to be extracted</typeparam>
        /// <param name="offset">offset in record data (needn't be aligned)</param>
        /// <returns>extracted value</returns>
        template<typename T> T extractValue(std::size_t offset) const
        {
            if (len < offset + sizeof(T)) {
                throw std::out_of_range("offset too large while accessing tree node");
            }
            return extractValueNoCheck<T>(offset);
        }

        /// <summary>
        /// extract a value from record data, swaps bytes if needed,
        /// return default value
        /// if offset is too large
        /// </summary>
        /// <typeparam name="T">type of value to be extracted</typeparam>
        /// <param name="offset">offset in record data (needn't be aligned)</param>
        /// <param name="defaultValue">default to be returned if offset is too large</param>
        /// <returns>extracted value or default value</returns>
        template<typename T> T extractValue(std::size_t offset, T defaultValue) const noexcept
        {
            if (len < offset + sizeof(T)) {
                return defaultValue;
            }
            return extractValueNoCheck<T>(offset);
        }
        enum TreeLevel {
            LevelRoot = 0,
            LevelGroup = 1,
            LevelSeries = 2,
            LevelSweep = 3,
            LevelTrace = 4
        };
        int32_t extractInt32(std::size_t offset) const { return extractValue<int32_t>(offset); };
        uint16_t extractUInt16(std::size_t offset) const { return extractValue<uint16_t>(offset); };
        double extractLongReal(std::size_t offset) const { return extractValue<double>(offset); };
        double extractLongRealNoThrow(std::size_t offset) const noexcept //! instead of throwing an exception, returns NaN if out of range
        {
            return extractValue(offset, std::numeric_limits<double>::quiet_NaN());
        }
        char getChar(std::size_t offset) const;
        const std::string_view getString(std::size_t offset) const;
        const UserParamDescr getUserParamDescr(std::size_t offset) const;
        template<std::size_t N> const std::string_view getString(std::size_t offset) const
        {
            if (len < offset + N) {
                throw std::out_of_range("offset to large while accessing tree node");
            }
            const auto* p = Data + offset;
            if (p[N - 1]) {
                // in theory, string is not zero terminated
                // unfortunately, some PM version mess this up
                // by not prperly zero-initializing the cahr array
                return std::string_view(p, std::min(std::strlen(p), N));
            }
            else {
                return std::string_view(p);
            }
        }
        hkTreeNode* getParent() const { return Parent; };
        bool getIsSwapped() const { return isSwapped; };
        int getLevel() const { return level; };
        double getTime0() const;

        /// <summary>
        /// use time recorded in node as time reference for rel. times
        /// </summary>
        void setAsTime0();

    private:
        hkTree* tree{};

    public:
        hkTreeNode* Parent;
        //std::unique_ptr<char[]> Data;
        const char* Data;
        std::size_t len; //!< Length (in bytes) of data
        std::vector<hkTreeNode> Children;
        int level;
        bool isSwapped;

        friend class hkTree;
    };

    class hkTree
    {
        std::vector<int32_t> LevelSizes;
        hkTreeNode RootNode;
        std::string ID;
        std::unique_ptr<char[]> Data{};
        double time0{};
        bool isSwapped;
        void LoadToNode(hkTreeNode* parent, hkTreeNode& node, char** pdata, int level);
    public:
        hkTree() : LevelSizes{}, RootNode{}, isSwapped{ false } {};
        std::string getID() {
            return ID;
        };

        /// <summary>
        /// Initialize tree from istream
        /// </summary>
        /// <param name="id">id (pgf, pul, ...) of tree</param>
        /// <param name="infile">input stream (usually a filestream)</param>
        /// <param name="offset">offset of start of tree in infile stream</param>
        /// <param name="len">length in bytes of tree data in file (this data contains the total of the tree)</param>
        /// <returns>true on success</returns>
        bool InitFromStream(const std::string_view& id, std::istream& infile, int offset, unsigned int len);

        /// <summary>
        /// Initialize tree from data buffered in memory
        /// </summary>
        /// <param name="id">id (pgf, pul, ...) of tree</param>
        /// <param name="buffer">pointer to data stored in memory, must be permanent for lifetime of hkTree</param>
        /// <param name="len">length in bytes of buffer (buffer contains the total of the tree)</param>
        /// <returns>true on success</returns>
        bool InitFromBuffer(const std::string_view& id, char* buffer, std::size_t len);
        hkTreeNode& GetRootNode() { return RootNode; };
        std::size_t GetNumLevels() { return LevelSizes.size(); };    //!< return number of levels this tree has
        bool getIsSwapped() const { return isSwapped; };
        friend hkTreeNode;
    };
}
#endif // !HK_TREE_H
