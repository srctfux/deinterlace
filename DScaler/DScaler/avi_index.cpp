/* $Id: avi_index.cpp,v 1.1 2008-03-11 10:07:38 adcockj Exp $ */

/** \file
 * OpenDML indexing
 * \author Nick Kochakian
 */

#include "stdafx.h"
#include "avi.h"
#include "avi_internal.h"

void aviIndexAddSuperEntry(AVI_FILE *file, stream_t type, DWORD duration,
                           int64 offset, DWORD size);

/** Initializes and writes the data for a new super index
 * \param file An open file
 * \param type A *_STREAM constant
 */

void aviWriteInitialSuperIndex(AVI_FILE *file, stream_t type)
{
    STREAM_SUPER_INDEX *index;
    STREAM_CC          *cc;

    index = &file->superIndex[type];
    cc    = aviGetStreamCC(file, type);

    /* Get the offset to the beginning of the super index */
    index->offset = fileTell(file);

    /* Set the correct cc for the index and start with no entries in use */
    index->header.dwChunkId = cc->data;

    index->header.nEntriesInUse = 0;

    /* Write the header */
    fileWrite(file, &index->header, sizeof(AVISUPERINDEX));

    /* Reserve space for a predefined number of entries */
    fileReserveSpace(file, SUPER_INDEX_ENTRIES * sizeof(AVISUPERINDEX_ENTRY));
}

/** Writes any data being held for the super index into the file
 * \param file An open file
 * \param type A *_STREAM constant
 */

void aviIndexFlushSuper(AVI_FILE *file, stream_t type)
{
    STREAM_SUPER_INDEX *index;
    int64              offset;

    index = &file->superIndex[type];

    if (index->current <= 0)
       return;

    offset = fileTell(file);

    /* Update the number of super entries in use for this stream */
    assert(index->header.nEntriesInUse < SUPER_INDEX_ENTRIES);
    assert(index->header.nEntriesInUse + index->current <= SUPER_INDEX_ENTRIES);

    index->header.nEntriesInUse += index->current;
    fileSeek(file, index->offset + offsetof(AVISUPERINDEX, nEntriesInUse));
    fileWrite(file, &index->header.nEntriesInUse,
              sizeof(index->header.nEntriesInUse));

    /* Add the new entries to the super index's array */
    fileSeek(file, index->offset + sizeof(AVISUPERINDEX) +
                      ((index->header.nEntriesInUse - index->current) *
                       sizeof(AVISUPERINDEX_ENTRY)));
    fileWrite(file, index->entry, index->current *
                                     sizeof(AVISUPERINDEX_ENTRY));

    fileSeek(file, offset);

    index->current = 0;
}

/** Writes any entries in the standard index and updates the super index
 * \param file An open file
 * \param type A *_STREAM constant
 */

void aviIndexFlushStd(AVI_FILE *file, stream_t type)
{
    AVISTDINDEX      header;
    STREAM_STD_INDEX *index;
    STREAM_CC        *cc;
    int64            begin;
    DWORD            size;

    index = &file->stdIndex[type];

    if (index->current <= 0)
       return;

    cc    = aviGetStreamCC(file, type);
    begin = fileTell(file);

    aviBeginChunk(file, cc->index);
    header.wLongsPerEntry = 2;
    header.bIndexSubType  = 0;
    header.bIndexType     = AVI_INDEX_OF_CHUNKS;
    header.nEntriesInUse  = index->current;
    header.dwChunkId      = cc->data;
    header.qwBaseOffset   = aviGetBaseOffset(file, MOVI_BASE);
    header.dwReserved     = 0;

    fileWrite(file, &header, sizeof(AVISTDINDEX));
    fileWrite(file, index->entry,
              index->current * sizeof(AVISTDINDEX_ENTRY));

    size = aviEndChunk(file) + 8;

    index->current = 0;

    /* Add this chunk to the super index */
    aviIndexAddSuperEntry(file, type, index->duration, begin, size);

    index->duration = 0;
}

/** Flushes all standard and super indices for every stream
 * \param file An open file
 */

void aviIndexFlush(AVI_FILE *file)
{
    int i;

    /* Write data that's most likely closest to the current write position */
    for (i = 0; i < NUM_STREAMS; i++)
        aviIndexFlushStd(file, (stream_t)i);

    /* Write data that's closer to the beginning of the file */
    for (i = 0; i < NUM_STREAMS; i++)
        aviIndexFlushSuper(file, (stream_t)i);
}

/** Adds a standard index entry
 * \param file     The file to write to
 * \param type     The type of stream to add the index to
 * \param begin    The absolute offset to the beginning of the chunk's data
 * \param size     The length of the chunk's data in bytes starting from begin
 * \param duration The duration of this entry in stream ticks
 * \param keyFrame Determines if the entry is really a key frame. Only affects
 *                 video streams.
 */

void aviIndexAddEntry(AVI_FILE *file, stream_t type, int64 begin, DWORD size,
                      DWORD duration, BOOL keyFrame)
{
    STREAM_STD_INDEX  *index;
    AVISTDINDEX_ENTRY *entry;

    index = &file->stdIndex[type];

    /* Add this index to the count of indices for this stream */
    file->stdIndexCount[type]++;

    /* Update the length of the current set of indices */
    index->duration += duration;

    assert(index->current < MAX_STD_INDEX);
    entry = &index->entry[index->current];

    if (type==VIDEO_STREAM && !keyFrame)
       size |= 0x80000000;

    /* Make a relative offset */
    entry->dwOffset = (DWORD)(begin - aviGetBaseOffset(file, MOVI_BASE));

    entry->dwSize = size;

    index->current++;
    if (index->current >= MAX_STD_INDEX)
    {
        /* Write the index to the file */
        aviIndexFlushStd(file, type);
    }
}

/** Adds an entry into the super index
 * \param file     The file to write to
 * \param type     The type of stream to add the entry to
 * \param duration The duration that all of the index chunks at \a offset cover
 * \param offset   An absolute offset to the beginning of the chunk
 * \param size     The size of the chunk (this includes the header)
 */

void aviIndexAddSuperEntry(AVI_FILE *file, stream_t type, DWORD duration,
                           int64 offset, DWORD size)
{
    STREAM_SUPER_INDEX  *index;
    AVISUPERINDEX_ENTRY *entry;

    index = &file->superIndex[type];

    assert(index->current < MAX_SUPER_INDEX);
    entry = &index->entry[index->current];

    entry->dwDuration = duration;
    entry->dwSize     = size;
    entry->qwOffset   = offset;

    index->current++;
    if (index->current >= MAX_SUPER_INDEX)
    {
        /* Write the super index into the file */
        aviIndexFlushSuper(file, type);
    }
}

/** Resets the standard index counters for each stream
 * \param file The file to modify
 * \note The counters are used to determine how many index entries have been
 *       written into the current RIFF chunk so they should be cleared when a
 *       RIFF chunk has ended
 */

void aviIndexClearCounters(AVI_FILE *file)
{
    memset(file->stdIndexCount, 0, sizeof(file->stdIndexCount));
}

/** Copies the values from the current index counters to the legacy index
 * counts
 * \param file The file to modify
 */

void aviIndexSetLegacyCounters(AVI_FILE *file)
{
    assert(sizeof(file->legacy.indices)==sizeof(file->stdIndexCount));
    memcpy(file->legacy.indices, file->stdIndexCount,
                                 sizeof(file->legacy.indices));
}