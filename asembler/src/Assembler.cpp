#include "../inc/Assembler.h"
int Assembler::idSymbol = 0;
int Assembler::idSect = 0;
int Assembler::lineBefore = 0;
int Assembler::lineAfter = 0;
Assembler::Assembler(string input, string pathOutput)
{
    this->input = input;
    this->pathOutput = pathOutput;
    output.open(pathOutput);
    this->cnt = 0;
    this->currSect = "";
    currNumOfLine = 0;
    SectionTable undefinedSect(idSect++, "undefined", 0);
    sectionTable["undefined"] = undefinedSect;

    SymbolTable symbolUndefSect(idSymbol++, true, false, true, "undefined", "undefined", 0);
    symbolTable["undefined"] = symbolUndefSect;

    SectionTable apsoluteSect(-1, "apsolute", 0);
    sectionTable["apsolute"] = apsoluteSect;

    SymbolTable symbolApsSect(idSymbol++, true, false, true, "apsolute", "apsolute", 0);
    symbolTable["apsolute"] = symbolApsSect;

    error = false;
    if (removeExcess() == 0 || firstPass() == 0 || secondPass() == 0)
        error = true;
    if (!error)
    {
        makeText();
        makeBinary();
    }
}
void Assembler::write(ofstream &binaryFile, string s)
{
    int strSize = s.length();
    binaryFile.write((char *)(&strSize), sizeof(strSize));
    binaryFile.write(s.c_str(), strSize);
}
void Assembler::addRelToBinary(ofstream &binaryFile)
{
    int sizeReloc = relocationTable.size();
    binaryFile.write((char *)&sizeReloc, sizeof(sizeReloc));
    for (RelocationTable relData : relocationTable)
    {
        binaryFile.write((char *)(&relData.data), sizeof(relData.data));
        write(binaryFile, relData.name);
        binaryFile.write((char *)(&relData.offset), sizeof(relData.offset));
        write(binaryFile, relData.symbolName);
        binaryFile.write((char *)(&relData.addend), sizeof(relData.addend));
        write(binaryFile, relData.type);
    }
}
void Assembler::addSymbToBinary(ofstream &binaryFile)
{
    int sizeSym = symbolTable.size();
    binaryFile.write((char *)&sizeSym, sizeof(sizeSym));

    for (auto it : symbolTable)
    {
        write(binaryFile, it.first);
        binaryFile.write((char *)(&it.second.id_symbol), sizeof(it.second.id_symbol));
        binaryFile.write((char *)(&it.second.defined), sizeof(it.second.defined));
        write(binaryFile, it.second.name);
        binaryFile.write((char *)(&it.second.is_extern), sizeof(it.second.is_extern));
        binaryFile.write((char *)(&it.second.binding), sizeof(it.second.binding));
        write(binaryFile, it.second.section);
        binaryFile.write((char *)(&it.second.value), sizeof(it.second.value));
    }
}
void Assembler::addSecToBinary(ofstream &binaryFile)
{
    int sizeSec = sectionTable.size();
    binaryFile.write((char *)&sizeSec, sizeof(sizeSec));
    for (auto it : sectionTable)
    {
        write(binaryFile, it.first);
        binaryFile.write((char *)(&it.second.size), sizeof(it.second.size));
        write(binaryFile, it.second.name);
        binaryFile.write((char *)(&it.second.idSection), sizeof(it.second.idSection));
        int sizeOffs = it.second.offsets.size();
        binaryFile.write((char *)&sizeOffs, sizeof(sizeOffs));
        int x = 0;
        for (auto o : it.second.offsets)
        {
            x++;
            binaryFile.write((char *)&o, sizeof(o));
        }
        int sizeData = it.second.data.size();
        binaryFile.write((char *)&sizeData, sizeof(sizeData));
        x = 0;
        for (auto c : it.second.data)
        {
            x++;
            binaryFile.write((char *)&c, sizeof(c));
        }
    }
}
void Assembler::makeBinary()
{
    string binaryName;
    if (regex_match(pathOutput, regex("^(.*)\\.txt$")))
        binaryName = regex_replace(pathOutput, regex("^(.*)\\.txt$"), "$1.o");
    else
        binaryName = pathOutput;
    ofstream binaryFile(binaryName, ios::out | ios::binary);
    unsigned int strSize;
    addSecToBinary(binaryFile);
    addSymbToBinary(binaryFile);
    addRelToBinary(binaryFile);
    binaryFile.close();
}
bool Assembler::errors()
{
    if (errorNotification.size() != 0)
    {
        auto tmp = errorNotification.begin();
        cout << "Greske:" << endl;
        while (tmp != errorNotification.end())
        {
            cout << realNumOfErrorLine[tmp->first] << ". - " << tmp->second << endl;
            ++tmp;
        }
        return true;
    }
    else
        return false;
}
int Assembler::errorDetected(int currNumOfLine, string mess)
{
    errorNotification[currNumOfLine] = mess;
    return 0;
}

