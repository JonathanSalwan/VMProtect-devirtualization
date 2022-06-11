/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

class APP_THREAD_STATISTICS;
class BUFFER_LIST_STATISTICS;
class OVERALL_STATISTICS
{
  public:
    OVERALL_STATISTICS() {};
    VOID Init()
    {
        _numElementsProcessed           = 0;
        _numBuffersFilled               = 0;
        _numBuffersProcessedInAppThread = 0;
        _numTimesWaitedForFull          = 0;
        _numTimesWaitedForFree          = 0;
        _cyclesProcessingBuffer         = 0;
        _cyclesWaitingForFreeBuffer     = 0;
        _cyclesWaitingForFullBuffer     = 0;
        _totalCycles                    = 0;
        if (KnobStatistics)
        {
            _startProgramAtCycle = ReadProcessorCycleCounter();
        }
    }
    VOID AccumulateAppThreadStatistics(APP_THREAD_STATISTICS* statistics, BOOL accumulateFreeStats);
    VOID IncorporateBufferStatistics(BUFFER_LIST_STATISTICS* statistics, BOOL isFull);

    VOID DumpNumBuffersFilled()
    {
        if (!KnobLiteStatistics)
        {
            return;
        }
        _totalCycles = ReadProcessorCycleCounter() - _startProgramAtCycle;
        printf("\n\nOVERALL STATISTICS\n");
        printf("  numElementsProcessed               %14u\n", _numElementsProcessed);
        printf("  numBuffersFilled                   %14u\n", _numBuffersFilled);
        printf("  numBuffersProcessedInAppThread     %14u\n", _numBuffersProcessedInAppThread);
        if (KnobStatistics)
        {
            _fp = fopen((KnobStatisticsOutputFile.Value()).c_str(), "a");
            fprintf(_fp, "\n\nOVERALL STATISTICS\n");
            fprintf(_fp, "  totalElementsProcessed               %14u\n", _numElementsProcessed);
            fprintf(_fp, "  totalBuffersFilled                   %14u\n", _numBuffersFilled);
            fprintf(_fp, "  totalBuffersProcessedInAppThread     %14u\n", _numBuffersProcessedInAppThread);
        }
    }

    VOID Dump()
    {
        printf("  numTimesWaitedForFull              %14s\n", decstr(_numTimesWaitedForFull).c_str());
        printf("  numTimesWaitedForFree              %14s\n", decstr(_numTimesWaitedForFree).c_str());
        printf("totalThreadCycles            %14s\n", decstr(_totalCycles).c_str());
        printf("  cyclesProcessingBuffer     %14s  %%of total: %05.2f\n", decstr(_cyclesProcessingBuffer).c_str(),
               (static_cast< float >(_cyclesProcessingBuffer) * 100.0) / static_cast< float >(_totalCycles));
        printf("  cyclesWaitingForFreeBuffer %14s  %%of total: %05.2f\n", decstr(_cyclesWaitingForFreeBuffer).c_str(),
               (static_cast< float >(_cyclesWaitingForFreeBuffer) * 100.0) / static_cast< float >(_totalCycles));
        printf("  cyclesWaitingForFullBuffer %14s  %%of total: %05.2f\n", decstr(_cyclesWaitingForFullBuffer).c_str(),
               (static_cast< float >(_cyclesWaitingForFullBuffer) * 100.0) / static_cast< float >(_totalCycles));

        fprintf(_fp, "  numTimesWaitedForFull              %14s\n", decstr(_numTimesWaitedForFull).c_str());
        fprintf(_fp, "  numTimesWaitedForFree              %14s\n", decstr(_numTimesWaitedForFree).c_str());
        fprintf(_fp, "totalThreadCycles            %14s\n", decstr(_totalCycles).c_str());
        fprintf(_fp, "  cyclesProcessingBuffer     %14s  %%of total: %05.2f\n", decstr(_cyclesProcessingBuffer).c_str(),
                (static_cast< float >(_cyclesProcessingBuffer) * 100.0) / static_cast< float >(_totalCycles));
        fprintf(_fp, "  cyclesWaitingForFreeBuffer %14s  %%of total: %05.2f\n", decstr(_cyclesWaitingForFreeBuffer).c_str(),
                (static_cast< float >(_cyclesWaitingForFreeBuffer) * 100.0) / static_cast< float >(_totalCycles));
        fprintf(_fp, "  cyclesWaitingForFullBuffer %14s  %%of total: %05.2f\n", decstr(_cyclesWaitingForFullBuffer).c_str(),
                (static_cast< float >(_cyclesWaitingForFullBuffer) * 100.0) / static_cast< float >(_totalCycles));
    }

