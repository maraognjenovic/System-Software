#include <iostream>
#include <fstream>
#include <iomanip>
#include "../inc/Linker.h"
using namespace std;
Linker::Linker(vector<string> inputFilesAll, string outputFileName, map<string, int> sectionAddresses, bool isHex)
{
    this->outputFileName = outputFileName;
    this->inputFilesAll = inputFilesAll;
    this->isHex = isHex;
    this->sectionAddresses = sectionAddresses;
    if (readDataFromBinary() == 0 || makeAgrSec() == 0 || makeAgrSymb() == 0 || makeAgrReloc() == 0 || makeAgrContSec() == 0 || (isHex ? solveRelocHex() : solveRelocLink()) == 0)
        errors();
    if (!errors())
        makeText();
}

bool Linker::errors()
{
    if (errorNotification.size() != 0)
    {
        cout << "Linker Greske:" << endl;
        for (auto tmp : errorNotification)
            cout << tmp << endl;
        return true;
    }
    else
        return false;
}

void Linker::read(ifstream &inputBinary, string &loc)
{
    unsigned strSize;
    inputBinary.read((char *)(&strSize), sizeof(strSize));
    loc.resize(strSize);
    inputBinary.read((char *)loc.c_str(), strSize);
}

int Linker::readSymbTable(ifstream &inputFile, const string singleFile)
{
    map<string, SymbolTable> symbolTblMap;
    int numSymbs = 0;
    inputFile.read((char *)&numSymbs, sizeof(numSymbs));
    int i = 0;
    while (i < numSymbs)
    {
        SymbolTable symTbl;
        string symName;

        read(inputFile, symName);
        inputFile.read((char *)(&symTbl.id_symbol), sizeof(symTbl.id_symbol));
        inputFile.read((char *)(&symTbl.defined), sizeof(symTbl.defined));
        read(inputFile, symTbl.name);
        inputFile.read((char *)(&symTbl.is_extern), sizeof(symTbl.is_extern));
        inputFile.read((char *)(&symTbl.binding), sizeof(symTbl.binding));
        read(inputFile, symTbl.section);
        inputFile.read((char *)(&symTbl.value), sizeof(symTbl.value));
        symbolTblMap[symName] = symTbl;
        i++;
    }
    symForEachFile.setSymTable(singleFile, symbolTblMap);
    return 1;
}

int Linker::readRelocTable(ifstream &inputFile, const string singleFile)
{
    int numRelocs = 0;
    inputFile.read((char *)&numRelocs, sizeof(numRelocs));
    vector<RelocTable> relocTableVec;
    int i = 0;
    while (i < numRelocs)
    {
        RelocTable relocData;
        inputFile.read((char *)(&relocData.data), sizeof(relocData.data));
        read(inputFile, relocData.name);
        inputFile.read((char *)(&relocData.offset), sizeof(relocData.offset));
        read(inputFile, relocData.symbolName);
        inputFile.read((char *)(&relocData.addend), sizeof(relocData.addend));
        read(inputFile, relocData.type);
        relocTableVec.push_back(relocData);
        i++;
    }

    relocForEachFile.setRelocTable(singleFile, relocTableVec);
    return 1;
}

int Linker::readSecTable(ifstream &inputFile, const string singleFile)
{
    map<string, SectionTable> sectionTblMap;

    int numSecs = 0;
    inputFile.read((char *)&numSecs, sizeof(numSecs));
    int i = 0;
    while (i < numSecs)
    {
        SectionTable sectionTbl;
        string secName;
        read(inputFile, secName);
        inputFile.read((char *)(&sectionTbl.size), sizeof(sectionTbl.size));
        sectionTbl.vma = 0;
        read(inputFile, sectionTbl.name);
        inputFile.read((char *)(&sectionTbl.idSection), sizeof(sectionTbl.idSection));
        int numOfOffs;
        inputFile.read((char *)(&numOfOffs), sizeof(numOfOffs));
        int k = 0;
        while (k < numOfOffs)
        {
            int x;
            inputFile.read((char *)(&x), sizeof(x));
            sectionTbl.offsets.push_back(x);
            k++;
        }
        int numOfData;
        inputFile.read((char *)(&numOfData), sizeof(numOfData));
        int j = 0;
        while (j < numOfData)
        {
            char c;
            inputFile.read((char *)(&c), sizeof(c));
            sectionTbl.data.push_back(c);
            j++;
        }
        sectionTblMap[secName] = sectionTbl;
        i++;
    }
    secForEachFile[singleFile] = sectionTblMap;
    return 1;
}