int Assembler::workOnLabel(string labela)
{
    if (!currSect.empty())
    {
        auto symbol = symbolTable.find(labela);
        if (symbol == symbolTable.end())
        {
            SymbolTable newSymbol(idSymbol++, true, false, true, labela, currSect, cnt);
            symbolTable[labela] = newSymbol;
        }
        else
        {
            if (symbol->second.getDefined())
                return errorDetected(currNumOfLine, "Simbol je vec definisan u okviru ovog fajla");
            if (symbol->second.getIs_extern())
                return errorDetected(currNumOfLine, "Simbol je vec definisan u okviru drugog fajla");

            symbol->second.setDefined(true);
            symbol->second.setValue(cnt);
            symbol->second.setSection(currSect);
        }
        return 1;
    }
    else
        return errorDetected(currNumOfLine, "Labela nije definisana u okviru sekcije");
}

int Assembler::workOnSection(string name)
{
    cnt = 0;
    currSect = name;

    SymbolTable new_section_symbol(idSymbol++, true, false, true, name, currSect, cnt);
    symbolTable[name] = new_section_symbol;

    SectionTable new_section(idSect++, name, 0);
    sectionTable[name] = new_section;
    return 1;
}

int Assembler::workOnGlobalSymbol(string globalSymbol)
{

    auto symbol = symbolTable.find(globalSymbol);
    if (symbol != symbolTable.end())
        symbol->second.setBinding(false);
    else
    {
        SymbolTable newSymbol(idSymbol++, false, false, false, globalSymbol, "undefined", 0);
        symbolTable[globalSymbol] = newSymbol;
    }
    return 1;
}
int Assembler::workOnExternSymbol(const string externSymbol)
{
    auto symbol = symbolTable.find(externSymbol);
    if (symbol == symbolTable.end())
    {
        SymbolTable newSymbol(idSymbol++, false, true, false, externSymbol, "undefined", 0);
        symbolTable[externSymbol] = newSymbol;
    }
    else if (symbol->second.getDefined())
        return errorDetected(currNumOfLine, "Eksterni simbol je vec definisan");
    return 1;
}
int Assembler::getValueFromString(string stringLi)
{
    smatch getSmatch;
    int returnValue = 0;
    if (regex_search(stringLi, getSmatch, hexadecimal))
        returnValue = stoi(getSmatch.str(1).substr(2), 0, 16);
    else
    {
        regex_match(stringLi, getSmatch, decimal);
        returnValue = stoi(getSmatch.str(1));
    }
    return returnValue;
}
int Assembler::workOnEqu(string symbolName, string value)
{
    auto symbol = symbolTable.find(symbolName);
    if (symbol == symbolTable.end())
    {

        SymbolTable newSymbol(idSymbol++, true, false, true, symbolName, "apsolute", getValueFromString(value));
        symbolTable[symbolName] = newSymbol;

        pushBackSection("apsolute", sectionTable["apsolute"].getSize(), 0xff & getValueFromString(value), 0xff & (getValueFromString(value) >> 8));
        sectionTable["apsolute"].addSize(2);
        return 1;
    }
    else
    {
        if (symbol->second.getDefined())
            return errorDetected(currNumOfLine, "equ ne moze da definise vec definisani simbol");
        if (symbol->second.getIs_extern())
            return errorDetected(currNumOfLine, "equ ne moze da definise eksterni simbol");

        symbol->second.setDefined(true);
        symbol->second.setSection("apsolute");
        symbol->second.setValue(getValueFromString(value));
        pushBackSection("apsolute", sectionTable["apsolute"].getSize(), 0xff & getValueFromString(value), 0xff & (getValueFromString(value) >> 8));
        sectionTable["apsolute"].addSize(2);
        return 1;
    }
}
int Assembler::workOnSkipFirst(string value)
{
    if (currSect.empty())
        return errorDetected(currNumOfLine, "Skip se mora nalaziti u okviru sekcije");

    int skip_value = getValueFromString(value);
    addSize(currSect, skip_value);
    return 1;
}

int Assembler::workOnWordFirst()
{
    if (currSect.empty())
        return errorDetected(currNumOfLine, "Word se mora nalaziti u okviru sekcije");

    addSize(currSect, 2);
    return 1;
}
void Assembler::addSize(string s, int n)
{
    cnt += n;
    sectionTable[currSect].addSize(n);
}