  private:
    UINT64 _cyclesProcessingBuffer;
    UINT64 _cyclesWaitingForFreeBuffer;
    UINT64 _cyclesWaitingForFullBuffer;
    UINT64 _startProgramAtCycle;
    UINT64 _totalCycles;
    UINT64 _numElementsProcessed;
    UINT32 _numBuffersFilled;
    UINT32 _numBuffersProcessedInAppThread;
    UINT32 _numTimesWaitedForFull;
    UINT32 _numTimesWaitedForFree;
    FILE* _fp;
} overallStatistics;

class BUFFER_LIST_STATISTICS
{
  public:
    BUFFER_LIST_STATISTICS() : _numTimesWaited(0), _cyclesWaitingForBuffer(0) {}
    VOID UpdateCyclesWaitingForBuffer() { _cyclesWaitingForBuffer += ReadProcessorCycleCounter() - _startToWaitForBufferAtCycle; }
    VOID StartCyclesWaitingForBuffer() { _startToWaitForBufferAtCycle = ReadProcessorCycleCounter(); }
    UINT64 CyclesWaitingForBuffer() { return _cyclesWaitingForBuffer; }

    VOID IncrementNumTimesWaited() { _numTimesWaited++; }
    UINT32 NumTimesWaitied() { return (_numTimesWaited); }

  private:
    UINT32 _numTimesWaited;
    UINT64 _startToWaitForBufferAtCycle;
    UINT64 _cyclesWaitingForBuffer;
};

class APP_THREAD_STATISTICS
{
  public:
    APP_THREAD_STATISTICS()
    {
        _numBuffersFilled               = 0;
        _numBuffersProcessedInAppThread = 0;
        _numElementsProcessed           = 0;
        _cyclesProcessingBuffer         = 0;
        _cyclesWaitingForFreeBuffer     = 0;
        _totalCycles                    = 0;
        _startAtCycle                   = ReadProcessorCycleCounter();
    }

    VOID DumpNumBuffersFilled()
    {
        if (!KnobLiteStatistics)
        {
            return;
        }
        _totalCycles = ReadProcessorCycleCounter() - _startAtCycle;
        printf("\n\nTHREAD STATISTICS %14u\n", 0);
        printf("  numElementsProcessed               %14s\n", decstr(_numElementsProcessed).c_str());
        printf("  numBuffersFilled                   %14s\n", decstr(_numBuffersFilled).c_str());
        printf("  numBuffersProcessedInAppThread     %14s\n", decstr(_numBuffersProcessedInAppThread).c_str());

        if (KnobStatistics)
        {
            _fp = fopen((KnobStatisticsOutputFile.Value()).c_str(), "a");
            fprintf(_fp, "\n\nTHREAD STATISTICS\n");
            fprintf(_fp, "  numElementsProcessed               %14s\n", decstr(_numElementsProcessed).c_str());
            fprintf(_fp, "  numBuffersFilled                   %14s\n", decstr(_numBuffersFilled).c_str());
            fprintf(_fp, "  numBuffersProcessedInAppThread     %14s\n", decstr(_numBuffersProcessedInAppThread).c_str());
        }
    }