int Linker::readDataFromBinary()
{
    for (const auto &singleFile : inputFilesAll)
    {
        ifstream inputFile(singleFile, ios::binary);
        if (!inputFile.is_open())
            cout << "Ulazni fajl se nije otvorio!" << endl;
        if (inputFile.fail())
        {
            errorNotification.push_back(singleFile + ": ne postoji!");
            return 0;
        }
        readSecTable(inputFile, singleFile);
        readSymbTable(inputFile, singleFile);
        readRelocTable(inputFile, singleFile);
        inputFile.close();
    }

    return 1;
}
int Linker::makeAgrSymb()
{
    int nextId = 0;
    for (auto iter = outputSecTable.begin(); iter != outputSecTable.end(); iter++)
    {
        SymbolTable symbol(iter->second.idSection, true, false, true, iter->second.name, iter->second.name, iter->second.vma);
        if (symbol.id_symbol > nextId)
            nextId = symbol.id_symbol;

        outputSymTable[symbol.name] = symbol;
    }
    nextId++;
    map<string, SymbolTable> allExternSymb;

    for (auto filename : inputFilesAll)
    {
        auto symbolTableSingleFile = symForEachFile.getSymTable(filename);
        for (auto iter = symbolTableSingleFile.begin(); iter != symbolTableSingleFile.end(); iter++)
        {
            if (iter->second.name != iter->second.section)
            {
                if (!iter->second.is_extern)
                {
                    auto sym = outputSymTable.find(iter->first);
                    switch (sym == outputSymTable.end())
                    {
                    case true:
                        iter->second.id_symbol = nextId++;
                        switch (iter->second.section != "apsolute")
                        {
                        case true:
                            iter->second.value = iter->second.value + sectionHelp[iter->second.section][filename].startAdrAgrSec;
                            break;
                        case false:
                            break;
                        }
                        outputSymTable[iter->first] = iter->second;
                        break;

                    case false:
                        errorNotification.push_back("Simbol je vec definisan!" + sym->first);
                        return 0;
                        break;
                    }
                }
                else
                    allExternSymb[iter->first] = iter->second;
            }
        }
    }
    for (auto iter = allExternSymb.begin(); iter != allExternSymb.end(); iter++)
    {
        auto sym = outputSymTable.find(iter->first);
        if (sym == outputSymTable.end())
        {
            switch (!isHex)
            {
            case true:
                iter->second.id_symbol = nextId++;
                outputSymTable[iter->first] = iter->second;
                break;

            case false:
                errorNotification.push_back("Simbol nije definisan ni u jednom fajlu! " + iter->first);
                return 0;
                break;
            }
        }
    }
    return 1;
}