int Assembler::workOnInstructionFirst(string lineWithSmth)
{
    string instCode;
    string regD;
    string regS;
    smatch getOperand;
    smatch getSmatch;
    string operand;
    if (currSect.empty())
        return errorDetected(currNumOfLine, "Instrukcija se mora nalaziti unutar sekcije");

    if (regex_search(lineWithSmth, getSmatch, insHIR))
        addSize(currSect, 1);

    else if (regex_search(lineWithSmth, getSmatch, insRegOneOp))
    {
        instCode = getSmatch.str(1);
        if (instCode == "int")
            addSize(currSect, 2);

        if (instCode == "not")
            addSize(currSect, 2);

        if (instCode == "pop")
            addSize(currSect, 3);

        if (instCode == "push")
            addSize(currSect, 3);

        return 1;
    }
    else if (regex_search(lineWithSmth, getSmatch, insJmpOneOp))
    {
        instCode = getSmatch.str(1);
        operand = getSmatch.str(2);

        switch (regex_search(operand, getOperand, jmpRegDir))
        {
        case true:
            addSize(currSect, 3);
            break;
        case false:
            switch (regex_search(operand, getOperand, jmpRegInd))
            {
            case true:
                addSize(currSect, 3);
                break;
            case false:
                switch (regex_search(operand, getOperand, jmpApsolute))
                {
                case true:
                    addSize(currSect, 5);
                    break;
                case false:
                    switch (regex_search(operand, getOperand, jmpPcRelative))
                    {
                    case true:
                        addSize(currSect, 5);
                        break;
                    case false:
                        switch (regex_search(operand, getOperand, jmpRegIndShift))
                        {
                        case true:
                            addSize(currSect, 5);
                            break;
                        case false:
                            switch (regex_search(operand, getOperand, jmpMemDir))
                            {
                            case true:
                                addSize(currSect, 5);
                                break;
                            case false:
                                return errorDetected(currNumOfLine, "Tip adresiranja nije validan");
                            }
                        }
                    }
                }
            }
        }
        return 1;
    }
    else if (regex_search(lineWithSmth, getSmatch, insLdStTwoOp))
    {
        operand = getSmatch.str(3);
        regD = getSmatch.str(2);
        instCode = getSmatch.str(1);

        switch (regex_search(operand, getOperand, LdStRegDir))
        {
        case true:
            addSize(currSect, 3);
            break;
        case false:
            switch (regex_search(operand, getOperand, LdStRegInd))
            {
            case true:
                addSize(currSect, 3);
                break;
            case false:
                switch (regex_search(operand, getOperand, LdStApsolute))
                {
                case true:
                    addSize(currSect, 5);
                    break;
                case false:
                    switch (regex_search(operand, getOperand, LdStPcRelative))
                    {
                    case true:
                        addSize(currSect, 5);
                        break;
                    case false:
                        switch (regex_search(operand, getOperand, LdStRegIndShift))
                        {
                        case true:
                            addSize(currSect, 5);
                            break;
                        case false:
                            switch (regex_search(operand, getOperand, LdStMemDir))
                            {
                            case true:
                                addSize(currSect, 5);
                                break;
                            case false:
                                return errorDetected(currNumOfLine, "Tip adresiranja nije validan");
                            }
                        }
                    }
                }
            }
        }
    }
    else if (regex_search(lineWithSmth, getSmatch, insRegTwoOp))
    {
        regS = getSmatch.str(3);
        regD = getSmatch.str(2);
        instCode = getSmatch.str(1);
        addSize(currSect, 2);
    }
    else
        return errorDetected(currNumOfLine, "Nepostojeca instrukcija");
    return 1;
}