    VOID Dump()
    {
        printf("  numTimesWaitedForFree              %14s\n", decstr(_numTimesWaitedForFree).c_str());
        printf("totalThreadCycles            %14s\n", decstr(_totalCycles).c_str());
        printf("  cyclesProcessingBuffer     %14s  %%of total: %05.2f\n", decstr(_cyclesProcessingBuffer).c_str(),
               (static_cast< float >(_cyclesProcessingBuffer) * 100.0) / static_cast< float >(_totalCycles));
        printf("  cyclesWaitingForFreeBuffer %14s  %%of total: %05.2f\n", decstr(_cyclesWaitingForFreeBuffer).c_str(),
               (static_cast< float >(_cyclesWaitingForFreeBuffer) * 100.0) / static_cast< float >(_totalCycles));

        fprintf(_fp, "  numTimesWaitedForFree              %14s\n", decstr(_numTimesWaitedForFree).c_str());
        fprintf(_fp, "totalThreadCycles            %14s\n", decstr(_totalCycles).c_str());
        fprintf(_fp, "  cyclesProcessingBuffer     %14s  %%of total: %05.2f\n", decstr(_cyclesProcessingBuffer).c_str(),
                (static_cast< float >(_cyclesProcessingBuffer) * 100.0) / static_cast< float >(_totalCycles));
        fprintf(_fp, "  cyclesWaitingForFreeBuffer %14s  %%of total: %05.2f\n", decstr(_cyclesWaitingForFreeBuffer).c_str(),
                (static_cast< float >(_cyclesWaitingForFreeBuffer) * 100.0) / static_cast< float >(_totalCycles));
        fclose(_fp);
    }

    VOID IncorporateBufferStatistics(BUFFER_LIST_STATISTICS* myFreeBufferListStats)
    {
        _numTimesWaitedForFree      = myFreeBufferListStats->NumTimesWaitied();
        _cyclesWaitingForFreeBuffer = myFreeBufferListStats->CyclesWaitingForBuffer();
    }

    VOID UpdateCyclesProcessingBuffer() { _cyclesProcessingBuffer += ReadProcessorCycleCounter() - _startToProcessBufAtCycle; }
    VOID StartCyclesProcessingBuffer() { _startToProcessBufAtCycle = ReadProcessorCycleCounter(); }

    VOID AddNumElementsProcessed(UINT32 numElementsProcessed) { _numElementsProcessed += numElementsProcessed; }
    VOID IncrementNumBuffersProcessedInAppThread() { _numBuffersProcessedInAppThread++; }
    VOID IncrementNumBuffersFilled() { _numBuffersFilled++; }
    UINT32 NumBuffersProcessedInAppThread() { return _numBuffersProcessedInAppThread; }
    UINT64 NumBuffersElementsProcessed() { return _numElementsProcessed; }
    UINT32 NumBuffersFilled() { return _numBuffersFilled; }
    UINT32 NumTimesWaitedForFree() { return _numTimesWaitedForFree; }
    UINT64 CyclesProcessingBuffer() { return _cyclesProcessingBuffer; }
    UINT64 CyclesWaitingForFreeBuffer() { return _cyclesWaitingForFreeBuffer; }
    UINT64 TotalCycles() { return _totalCycles; }

  private:
    UINT64 _startToProcessBufAtCycle;
    UINT64 _cyclesProcessingBuffer;
    UINT64 _startAtCycle;
    UINT64 _totalCycles;
    UINT64 _numElementsProcessed;
    FILE* _fp;
    UINT32 _numBuffersProcessedInAppThread;
    UINT32 _numBuffersFilled;

    UINT32 _numTimesWaitedForFree;
    UINT64 _cyclesWaitingForFreeBuffer;
};

VOID OVERALL_STATISTICS::AccumulateAppThreadStatistics(APP_THREAD_STATISTICS* statistics, BOOL accumulateFreeStats)
{
    _numElementsProcessed += statistics->NumBuffersElementsProcessed();
    _numBuffersFilled += statistics->NumBuffersFilled();
    _numBuffersProcessedInAppThread += statistics->NumBuffersProcessedInAppThread();
    if (accumulateFreeStats)
    {
        _numTimesWaitedForFree += statistics->NumTimesWaitedForFree();
        _cyclesWaitingForFreeBuffer += statistics->CyclesWaitingForFreeBuffer();
    }
    _cyclesProcessingBuffer += statistics->CyclesProcessingBuffer();
}

VOID OVERALL_STATISTICS::IncorporateBufferStatistics(BUFFER_LIST_STATISTICS* statistics, BOOL isFull)
{
    if (isFull)
    {
        _numTimesWaitedForFull      = statistics->NumTimesWaitied();
        _cyclesWaitingForFullBuffer = statistics->CyclesWaitingForBuffer();
    }
    else
    {
        _numTimesWaitedForFree      = statistics->NumTimesWaitied();
        _cyclesWaitingForFreeBuffer = statistics->CyclesWaitingForBuffer();
    }
}