int Linker::makeAgrSec()
{
    map<string, int> prevEnd;
    int pomId = 1;
    for (auto filename : inputFilesAll)
        for (auto it : secForEachFile[filename])
            prevEnd[it.second.name] = 0;

    for (auto filename : inputFilesAll)
        for (auto it : secForEachFile[filename])
            if (it.second.name != "undefined")
            {
                SectionAdditionalData addiData(it.second.name, filename, it.second.size, prevEnd[it.second.name]);
                prevEnd[it.second.name] = prevEnd[it.second.name] + addiData.parentSecSize;
                sectionHelp[it.second.name][filename] = addiData;
            }

    for (auto it : prevEnd)
    {
        int pom;
        if (it.first == "apsolute")
            pom = -1;
        else if (it.first == "undefined")
            pom = 0;
        else
            pom = pomId++;

        SectionTable nextSecTable(pom, it.first, it.second, 0);
        this->outputSecTable[it.first] = nextSecTable;
    }

    if (isHex)
        if (replaceSectToVma() == 0)
            return 0;

    return 1;
}
int Linker::makeAgrContSec()
{
    for (auto iter = outputSecTable.begin(); iter != outputSecTable.end(); iter++)
    {
        string name = iter->first;
        if (iter->second.size != 0)
        {
            for (auto filename : inputFilesAll)
            {
                auto pom = sectionHelp[iter->first].find(filename);
                if (pom != sectionHelp[iter->first].end())
                {
                    auto sectionOffsets = this->secForEachFile[filename][name].offsets;
                    auto sectionData = this->secForEachFile[filename][name].data;
                    for (int i = 0; i < sectionOffsets.size() - 1; i++)
                    {
                        iter->second.offsets.push_back(sectionOffsets[i] + sectionHelp[name][filename].startAdrAgrSec);
                        for (int j = sectionOffsets[i]; j < sectionOffsets[i + 1]; j++)
                            iter->second.data.push_back(sectionData[j]);
                    }
                    iter->second.offsets.push_back(sectionOffsets[sectionOffsets.size() - 1] + sectionHelp[name][filename].startAdrAgrSec);
                    for (int k = sectionOffsets[sectionOffsets.size() - 1]; k < sectionData.size(); k++)
                        iter->second.data.push_back(sectionData[k]);
                }
            }
        }
    }
    return 1;
}
int Linker::makeAgrReloc()
{
    for (auto filename : inputFilesAll)
    {
        auto relocTableFile = relocForEachFile.getRelocTable(filename);
        for (auto relocData : relocTableFile)
        {
            RelocTable relocDataOutput(relocData.data, relocData.name, relocData.offset + sectionHelp[relocData.name][filename].startAdrAgrSec - outputSecTable[relocData.name].vma, relocData.type, relocData.addend, relocData.symbolName, filename);
            outputRelocTable.push_back(relocDataOutput);
        }
    }
    return 1;
}

int Linker::replaceSectToVma()
{
    int nextAdrNonSect = 0;
    for (auto pomPlace = sectionAddresses.begin(); pomPlace != sectionAddresses.end(); pomPlace++)
    {
        outputSecTable.find(pomPlace->first)->second.vma = pomPlace->second;

        for (auto pomFilename = sectionHelp[pomPlace->first].begin(); pomFilename != sectionHelp[pomPlace->first].end(); pomFilename++)
        {
            pomFilename->second.startAdrAgrSec += pomPlace->second;
            switch (pomFilename->second.startAdrAgrSec + pomFilename->second.parentSecSize > nextAdrNonSect)
            {
            case true:
                nextAdrNonSect = pomFilename->second.startAdrAgrSec + pomFilename->second.parentSecSize;
                break;
            case false:
                break;
            }
        }
    }
    for (auto pomOutOutSec = outputSecTable.begin(); pomOutOutSec != outputSecTable.end(); pomOutOutSec++)
    {
        if (pomOutOutSec->first != "undefined" && pomOutOutSec->first != "apsolute")
        {
            auto pomPlace = sectionAddresses.find(pomOutOutSec->first);
            if (pomPlace != sectionAddresses.end())
            {
                int leftAdr = pomOutOutSec->second.vma;
                int rightAdr = pomOutOutSec->second.vma + pomOutOutSec->second.size;

                if (rightAdr > 0xFF00)
                {
                    errorNotification.push_back("Sekcija nema vise memorije(-place i rezervisana mesta) " + pomOutOutSec->first);
                    return 0;
                }

                for (auto pomSections = outputSecTable.begin(); pomSections != outputSecTable.end(); pomSections++)
                {
                    if (pomSections->first != "undefined" && pomSections->first != "apsolute")
                    {
                        if (pomSections->first != pomOutOutSec->first)
                        {
                            auto pomPlace = sectionAddresses.find(pomSections->first);
                            if (pomPlace != sectionAddresses.end())
                                if (max(leftAdr, pomSections->second.vma) < min(rightAdr, pomSections->second.vma + pomSections->second.size))
                                {
                                    errorNotification.push_back("Sledecim dvema sekcijama su se ukrstile memorije: " + pomOutOutSec->first + " , " + pomSections->first);
                                    return 0;
                                }
                        }
                    }
                }
            }
        }
    }

    for (auto pomSections = outputSecTable.begin(); pomSections != outputSecTable.end(); pomSections++)
    {
        if (pomSections->first != "undefined" && pomSections->first != "apsolute")
        {
            auto pomPlace = sectionAddresses.find(pomSections->first);
            if (pomPlace == sectionAddresses.end())
            {
                pomSections->second.vma = nextAdrNonSect;
                for (auto iter = sectionHelp[pomSections->first].begin(); iter != sectionHelp[pomSections->first].end(); iter++)
                    iter->second.startAdrAgrSec += nextAdrNonSect;

                nextAdrNonSect += pomSections->second.size;
            }
            if (nextAdrNonSect > 0xFF00)
            {
                errorNotification.push_back("Ova sekcija nema vise memorije(-place, rezervisana  mesta) " + pomSections->second.name);
                return 0;
            }
        }
    }

    for (auto pomSekcija = sectionHelp.begin(); pomSekcija != sectionHelp.end(); pomSekcija++)
        for (auto pomFilename = pomSekcija->second.begin(); pomFilename != pomSekcija->second.end(); pomFilename++)
            SectionAdditionalData addiData = pomFilename->second;

    return 1;
}

