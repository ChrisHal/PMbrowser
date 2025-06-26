/*
	Copyright 2020 - 2025 Christian R. Halaszovich

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

#include <cstdint>
#include <sstream>
#include <iomanip>
#include <array>
#include <algorithm>
#include <cassert>
#include <string_view>
#include "hkTree.h"
#include "PMparameters.h"
#include "time_handling.h"
#include "DatFile.h"

namespace hkLib {

    const std::array<const std::string_view, 7> RecordingModeNames = {
		"Inside-Out",
		"On-Cell",
		"Outside-Out",
		"Whole-Cell",
		"Current-Clamp",
		"Voltage-Clamp",
		"<none>"
	};

    const std::array<const std::string_view, 4> AmpModeNames = {
		"TestMode",
		"VCMode",
		"CCMode",
		"NoMode"
	};


    constexpr std::array ExtTriggerTypeNames {
        "TrigNone"sv,
        "TrigSeries"sv,
        "TrigSweep"sv,
        "TrigSweepNoLea"sv
    };

    constexpr std::array AmplModeTypeNames{
        "AnyAmplMode"sv,
        "VCAmplMode"sv,
        "CCAmplMode"sv,
        "IDensityMode"sv
    };
    constexpr std::array AdcTypeNames {
        "Off"sv,
        "Analog"sv,
        "Digitals"sv,
        "Digital"sv,
        "AdcVirtual"sv
    };

    std::array SegStoreTypeNames {
        "SegNoStore"sv,
        "SegStore"sv,
        "SegStoreStart"sv,
        "SegStoreEnd"sv
    };

    std::array<PMparameter, 36> parametersTrace{ {
		{false,false,"TrMark","",PMparameter::Int32,0},
		{false,false,"TrLabel","",PMparameter::String32,4},
		{false,false,"TraceID","",PMparameter::Int32,36},
        {false,false,"DataPoints","",PMparameter::Int32,44},
        {false,false,"Holding","V|A",PMparameter::LongReal,408},
		{false,false,"Internal Solution","",PMparameter::Int32,48},
		{false,false,"Leak traces","",PMparameter::Int32,60},
		{false,false,"TrDataKind","",PMparameter::Set16,64},
		{false,false,"Clipping","",PMparameter::Set16_Bit5,64},
		{false,false,"UseXStart","",PMparameter::Boolean,66},
		{false,true,"Recording Mode","",PMparameter::RecordingMode,68},
		{false,false,"XStart","s",PMparameter::LongReal,112},
		{false,false,"XInterval","s",PMparameter::LongReal,104},
		{false,false,"SampleRate","Hz",PMparameter::InvLongReal,104},
		{false,false,"Time offset","s",PMparameter::LongReal,80},
		{false,false,"Zero data","A|V",PMparameter::LongReal,88},
		{false,false,"Bandwidth","Hz",PMparameter::LongReal,144},
		{false,false,"PipetteResistance","Ohm",PMparameter::LongReal,152},
		{false,false,"CellPotential","V",PMparameter::LongReal,160},
		{true,true,"SealResistance","Ohm",PMparameter::LongReal,168},
		{true,true,"Cslow","F",PMparameter::LongReal,176},
		{false,false,"Gseries","S",PMparameter::LongReal,184},
		{true,true,"Rs","Ohm",PMparameter::InvLongReal,184},
		{false,false,"RsValue","",PMparameter::LongReal,192},
		{false,false,"Gleak","S",PMparameter::LongReal,200},
		{false,false,"MemConductance","S",PMparameter::LongReal,208},
		{false,false,"CM","F",PMparameter::LongReal,248},
		{false,false,"GM","S",PMparameter::LongReal,256},
		{false,false,"GS","S",PMparameter::LongReal,280},
		{false,false,"Phase","°",PMparameter::LongReal,264},
		{false,false,"Ext.Sol.","",PMparameter::Int32,244},
		{false,false,"IntSolVal","",PMparameter::LongReal,424},
		{false,false,"ExtSolVal","",PMparameter::LongReal,432},
		{false,false,"IntSolName","",PMparameter::String32,440},
		{false,false,"ExtSolName","",PMparameter::String32,472},
		{false,true,"TrAdcChannel","",PMparameter::Int16,222}//,
		//{false,true,"TrSelfChannel","",PMparameter::Int16,288},
		//{false,true,"TrLinkDAChannel","",PMparameter::Int16,216}
		//false,false,"TrXTrace","",PMparameter::Int32,420
	} };

	std::array<PMparameter, 18> parametersSweep{ {
		{false,false,"SwMark","",PMparameter::Int32,0},
		{false,false,"SwLabel","",PMparameter::String32,4},
		{false,false,"Stim Count","",PMparameter::Int32,40},
		{false,false,"SwSweepCount","",PMparameter::Int32,44},
		{true,true,"Sweep Time raw","s",PMparameter::LongReal,48},
		{true,true,"Rel. Sweep Time","s",PMparameter::RelativeTime,48},
		{true,true,"Sweep Time","",PMparameter::DateTime,48},
		{true,true,"Timer Time","s",PMparameter::LongReal,56},
		{false,false,"User param. 1,2","",PMparameter::LongReal2,64},
		{false,false,"Pip. pressure","a.u.",PMparameter::LongReal,80},
		{false,false,"RMS noise","A",PMparameter::LongReal,88}, // not sure about units
		{false,false,"Temperature","°C",PMparameter::LongReal,96},
		{false,false,"DigitalIn","",PMparameter::Set16,112},
		{false,false,"DigitalOut","",PMparameter::Set16,116},
		{false,false,"SweepKind","",PMparameter::UInt16,114},
		{false,false,"SwSwMarkers","",PMparameter::LongReal4,120},
		{false,false,"Sweep holding 16x","",PMparameter::LongReal16,160},
		{false,false,"User param ex.","",PMparameter::LongReal8,288}
	} };

	std::array<PMparameter, 15> parametersSeries{ {
		{false,false,"SeMark","",PMparameter::Int32,0},
		{true,false,"SeLabel","",PMparameter::String32,4},
		{false,false,"SeComment","",PMparameter::String80,36},
		{false,false,"SeSeriesCount","",PMparameter::Int32,116},
		{false,false,"SeNumberSweeps","",PMparameter::Int32,120},
		{false,false,"SeMethodTag","",PMparameter::Int32,132},
		{true,false,"SeTime_raw","s",PMparameter::LongReal,136},
		{true,true,"Rel. SeTime","s",PMparameter::RelativeTime,136},
		{true,false,"SeTime","",PMparameter::DateTime,136},
		{false,false,"SeMethodName","",PMparameter::String32,312},
		{false,false,"SeUsername","",PMparameter::String80,872},
		{false,false,"SeUserDescr1","",PMparameter::UserParamDesc2,152},
		{false,false,"SeSeUserParams2","",PMparameter::LongReal4,1120},
		{false,false,"SeSeUserParamDescr2","",PMparameter::UserParamDesc4,1152},
		{false,false,"SeUserDescr2","",PMparameter::UserParamDesc8, 1408}
	} };

	std::array<PMparameter, 5> parametersGroup{ {
		{false, false, "GrMark","",PMparameter::Int32,0},
		{true, false, "GrLabel","",PMparameter::String32,4},
		{false,false,"GrText","",PMparameter::String80,36},
		{false,false,"ExperimentNumber","",PMparameter::Int32,116},
		{false,false,"GroupCount","",PMparameter::Int32,120}
	} };

	std::array<PMparameter, 8> parametersRoot{ {
		{false, false, "RoVersion","",PMparameter::Int32,0},
		{false, false, "RoMark","",PMparameter::Int32,4},
		{false, false, "RoVersionName","",PMparameter::String32,8},
		{false, false, "RoAuxFileName", "", PMparameter::String80,40},
		{false, false, "RootText", "", PMparameter::String400,120},
		{true, true, "RootStartTime_raw", "s", PMparameter::LongReal,520},
		{true, true, "RootStartTime", "", PMparameter::DateTime,520},
		{false,false, "RoMaxSamples","",PMparameter::Int32,528}
	} };

	std::array<PMparameter, 31> parametersAmpplifierState{ {
		{false, true, "StateVersion", "", PMparameter::String8, 0},
		{false, true, "CurrentGain", "V/A", PMparameter::LongReal, 8},
		{false, true, "F2Bandwidth", "Hz", PMparameter::LongReal,16},
		{false, true, "F2Frequency", "Hz", PMparameter::LongReal,24},
		{false, true, "RsValue", "Ohm", PMparameter::LongReal,32},
		{false, true, "RsFraction", "", PMparameter::LongReal,40},
		{false, true, "GLeak", "S", PMparameter::LongReal,48},
		{false, true, "CFastAmp1", "F", PMparameter::LongReal,56},
		{false, true, "CFastAmp2", "F", PMparameter::LongReal,64},
		{false, true, "CFastTau", "s", PMparameter::LongReal,72},
		{false, true, "CSlow", "F", PMparameter::LongReal,80},
		{false, true, "GSeries", "S", PMparameter::LongReal,88},
		{false, true, "VCStimDacScale", "", PMparameter::LongReal,96},
		{false, true, "CCStimDacScale", "", PMparameter::LongReal,104},
		{false, true, "VHold", "V", PMparameter::LongReal,112},
		{false, true, "LastVHold", "V", PMparameter::LongReal,120},
		{false, true, "VpOffset", "V", PMparameter::LongReal,128},
		{false,true,"VLiquidJunction","V",PMparameter::LongReal,136},
		{false,true,"CCIHold","A",PMparameter::LongReal,144},
		{false,true,"MConductance","S",PMparameter::LongReal,184},
		{false,true,"MCapacitance","F",PMparameter::LongReal,192},
		{false,true,"IMonAdc","",PMparameter::Int16,212},
		{false,true,"VMonAdc","",PMparameter::Int16,192},
		{false,true,"StimDac","",PMparameter::Int16,220},
		{false,true,"StimFilterOn","",PMparameter::Byte,282},
		{false,true,"StimFilter","Hz",PMparameter::LongReal,296},
		{false,true,"Mode","",PMparameter::AmpModeName,237},
		{false,true,"SerialNumber","",PMparameter::String8,200},
		{false,true,"VmonFactor","x",PMparameter::LongReal,336},
		{false,true,"VmonOffset","V",PMparameter::LongReal,360},
		{false,true,"CalibDate","",PMparameter::String16,344}
	} };

	std::array<PMparameter, 14> parametersStimSegment{ {
		{true, true, "seMark", "", PMparameter::Int32, 0},
        { true, true, "seClass", "",  PMparameter::StimSegmentClass ,4},
        { true, true, "seStoreKind", "", PMparameter::SegStoreType, 5},
        { true,true,"seVoltageIncMode","",PMparameter::StimIncrementMode,6},
        { true,true,"seDurationIncMode","",PMparameter::StimIncrementMode,7},
		{ true,true,"seVoltage","V",PMparameter::LongReal ,8},
		{ true,true,"seVoltageSource","",PMparameter::Int32,16},
		{ true,true,"seDeltaVFactor", "",PMparameter::LongReal, 20},
		{ true,true,"seDeltaVIncrement","V", PMparameter::LongReal, 28},
		{ true,true,"seDuration","s", PMparameter::LongReal , 36},
		{ true,true,"seDurationSource", "",PMparameter::Int32, 44},
		{ true,true,"seDeltaTFactor","", PMparameter::LongReal, 48},
		{ true,true,"seDeltaTIncrement","s", PMparameter::LongReal, 56},
		{ true,true,"seScanRate","", PMparameter::LongReal, 72}
		} };

    std::array<PMparameter, 67> parametersChannel{ {
        {true,true,"chMark","",PMparameter::Int32,0},
        {true,true,"chLinkedChannel","",PMparameter::Int32, 4},
        {true,true,"chCompressionFactor","",PMparameter::Int32, 8},
        {true,true,"chYUnit","",PMparameter::String8, 12},
        {true,true,"chAdcChannel","",PMparameter::Int16, 20},
        {true,true,"chAdcMode","",PMparameter::AdcTypeName, 22},
        {true,true,"chDoWrite","",PMparameter::Boolean, 23},
        {true,true,"stLeakStore","",PMparameter::Byte, 24},
        {true,true,"chAmplMode","",PMparameter::AmplModeType,25},
        {true,true,"chOwnSegTime","",PMparameter::Boolean,26},
        {true,true,"chSetLastSegVmemb","",PMparameter::Boolean,27},
        {true,true,"chDacChannel","",PMparameter::Int16,28},
        {true,true,"chDacMode","",PMparameter::AdcTypeName,30},
        {true,true,"chHasLockInSquare","",PMparameter::Byte,31},
        {true,true,"chRelevantXSegment","",PMparameter::Int32,32},
        {true,true,"chRelevantYSegment","",PMparameter::Int32,36},
        {true,true,"chDacUnit","",PMparameter::String8,40},
        {true,true,"chHolding","",PMparameter::LongReal,48},
        {true,true,"chLeakHolding","",PMparameter::LongReal,56},
        {true,true,"chLeakSize","",PMparameter::LongReal,64},
        {true,true,"chLeakHoldMode","",PMparameter::Byte,72},
        {true,true,"chLeakAlternate","",PMparameter::Boolean,73},
        {true,true,"chAltLeakAveraging","",PMparameter::Boolean,74},
        {true,true,"chLeakPulseOn","",PMparameter::Boolean,75},
        {true,true,"chStimToDacID","",PMparameter::Set16,76},
        {true,true,"chCompressionMode","",PMparameter::Set16,78},
        {true,true,"chCompressionSkip","",PMparameter::Int32,80},
        {true,true,"chDacBit","",PMparameter::Int16,84},
        {true,true,"chHasLockInSine","",PMparameter::Boolean,86},
        {true,true,"chBreakMode","",PMparameter::Byte,87},
        {true,true,"chZeroSeg","",PMparameter::Int32,88},
        {true,true,"chStimSweep","",PMparameter::Int32,92},
        {true,true,"chSine_Freq","Hz",PMparameter::InvLongReal,96},
        {true,true,"chSine_Amplitude","V",PMparameter::LongReal,104},
        {true,true,"chLockIn_VReversal","V",PMparameter::LongReal,112},
        {true,true,"chChirp_StartFreq","Hz",PMparameter::LongReal,120},
        {true,true,"chChirp_EndFreq","Hz",PMparameter::LongReal,128},
        {true,true,"chChirp_MinPoints","",PMparameter::LongReal,136},
        {true,true,"chSquare_NegAmpl","",PMparameter::LongReal,144},
        {true,true,"chSquare_DurFactor","",PMparameter::LongReal,152},
        {true,true,"chLockIn_Skip","",PMparameter::Int32,160},
        {true,true,"chPhoto_MaxCycles","",PMparameter::Int32,164},
        {true,true,"chPhoto_SegmentNo","",PMparameter::Int32,168},
        {true,true,"chLockIn_AvgCycles","",PMparameter::Int32,172},
        {true,true,"chImaging_RoiNo","",PMparameter::Int32,176},
        {true,true,"chChirp_Skip","",PMparameter::Int32,180},
        {true,true,"chChirp_Amplitude","",PMparameter::LongReal,184},
        {true,true,"chPhoto_Adapt","",PMparameter::Byte,192},
        {true,true,"chSine_Kind","",PMparameter::Byte,193},
        {true,true,"chChirp_PreChirp","",PMparameter::Byte,194},
        {true,true,"chSine_Source","",PMparameter::Byte,195},
        {true,true,"chSquare_NegSource","",PMparameter::Byte,196},
        {true,true,"chSquare_PosSource","",PMparameter::Byte,197},
        {true,true,"chChirp_Kind","",PMparameter::Byte,198},
        {true,true,"chChirp_Source","",PMparameter::Byte,199},
        {true,true,"chDacOffset","",PMparameter::LongReal,200},
        {true,true,"chAdcOffset","",PMparameter::LongReal,208},
        {true,true,"chTraceMathFormat","",PMparameter::Byte,216},
        {true,true,"chHasChirp","",PMparameter::Boolean,217},
        {true,true,"chSquare_Kind","",PMparameter::Byte,218},
        {true,true,"chSquare_BaseIncr","",PMparameter::LongReal,224},
        {true,true,"chSquare_Cycle","",PMparameter::LongReal,232},
        {true,true,"chSquare_PosAmpl","",PMparameter::LongReal,240},
        {true,true,"chCompressionOffset","",PMparameter::Int32,248},
        {true,true,"chPhotoMode","",PMparameter::Int32,252},
        {true,true,"chBreakLevel","",PMparameter::LongReal,256},
        {true,true,"chTraceMath","",PMparameter::String128,264}
    } };

    std::array<PMparameter, 31> parametersStimulation{ {
        {true,true,"stMark","",PMparameter::Int32,0},
        {true,true,"stEntryName","",PMparameter::String32,4},
        {true,true,"stFileName","",PMparameter::String32,36},
        {true,true,"stAnalName","",PMparameter::String32,68},
        {true,true,"stDataStartSegment","",PMparameter::Int32,100},
        {true,true,"stDataStartTime","s",PMparameter::LongReal,104},
        {true,true,"stSampleInterval","s",PMparameter::LongReal,112},
        {true,true,"stSweepInterval","s",PMparameter::LongReal,120},
        {true,true,"stLeakDelay","s",PMparameter::LongReal,128},
        {true,true,"stFilterFactor","",PMparameter::LongReal,136},
        {true,true,"stNumberSweeps","",PMparameter::Int32,144},
        {true,true,"stNumberLeaks","",PMparameter::Int32,148},
        {true,true,"stNumberAverages","",PMparameter::Int32,152},
        {true,true,"stActualAdcChannels","",PMparameter::Int32,156},
        {true,true,"stActualDacChannels","",PMparameter::Int32,160},
        {true,true,"stExtTrigger","",PMparameter::ExtTriggerTypeName,164},
        {true,true,"stNoStartWait","",PMparameter::Boolean,165},
        {true,true,"stUseScanRates","",PMparameter::Boolean,166},
        {true,true,"stNoContAq","",PMparameter::Boolean,167},
        {true,true,"stHasLockIn","",PMparameter::Boolean,168},
        {true,true,"stOldStartMacKind","",PMparameter::Char,169},
        {true,true,"stOldEndMacKind","",PMparameter::Boolean,170},
        {true,true,"stAutoRange","",PMparameter::Byte,171},
        {true,true,"stBreakNext","",PMparameter::Boolean,172},
        {true,true,"stIsExpanded","",PMparameter::Boolean,173},
        {true,true,"stLeakCompMode","",PMparameter::Boolean,174},
        {true,true,"stHasChirp","",PMparameter::Boolean,175},
        {true,true,"stOldStartMacro","",PMparameter::String32,176},
        {true,true,"stOldEndMacro","",PMparameter::String32,208},
        {true,true,"sIsGapFree","",PMparameter::Boolean,240},
        {true,true,"sHandledExternally","",PMparameter::Boolean,241}
    } };

    std::array<PMparameter, 6> parametersStimRoot{ {
        {true,true,"roVersion","",PMparameter::Int32,0},
        {true,true,"roMark","",PMparameter::Int32,4},
        {true,true,"roVersionName","",PMparameter::String32,8},
        {true,true,"roMaxSamples","",PMparameter::Int32,40},
        {true,true,"roParams","",PMparameter::LongReal10,48},
        {true,true,"roParamText","",PMparameter::StringArray10x32,128}
    } };

	constexpr char list_seperator{ ';' };

	void PMparameter::formatValueOnly(const hkTreeNode& node, std::ostream& ss) const
	{
		bool data_na{ false };
		try {
			switch (data_type) {
			case Byte:
            case Char:
			{
				auto c = node.extractValueOpt<char>(offset);
				if (c) {
					ss << int(*c);
				}
				else {
					data_na = true;
				}
			}
				break;
			case Int16: {
				auto v = node.extractValueOpt<std::int16_t>(offset);
				if (v) {
					ss << *v;
				}
				else {
					data_na = true;
				}
			}
				break;
			case UInt16: {
				auto v = node.extractValueOpt<std::uint16_t>(offset);
				if (v) {
					ss << *v;
				}
				else {
					data_na = true;
				}
			}
				break;
			case Set16: {
				auto t = node.extractValueOpt<std::uint16_t>(offset);
				if (!t) { data_na = true; }
				else {
					ss << 'b';
					std::uint16_t u = 1U << 15;
					while (u) {
						if (*t & u) {
							ss << '1';
						}
						else {
							ss << '0';
						}
						u >>= 1;
					}
				}
			}
					  break;
			case Int32:
			{
				auto v = node.extractValueOpt<std::int32_t>(offset);
				if (v) {
					ss << *v;
				}
				else {
					data_na = true;
				}
			}
				break;
			case UInt32:
			{
				auto v = node.extractValueOpt<std::uint32_t>(offset);
				if (v) {
					ss << *v;
				}
				else {
					data_na = true;
				}
			}
				break;
			case LongReal:
			{
				auto v = node.extractValueOpt<double>(offset);
				if (v) {
					ss << *v;
				}
				else {
					data_na = true;
				}
			}
				break;
			case DateTime:
			{
				auto v = node.extractValueOpt<double>(offset);
				if (v) {
					ss << formatPMtimeUTC(*v);
				}
				else {
					data_na = true;
				}
			}
				break;
			case InvLongReal:
			{
				auto v = node.extractValueOpt<double>(offset);
				if (v) {
					ss << 1.0 / *v;
				}
				else {
					data_na = true;
				}
			}
			break;
			case StringType:
				ss << iso_8859_1_to_utf8(node.getString(offset));
				break;
			case String8:
				ss << iso_8859_1_to_utf8(node.getString<8>(offset));
				break;
			case String16:
				ss << iso_8859_1_to_utf8(node.getString<16>(offset));
				break;
			case String32:
				ss << iso_8859_1_to_utf8(node.getString<32>(offset));
				break;
			case String80:
				ss << iso_8859_1_to_utf8(node.getString<80>(offset));
				break;
            case String128:
                ss << iso_8859_1_to_utf8(node.getString<128>(offset));
                break;
			case String400:
				ss << iso_8859_1_to_utf8(node.getString<400>(offset));
				break;
            case StringArray10x32:
                ss << '(';
                for(int i=0;i<10;++i){
                    ss << '"' << iso_8859_1_to_utf8(node.getString<32>(offset+i*32)) << '"' << list_seperator;
                }
                ss << ')';
                break;
			case Boolean:
				ss << std::boolalpha << bool(node.getChar(offset));
				break;
			case Set16_Bit5: {
				auto v = node.extractValueOpt<std::uint16_t>(offset);
				if (v) {
					bool b = (*v) & (1u << 5);
					ss << std::boolalpha << b;
				}
				else {
					data_na = true;
				}
			}
				break;
			case LongReal2: {
				auto a = node.extractValueOpt<double>(offset);
				auto b = node.extractValueOpt<double>(offset + 8);
				if (a && b) {
					ss << '(' << *a << list_seperator
						<< *b << ')';
				}
				else {
					data_na = true;
				}
			}
				break;
			case LongReal4: {
				ss << "(";
				for (std::size_t i = 0; i < 4; ++i) {
					auto v = node.extractValueOpt<double>(offset + 8 * i);
					if (v) {
						ss << *v << list_seperator;
					}
					else {
						ss << "n/a";
						break;
					}
				}
				ss << ")";
			}
				break;
			case LongReal8:
				ss << "(";
				for (std::size_t i = 0; i < 8; ++i) {
					auto v = node.extractValueOpt<double>(offset + 8 * i);
					if (v) {
						ss << *v << list_seperator;
					}
					else {
						ss << "n/a";
						break;
					}
				}
				ss << ")";
				break;
            case LongReal10:
                ss << "(";
                for (std::size_t i = 0; i < 10; ++i) {
                    auto v = node.extractValueOpt<double>(offset + 8 * i);
                    if (v) {
                        ss << *v << list_seperator;
                    }
                    else {
                        ss << "n/a";
                        break;
                    }
                }
                ss << ")";
                break;
			case LongReal16:
				ss << "(";
				for (std::size_t i = 0; i < 16; ++i) {
					auto v = node.extractValueOpt<double>(offset + 8 * i);
					if (v) {
						ss << *v << list_seperator;
					}
					else {
						ss << "n/a";
						break;
					}
				}
				ss << ")";
				break;
			case RecordingMode:
				ss << RecordingModeNames.at(static_cast<std::size_t>(node.getChar(offset)));
				break;
			case RelativeTime:
				ss << std::fixed << std::setprecision(3) <<
					(node.extractLongReal(offset) - node.getTime0())
					<< std::defaultfloat << std::setprecision(6);
				break;
			case AmpModeName:
				ss << AmpModeNames.at(static_cast<std::size_t>(node.getChar(offset)));
				break;
            case StimIncrementMode:
                ss << IncrementModeNames.at(static_cast<std::size_t>(node.getChar(offset)));
                break;
            case ExtTriggerTypeName:
                ss << ExtTriggerTypeNames.at(static_cast<std::size_t>(node.getChar(offset)));
                break;
            case AmplModeType:
                ss << AmplModeTypeNames.at(static_cast<std::size_t>(node.getChar(offset)));
                break;
            case AdcTypeName:
                ss << AdcTypeNames.at(static_cast<std::size_t>(node.getChar(offset)));
                break;
            case SegStoreType:
                ss << SegStoreTypeNames.at(static_cast<std::size_t>(node.getChar(offset)));
                break;
            case StimSegmentClass:
            {
                auto index = static_cast<std::size_t>(node.getChar(offset));
                ss << SegmentClassNames.at(index);
            }
                break;
			case UserParamDesc8: {
				formatUserParamDesc<8>(node, offset, ss);
			}
							   break;
			case UserParamDesc4: {
				formatUserParamDesc<4>(node, offset, ss);
			}
							   break;
			case UserParamDesc2: {
				formatUserParamDesc<2>(node, offset, ss);
			}
							   break;
			default:
				throw std::runtime_error("unknown data-type");
				break;
			}
		}
		catch (const std::out_of_range& e) {
			(void)e;
			ss << "n/a";
		}
		if (data_na) {
			ss << "n/a";
		};
	}

	void PMparameter::format(const hkTreeNode& node, std::ostream& ss) const
	{
		ss << name << '=';
		formatValueOnly(node, ss);
		ss << ' ';
		// hack to choose correcly for holding voltage or current
		if (node.getLevel() == hkTreeNode::LevelTrace && std::strcmp("V|A", unit) == 0)
		{
			int recording_mode = node.getChar(TrRecordingMode);
			if (recording_mode == CClamp) {
				ss << 'A';
			}
			else {
				ss << 'V';
			}
		}
		else {
			ss << unit;
		}
	}

	static std::string JSONescapeQuotes(const std::string_view& s) {
		std::string tmp;
		tmp.reserve(s.size());
		for (const char c : s) {
			if (c == '"') {
				tmp.append("\\\"");
			}
			else {
				tmp.push_back(c);
			}
		}
		return tmp;
	}

    constexpr auto format_json_needs_quotes = std::to_array({
        PMparameter::Set16, // for bitfields
        PMparameter::DateTime, // weird PowerMod date
        PMparameter::StringType,
        PMparameter::String8, // String of length 8
        PMparameter::String16,
        PMparameter::String32,
        PMparameter::String80,
        PMparameter::String400,
        PMparameter::Boolean,
        PMparameter::LongReal2, // array of 2 doubles
        PMparameter::LongReal4, // array of 4 doubles
        PMparameter::LongReal8,	// array of 8 double
        PMparameter::LongReal16,  // 16 double
        PMparameter::RecordingMode,
        PMparameter::AmpModeName,
        PMparameter::UserParamDesc4, // 4x UserParamDesc
        PMparameter::UserParamDesc2,
        PMparameter::UserParamDesc8
    });


    void PMparameter::formatJSON(const hkTreeNode& node, std::ostream& ss, bool include_unit) const
	{
        const bool do_quotes = include_unit || (std::find(format_json_needs_quotes.begin(),
                                         format_json_needs_quotes.end(), data_type)!=
                               format_json_needs_quotes.end());
        ss << '"' << name << "\": ";
        if(do_quotes) ss << '"';
		std::stringstream tmp;
		formatValueOnly(node, tmp);
		ss << JSONescapeQuotes(tmp.str());
        if(include_unit) {
            // hack to choose correctly for holding voltage or current
            if (node.getLevel() == hkTreeNode::LevelTrace && std::strcmp("V|A", unit) == 0)
            {
                int recording_mode = node.getChar(TrRecordingMode);
                if (recording_mode == CClamp) {
                    ss << " A";
                }
                else {
                    ss << " V";
                }
            }
            else {
                if (unit[0]) { ss << ' ' << JSONescapeQuotes(unit); }
            }
        }
        if(do_quotes) ss << '"';
	}

	void PMparameter::format(const hkTreeNode& node, std::string& s) const
	{
		std::stringstream ss;
		format(node, ss);
		s = ss.str();
	}
}