int Assembler::firstPass()
{
    error = false;
    currNumOfLine = 0;
    for (string line : inputLines)
    {
        if (line.empty())
            continue;
        currNumOfLine++;
        smatch getSmatch;
        if (regex_search(line, getSmatch, label))
        {
            string label_name = getSmatch.str(1);
            if (workOnLabel(label_name) == 0)
                error = true;
        }
        else
        {
            if (regex_search(line, getSmatch, labelFollowedBySmth))
            {
                string label_name = getSmatch.str(1);
                line = getSmatch.str(2);
                if (workOnLabel(label_name) == 0)
                    error = true;
            }
            if (regex_search(line, getSmatch, globalDirective))
            {
                string symbols = getSmatch.str(1);
                stringstream ss(symbols);
                string symbol;
                while (getline(ss, symbol, ','))
                    workOnGlobalSymbol(symbol);
            }
            else if (regex_search(line, getSmatch, externDirective))
            {
                string symbols = getSmatch.str(1);
                stringstream ss(symbols);
                string symbol;
                while (getline(ss, symbol, ','))
                    if (workOnExternSymbol(symbol) == 0)
                        error = true;
            }
            else if (regex_search(line, getSmatch, sectionDirective))
            {
                string name = getSmatch.str(1);
                workOnSection(name);
            }
            else if (regex_search(line, getSmatch, skipDirective))
            {
                string value = getSmatch.str(1);
                if (workOnSkipFirst(value) == 0)
                    error = true;
            }
            else if (regex_search(line, getSmatch, equDirective))
            {
                string symbolName = getSmatch.str(1);
                string value = getSmatch.str(2);
                if (workOnEqu(symbolName, value) == 0)
                    error = true;
            }
            else if (regex_search(line, getSmatch, endDirective))
            {
                break;
            }
            else if (regex_search(line, getSmatch, wordDirective))
            {
                string data = getSmatch.str(1);
                stringstream ss(data);
                string symbol_or_number;
                while (getline(ss, symbol_or_number, ','))
                    if (workOnWordFirst() == 0)
                        error = true;
            }
            else
            {
                if (workOnInstructionFirst(line) == 0)
                    error = true;
            }
        }
    }
    if (error)
        return 0;
    else
        return 1;
}
void Assembler::makeText()
{
    output << "#Tabela simbola" << endl
           << "Id\t\t\tTip\t\t  Vrednost\t\tSekcija\t\t\tNaziv" << endl;
    for (auto it : symbolTable)
    {
        output << setw(4) << setfill(nula) << hex << (0xffff & it.second.getId_symbol()) << "\t|\t";
        if (!it.second.getBinding())
        {
            if (!it.second.getDefined())
            {
                if (it.second.getIs_extern())
                    output << "extern\t|\t";
                else
                    output << "undefined\t|\t";
            }
            else
                output << "global\t|\t";
        }
        else
            output << "local\t|\t";

        if (it.second.getName() != "apsolute" && it.second.getName() != "undefined")
            output << " " << setw(4) << setfill(nula) << hex << (0xffff & it.second.getValue()) << "\t|\t" << it.second.getSection() << "\t\t|\t" << it.second.getName() << endl;
        else
            output << " " << setw(4) << setfill(nula) << hex << (0xffff & it.second.getValue()) << "\t|\t" << it.second.getSection() << "\t|\t" << it.second.getName() << endl;
    }
    output << dec << endl;
    output << "#Tabela sekcija" << endl
           << "Id\t\t Naziv\t\t\tVelicina" << endl;

    for (auto it : sectionTable)
    {
        if (it.second.getIdSection() >= 0)
            output << " ";
        if (it.second.getName() != "apsolute" && it.second.getName() != "undefined")
            output << it.second.getIdSection() << "\t|\t" << it.second.getName() << "\t\t|\t" << setw(4) << setfill(nula) << hex << (0xffff & it.second.getSize()) << endl;
        else
            output << it.second.getIdSection() << "\t|\t" << it.second.getName() << "\t|\t" << setw(4) << setfill(nula) << hex << (0xffff & it.second.getSize()) << endl;
    }
    output << endl;

    for (auto it : sectionTable)
    {
        output << "#.Rel." << it.first << endl;

        output << "Ofset\t\tTip\t\t\t Podatak/Instrukcija    Sekcija\t    Simbol" << endl;
        for (RelocationTable relocData : relocationTable)
        {
            if (relocData.getSectionName() == it.first)
            {
                if (relocData.getData())
                    output << setw(4) << setfill(nula) << hex << (0xffff & relocData.getOffset()) << "\t|\t" << relocData.getType() << "\t|  "
                           << " podatak"
                           << "\t\t|\t" << relocData.getSectionName() << "\t|\t" << relocData.getSymbolName() << endl;
                else
                    output << setw(4) << setfill(nula) << hex << (0xffff & relocData.getOffset()) << "\t|\t" << relocData.getType() << "\t|  "
                           << " instrukcija"
                           << "\t\t|\t" << relocData.getSectionName() << "\t|\t" << relocData.getSymbolName() << endl;
            }
        }
        output << endl
               << "#." << dec << it.first << endl;

        SectionTable symTable = it.second;
        if (symTable.getSize() == 0)
        {
            output << dec << endl
                   << endl;
            continue;
        }

        long unsigned int i = 0;
        while (i < symTable.offsets.size() - 1)
        {
            int currOffset = symTable.offsets[i];
            int next = symTable.offsets[i + 1];
            output << setw(4) << setfill(nula) << hex << (0xffff & currOffset) << ": ";
            int j = currOffset;
            while (j < next)
            {
                char c = symTable.data[j];
                output << setw(2) << setfill(nula) << hex << (0xff & c) << " ";
                j++;
            }
            output << endl;
            i++;
        }
        int sizeOff = symTable.offsets.size() - 1;
        int next = symTable.data.size();
        output << setw(4) << setfill(nula) << hex << (0xffff & symTable.offsets[sizeOff]) << ": ";
        int j = symTable.offsets[sizeOff];
        while (j < next)
            output << setw(2) << setfill(nula) << hex << (0xff & symTable.data[j++]) << " ";

        output << dec << endl
               << endl;
    }
    output.close();
}
int Assembler::workOnAps(string apsSymbol)
{
    auto symbol = symbolTable.find(apsSymbol);
    if (symbol != symbolTable.end())
    {
        SymbolTable sTable = symbol->second;
        if (sTable.getSection() == "apsolute")
        {
            return sTable.getValue();
        }
        else
        {
            RelocationTable relocData(false, currSect, cnt + 4, "R_386_16", 0, "");
            int retValue;
            if ((!sTable.getBinding()) || (sTable.getIs_extern()))
            {
                relocData.setSymbolName(sTable.getName());
                retValue = 0;
            }
            else
            {
                relocData.setSymbolName(sTable.getSection());
                retValue = sTable.getValue();
            }
            relocationTable.push_back(relocData);
            return retValue;
        }
    }
    else
        return errorDetected(currNumOfLine, "Simbol se ne nalazi u tabeli simbola");

    return 0;
}