int Linker::solveRelocLink()
{
    for (auto relocData : outputRelocTable)
    {
        auto iter = outputSecTable.find(relocData.symbolName);
        int additValue;
        if (iter != outputSecTable.end())
            additValue = sectionHelp.find(relocData.symbolName)->second.find(relocData.filename)->second.startAdrAgrSec;
        int low, high, pom;
        switch (relocData.data)
        {
        case true:
            pom = relocData.offset + 1;
            break;
        case false:
            pom = relocData.offset - 1;
            break;
        }
        low = outputSecTable.find(relocData.name)->second.data[relocData.offset];
        high = outputSecTable.find(relocData.name)->second.data[pom];
        outputSecTable[relocData.name].data[relocData.offset] = (0xff & ((int)((high << 8) + (0xff & low)) + additValue));
        outputSecTable[relocData.name].data[pom] = (0xff & (((int)((high << 8) + (0xff & low)) + additValue) >> 8));
    }

    return 1;
}
int Linker::solveRelocHex()
{
    for (auto relocData : outputRelocTable)
    {
        int pcRelOffs = 0;
        auto it = outputSecTable.find(relocData.symbolName);
        int additValue;
        if (it != outputSecTable.end())
            additValue = sectionHelp.find(relocData.symbolName)->second.find(relocData.filename)->second.startAdrAgrSec;
        else
            additValue = outputSymTable.find(relocData.symbolName)->second.value;

        if (relocData.type == "R_386_16_PC")
        {
            pcRelOffs = relocData.offset + outputSecTable.find(relocData.name)->second.vma;
            if (!relocData.data)
                pcRelOffs--;
        }
        int low, high, pom;
        switch (relocData.data)
        {
        case true:
            pom = relocData.offset + 1;
            break;
        case false:
            pom = relocData.offset - 1;
            break;
        }
        low = outputSecTable.find(relocData.name)->second.data[relocData.offset];
        high = outputSecTable.find(relocData.name)->second.data[pom];
        outputSecTable[relocData.name].data[relocData.offset] = (0xff & ((int)((high << 8) + (0xff & low)) + additValue - pcRelOffs));
        outputSecTable[relocData.name].data[pom] = (0xff & (((int)((high << 8) + (0xff & low)) + additValue - pcRelOffs) >> 8));
    }
    return 1;
}

