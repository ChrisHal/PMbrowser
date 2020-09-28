#include <istream>
#ifdef _DEBUG
#include <iostream>
#endif
#include <algorithm>
#include <cstring>
#include <cassert>
#include <cinttypes>
#include "helpers.h"
#include "DatFile.h"

bool DatFile::InitFromStream(std::istream& infile)
{
    char buffer[BundleHeaderSize];
    if (!infile) {
        throw std::runtime_error("cannot access file");
    }
    infile.read(buffer, BundleHeaderSize);
    if (!infile) {
        throw std::runtime_error("cannot read file");
    }
    bool isValid = std::strncmp(buffer, BundleSignature, 8) == 0;
    BundleHeader* bh = reinterpret_cast<BundleHeader*>(buffer);
    if (!isValid) {
        throw std::runtime_error("invalid file");
    }
    if (!bh->IsLittleEndian) {
        isSwapped = true;
    }
    Time = bh->Time;
    if (isSwapped) {
        swapInPlace(Time);
    }


    //int filesize = infile.seekg(0, std::ios::end).tellg();
    auto nitems = bh->Items;
    if (isSwapped) {
        swapInPlace(nitems);
    }

    nitems = std::min(nitems, 12); // make sure malformed files do not cause out of bounds read
    for (int i = 0; i < nitems; ++i) {
        //assert(!!infile);
        auto& item = bh->BundleItems[i];
        if (isSwapped) {
            swapInPlace(item.Length);
            swapInPlace(item.Start);
        }
        bool res = true;
        if (std::strcmp(item.Extension, ExtDat) == 0) {
            offsetDat = item.Start;
            lenDat = item.Length;
        }
        else if (std::strcmp(item.Extension, ExtPul) == 0) {
            // process pulse tree
#ifdef _DEBUG
            std::cerr << "loading pul tree\n";
#endif // _DEBUG
            res = PulTree.InitFromStream(infile, item.Start, item.Length);
        }
        else if (std::strcmp(item.Extension, ExtPgf) == 0) {
            // process pgf tree
#ifdef _DEBUG
            std::cerr << "loading pgf tree\n";
#endif // _DEBUG
            res = PgfTree.InitFromStream(infile, item.Start, item.Length);
        }
        else if (std::strcmp(item.Extension, ExtAmp) == 0) {
#ifdef _DEBUG
            std::cerr << "loading amp tree\n";
#endif // _DEBUG
            // process amp tree
            res = AmpTree.InitFromStream(infile, item.Start, item.Length);
        }
        if (!res) {
            throw std::runtime_error("error processing tree");
        }
    }
    return true;
}
