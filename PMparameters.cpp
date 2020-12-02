#include<sstream>
#include"PMparameters.h"

// NOTE: This is preliminary, some paramaters are not yet included
std::array<PMparameter, 29>parametersTrace = {
	false,false,"Tr. Mark","",PMparameter::Int32,0,
	false,false,"Tr. Label","",PMparameter::StringType,4,
	false,false,"TraceID","",PMparameter::Int32,36,
	false,false,"TrHolding","V|A",PMparameter::LongReal,408,
	false,false,"Internal Solution","",PMparameter::Int32,48,
	false,false,"Leak traces","",PMparameter::Int32,60,
	false,false,"UseXStart","",PMparameter::Boolean,66,
	false,false,"XStart","s",PMparameter::LongReal,112,
	false,false,"Time offset","s",PMparameter::LongReal,80,
	false,false,"Zero data","A|V",PMparameter::LongReal,88,
	false,false,"Bandwidth","Hz",PMparameter::LongReal,144,
	false,false,"PipetteResistance","Ohm",PMparameter::LongReal,152,
	false,false,"CellPotential","V",PMparameter::LongReal,160,
	true,true,"SealResistance","Ohm",PMparameter::LongReal,168,
	true,true,"Cslow","F",PMparameter::LongReal,176,
	true,true,"Gseries","S",PMparameter::LongReal,184,
	false,false,"RsValue","",PMparameter::LongReal,192,
	false,false,"Gleak","S",PMparameter::LongReal,200,
	false,false,"Mem. conductance","S",PMparameter::LongReal,208,
	false,false,"CM","F",PMparameter::LongReal,248,
	false,false,"GM","S",PMparameter::LongReal,256,
	false,false,"GS","S",PMparameter::LongReal,280,
	false,false,"Phase","°",PMparameter::LongReal,264,
	false,false,"Ext.Sol.","",PMparameter::Int32,244,
	false,false,"IntSolVal","",PMparameter::LongReal,424,
	false,false,"ExtSolVal","",PMparameter::LongReal,432,
	false,false,"IntSolName","",PMparameter::StringType,440,
	false,false,"ExtSolName","",PMparameter::StringType,472,
	false,false,"DataPedestal","",PMparameter::LongReal,504
};

std::array<PMparameter, 16>parametersSweep = {
	false,false,"SwMark","",PMparameter::Int32,0,
	false,false,"SwLabel","",PMparameter::StringType,4,
	false,false,"Stim Count","",PMparameter::Int32,40,
	true,true,"Sweep Time","s",PMparameter::LongReal,48,
	true,true,"Timer Time","s",PMparameter::LongReal,56,
	false,false,"User param. 1","",PMparameter::LongReal,64,
	false,false,"User param. 2","",PMparameter::LongReal,72,
	false,false,"Pip. pressure","a.u.",PMparameter::LongReal,80,
	false,false,"RMS noise","A",PMparameter::LongReal,88, // not sure about units
	false,false,"Temperature","°C",PMparameter::LongReal,96,
	false,false,"DigitalIn","",PMparameter::UInt16,112,
	false,false,"DigitalOut","",PMparameter::UInt16,116,
	false,false,"SweepKind","",PMparameter::UInt16,114,
	false,false,"SwSwMarkers","",PMparameter::LongReal4,120,
	false,false,"Sweep hodling 16x","",PMparameter::LongReal16,160,
	false,false,"User param ex.","",PMparameter::LongReal8,288
};

std::array<PMparameter, 9>parametersSeries = {
	false,false,"SeMark","",PMparameter::Int32,0,
	false,false,"SeLabel","",PMparameter::StringType,4,
	false,false,"SeComment","",PMparameter::StringType,36,
	false,false,"SeSeriesCount","",PMparameter::Int32,116,
	false,false,"SeNumberSweeps","",PMparameter::Int32,120,
	false,false,"SeMethodTag","",PMparameter::Int32,132,
	true,false,"SeTime","s",PMparameter::LongReal,136,
	false,false,"SeMethodName","",PMparameter::StringType,312,
	false,false,"SeUsername","",PMparameter::StringType,872
};

std::array<PMparameter, 5>parametersGroup = {
	false, false, "GrMark","",PMparameter::Int32,0,
	false, false, "Label","",PMparameter::StringType,4,
	false,false,"GrText","",PMparameter::StringType,36,
	false,false,"ExperimentNumber","",PMparameter::Int32,116,
	false,false,"GroupCount","",PMparameter::Int32,120
};

std::array<PMparameter, 7>parametersRoot = {
	false, false, "RoVersion","",PMparameter::Int32,0,
	false, false, "RoMark","",PMparameter::Int32,4,
	false, false, "RoVersionName","",PMparameter::StringType,8,
	false, false, "RoAuxFileName", "", PMparameter::StringType,40,
	false, false, "RootText", "", PMparameter::StringType,120,
	true, true, "RootStartTime", "", PMparameter::DateTime,520,
	false,false, "RoMaxSamples","",PMparameter::Int32,528
};

void PMparameter::format(const hkTreeNode& node, std::stringstream& ss) const
{
	ss << name << "=";
	try {
		switch (data_type) {
		case Byte:
			ss << int(node.getChar(offset));
			break;
		case Int16:
			ss << node.extractValue<std::int16_t>(offset);
			break;
		case UInt16:
			ss << node.extractUInt16(offset);
			break;
		case Int32:
			ss << node.extractInt32(offset);
			break;
		case UInt32:
			ss << node.extractValue<std::uint32_t>(offset);
			break;
		case LongReal:
		case DateTime:
			ss << node.extractLongReal(offset);
			break;
		case StringType:
			ss << node.getString(offset);
			break;
		case Boolean:
			if (node.getChar(offset)) {
				ss << "true";
			}
			else {
				ss << "false";
			}
			break;
		case LongReal4:
			ss << "(";
			for (int i = 0; i < 4; ++i) {
				ss << node.extractLongReal(offset + 8 * i) << ",";
			}
			ss << ")";
			break;
		case LongReal8:
			ss << "(";
			for (int i = 0; i < 8; ++i) {
				ss << node.extractLongReal(offset + 8 * i) << ",";
			}
			ss << ")";
			break;
		case LongReal16:
			ss << "(";
			for (int i = 0; i < 16; ++i) {
				ss << node.extractLongReal(offset + 8 * i) << ",";
			}
			ss << ")";
			break;
		}
	}
	catch (const std::out_of_range& e) {
		(void)e;
		ss << "n/a";
	}
	ss << " " << unit;
}

void PMparameter::format(const hkTreeNode& node, std::string& s) const
{
	std::stringstream ss;
	format(node, ss);
	s = ss.str();
}