void Linker::makeText()
{
    ofstream outputText(this->outputFileName);
    outputText << "#Tabela sekcija" << endl
               << "Id\t\t  Velicina\t\t VA\t\t\t  Ime" << endl;
    for (auto it : outputSecTable)
    {
        if (it.second.idSection >= 0)
            outputText << it.second.idSection << "\t\t|\t";
        else
            outputText << "-" << -it.second.idSection << "\t\t|\t";

        outputText << setw(4) << setfill(nula) << hex << (0xffff & it.second.size) << "\t|\t";
        outputText << setw(4) << setfill(nula) << hex << (0xffff & it.second.vma) << "\t|\t" << it.second.name << endl;
    }

    outputText << dec << endl
               << "#Tabela simbola" << endl
               << "Id\t\t  Vrednost\t\t Tip\t\t\t\tIme\t\t\t\t  Sekcija" << endl;
    for (auto it : outputSymTable)
    {
        // outputText << setw(4) << setfill(nula) << hex << (0xffff & it.second.id_symbol) << "\t|\t" << setw(4) << setfill(nula) << hex << (0xffff & it.second.value) << "\t|";
        if(it.second.id_symbol>=0)
        outputText << setw(4) << setfill(nula) << hex << (0xffff & it.second.id_symbol) ;
        else
        outputText << "-" <<setw(3)<< setfill(nula) << hex << (0xffff & -it.second.id_symbol) ;
        outputText << "\t|\t" << setw(4) << setfill(nula) << hex << (0xffff & it.second.value) << "\t|";
        switch (it.second.binding)
        {
        case true:
            outputText << "\tlocal\t|";
            break;
        case false:
            switch (it.second.defined)
            {
            case true:
                outputText << "\tglobal\t|";
                break;
            case false:
                switch (it.second.is_extern)
                {
                case true:
                    outputText << "\textern\t|";
                    break;
                case false:
                    outputText << "\tundef\t|";
                    break;
                }
                break;
            }
        }
        outputText << "\t\t" << it.second.name << "\t\t\t"
                   << "\t" << it.second.section << endl;
    }
    outputText << dec << endl
               << "#Relokaciona tablela" << endl
               << "Ime fajla\t\t\tOffset\t\t\tTip\t\t\tPodatak/Instrukcija\t\tIme simbola\t\t\tIme sekcije" << endl;

    for (RelocTable relocData : outputRelocTable)
    {
        outputText << relocData.filename << "\t|\t" << setw(4) << setfill(nula) << hex << (0xffff & relocData.offset);
        outputText << "\t|\t" << relocData.type << "\t|\t";
        if (relocData.data)
            outputText << "  podatak"
                       << "\t\t";
        else
            outputText << "instrukcija"
                       << "\t\t";
        outputText << "\t|\t\t" << relocData.symbolName << "\t\t\t" << relocData.name << endl;
    }
    outputText << dec << endl;
    if (isHex)
    {
        for (auto it : outputSecTable)
        {
            if (it.second.data.size() != 0)
                outputText << "#Sadrzaj." << it.first << endl
                           << "Adresa\t\t\tSadrzaj" << endl;
            SectionTable sTable = it.second;
            int counter = 0, i = 0;
            while (i < sTable.data.size())
            {
                if (counter % 8 == 0)
                    outputText << setw(4) << setfill(nula) << hex << (0xffff & counter + sTable.vma) << "  :  ";

                outputText << hex << setfill(nula) << setw(2) << (0xff & sTable.data[i]) << " ";
                counter++;
                if (counter % 8 == 0)
                    outputText << endl;
                i++;
            }
            outputText << endl
                       << dec << endl;
        }
    }
    else
    {
        for (auto it : outputSecTable)
        {
            outputText << "#Sadrzaj." << it.first << "(" << it.second.size << ")" << endl;
            if (it.second.size == 0)
                continue;

            SectionTable sTable = it.second;
            int i = 0;
            while (i < sTable.offsets.size() - 1)
            {
                outputText << setw(4) << setfill(nula) << hex << (0xffff & sTable.offsets[i]) << ": ";
                int j = sTable.offsets[i]; //current offs
                while (j < sTable.offsets[i + 1])
                {
                    outputText << hex << setfill(nula) << setw(2) << (0xff & sTable.data[j]) << " ";
                    j++;
                }
                outputText << endl;
                i++;
            }
            outputText << setw(4) << setfill(nula) << hex << (0xffff & sTable.offsets[sTable.offsets.size() - 1]) << ": ";
            int j = sTable.offsets[sTable.offsets.size() - 1]; //current offs
            while (j < sTable.data.size())
            {
                outputText << hex << setfill(nula) << setw(2) << (0xff & sTable.data[j]) << " ";
                j++;
            }
            outputText << endl
                       << dec << endl;
        }
    }
    outputText << "#Pomocni podaci:" << endl
               << hex;

    for (auto pomSekcija : sectionHelp)
    {
        outputText << "Sekcija: " << pomSekcija.first << endl
                   << "      Ime fajla     |  Ime sekcije  |Velicina sekcije|Opseg sekcije" << endl;
        for (auto pomFilename : pomSekcija.second)
        {
            outputText << "\t" << pomFilename.first;
            outputText << "\t\t" << pomFilename.second.section_name << "\t  \t\t\t" << pomFilename.second.parentSecSize;
            outputText << " \t\t\tod " << pomFilename.second.startAdrAgrSec << " do ";
            outputText << pomFilename.second.startAdrAgrSec + pomFilename.second.parentSecSize << endl;
        }
        outputText << endl;
    }
    outputText.close();
}
