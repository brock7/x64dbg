#include "linearanalysis.h"
#include "console.h"
#include "memory.h"
#include "function.h"

LinearAnalysis::LinearAnalysis(duint base, duint size) : Analysis(base, size)
{
}

void LinearAnalysis::Analyse()
{
    dputs("Starting analysis...");
    DWORD ticks = GetTickCount();

    PopulateReferences();
    dprintf("%u called functions populated\n", _functions.size());
    AnalyseFunctions();

    dprintf("Analysis finished in %ums!\n", GetTickCount() - ticks);
}

void LinearAnalysis::SetMarkers()
{
    FunctionDelRange(_base, _base + _size - 1, false);
    for(auto & function : _functions)
    {
        if(!function.end)
            continue;
        FunctionAdd(function.start, function.end, false);
    }
}

void LinearAnalysis::SortCleanup()
{
    //sort & remove duplicates
    std::sort(_functions.begin(), _functions.end());
    auto last = std::unique(_functions.begin(), _functions.end());
    _functions.erase(last, _functions.end());
}

void LinearAnalysis::PopulateReferences()
{
    //linear immediate reference scan (call <addr>, push <addr>, mov [somewhere], <addr>)
    for(duint i = 0; i < _size;)
    {
        duint addr = _base + i;
        if(_cp.Disassemble(addr, TranslateAddress(addr), MAX_DISASM_BUFFER))
        {
            duint ref = GetReferenceOperand();
            if(ref)
                _functions.push_back({ ref, 0 });
            i += _cp.Size();
        }
        else
            i++;
    }
    SortCleanup();
}

void LinearAnalysis::AnalyseFunctions()
{
    for(size_t i = 0; i < _functions.size(); i++)
    {
        FunctionInfo & function = _functions[i];
        if(function.end)  //skip already-analysed functions
            continue;
        duint maxaddr = _base + _size;
        if(i < _functions.size() - 1)
            maxaddr = _functions[i + 1].start;

        duint end = FindFunctionEnd(function.start, maxaddr);
        if(end)
        {
            if(_cp.Disassemble(end, TranslateAddress(end), MAX_DISASM_BUFFER))
                function.end = end + _cp.Size() - 1;
            else
                function.end = end;
        }
    }
}

duint LinearAnalysis::FindFunctionEnd(duint start, duint maxaddr)
{
    //disassemble first instruction for some heuristics
    if(_cp.Disassemble(start, TranslateAddress(start), MAX_DISASM_BUFFER))
    {
        //JMP [123456] ; import
        if(_cp.InGroup(CS_GRP_JUMP) && _cp.x86().operands[0].type == X86_OP_MEM)
            return 0;
    }

    //linear search with some trickery
    duint end = 0;
    duint jumpback = 0;
    for(duint addr = start, fardest = 0; addr < maxaddr;)
    {
        if(_cp.Disassemble(addr, TranslateAddress(addr), MAX_DISASM_BUFFER))
        {
            if(addr + _cp.Size() > maxaddr)  //we went past the maximum allowed address
                break;

            const cs_x86_op & operand = _cp.x86().operands[0];
            if((_cp.InGroup(CS_GRP_JUMP) || _cp.IsLoop()) && operand.type == X86_OP_IMM)   //jump
            {
                duint dest = (duint)operand.imm;

                if(dest >= maxaddr)   //jump across function boundaries
                {
                    //currently unused
                }
                else if(dest > addr && dest > fardest)   //save the farthest JXX destination forward
                {
                    fardest = dest;
                }
                else if(end && dest < end && (_cp.GetId() == X86_INS_JMP || _cp.GetId() == X86_INS_LOOP)) //save the last JMP backwards
                {
                    jumpback = addr;
                }
            }
            else if(_cp.InGroup(CS_GRP_RET))   //possible function end?
            {
                end = addr;
                if(fardest < addr)  //we stop if the farthest JXX destination forward is before this RET
                    break;
            }

            addr += _cp.Size();
        }
        else
            addr++;
    }
    return end < jumpback ? jumpback : end;
}

duint LinearAnalysis::GetReferenceOperand()
{
    for(int i = 0; i < _cp.x86().op_count; i++)
    {
        const cs_x86_op & operand = _cp.x86().operands[i];
        if(_cp.InGroup(CS_GRP_JUMP) || _cp.IsLoop())  //skip jumps/loops
            continue;
        if(operand.type == X86_OP_IMM)  //we are looking for immediate references
        {
            duint dest = (duint)operand.imm;
            if(dest >= _base && dest < _base + _size)
                return dest;
        }
    }
    return 0;
}