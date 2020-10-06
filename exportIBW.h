#pragma once
void ExportAllTraces(std::istream& datafile, DatFile& datf, const std::string& path, const std::string& prefix);
void ExportTrace(std::istream& datafile, hkTreeNode& TrRecord, const std::string& filename, const std::string& wavename);