int Assembler::workOnPCRelative(string pcRelativeSymbol)
{

    auto symbol = symbolTable.find(pcRelativeSymbol);
    if (symbol != symbolTable.end())
    {
        SymbolTable sTable = symbol->second;
        int addend = -2;
        if (sTable.getSection() == "apsolute")
        {

            RelocationTable relocData(false, currSect, cnt + 4, "R_386_16_PC", 0, sTable.getName());
            relocData.setSymbolName(sTable.getName());
            relocationTable.push_back(relocData);
            return addend;
        }
        else
        {
            RelocationTable relocData(false, currSect, cnt + 4, "R_386_16_PC", 0, "");

            int retValue;
            if ((!sTable.getBinding()) || (sTable.getIs_extern()))
            {
                relocData.setSymbolName(sTable.getName());
                retValue = addend;
            }
            else
            {
                if (currSect == sTable.getSection())
                {
                    retValue = sTable.getValue() + addend - (cnt + 3);
                    return retValue;
                }
                else
                {
                    relocData.setSymbolName(sTable.getSection());
                    retValue = sTable.getValue() + addend;
                }
            }
            relocationTable.push_back(relocData);
            return retValue;
        }
    }
    else
        return errorDetected(currNumOfLine, "Simbol se ne nalazi u tabeli simbola");

    return 0;
}
void Assembler::processLine(string &line)
{
    line = regex_replace(line, regex("([^#]*)#.*"), "$1", regex_constants::format_first_only);
    line = regex_replace(line, regex("\\t"), " ");
    line = regex_replace(line, regex(" ?: ?"), ":");
    line = regex_replace(line, regex(" ?, ?"), ",");
    line = regex_replace(line, regex(" {2,}"), " ");
    line = regex_replace(line, regex("^( *)([^ ].*[^ ])( *)$"), "$2");
}
int Assembler::removeExcess()
{

    string line;
    //inputFile.open(input.c_str());

    //ifstream inputFile("/home/ss/CLionProjects/projekatSSS/test/"+input);
    ifstream inputFile(input);
    //ifstream inputFile("Desktop/projmain.s");
    if (!inputFile.is_open())
    {
        cout << "NIJE SE LEPO OTVORILO";
        return 0;
    }
    while (getline(inputFile, line))
    {
        processLine(line);
        if (line != "" && line != " ")
        {
            realNumOfErrorLine[++lineAfter] = ++lineBefore;
            inputLines.push_back(line);
        }
        else
            lineBefore++;
    }
    inputFile.close();
    return 1;
}
int Assembler::getInstructionCode(string s)
{
    int value = 0;
    if (s == "call")
        value = 0x30;
    if (s == "jmp")
        value = 0x50;
    if (s == "jeq")
        value = 0x51;
    if (s == "jne")
        value = 0x52;
    if (s == "jgt")
        value = 0x53;
    if (s == "ldr")
        value = 0xA0;
    if (s == "str")
        value = 0xB0;
    if (s == "psw")
        value = 0x8;
    if (s == "xchg")
        value = 0x60;
    if (s == "add")
        value = 0x70;
    if (s == "sub")
        value = 0x71;
    if (s == "mul")
        value = 0x72;
    if (s == "div")
        value = 0x73;
    if (s == "cmp")
        value = 0x74;
    if (s == "and")
        value = 0x81;
    if (s == "or")
        value = 0x82;
    if (s == "xor")
        value = 0x83;
    if (s == "test")
        value = 0x84;
    if (s == "shl")
        value = 0x90;
    if (s == "shr")
        value = 0x91;
    return value;
}
void Assembler::pushBackSection2(string curr, int off, int d)
{
    sectionTable[curr].data.push_back(d);
    sectionTable[curr].offsets.push_back(off);
}
int Assembler::workOnInstructionSecond(string lineWithSmth)
{
    int value;
    string operand;
    smatch getOperand;
    int insDesc;
    int regDesc;
    int address;
    smatch getSmatch;
    string instCode;
    int regNum;
    string displacement;
    string regD;
    string regS;

    if (regex_search(lineWithSmth, getSmatch, insHIR))
    {
        instCode = getSmatch.str(1);

        if (instCode == "ret")

            pushBackSection2(currSect, cnt, 0x40);

        else if (instCode == "halt")

            pushBackSection2(currSect, cnt, 0x00);

        else if (instCode == "iret")

            pushBackSection2(currSect, cnt, 0x20);

        cnt += 1;
    }
    else if (regex_search(lineWithSmth, getSmatch, insRegOneOp))
    {
        instCode = getSmatch.str(1);
        if (getSmatch.str(2) == "psw")
            regNum = 8;
        else
            regNum = getSmatch.str(2).at(1) - 48;

        if (instCode == "int")
        {
            pushBackSection(currSect, cnt, 0x10, (regNum << 4) + 15);
            cnt += 2;
        }
        else if (instCode == "push")
        {
            pushBackSection(currSect, cnt, 0xB0, (regNum << 4) + 6);
            sectionTable[currSect].data.push_back(0x12);
            cnt += 3;
        }
        else if (instCode == "pop")
        {
            pushBackSection(currSect, cnt, 0xA0, (regNum << 4) + 6);
            sectionTable[currSect].data.push_back(0x42);
            cnt += 3;
        }
        else if (instCode == "not")
        {
            pushBackSection(currSect, cnt, 0x80, (regNum << 4) + 15);
            cnt += 2;
        }
    }
    else if (regex_search(lineWithSmth, getSmatch, insJmpOneOp))
    {
        instCode = getSmatch.str(1);
        operand = getSmatch.str(2);
        regDesc = 0xF0;
        insDesc = getInstructionCode(instCode);
        if (regex_search(operand, getOperand, jmpApsolute))
        {
            switch (regex_match(operand, symbol))
            {
                int valueOut;
            case true:
                regDesc += 0xF;
                address = 0;
                valueOut = workOnAps(operand);
                pushBackSection(currSect, cnt, insDesc, regDesc);
                pushBackSection3Data(currSect, address, 0xff & (valueOut >> 8), 0xff & valueOut);
                cnt += 5;
                break;
            case false:
                int value = getValueFromString(operand);
                regDesc += 0xF;
                address = 0;
                pushBackSection(currSect, cnt, insDesc, regDesc);
                pushBackSection3Data(currSect, address, 0xff & (value >> 8), 0xff & (value));
                cnt += 5;
                break;
            }
        }
        else if (regex_search(operand, getOperand, jmpPcRelative))
        {
            operand = getOperand.str(1);
            regDesc += 0x7;
            address = 0x05;

            int valueOut = workOnPCRelative(operand);

            pushBackSection(currSect, cnt, insDesc, regDesc);
            pushBackSection3Data(currSect, address, 0xff & (valueOut >> 8), 0xff & (valueOut));
            cnt += 5;
        }
        else if (regex_search(operand, getOperand, jmpRegDir))
        {
            if (getOperand.str(1) == "psw")
                regNum = 8;
            else
                regNum = getOperand.str(1).at(1) - 48;

            regDesc += regNum;
            address = 0x01;

            pushBackSection(currSect, cnt, insDesc, regDesc);
            sectionTable[currSect].data.push_back(address);
            cnt += 3;
        }
        else if (regex_search(operand, getOperand, jmpRegInd))
        {
            if (getOperand.str(1) == "psw")
                regNum = 8;
            else
                regNum = getOperand.str(1).at(1) - 48;

            regDesc += regNum;
            address = 0x02;
            pushBackSection(currSect, cnt, insDesc, regDesc);
            sectionTable[currSect].data.push_back(address);
            cnt += 3;
        }
        else if (regex_search(operand, getOperand, jmpRegIndShift))
        {
            displacement = getOperand.str(2);
            if (getOperand.str(1) == "psw")
                regNum = 8;
            else
                regNum = getOperand.str(1).at(1) - 48;

            regDesc += regNum;
            address = 0x03;
            int valueOut;
            switch (regex_match(displacement, symbol))
            {
            case true:
                valueOut = workOnAps(displacement);
                break;
            case false:
                valueOut = getValueFromString(displacement);
                break;
            }
            pushBackSection(currSect, cnt, insDesc, regDesc);
            pushBackSection3Data(currSect, address, 0xff & (valueOut >> 8), 0xff & (valueOut));
            cnt += 5;
        }
        else if (regex_search(operand, getOperand, jmpMemDir))
        {

            operand = getOperand.str(1);
            regDesc += 0xF;
            address = 0x04;
            int valueOut;
            switch (regex_match(operand, symbol))
            {
            case true:
                valueOut = workOnAps(operand);
                break;
            case false:
                valueOut = getValueFromString(operand);
                break;
            }
            pushBackSection(currSect, cnt, insDesc, regDesc);
            pushBackSection3Data(currSect, address, 0xff & (valueOut >> 8), 0xff & (valueOut));
            cnt += 5;
        }
    }
    else if (regex_search(lineWithSmth, getSmatch, insLdStTwoOp))
    {
        instCode = getSmatch.str(1);
        regD = getSmatch.str(2);
        operand = getSmatch.str(3);
        insDesc = getInstructionCode(instCode);
        if (regD == "psw")
            regDesc = 0x8;
        else
            regDesc = regD.at(1) - 48;

        regDesc <<= 4;
        if (regex_search(operand, getOperand, LdStApsolute))
        {
            operand = getOperand.str(1);
            if (regex_match(operand, symbol))
            {
                regDesc += 0xF;
                address = 0;
                value = workOnAps(operand);
                pushBackSection(currSect, cnt, insDesc, regDesc);
                pushBackSection3Data(currSect, address, 0xff & (value >> 8), 0xff & (value));
                cnt += 5;
            }
            else
            {
                value = getValueFromString(operand);
                regDesc += 0xF;
                address = 0;
                pushBackSection(currSect, cnt, insDesc, regDesc);
                pushBackSection3Data(currSect, address, 0xff & (value >> 8), 0xff & (value));
                cnt += 5;
            }
        }
        else if (regex_search(operand, getOperand, LdStPcRelative))
        {

            operand = getOperand.str(1);
            regDesc += 0x7;
            address = 0x03;
            value = workOnPCRelative(operand);

            pushBackSection(currSect, cnt, insDesc, regDesc);
            pushBackSection3Data(currSect, address, 0xff & (value >> 8), 0xff & (value));
            cnt += 5;
        }
        else if (regex_search(operand, getOperand, LdStRegDir))
        {
            if (getOperand.str(1) == "psw")
                regNum = 8;
            else
                regNum = getOperand.str(1).at(1) - 48;

            regDesc += regNum;
            address = 0x01;
            pushBackSection(currSect, cnt, insDesc, regDesc);
            sectionTable[currSect].data.push_back(address);
            cnt += 3;
        }
        else if (regex_search(operand, getOperand, LdStRegInd))
        {
            if (getOperand.str(1) == "psw")
                regNum = 8;
            else
                regNum = getOperand.str(1).at(1) - 48;
            regDesc += regNum;
            address = 0x02;
            pushBackSection(currSect, cnt, insDesc, regDesc);
            sectionTable[currSect].data.push_back(address);
            cnt += 3;
        }
        else if (regex_search(operand, getOperand, LdStRegIndShift))
        {
            displacement = getOperand.str(2);
            if (getOperand.str(1) == "psw")
                regNum = 8;

            else
                regNum = getOperand.str(1).at(1) - 48;

            regDesc += regNum;
            address = 0x03;
            if (regex_match(displacement, symbol))
                value = workOnAps(displacement);
            else
                value = getValueFromString(displacement);

            pushBackSection(currSect, cnt, insDesc, regDesc);
            pushBackSection3Data(currSect, address, 0xff & (value >> 8), 0xff & (value));
            cnt += 5;
        }
        else if (regex_search(operand, getOperand, LdStMemDir))
        {

            regDesc += 0xF;
            address = 0x04;
            if (regex_match(operand, symbol))
                value = workOnAps(operand);
            else
                value = getValueFromString(operand);

            pushBackSection(currSect, cnt, insDesc, regDesc);
            pushBackSection3Data(currSect, address, 0xff & (value >> 8), 0xff & (value));
            cnt += 5;
        }
        else
            return errorDetected(currNumOfLine, "Tip adresiranja nije validan");
    }
    else if (regex_search(lineWithSmth, getSmatch, insRegTwoOp))
    {
        instCode = getSmatch.str(1);
        regD = getSmatch.str(2);
        regS = getSmatch.str(3);
        insDesc = getInstructionCode(instCode);

        if (regD != "psw")
            regDesc = regD.at(1) - 48;
        else
            regDesc = 0x8;

        regDesc <<= 4;

        if (regS != "psw")
            regDesc = regDesc + regS.at(1) - 48;

        else
            regDesc = regDesc + 0x8;

        pushBackSection(currSect, cnt, insDesc, regDesc);
        cnt += 2;
    }
    else
        return errorDetected(currNumOfLine, "Nepostojeca instrukcija");
    return 1;
}
int Assembler::secondPass()
{
    currNumOfLine = 0;
    currSect = "";
    cnt = 0;
    for (auto line : inputLines)
    {
        currNumOfLine++;
        smatch getSmatch;
        if (!regex_search(line, getSmatch, label))
        {
            if (regex_search(line, getSmatch, labelFollowedBySmth))
                line = getSmatch.str(2);

            if (regex_search(line, getSmatch, globalDirective) || regex_search(line, getSmatch, externDirective) || regex_search(line, getSmatch, equDirective))
            {
            }
            else if (regex_search(line, getSmatch, sectionDirective))
            {
                cnt = 0;
                currSect = getSmatch.str(1);
            }
            else if (regex_search(line, getSmatch, wordDirective))
            {
                string data = getSmatch.str(1);
                stringstream ss(data);
                string symbol_or_number;
                while (getline(ss, symbol_or_number, ','))
                    if (workOnWordSecond(symbol_or_number) == 0)
                        error = true;
            }
            else if (regex_search(line, getSmatch, skipDirective))
            {
                string value = getSmatch.str(1);
                int k = getValueFromString(value);
                sectionTable[currSect].offsets.push_back(cnt);
                int i = 0;
                while (i < k)
                {
                    sectionTable[currSect].data.push_back(0);
                    i++;
                }
                cnt += k;
            }
            else if (regex_search(line, getSmatch, endDirective))
                break;
            else if (!workOnInstructionSecond(line))
                error = true;
        }
    }
    if (error)
        return 0;
    else
        return 1;
}
void Assembler::pushBackSection(string currSection, int cnt, int data1, int data2)
{
    sectionTable[currSection].offsets.push_back(cnt);
    sectionTable[currSection].data.push_back(data1);
    sectionTable[currSection].data.push_back(data2);
}
void Assembler::pushBackSection3Data(string currSection, int data1, int data2, int data3)
{
    sectionTable[currSection].data.push_back(data1);
    sectionTable[currSection].data.push_back(data2);
    sectionTable[currSection].data.push_back(data3);
}
int Assembler::workOnWordSecond(string word_argument)
{
    if (regex_match(word_argument, symbol))
    {
        auto symbol = symbolTable.find(word_argument);
        if (symbol != symbolTable.end())
        {
            SymbolTable symTable = symbol->second;
            if (symTable.getSection() == "apsolute")
                pushBackSection(currSect, cnt, symTable.getValue() & 0xff, (symTable.getValue() >> 8) & 0xff);
            else
            {
                if (symTable.getDefined())
                {
                    if (symTable.getBinding())
                    {
                        pushBackSection(currSect, cnt, symTable.getValue() & 0xff, (symTable.getValue() >> 8) & 0xff);
                        RelocationTable relocData(true, currSect, cnt, "R_386_16", 0, symTable.getSection());
                        relocationTable.push_back(relocData);
                    }
                    else
                    {
                        pushBackSection(currSect, cnt, 0 & 0xff, (0 >> 8) & 0xff);
                        RelocationTable relocData(true, currSect, cnt, "R_386_16", 0, symTable.getName());
                        relocationTable.push_back(relocData);
                    }
                }
                else
                {
                    pushBackSection(currSect, cnt, 0 & 0xff, (0 >> 8) & 0xff);
                    RelocationTable relocData(true, currSect, cnt, "R_386_16", 0, symTable.getName());
                    relocationTable.push_back(relocData);
                }
            }
        }
        else
            return errorDetected(currNumOfLine, "Word se ne sme koristiti sa nedefinisanim simbolom");
    }
    else
    {
        char c = getValueFromString(word_argument);
        pushBackSection(currSect, cnt, 0xff & c, 0xff & (c >> 8));
    }
    cnt += 2;
    return 1;
}
